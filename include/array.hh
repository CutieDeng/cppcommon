#pragma once 

#include <string.h>

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
void init_capacity(Array<T> &self, ptrdiff_t capacity, allocator::Allocator allocator, error::ErrInfo &err) {
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

template <typename T>
void init_buffer(Array<T> &self, Slice<T> slice) {
    self.ptr = slice.ptr;
    self.size = 0; 
    self.capacity = slice.len;
}

template <typename T>
Slice<T> allocated_slice(Array<T> self) {
    Slice<T> rst;
    rst.ptr = self.ptr;
    rst.len = self.capacity;
    return rst;
}

template <typename T>
Slice<T> slice(Array<T> self) {
    Slice<T> rst;
    rst.ptr = self.ptr;
    rst.len = self.size;
    return rst;
}

template <typename T>
void deinit(Array<T> &self, allocator::Allocator allocator) {
    Slice<T> sl = allocated_slice(self);
    free(allocator, sl);
}

template <typename T>
void from_owned_slice(Array<T> &self, Slice<T> slice) {
    self.ptr = slice.ptr;
    self.size = slice.len;
    self.capacity = slice.len;
}

template <typename T>
void to_owned_slice(Array<T> &self, allocator::Allocator allocator, Slice<T> &rst, error::ErrInfo &err) {
    Slice<T> old_mem = allocated_slice(self);
    ptrdiff_t items_len = size(self);
    bool r = resize(allocator, old_mem, items_len);
    if (r) {
        err.code = error::table::success;
        rst = old_mem;
        self.ptr = nullptr; 
        self.size = 0;
        self.capacity = 0;
        return ;
    }
    Slice<T> sub_mem;
    sub_mem.ptr = self.ptr;
    sub_mem.len = self.size;
    dupe(allocator, sub_mem, rst, err);
    if (err.code) {
        return ;
    }
    clear_and_free(self, allocator);
}

template <typename T>
void clear_and_free(Array<T> &self, allocator::Allocator allocator) {
    Slice<T> sl = allocated_slice(self);
    free(allocator, sl);
    self.ptr = nullptr;
    self.size = 0;
    self.capacity = 0;
}

template <typename T>
void append_slice_assume_capacity(Array<T> &self, Slice<T> items) {
    ptrdiff_t s = len(items);
    T *a = ptr(self);
    T *b = ptr(items);
    for (ptrdiff_t i = 0; i < s; i += 1) {
        a[i] = b[i];
    }
    self.size += items.len;
}

template <typename T>
void clone(Array<T> self, allocator::Allocator allocator, Array<T> &rst, error::ErrInfo &err) {
    Array<T> r;
    init_capacity(r, capacity(self), allocator, err);
    if (err.code) { return ; }
    append_slice_assume_capacity(r, slice(self));
    rst = r;
}

ptrdiff_t grow_capacity(ptrdiff_t current, ptrdiff_t minimum) {
    ptrdiff_t n = current;
    while (true) {
        n += n / 2 + 8;
        if (n >= minimum) {
            return n;
        }
    }
}

template <typename T>
void ensure_total_capacity_precise(Array<T> &self, ptrdiff_t new_capacity, allocator::Allocator allocator, error::ErrInfo &err) {
    ptrdiff_t capa = capacity(self);
    if (capa >= new_capacity) {
        err.code = error::table::success;
        return ;
    }
    if (resize(allocator, allocated_slice(self), new_capacity)) {
        self.capacity = new_capacity * (ptrdiff_t ) sizeof (T);
        err.code = error::table::success;
        return ;
    } 
    Slice<T> e;
    alloc(allocator, new_capacity, e, err);
    if (err.code) { return ; } 
    memcpy(e.ptr, self.ptr, self.size);
    free(allocator, allocated_slice(self));
    self.ptr = e.ptr;
    self.capacity = e.len;
}

template <typename T>
void ensure_total_capacity(Array<T> &self, ptrdiff_t new_capacity, allocator::Allocator allocator, error::ErrInfo &err) {
    ptrdiff_t capa = capacity(self);
    if (capa < new_capacity) {
        ptrdiff_t new_capacity = grow_capacity(capa, new_capacity);
        if (!resize(allocator, allocated_slice(self), new_capacity)) {
            // ...
            ensure_total_capacity_precise(self, new_capacity, allocator, err);
            if (err.code) { return ; } 
        } else {
            self.capacity = new_capacity * (ptrdiff_t ) sizeof (T);
            err.code = error::table::success;
        }
    } else {
        err.code = error::table::success;
    }
}

template <typename T>
void ensure_unused_capacity(Array<T> &self, ptrdiff_t additional_count, allocator::Allocator allocator, error::ErrInfo &err) {
    ensure_total_capacity(self, size(self) + additional_count, allocator, err);
}

template <typename T>
void add_many_at(Array<T> &self, ptrdiff_t idx, ptrdiff_t count, allocator::Allocator allocator, Slice<T> &rst, error::ErrInfo &err) {
    ptrdiff_t new_len = len(self) + count;
    if (capacity(self) < new_len) {
        if (!resize(allocator, allocated_slice(self), new_len)) {
            ensure_unused_capacity(self, count, allocator, err);
            if (err.code) { return ; }
        } else {
            self.capacity = new_len * (ptrdiff_t ) sizeof (T);
        }
    }
    rst = add_many_at_assume_capacity(self, idx, count);
    err.code = error::table::success;
}

template <typename T>
Slice<T> add_many_at_assume_capacity(Array<T> &self, ptrdiff_t idx, ptrdiff_t count) {
    ptrdiff_t new_len_count = self.size + count * (ptrdiff_t ) sizeof (T);
    ptrdiff_t new_len = len(self) + count;
    T *p = ptr(self);
    Slice<T> rst;
    rst.ptr = &p[idx];
    rst.len = count * (ptrdiff_t ) sizeof (T);
    for (ptrdiff_t x = new_len; x > idx + count; x -= 1) {
        p[x-1] = p[x-1-count];
    }
    self.size = new_len_count;
    return rst;
}

template <typename T>
void add_one_assume_capacity(Array<T> &self, T *&item) {
    item = &ptr(self)[size(self)];
    self.size += sizeof (T);
}

template <typename T>
void add_one(Array<T> &self, T *&item, allocator::Allocator allocator, error::ErrInfo &err) {
    ensure_unused_capacity(self, 1, allocator, err);
    if (err.code) { return ; }
    add_one_assume_capacity(self, item);
}

template <typename T>
void pop(Array<T> &self, T &value) {
    value = ptr(self)[size(self)-1];
    self.size -= sizeof (T);
}

template <typename T>
bool pop_or_null(Array<T> &self, T &value) {
    if (size(self) == 0) {
        return false;
    }
    pop(self, value);
    return true;
}

template <typename T>
void get_last(Array<T> self, T &value) {
    value = ptr(self)[size(self)-1];
}

template <typename T>
bool get_last_or_null(Array<T> self, T &value) {
    if (size(self) == 0) {
        return false;
    }
    get_last(self, value);
    return true;
}

}