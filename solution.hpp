#include <exception>
#include <string_view>
#include <string>
#include "test.hpp"


template<class T>
class Vector {
    /*
     * You should decide what fields to be included in the class
     */
public:
    using element = T;
    using iterator = element *;
    using size_type = size_t;

    Vector() {
        /*
         * Implement a constructor
         */
        UNIMPLEMENTED
    }
    Vector(const Vector&) {
        /*
         * Implement a copy constructor
         */
        UNIMPLEMENTED
    }
    Vector(Vector&&) noexcept {
        /*
         * Implement a move constructor
         */
        UNIMPLEMENTED
    }

    ~Vector() {
        /*
         * Implement a destructor.
         */
        UNIMPLEMENTED
    }
    void push_back(const T&) {
        /*
         * Provide a way to push a new element to the rear.
         */
        UNIMPLEMENTED
    }
    size_type size() {
        /*
         * Provide a way check the size.
         */
        UNIMPLEMENTED
    }
    T& operator[](size_t) {
        /*
         * Provide a way access the element.
         */
        UNIMPLEMENTED
    }
    iterator begin() {
        /*
         * Provide a way to get the begin iterator.
         */
        UNIMPLEMENTED
    }
    iterator end() {
        /*
         * Provide a way to get the end iterator.
         */
        UNIMPLEMENTED
    }
    bool operator==(const Vector&) const {
        /*
         * Provide a way compare two vector.
         */
        UNIMPLEMENTED
    }
    std::vector<T> to_std() const {
        /*
         * Provide a way transform the current vector into `std::vector`
         */
        UNIMPLEMENTED
    }
    void erase(iterator) {
        /*
         * Provide a way to erase an element at the given iterator
         */
        UNIMPLEMENTED
    }
};
