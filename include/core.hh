#pragma once

#include "predef.hh"

namespace common {

    struct RawSlice {
        def::OpaquePtr ptr;
        ptrdiff_t len;
    }; 

    template <typename V>
    struct Slice : public RawSlice {
    };

    template <typename V>
    V *ptr(Slice<V> &slice) {
        return (V *)slice.ptr;
    }

    template <typename V>
    ptrdiff_t len(Slice<V> &slice) {
        return slice.len;
    }

    template <typename T>
    Slice<T> init(T *ptr, ptrdiff_t len) {
        Slice<T> ret;
        ret.ptr = (def::OpaquePtr)ptr;
        ret.len = len;
        return ret;
    }

    template <typename T>
    Slice<T> init(RawSlice slice) {
        return init((T*) slice.ptr, slice.len);
    }

    template <typename T>
    T &index(Slice<T> &slice, ptrdiff_t n) {
        return ptr(slice)[n];
    }

}