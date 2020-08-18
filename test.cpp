#include "test.hpp"
#include "solution.hpp"
#include <random>

#define TEST_SIZE 1000
#define ERASE_SIZE (TEST_SIZE / 2)

struct EvilStruct {
    static size_t dtor_call;
    static size_t ctor_call;
    std::vector<int> content;

    static void clear_state() {
        dtor_call = ctor_call = 0;
    }

    EvilStruct() {
        for (int i = 0; i < 20; ++i) {
            content.push_back(rand());
        }
        ctor_call++;
    }

    EvilStruct(const EvilStruct &that) : content(that.content) {
        ctor_call++;
    };

    EvilStruct(EvilStruct &&that) noexcept: content(std::move(that.content)) {
        ctor_call++;
    }

    ~EvilStruct() {
        dtor_call++;
    }

    EvilStruct& operator=(const EvilStruct&) = default;

    bool operator==(const EvilStruct &that) const {
        return content == that.content;
    }
};

size_t EvilStruct::dtor_call = 0;
size_t EvilStruct::ctor_call = 0;

int main() {
    Test test;
    test.add(10, "test", [] {
        int a = 12;
        int b = 346;
        EXPECT_EQ(a, b);
    });

    test.add(10, "push_back", [] {
        Vector<int> vector;
        std::vector<int> standard;
        for (int i = 0; i < TEST_SIZE; ++i) {
            vector.push_back(i);
            standard.push_back(i);
        }
        EXPECT_EQ(vector.to_std(), standard);
    });

    test.add(10, "size", [&] {
        for (int i = 0; i < 20; ++i) {
            Vector<int> vector;
            unsigned length = std::rand() % 10000u + 50u;
            for (unsigned j = 0; j < length; ++j) {
                vector.push_back(j);
            }
            EXPECT_EQ(vector.size(), length);
        }
    });

    test.add(10, "destructor", [&] {
        {
            Vector<EvilStruct> a;
            for (int i = 0; i < TEST_SIZE; ++i) {
                a.push_back(EvilStruct());
            }
            EXPECT_EQ(EvilStruct::ctor_call - EvilStruct::dtor_call, TEST_SIZE)
        }
        EXPECT_EQ(EvilStruct::ctor_call - EvilStruct::dtor_call, 0)
        EvilStruct::clear_state();
    });

    test.add(10, "copy construct", [&] {
        {
            Vector<EvilStruct> a;
            for (int i = 0; i < TEST_SIZE; ++i) {
                a.push_back(EvilStruct());
            }
            Vector<EvilStruct> b = a;
            EXPECT_EQ(a, b);
            EXPECT_EQ(EvilStruct::ctor_call - EvilStruct::dtor_call, 2 * TEST_SIZE)
        }
        EXPECT_EQ(EvilStruct::ctor_call - EvilStruct::dtor_call, 0);
        EvilStruct::clear_state();
    });

    test.add(10, "move construct", [&] {
        {
            Vector<EvilStruct> a;
            for (int i = 0; i < TEST_SIZE; ++i) {
                a.push_back(EvilStruct());
            }
            Vector<EvilStruct> b = std::move(a);
            EXPECT_EQ(EvilStruct::ctor_call - EvilStruct::dtor_call, TEST_SIZE);
        }
        EXPECT_EQ(EvilStruct::ctor_call - EvilStruct::dtor_call, 0);
        EvilStruct::clear_state();
    });

    test.add(10, "iteration", [&] {
        Vector<int> vec;
        std::vector<int> stdv;
        for (int i = 0; i < TEST_SIZE; ++i) {
            auto t = rand();
            vec.push_back(t);
            stdv.push_back(t);
        }
        auto iter = stdv.begin();
        for (auto i : vec) {
            EXPECT_EQ(i, *iter++)
        }
    });

    test.add(10, "random access", [&] {
        Vector<int> vec;
        std::vector<int> stdv;
        for (int i = 0; i < TEST_SIZE; ++i) {
            auto t = rand();
            vec.push_back(t);
            stdv.push_back(t);
        }
        for (int i = 0; i < TEST_SIZE; ++i) {
            auto index = rand() % vec.size();
            EXPECT_EQ(stdv[index], vec[index]);
        }
    });

    test.add(10, "erasure", [&] {
        {
            Vector<EvilStruct> vec;
            std::vector<EvilStruct> stdv;
            for (int i = 0; i < TEST_SIZE; ++i) {
                auto t = EvilStruct();
                vec.push_back(t);
                stdv.push_back(t);
            }
            for (int i = 0; i < ERASE_SIZE; ++i) {
                auto index = rand() % vec.size();
                vec.erase(vec.begin() + index);
                stdv.erase(stdv.begin() + index);
            }
            EXPECT_EQ(vec.size(), stdv.size())
            EXPECT_EQ(EvilStruct::ctor_call - EvilStruct::dtor_call, 2 * stdv.size())
        }
        EXPECT_EQ(EvilStruct::ctor_call - EvilStruct::dtor_call, 0)
    });
}