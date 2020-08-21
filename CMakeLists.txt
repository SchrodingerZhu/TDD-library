cmake_minimum_required(VERSION 3.1.0)
project(qtjudge)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wall -Wextra")
add_executable(test test.cpp)
if (NOT WIN32)
    target_link_libraries(test dl)
endif()

