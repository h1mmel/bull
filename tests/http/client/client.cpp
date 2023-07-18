#include <iostream>
#include <thread>
#include <chrono>

#include "bull/http.h"

void OnResponse(const std::string& response) {
    std::cout << response << std::endl;
}

int main(int argc, char const *argv[])
{
    bull::http::HttpClient http;
    bull::Status stat = http.Get("http://www.example.com", {}, OnResponse);
    if (!stat.IsOk()) {
        std::cerr << stat.Message() << std::endl;
        return -1;
    }
    while (true) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    return 0;
}
