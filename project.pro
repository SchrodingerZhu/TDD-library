CONFIG += debug c++17 sanitizer sanitize_address
QMAKE_CXXFLAGS_WARN_ON = -Wall -Wextra -Werror

HEADERS += tests/test.hpp \
    student/solution.hpp

INCLUDEPATH += tests \
    student

SOURCES += tests/test.cpp
TARGET = test
!win32 {
    LIBS += -ldl
}
