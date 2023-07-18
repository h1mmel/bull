cmake_minimum_required(VERSION 3.20)

project(bull)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0 -g")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3")

add_library(bull STATIC "src/http.cpp"
                        "src/session.cpp")

target_include_directories(bull PUBLIC ${PROJECT_SOURCE_DIR})
target_include_directories(bull PUBLIC ${PROJECT_SOURCE_DIR}/include/)

target_link_libraries(bull pthread)