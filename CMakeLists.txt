cmake_minimum_required(VERSION 3.1.0)
project(qtjudge)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wall -Wextra")
find_package(Threads REQUIRED)
add_executable(test test.cpp)
target_link_libraries(test Threads::Threads -lunwind -lunwind-generic)