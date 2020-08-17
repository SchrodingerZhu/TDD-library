#include "test.hpp"
#include "solution.hpp"
#include <random>
int main() {
    Test test;
    test.add(10, "addition", [] {
        EXPECT_EQ(solution::add(1, 1), 1 + 1)
        EXPECT_EQ(solution::add(20, -10), 10)
    });
    test.add(10, "subtraction", [] {
        EXPECT_EQ(solution::sub(33, 33), 0)
        EXPECT_EQ(solution::sub(20, -10), 30)
    });
    test.add(10, "mod", [] {
        EXPECT_EQ(solution::mod(33, 10), 2)
        EXPECT_EQ(solution::mod(122, 120), 2)
    });
    test.add(20, "random test", [] {
        auto dev = std::random_device();
        for (int i = 0; i < 10000; ++i) {
            int a = dev(), b = dev();
            EXPECT_EQ(solution::add(a, b), a + b)
            EXPECT_EQ(solution::sub(a, b), a - b)
            EXPECT_EQ(solution::mod(a, b), a % b)
        }
    });
}