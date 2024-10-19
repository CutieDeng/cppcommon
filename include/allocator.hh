#pragma once

#include "predef.hh"
#include "core.hh"

namespace common::allocator {

using namespace def;

using AllocFn = OpaquePtr (SelfPtr self, ptrdiff_t len, ptrdiff_t ptr_align, ptrdiff_t id);
using ResizeFn = bool (SelfPtr self, RawSlice buf, ptrdiff_t buf_align, ptrdiff_t new_len, ptrdiff_t id);
using FreeFn = void (SelfPtr self, RawSlice buf, ptrdiff_t buf_align, ptrdiff_t id);
struct AllocatorTable {
    AllocFn *alloc;
    ResizeFn *resize;
    FreeFn *free;
}; 

struct Allocator {
    AllocatorTable *table;
    OpaquePtr self;
};

}

namespace common::allocator::impl0 {

using namespace common::allocator;

}

#include "allocator/c.cc"

#include "allocator/impl.cc"
