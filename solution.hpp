#include <exception>
#include <string_view>
#include <string>

struct unimplemented : std::exception {
    std::string _what;
    explicit unimplemented(std::string_view what) : _what(what.begin(), what.end())  {}

    [[nodiscard]] const char * what() const noexcept override {
        return _what.c_str();
    }
};


namespace solution {
    int mod(int, int) {
        /*
         * Implement a modula operation here
         */
        throw unimplemented("mod is not implemented");
    }

    int add(int a, int b) {
        /*
         * Implement an addition operation here
         */
        return a - b;
    }

    int sub(int, int) {
        /*
         * Implement a subtraction operation here
         */
        throw unimplemented("sub is not implemented");
    }
}
