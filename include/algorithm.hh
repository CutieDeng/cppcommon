#pragma once 

#include "core.hh"
#include "err.hh"

namespace common::algorithm {

using namespace def;

template <typename T>
struct SplitIterator {
    Slice<T> buffer;
    bool index_exists;
    ptrdiff_t index;
    Slice<T> delimiter;
};

template <typename T>
Slice<T> first(SplitIterator<T> &self) {
    if (!(self.index_exists && self.index == 0)) {
        t: throw "invalid invocation";
    }
    Slice<T> rst;
    error::ErrInfo e;    
    next(self, rst, e);
    if (e.code) { goto t; } 
    return rst;
}

template <typename T>
void next(SplitIterator<T> &self, Slice<T> &rst, error::ErrInfo &err) {
    if (!self.index_exists) { 
        err.code = error::table::split_eof;
        return ;
    }
    T *b = ptr(self.buffer);
    T *d = ptr(self.delimiter);
    ptrdiff_t limit = len(self.buffer);
    ptrdiff_t l = len(self.delimiter);
    ptrdiff_t s = self.index;
    ptrdiff_t idx = self.index;
    for (;;) {
        if (idx == limit) {
            self.index = limit;
            self.index_exists = false;
            goto just_exit;
        }
        for (ptrdiff_t i = 0; i < l; i += 1) {
            if (idx + i >= limit) {
                goto failure_branch;
            }
            if (b[idx + i] != d[i]) {
                goto failure_branch;
            }
        }
        {
            true_branch:
            self.index = idx + l; 
            just_exit:
            rst.ptr = &b[s];
            rst.len = (idx - s) * (ptrdiff_t ) sizeof (T);
            err.code = error::table::success;
            return ;
        }
        if (false) {
            failure_branch:
            ;
        }
        idx += 1;
    }
}

template <typename T>
Slice<T> rest(SplitIterator<T> &self) {
    Slice<T> rst;
    rst.ptr = (OpaquePtr) &ptr(self.buffer)[self.index];
    rst.len = self.buffer.len - (self.index * (ptrdiff_t ) sizeof (T));
    return rst;
}

template <typename T>
void reset(SplitIterator<T> &self) {
    self.index = 0;
    self.index_exists = true;
}

template <typename T>
void split(SplitIterator<T> &self, Slice<T> buffer, Slice<T> delimiter) {
    self.buffer = buffer;
    self.delimiter = delimiter;
    self.index = 0;
    self.index_exists = true;
}

}
