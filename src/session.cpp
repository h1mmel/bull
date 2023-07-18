#include "src/session.h"

#include <iostream>

namespace bull {
namespace session {

DnsTable* DnsTable::GetInstance() {
    static DnsTable table;
    return &table;
}

std::vector<std::pair<std::string, uint16_t>> DnsTable::Find(const std::string& domain) {
    auto it = dns_table_.find(domain);
    if (it != dns_table_.end()) {
        return it->second;
    }
    return {};
}

void DnsTable::Insert(const std::string& domain, const std::vector<std::pair<std::string, uint16_t>>& addrs) {
    dns_table_.insert({domain, addrs});
}

Session::Session(types::TcpSocket&& socket) : socket_(std::move(socket)) {}

void Session::SetSession(types::TcpSocket& socket) {
    socket_ = std::move(socket);
}

void Session::DoWrite(std::string request, ResponseHandler handler) {
    boost::asio::async_write(socket_, boost::asio::buffer(request),
        std::bind(&Session::HandleWrite, this, std::placeholders::_1, std::placeholders::_2, handler));
}

void Session::HandleWrite(const types::ErrorCode& ec,
                          std::size_t bytes_transferred,
                          ResponseHandler handler) {
    if (!ec) {
        std::shared_ptr<types::StreamBuffer> data = std::make_shared<types::StreamBuffer>();
        boost::asio::async_read_until(socket_, *data, "\r\n\r\n",
            std::bind(&Session::HandleHeader, this, std::placeholders::_1, std::placeholders::_2, data, handler));
    } else {
        handler("HandleWrite Error: " + std::to_string(ec.value()) + " " + ec.message());
    }
}

void Session::HandleHeader(const types::ErrorCode& ec,
                           std::size_t bytes_transferred,
                           std::shared_ptr<types::StreamBuffer> data,
                           ResponseHandler handler) {
    if (!ec) {
        response_header_ = std::string(boost::asio::buffer_cast<const char*>(data->data()), bytes_transferred);
        std::cout << response_header_ << std::endl;
        ParseHeader();
        uint32_t pre_body_len = data->size() - bytes_transferred;
        if (pre_body_len) {
            been_read_ += pre_body_len;
            data->consume(bytes_transferred);
            std::string body(boost::asio::buffer_cast<const char*>(data->data()), pre_body_len);
            response_body_ += body;
        }
        data->consume(pre_body_len);
        socket_.async_read_some(data->prepare(1024),
            std::bind(&Session::HandleBody, this, std::placeholders::_1, std::placeholders::_2, data, handler));
    } else {
        handler("HandleHeader Error: " + std::to_string(ec.value()) + " " + ec.message());
    }
}

void Session::HandleBody(const types::ErrorCode& ec,
                         std::size_t bytes_transferred,
                         std::shared_ptr<types::StreamBuffer> data,
                         ResponseHandler handler) {
    if (!ec) {
        if (been_read_ + bytes_transferred < content_length_) {
            been_read_ += bytes_transferred;
            std::string body(boost::asio::buffer_cast<const char*>(data->data()), bytes_transferred);
            response_body_ += body;
            data->consume(bytes_transferred);
            socket_.async_read_some(data->prepare(1024),
                std::bind(&Session::HandleBody, this, std::placeholders::_1, std::placeholders::_2, data, handler));           
        } else {
            been_read_ = 0;
            std::string body(boost::asio::buffer_cast<const char*>(data->data()), bytes_transferred);
            response_body_ += body;
            handler(response_body_);
        }        
    } else {
        handler("HandleBody Error: " + std::to_string(ec.value()) + " " + ec.message());
    }
}

void Session::ParseHeader() {
    std::size_t start = response_header_.find("Content-Length: ");
    if (start != std::string::npos) {
        is_chunked_ = false;
        start += sizeof("Content-Length: ");
        std::size_t end = response_header_.substr(start).find("\r\n");
        content_length_ = std::stoull(response_header_.substr(start - 1, end - start));
    } else {
        is_chunked_ = true;
    }
}

Session::~Session() {}

}   // namespace session
}   // namespace bull
