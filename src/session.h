#ifndef _SESSION_H_
#define _SESSION_H_

#include <unordered_map>
#include <cstdint>

#include "boost/asio.hpp"
#include "src/asio_type.h"

namespace bull {
namespace session {

class DnsTable {
 public:
    static DnsTable* GetInstance();

    std::vector<std::pair<std::string, uint16_t>> Find(const std::string& domain);
    void Insert(const std::string& name, const std::vector<std::pair<std::string, uint16_t>>& addrs);

 private:
    DnsTable() = default;
    ~DnsTable() = default;

    std::unordered_map<std::string, std::vector<std::pair<std::string, uint16_t>>> dns_table_;
};

class Session {
 public:
    typedef std::function<void(const std::string&)> ResponseHandler;

    explicit Session(types::TcpSocket&& socket);
    ~Session();

    void SetSession(types::TcpSocket& socket);
    void DoWrite(std::string request, ResponseHandler handler);

 private:
    void HandleWrite(const types::ErrorCode& ec, std::size_t bytes_transferred, ResponseHandler handler);
    void HandleHeader(const types::ErrorCode& ec, std::size_t bytes_transferred, std::shared_ptr<types::StreamBuffer> ptr, ResponseHandler handler);
    void HandleBody(const types::ErrorCode& ec, std::size_t bytes_transferred, std::shared_ptr<types::StreamBuffer> body, ResponseHandler handler);
    void ParseHeader();

    types::TcpSocket socket_;
    std::string proto_;
    std::string host_;
    std::string uri_;
    std::string request_;
    std::string status_;
    std::string response_header_;
    uint64_t content_length_;
    uint64_t been_read_;
    std::string response_body_;
    bool is_chunked_;
};

}   // namespace session
}   // namespace bull

#endif
