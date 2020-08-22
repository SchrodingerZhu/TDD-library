CONFIG += debug c++17 sanitizer sanitize_address

HEADERS += tests/test.hpp \
    student/solution.hpp

INCLUDEPATH += tests \
    student

SOURCES += tests/test.cpp
TARGET = test
!win32 {
    LIBS += -ldl
}
