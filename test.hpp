#ifndef TEST_HPP
#define TEST_HPP

#include <functional>
#include <vector>
#include <iostream>
#include <tuple>
#include <string_view>
#include <cstring>
#include <csignal>
#include <thread>
#include <csetjmp>
#include <experimental/type_traits>
#include <sstream>
#include <string>
#ifndef _WIN32
#include <stdio.h>
#include <zconf.h>
#endif

namespace Color {
    enum Code {
        FG_RED = 31,
        FG_GREEN = 32,
        FG_YELLOW = 33,
        FG_BLUE = 34,
        FG_CYAN = 36,
        FG_DEFAULT = 39,
        BG_RED = 41,
        BG_GREEN = 42,
        BG_BLUE = 44,
        BG_DEFAULT = 49,
    };

    class Modifier {
        Code code;
    public:
        explicit Modifier(Code pCode) : code(pCode) {}

        friend std::ostream &
        operator<<(std::ostream &os, const Modifier &mod) {
#ifdef _WIN32 // It is hard to check windows ANSI environment
            return os;
#else
            if (isatty(STDOUT_FILENO))
                return os << "\033[" << mod.code << "m";
            else {
                return os;
            }
#endif
        }
    };
}

struct unimplemented : std::exception {
    std::string _what;

    explicit unimplemented(const char *func, const char *file, int line) {
        _what = "unimplemented ";
        _what.append(func);
        _what.append("\n   (");
        _what.append(file);
        _what.push_back(':');
        _what.append(std::to_string(line));
        _what.append(")");
    }

    [[nodiscard]] const char *what() const noexcept override {
        return _what.c_str();
    }
};

#define EXPECT_TRUE(X) \
    if (!(X)) throw std::runtime_error("expected " #X " to be true, but it failed");

#define EVAL_FILE __FILE__
#define EVAL_LINE __LINE__
#define EVAL_FUNC __PRETTY_FUNCTION__

#define EXPECT_EQ(X, Y) \
    if (!((X) == (Y))) throw std::runtime_error(construct_display((X), (Y), #X, #Y, EVAL_FILE, EVAL_LINE));
#define UNIMPLEMENTED \
    throw unimplemented(EVAL_FUNC, EVAL_FILE, EVAL_LINE);

template<typename T>
using displayable_inner = decltype(std::cout << std::declval<T &>());

template<typename T>
constexpr bool displayable = std::experimental::is_detected_v<displayable_inner, T>;

template<class A, class B>
std::string construct_display(const A &a, const B &b, const char *x, const char *y, const char *file, int line) {
    std::stringstream ss;
    ss << "expected ";
    if constexpr (displayable<A>) {
        ss << x << " [= " << a << "] to be equal to ";
    } else {
        ss << x << " to be equal to ";
    }
    if constexpr (displayable<B>) {
        ss << y << " [= " << b << "], but it failed" << std::endl;
    } else {
        ss << y << ", but it failed" << std::endl;
    }
    ss << "   (" << file << ":" << line << ")";
    return ss.str();
}


namespace internal {
#if __has_include(<execinfo.h>)

#include <execinfo.h>  // for backtrace
#include <dlfcn.h>     // for dladdr
#include <cxxabi.h>    // for __cxa_demangle

    std::string _backtrace_impl(int skip = 1) {
        void *callstack[128];
        const int nMaxFrames = sizeof(callstack) / sizeof(callstack[0]);
        char buf[1024];
        int nFrames = backtrace(callstack, nMaxFrames);
        char **symbols = backtrace_symbols(callstack, nFrames);

        std::ostringstream trace_buf;
        for (int i = skip; i < nFrames; i++) {
            Dl_info info;
            if (dladdr(callstack[i], &info)) {
                char *demangled = NULL;
                int status;
                demangled = abi::__cxa_demangle(info.dli_sname, NULL, 0, &status);
                snprintf(buf, sizeof(buf), " %-3d %0*p %s + %zd\n",
                         i, 2 + sizeof(void *) * 2, callstack[i],
                         status == 0 ? demangled : info.dli_sname,
                         (char *) callstack[i] - (char *) info.dli_saddr);
                free(demangled);
            } else {
                snprintf(buf, sizeof(buf), "%-3d %*0p\n",
                         i, 2 + sizeof(void *) * 2, callstack[i]);
            }
            trace_buf << buf;

            snprintf(buf, sizeof(buf), "     %s\n", symbols[i]);
            trace_buf << buf;
        }
        free(symbols);
        if (nFrames == nMaxFrames)
            trace_buf << "[truncated]\n";
        return trace_buf.str();
    }
    void backtrace() {
        std::cout << _backtrace_impl();
    }
#else
    void backtrace() {
        std::cout << "   backtrace is not supported in this runtime" << std::endl;
    }
#endif
    std::jmp_buf jmp_buffer;

    extern "C" void register_func(int value) {
        std::longjmp(jmp_buffer, value);
    }

    void init() {
        std::signal(SIGSEGV, register_func);
        std::signal(SIGFPE, register_func);
    }
}

struct Test {
    long current = 0;
    long total = 0;
    std::vector<std::tuple<std::function<void()>, long, std::string_view>> registry;

    Test() {
        internal::init();
    }

    void add(long score, std::string_view name, std::function<void()> func) {
        total += score;
        registry.emplace_back(std::move(func), score, name);
    }

    ~Test() {
        for (auto &i : registry) {
            std::cout << Color::Modifier(Color::FG_BLUE) << "testing <" << std::get<2>(i) << ">: ";
            std::memset(&internal::jmp_buffer, 0, sizeof(internal::jmp_buffer));
            try {
                int val;
                val = setjmp(internal::jmp_buffer);
                if (val == 0) {
                    std::get<0>(i)();
                } else {
                    switch (val) {
                        case SIGFPE:
                            throw std::runtime_error("arithmetic error");
                        case SIGSEGV:
                            throw std::runtime_error("segment fault");
                        default:
                            throw std::runtime_error("unknown error");
                    }
                }
                current += std::get<1>(i);;
                std::cout << Color::Modifier(Color::FG_GREEN) << "[SUCCESS]"
                          << Color::Modifier(Color::FG_DEFAULT) << std::endl;
            } catch (std::exception &exp) {
                std::cout << Color::Modifier(Color::FG_RED) << "[FAILURE]"
                          << Color::Modifier(Color::FG_CYAN) << "\n - message: \n   "
                          << Color::Modifier(Color::FG_DEFAULT) << exp.what() << std::endl;
                std::cout << Color::Modifier(Color::FG_CYAN) << " - backtrace: "
                          << Color::Modifier(Color::FG_DEFAULT) << std::endl;
                internal::backtrace();
            }
        }
        std::cout << Color::Modifier(Color::FG_YELLOW) << "[RESULT] " << current << "/" << total <<
                  std::endl;
    }

};

#endif // TEST_HPP


