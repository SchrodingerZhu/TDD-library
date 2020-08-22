cmake_minimum_required(VERSION 3.1.0)
project(test)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -g -Wall -Wextra -Werror")
find_package(Threads)
include_directories(student tests)
add_executable(test tests/test.cpp)
target_link_libraries(test Threads::Threads)
if (NOT WIN32)
    target_link_libraries(test dl)
endif()

