#include "bull/http.h"

#include <iostream>
#include <regex>

#include "boost/asio.hpp"

#include "src/session.h"
#include "src/context_pool.h"

namespace bull {
namespace http {

class HttpClient::HttpImpl {
 public:
    HttpImpl();
    ~HttpImpl();

    Status Get(const std::string& url, const HttpHeader& header, ResponseHandler handler);
    Status Head(const std::string& url, const HttpHeader& header, ResponseHandler handler);
    Status Post(const std::string& url, const HttpHeader& header, ResponseHandler handler);
    Status Put(const std::string& url, const HttpHeader& header, ResponseHandler handler);

 private:
    bool ParseUrl(const std::string& url);
    bool BuildRequest(int method, const std::string& url, const HttpHeader& header);
    void HandleResolve(const types::ErrorCode& ec, types::ResultType endpoints, ResponseHandler handler);
    void HandleConnect(const types::ErrorCode& ec, const types::EndPoint& endpoint, ResponseHandler handler);

    std::string proto_;
    std::string host_;
    std::string uri_;
    std::string request_;
    types::IoContext& context_;
    types::TcpSocket socket_;
    types::TcpResolver resovler_;
    std::vector<std::shared_ptr<session::Session>> session_table_;
};

HttpClient::HttpImpl::HttpImpl()
    : context_(session::ContextPool::GetInstance()->GetIoContext()),
      socket_(context_),
      resovler_(context_) {
    socket_.open(types::Tcp::v4());
}

bool HttpClient::HttpImpl::ParseUrl(const std::string& url) {
    std::size_t pos = url.find("://");
    if (pos == std::string::npos) {
        proto_ = "http";
        std::regex rx("([^/]+)(.*)");
        std::smatch match;
        if (std::regex_match(url, match, rx)) {
            host_ = match[1];
            uri_ = match[2];
            if (uri_.empty()) uri_ = "/";
            return true;
        }
    } else {
        std::regex rx("(http|https)://([^/]+)(.*)");
        std::smatch match;
        if (std::regex_match(url, match, rx)) {
            proto_ = match[1];
            host_ = match[2];
            uri_ = match[3];
            if (uri_.empty()) uri_ = "/";
            return true;
        }
    }
    return false;
}

bool HttpClient::HttpImpl::BuildRequest(int method, const std::string& url, const HttpHeader& header) {
    if (!ParseUrl(url)) {
        std::cerr << "Invalid URL: " << url << std::endl;
        return false;
    }
    std::string version = "1.1";
    switch (method) {
    case kGet:
        request_ += "GET";
        break;
    case kHead:
        request_ += "HEAD";
        break;
    case kPost:
        request_ += "POST";
        break;
    case kPut:
        request_ += "PUT";
        break;
    case kDelete:
        request_ += "DELETE";
        break;
    }
    request_ += " " + uri_ + " HTTP/" + version + "\r\n";
    std::unordered_map<std::string, std::string> default_headers = {
        {"Host", host_},
        {"User-Agent", "Mozilla/5.0"},
        {"Accept", "*/*"},
    };
    std::unordered_map<std::string, std::string> all_headers = default_headers;
    std::stringstream request_headers;
    for (const auto& pair : all_headers) {
        request_headers << pair.first << ": " << pair.second << "\r\n";
    }
    request_ += request_headers.str() + "\r\n";
    return true;
}

void HttpClient::HttpImpl::HandleResolve(const types::ErrorCode& ec,
                                         types::ResultType endpoints,
                                         ResponseHandler handler) {
    if (!ec) {
        boost::asio::async_connect(socket_, endpoints,
            std::bind(&HttpClient::HttpImpl::HandleConnect, this, std::placeholders::_1, std::placeholders::_2, handler));
    } else {
        handler("HandleResolve Error: " + std::to_string(ec.value()) + " " + ec.message());
    }
}

void HttpClient::HttpImpl::HandleConnect(const types::ErrorCode& ec,
                                         const types::EndPoint& endpoint,
                                         ResponseHandler handler) {
    if (!ec) {
        std::cout << "connect to " << endpoint.address().to_string() << ":" << endpoint.port() << " success!" << std::endl;
        std::cout << "\n" << request_ << std::endl;
        session_table_.emplace_back(std::make_shared<session::Session>(std::move(socket_)));
        session_table_.back()->DoWrite(request_, handler);
    } else {
        handler("HandleConnect Error: " + std::to_string(ec.value()) + " " + ec.message());
    }
}

Status HttpClient::HttpImpl::Get(const std::string& url, const HttpHeader& header, ResponseHandler handler) {
    if (BuildRequest(kGet, url, header)) {
        resovler_.async_resolve(host_, proto_,
            std::bind(&HttpClient::HttpImpl::HandleResolve, this, std::placeholders::_1, std::placeholders::_2, handler));
        return Status(0);
    }
    return Status(1);
}

Status HttpClient::HttpImpl::Head(const std::string& url, const HttpHeader& header, ResponseHandler handler) {
    if (BuildRequest(kHead, url, header)) {
        resovler_.async_resolve(host_, proto_,
            std::bind(&HttpClient::HttpImpl::HandleResolve, this, std::placeholders::_1, std::placeholders::_2, handler));
        return Status(0);
    }
    return Status(0);
}

Status HttpClient::HttpImpl::Post(const std::string& url, const HttpHeader& header, ResponseHandler handler) {
    return Status(0);
}

Status HttpClient::HttpImpl::Put(const std::string& url, const HttpHeader& header, ResponseHandler handler) {
    return Status(0);
}

HttpClient::HttpImpl::~HttpImpl() {
    socket_.close();
}

HttpClient::HttpClient() : impl_(std::make_shared<HttpImpl>()) {}

Status HttpClient::Get(const std::string& url, const HttpHeader& header, ResponseHandler handler) {
    return impl_->Get(url, header, handler);
}

Status HttpClient::Head(const std::string& url, const HttpHeader& header, ResponseHandler handler) {
    return impl_->Head(url, header, handler);
}

Status HttpClient::Post(const std::string& url, const HttpHeader& header, ResponseHandler handler) {
    return impl_->Post(url, header, handler);
}

Status HttpClient::Put(const std::string& url, const HttpHeader& header, ResponseHandler handler) {
    return impl_->Put(url, header, handler);
}

HttpClient::~HttpClient() {}

HttpHeader::HttpHeader() {}

void HttpHeader::SetField(const std::string& key, const std::string& value) {
    field_table_.insert({key, value});
}

std::string HttpHeader::ToString() {
    std::string header;
    for (auto field : field_table_) {
        header += field.first + ":" + field.second + "\r\n";
    }
    return header;
}

HttpHeader::~HttpHeader() {}

}   // namespace http
}   // namespace bull
