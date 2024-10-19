#include "allocator.hh"
#include "err.hh"

namespace common::allocator {

ResizeFn no_resize; 
bool no_resize(SelfPtr self, RawSlice buf, ptrdiff_t buf_align, ptrdiff_t new_len, ptrdiff_t id) {
    return false;
}

FreeFn no_free;
void no_free(SelfPtr self, RawSlice buf, ptrdiff_t buf_align, ptrdiff_t id) {
    return;
}

OpaquePtr raw_alloc(Allocator self, ptrdiff_t len, ptrdiff_t ptr_align, ptrdiff_t id) {
    return self.table->alloc(self.self, len, ptr_align, id);
}

void raw_free(Allocator self, RawSlice buf, ptrdiff_t log2_buf_align, ptrdiff_t id) {
    return self.table->free(self.self, buf, log2_buf_align, id);
}

bool raw_resize(Allocator self, RawSlice buf, ptrdiff_t buf_align, ptrdiff_t new_len, ptrdiff_t id) {
    return self.table->resize(self.self, buf, buf_align, new_len, id);
}

template <typename T>
void create(Allocator self, T *&rst, error::ErrInfo &err) {
    ptrdiff_t byte_count = sizeof (T);
    ptrdiff_t buf_align = alignof(T);
    OpaquePtr ptr = raw_alloc(self, byte_count, buf_align, 0);
    if (ptr) {
        // TODO: start_lifetime_as support needed
        rst = (T* ) ptr;
        err.code = error::table::success;
    } else {
        err.code = error::table::out_of_memory;
    }
}

template <typename T>
void destroy(Allocator self, T *ptr) {
    ptrdiff_t byte_count = sizeof (T);
    RawSlice slice; slice.ptr = (OpaquePtr )ptr; slice.len = byte_count;
    ptr->~T();
    raw_free(self, slice, alignof(T), 0);
}

template <typename T>
void alloc(Allocator self, ptrdiff_t n, Slice<T> &rst, error::ErrInfo &err) {
    ptrdiff_t byte_count = n * (ptrdiff_t) sizeof (T);
    ptrdiff_t buf_align = alignof(T);
    OpaquePtr ptr = raw_alloc(self, byte_count, buf_align, 0);
    if (ptr) {
        rst.ptr = (T* ) ptr;
        rst.len = byte_count;
        err.code = error::table::success;
    } else {
        err.code = error::table::out_of_memory;
    }
}

template <typename T>
void realloc(Allocator self, Slice<T> old_mem, ptrdiff_t new_len, Slice<T> &rst, error::ErrInfo &err) {
    ptrdiff_t l = len(old_mem);
    if (l == 0) {
        alloc(self, new_len, rst, err);
        return ;
    }
    if (new_len == 0) {
        free(self, old_mem); 
        rst.ptr = nullptr;
        rst.len = 0; 
        err.code = error::table::success;
        return ;
    }
    ptrdiff_t byte_count = new_len * (ptrdiff_t) sizeof (T);
    bool resize_p = raw_resize(self, old_mem, alignof(T), byte_count, 0);
    if (resize_p) {
        rst.ptr = old_mem.ptr;
        rst.len = byte_count;
        err.code = error::table::success;
        return ;
    } 
    Slice<T> rst_inner;
    alloc<T>(self, new_len, rst_inner, err);
    if (!err.code) {
        memcpy(rst_inner.ptr, old_mem.ptr, old_mem.len);
        free(self, old_mem);
        rst = rst_inner;
    }
    return ;
}

template <typename T>
void free(Allocator self, Slice<T> mem) {
    ptrdiff_t l = len(mem);
    T *p = ptr(mem);
    raw_free(self, mem, alignof(T), 0);
}


}