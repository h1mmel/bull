#ifndef _HTTP_H_
#define _HTTP_H_

#include <string>
#include <memory>
#include <unordered_map>

#include "bull/status.h"

namespace bull {
namespace http {

class HttpHeader;

enum Method { kGet, kHead, kPost, kPut, kDelete, kConnect, kOptions, kTrace };

class HttpClient {
 public:
    typedef std::function<void(const std::string&)> ResponseHandler;

    HttpClient();

    HttpClient(const HttpClient&) = delete;
    HttpClient& operator=(const HttpClient&) = delete;

    ~HttpClient();

    virtual Status Get(const std::string& url, const HttpHeader& header, ResponseHandler handler);
    virtual Status Head(const std::string& url, const HttpHeader& header, ResponseHandler handler);
    virtual Status Post(const std::string& url, const HttpHeader& header, ResponseHandler handler);
    virtual Status Put(const std::string& url, const HttpHeader& header, ResponseHandler handler);

 protected:
    class HttpImpl;
    std::shared_ptr<HttpImpl> impl_;
};

class HttpHeader {
 public:
    HttpHeader();
    ~HttpHeader();

    void SetField(const std::string& key, const std::string& value);
    std::string ToString();

 private:
    std::unordered_map<std::string, std::string> field_table_;
};

}   // namespace http
}   // namespace bull

#endif  // _HTTP_H_
