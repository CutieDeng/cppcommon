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

template <typename T>
void create(Allocator self, T *&rst, error::ErrInfo &err) {
    ptrdiff_t byte_count = sizeof (T);
    ptrdiff_t buf_align = alignof(T);
    OpaquePtr ptr = raw_alloc(self, byte_count, buf_align, 0);
    if (ptr) {
        // TODO: start_lifetime_as support needed
        rst = new (ptr) T;
        err.code = error::table::success;
    } else {
        err.code = error::table::out_of_memory;
    }
}

template <typename T>
void destroy(Allocator self, T *ptr) {
    ptrdiff_t byte_count = sizeof (T);
    RawSlice slice; slice.ptr = (OpaquePtr )ptr; slice.len = byte_count;
    raw_free(self, slice, alignof(T), 0);
}

template <typename T>
void alloc(Allocator self, ptrdiff_t n, Slice<T> &rst, error::ErrInfo &err) {
    ptrdiff_t byte_count = n * (ptrdiff_t) sizeof (T);
    ptrdiff_t buf_align = alignof(T);
    OpaquePtr ptr = raw_alloc(self, byte_count, buf_align, 0);
    if (ptr) {
        rst.ptr = new (ptr) T[n];
        rst.len = n;
        err.code = error::table::success;
    } else {
        err.code = error::table::out_of_memory;
    }
}


}