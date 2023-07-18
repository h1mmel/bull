cmake_minimum_required(VERSION 3.20)

project(client)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0 -g")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3")

add_executable(client "client.cpp")

message(${PROJECT_SOURCE_DIR})

target_include_directories(client PUBLIC ${PROJECT_SOURCE_DIR}/../../../include/)

target_link_libraries(client ${PROJECT_SOURCE_DIR}/../../../build/libbull.a)
target_link_libraries(client pthread)