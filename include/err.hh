#pragma once

#include "predef.hh"

namespace common::error::table {

enum ErrorConst : ptrdiff_t {
#define DEFERR(idx, val) \
    idx,
#include "err/config"
#undef DEFERR
};

char const *error_name_table[] = {
#define DEFERR(idx, val) \
    val,
#include "err/config"
#undef DEFERR
};

}

namespace common::error {

using namespace common::def;

struct ErrInfo {
    table::ErrorConst code;
    OpaquePtr opaque;
};

void init(ErrInfo &info) {
    info.code = table::undefined;
}

}