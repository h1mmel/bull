#ifndef _STATUS_H_
#define _STATUS_H_

namespace bull {

enum Code {
    kOK = 0,
    kError
};

class Status {
 public:
    Status() : code_(0) {}
    explicit Status(int code) : code_(code) {}

    ~Status() = default;

    bool IsOk() { return code_ == Code::kOK; }

    std::string Message() {
        std::string msg = "";
        switch (code_)
        {
        case Code::kOK:
            msg = "ok";
            break;
        }
        return msg;
    }

 private:
    int code_;
};

}   // namespace bull

#endif  // _STATUS_H_
