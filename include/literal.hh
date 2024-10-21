#pragma once

#include "core.hh"

namespace common::literal::helper {

using namespace def;

template <size_t N>
struct CharSeq {
    char contents[N];
    constexpr CharSeq(char const (&a)[N]) {
        for (ptrdiff_t i = 0; i < (ptrdiff_t )N ; i += 1) {
            contents[i] = a[i];
        }
    }
};

template <CharSeq seq>
Slice<char> from() {
    static CharSeq private_static = seq;
    Slice<char> rst;
    rst.ptr = (OpaquePtr ) private_static.contents;
    rst.len = sizeof seq - 1;
    static_assert(seq.contents[sizeof seq - 1] == '\0');
    return rst;
}

}

namespace common::literal {

template <helper::CharSeq seq>
Slice<char> operator ""_slice () {
    return helper::from<seq>();
}

}