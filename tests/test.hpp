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

struct warnings : std::exception {
    std::vector<std::string> msgs;
    std::string final;

    template<class T>
    explicit warnings(T &&msgs) : msgs(std::forward<T>(msgs)) {
        std::stringstream ss;
        for (auto &i : msgs) {
            ss << "   " << i << std::endl;
        }
        final = ss.str();
    }

    [[nodiscard]] const char *what() const noexcept override {
        return final.c_str();
    }
};

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
#define _UNIMPLEMENTED \
    unimplemented(EVAL_FUNC, EVAL_FILE, EVAL_LINE)
#define UNIMPLEMENTED \
    throw _UNIMPLEMENTED;
#define WARN(msg) \
    internal::warn((msg));
#define WARN_UNIMPLEMENTED \
    WARN(_UNIMPLEMENTED);
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


#ifndef _WIN32
#include <execinfo.h>  // for backtrace
#include <dlfcn.h>     // for dladdr
#include <cxxabi.h>    // for __cxa_demangle
namespace {
    void *last_frames[20];
    size_t last_size;
    std::string exception_name;

    std::string demangle(const char *name) {
        int status;
        std::unique_ptr<char, void (*)(void *)> realname(abi::__cxa_demangle(name, 0, 0, &status), &std::free);
        return status ? "failed" : &*realname;
    }
}

extern "C" {
void __cxa_throw(void* ex, void* info, void (_GLIBCXX_CDTOR_CALLABI * dest) (void *)) {
    exception_name = demangle(reinterpret_cast<const std::type_info *>(info)->name());
    last_size = backtrace(last_frames, sizeof last_frames / sizeof(void *));

    static void (*const rethrow)(void *, void *, void(*)(void *)) __attribute__ ((noreturn)) = (void (*)(void *, void *,
                                                                                                         void(*)(void *))) dlsym(
            RTLD_NEXT, "__cxa_throw");
    rethrow(ex, info, dest);
}
}
#endif

namespace internal {
    std::vector<std::string> warnings;

    template<class T>
    void warn(T &&except) {
        warnings.emplace_back(except.what());
    }
#if __has_include(<execinfo.h>)
    std::string _backtrace_impl(int skip = 1) {
        const int nMaxFrames = sizeof last_frames / sizeof(void *);
        char buf[1024];
        char **symbols = backtrace_symbols(last_frames, last_size);
        std::ostringstream trace_buf;
        for (size_t i = skip; i < last_size; i++) {
            Dl_info info;
            if (dladdr(last_frames[i], &info)) {
                char *demangled = NULL;
                int status;
                demangled = abi::__cxa_demangle(info.dli_sname, NULL, 0, &status);
                snprintf(buf, sizeof(buf), " %-3zu %*p %s + %zd\n",
                         i, int(2 + sizeof(void *) * 2), last_frames[i],
                         status == 0 ? demangled : info.dli_sname,
                         (char *) last_frames[i] - (char *) info.dli_saddr);
                free(demangled);
            } else {
                snprintf(buf, sizeof(buf), "%-3zu %*p\n",
                         i, int(2 + sizeof(void *) * 2), last_frames[i]);
            }
            trace_buf << buf;

            snprintf(buf, sizeof(buf), "     %s\n", symbols[i]);
            trace_buf << buf;
        }
        free(symbols);
        if (last_size == nMaxFrames)
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
    thread_local std::jmp_buf jmp_buffer;

    extern "C" void error_handler(int value) {
        asm(".cfi_signal_frame");
#if __has_include(<execinfo.h>)
        last_size = ::backtrace(last_frames, sizeof last_frames / sizeof(void *));
#endif
        switch (value) {
            case SIGFPE:
                WARN(std::runtime_error("arithmetic error"));
                break;
            case SIGSEGV:
                WARN(std::runtime_error("segment fault"));
                break;
            default:
                WARN(std::runtime_error("unknown error"));
                break;
        }
        std::longjmp(jmp_buffer, value);
    }

    void init() {
        std::signal(SIGSEGV, error_handler);
        std::signal(SIGFPE, error_handler);
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
            internal::warnings.clear();
            std::unique_ptr<std::exception> thread_error;
            try {
                std::thread handle {
                        [&]{
                            internal::init();
                            if (setjmp(internal::jmp_buffer) == 0) {
                                try {
                                    std::get<0>(i)();
                                }
                                catch (unimplemented &exp) {
                                    thread_error = std::make_unique<unimplemented>(exp);
                                }
                                catch (std::runtime_error &exp) {
                                    thread_error = std::make_unique<std::runtime_error>(exp);
                                }
                                catch (std::exception &exp) {
                                    thread_error = std::make_unique<std::exception>(exp);
                                }
                            }
                        }
                };
                handle.join();
                if (thread_error) {
                    std::cout << Color::Modifier(Color::FG_RED) << "[FAILURE]"
                              << Color::Modifier(Color::FG_CYAN) << "\n - message: \n   "
                              << Color::Modifier(Color::FG_DEFAULT) << thread_error->what() << std::endl;
                    std::cout << Color::Modifier(Color::FG_CYAN) << " - backtrace: "
                              << Color::Modifier(Color::FG_DEFAULT) << std::endl;
                    internal::backtrace();
                    continue;
                }
                if (!internal::warnings.empty()) {
                    auto exp = warnings(internal::warnings);
                    std::cout << Color::Modifier(Color::FG_RED) << "[FAILURE]"
                              << Color::Modifier(Color::FG_CYAN) << "\n - message: \n   "
                              << Color::Modifier(Color::FG_DEFAULT) << exp.what() << std::endl;
                    std::cout << Color::Modifier(Color::FG_CYAN) << " - backtrace: "
                              << Color::Modifier(Color::FG_DEFAULT) << std::endl;
                    internal::backtrace();
                    continue;
                }
                current += std::get<1>(i);;
                std::cout << Color::Modifier(Color::FG_GREEN) << "[SUCCESS]"
                          << Color::Modifier(Color::FG_DEFAULT) << std::endl;
            } catch (std::exception &exp) {
                std::cout << Color::Modifier(Color::FG_RED) << "[FAILURE]"
                          << Color::Modifier(Color::FG_CYAN) << "\n - message: \n   "
                          << Color::Modifier(Color::FG_DEFAULT) << exp.what() << std::endl;
                if (!internal::warnings.empty()) {
                    std::cout << warnings(internal::warnings).what() << std::endl;
                }
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


