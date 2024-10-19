#include "allocator.hh"

#include <stdlib.h>

namespace common::allocator::libc {

namespace {
    OpaquePtr alloc(ptrdiff_t len, ptrdiff_t buf_align) {
        // TODO delete useless checks
        if (len < 0) {
            return nullptr;
        }
        if (buf_align < 0) {
            return nullptr;
        }
        OpaquePtr rst;
        if (buf_align < 8) {
            rst = ::malloc(len);
        } else {
            rst = ::aligned_alloc(buf_align, len);
        }
        return rst;
    }
    bool resize(RawSlice buf, ptrdiff_t buf_align, ptrdiff_t new_len) {
        if (buf.len >= new_len) {
            return buf.ptr;
        }
        return false;
    }
    void free(RawSlice buf, ptrdiff_t buf_align) {
        ::free(buf.ptr);
    }
}

OpaquePtr alloc_wrap(SelfPtr self, ptrdiff_t len, ptrdiff_t ptr_align, ptrdiff_t id) {
    return alloc(len, ptr_align);
}

bool resize_wrap(SelfPtr self, RawSlice buf, ptrdiff_t buf_align, ptrdiff_t new_len, ptrdiff_t id) {
    return resize(buf, buf_align, new_len);
}

void free_wrap(SelfPtr self, RawSlice buf, ptrdiff_t buf_align, ptrdiff_t id) {
    free(buf, buf_align);
}

AllocatorTable table = {
    .alloc = alloc_wrap,
    .resize = resize_wrap,
    .free = free_wrap,
};

Allocator allocator = {
    .table = &table,
    .self = nullptr,
};

}