#pragma once

#include "predef.hh"
#include "core.hh"
#include "allocator.hh"

namespace common::rc {

using namespace def;

template <typename T>
struct RcPayload {
    T value;
    ptrdiff_t count;
    ptrdiff_t weak_count;
    using DropT = void (*)(OpaquePtr, T &);
    DropT drop_fn;
    OpaquePtr o;
    allocator::Allocator allocator;
};

template <typename T>
struct Rc {
    RcPayload<T> *ptr;
};

template <typename T>
struct Weak {
    RcPayload<T> *ptr;
};

template <typename T>
Rc<T> clone(Rc<T> &rc) {
    rc.ptr->count += 1;
    return rc;
}

template <typename T>
Weak<T> clone(Weak<T> &wc) {
    wc.ptr->weak_count += 1;
    return wc;
}

template <typename T>
Weak<T> weak_from(Rc<T> &rc) {
    rc.ptr->weak_count += 1;
    Weak<T> w;
    w.ptr = rc.ptr;
    return w;
}

template <typename T>
void uninit(Rc<T> &rc, T *&rst, typename RcPayload<T>::DropT d, OpaquePtr o, allocator::Allocator allocator, error::ErrInfo &err) {
    create(allocator, rc.ptr, err);
    if (err.code) { return ; } 
    rc.ptr->count = 1;
    rc.ptr->weak_count = 0;
    rc.ptr->o = o;
    rc.ptr->drop_fn = d;
    rc.ptr->allocator = allocator;
}

template <typename T>
void deinit(Rc<T> &rc) {
    rc.ptr->count -= 1;
    if (rc.ptr->count == 0) {
        rc.ptr->drop_fn(rc.ptr->o, rc.ptr->value);
    }
    if (rc.ptr->weak_count == 0) {
        destroy(rc.ptr->allocator, rc.ptr);
    }
}

template <typename T>
void deinit(Weak<T> &w) {
    w.ptr->weak_count -= 1;
    if (w.ptr->count == 0 && w.ptr->count == 0) {
        destroy(w.ptr->allocator, w.ptr);
    }
}

template <typename T>
void rc_from_weak(Weak<T> &w, Rc<T> &rst, error::ErrInfo &err) {
    if (w.ptr->count == 0) {
        err.code = error::table::weak_upgrade_failure;
        err.opaque = w.ptr;
        return ;
    }
    w.ptr->count += 1;
    rst.ptr = w.ptr;
    err.code = error::table::success;
}

template <typename T>
T *ptr(Rc<T> &r) {
    return &r.ptr->value;
}

}
