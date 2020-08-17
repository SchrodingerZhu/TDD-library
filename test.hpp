#ifndef TEST_HPP
#define TEST_HPP

#include <functional>
#include <vector>
#include <iostream>
#include <tuple>
#include <string_view>

#define EXPECT_TRUE(X) \
    if (!(X)) throw std::runtime_error("expected " #X " to be true, but it failed");

#define EXPECT_EQ(X, Y) \
    if ((X) != (Y)) throw std::runtime_error("expected " #X " to be equal to " #Y ", but it failed");

struct Test {
    long current = 0;
    long total = 0;
    std::vector<std::tuple<std::function<void()>, long, std::string_view>> registry;

    void add(long score, std::string_view name, std::function<void()> func) {
        total += score;
        registry.emplace_back(std::move(func), score, name);
    }

    ~Test() {
        for (auto& i : registry) {
            std::cout << "testing <" << std::get<2>(i) << ">: ";
            try {
                std::get<0>(i)();
                current += std::get<1>(i);;
                std::cout << "[SUCCESS]" << std::endl;
            } catch (std::exception& exp) {
                std::cout << "[FAILURE]\n - " << exp.what() << std::endl;
            }
        }
        std::cout << "[RESULT] " << current << "/" << total << std::endl;
    }
};
#endif // TEST_HPP


