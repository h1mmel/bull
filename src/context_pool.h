#ifndef _CONTEXT_POOL_H_
#define _CONTEXT_POOL_H_

#include <sys/prctl.h>

#include <thread>
#include <memory>

#include "boost/asio.hpp"
#include "src/asio_type.h"

namespace bull {
namespace session {

class ContextPool {
 public:
    static ContextPool* GetInstance() {
        static ContextPool pool;
        return &pool;
    }

    ContextPool(const ContextPool&) = delete;
    ContextPool& operator=(const ContextPool&) = delete;

    types::IoContext& GetIoContext() {
        types::IoContext& cxt = context_pool_[next_++];
        if (next_ == context_pool_.size()) {
            next_ = 0;
        }
        return cxt;
    }

    void Stop() {
        for (auto& work : work_pool_) {
            work->get_io_context().stop();
            work.reset();
        }

        for (auto& t : thread_pool_) {
            t.join();
        }
    }

 private:
    ContextPool(int size = std::thread::hardware_concurrency() - 1)
        : context_pool_(size),
          work_pool_(size),
          next_(0) {
        for (int i = 0; i < size; i++) {
            work_pool_[i] = std::make_unique<types::Work>(context_pool_[i]);
        }

        for (int i = 0; i < size; i++) {
            thread_pool_.emplace_back([this, i](){
                std::string name = "io_thread_" + std::to_string(i);
                prctl(PR_SET_NAME, name.c_str());
                context_pool_[i].run();
            });
        }
    }

    ~ContextPool() {
        for (auto& work : work_pool_) {
            work->get_io_context().stop();
            work.reset();
        }

        for (auto& t : thread_pool_) {
            t.join();
        }
    }

    std::vector<types::IoContext> context_pool_;
    std::vector<types::WorkPtr> work_pool_;
    std::vector<std::thread> thread_pool_;
    uint32_t next_;
};

}   // namespace session
}   // namespace bull

#endif
