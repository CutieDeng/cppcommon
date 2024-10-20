#pragma once 

#include "allocator.hh"
#include "predef.hh"

namespace common::array {

struct RawArray {
    void *ptr;
    ptrdiff_t size;
    ptrdiff_t capacity;
};

template <typename T>
struct Array : public RawArray {
};

template <typename T>
ptrdiff_t size(Array<T> self) {
    return self.size / sizeof (T);
}

template <typename T>
ptrdiff_t capacity(Array<T> self) {
    return self.capacity / sizeof (T);
}

template <typename T>
T *ptr(Array<T> self) {
    return (T *) self.ptr;
}

template <typename T>
void init(Array<T> &self) {
    self.ptr = nullptr;
    self.size = 0;
    self.capacity = 0;
}

template <typename T>
void init_with_capacity(Array<T> &self, ptrdiff_t capacity, allocator::Allocator allocator, error::ErrInfo &err) {
    Slice<T> slice;
    alloc<T>(allocator, capacity, slice, err);
    if (err.code) {
        return ;
    }
    err.code = error::table::success;
    self.ptr = slice.ptr;
    self.size = 0;
    self.capacity = slice.len;
    return ;
}

}