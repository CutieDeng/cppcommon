#include "predef.hh"

namespace common::tuple {

template <typename L, typename R>
struct Pair : R {
    using LeftType = L;
    using RightType = R;
    L value;
    static constexpr ptrdiff_t size = 1 + R::size;
};

}

namespace common::tuple::impl {

template <typename ...>
struct TupleHelper; 

struct NullTuple {
    static constexpr ptrdiff_t size = 0;
};

template <typename T, typename... Args>
struct TupleHelper<T, Args...> {
    using Type = Pair<T, typename TupleHelper<Args...>::Type>; 
};

template <>
struct TupleHelper<> {
    using Type = NullTuple;
};

}

namespace common::tuple {

template <typename ...Args>
using Tuple = impl::TupleHelper<Args...>::Type;

}

namespace common::tuple::impl {
    
template <typename T, ptrdiff_t idx>
struct TupleGetHelper;

template <typename T>
struct TupleGetHelper<T, 0> {
    using Type = T::LeftType;
};

template <typename T, ptrdiff_t idx>
struct TupleGetHelper : TupleGetHelper<typename T::RightType, idx - 1> {

};

}

namespace common::tuple {

template <typename T, ptrdiff_t idx>
using TupleGet = impl::TupleGetHelper<T, idx>::Type;

}

namespace common::tuple::impl {

template <typename T, ptrdiff_t idx>
struct TupleDataGetHelper {
    static auto &get(T &t) {
        return TupleDataGetHelper<typename T::RightType, idx - 1>::get(t);
    }
};

template <typename T>
struct TupleDataGetHelper<T, 0> {
    static auto &get(T &t) {
        return t.value;
    }
};

}

namespace common::tuple {

template <ptrdiff_t idx, typename T>
auto &get(T &t) {
    return impl::TupleDataGetHelper<T, idx>::get(t);
}

}

namespace common::tuple::impl {

template <typename T, ptrdiff_t idx>
struct TupleForHelper {
    template <typename F>
    static void for_each(T &t, F &f) {
        typename T::RightType &next = t;
        f(t.value, idx);
        TupleForHelper<typename T::RightType, idx + 1>::for_each(next, f);
    }
};

template <ptrdiff_t idx>
struct TupleForHelper<NullTuple, idx> {
    template <typename F>
    static void for_each(NullTuple &t, F &f) {
    }
};

}

namespace common::tuple {

template <typename T, typename F>
void for_each(T &tuple, F &fn) {
    impl::TupleForHelper<T, 0>::for_each(tuple, fn);
}

}

namespace common::tuple {

template <typename T>
ptrdiff_t size(T &t) {
    return T::size;
}

}

namespace common::tuple::impl {

template <typename T, typename T2>
struct ConcatHelper {
    using Type = Pair<typename T::LeftType, typename ConcatHelper<typename T::RightType, T2>::Type>;
    static void concat_impl(Type &rst, T &t, T2 &t2) {
        typename Type::RightType &rst_r = rst;
        typename T::RightType &t_r = t;
        ConcatHelper<typename T::RightType, T2>::concat_impl(rst_r, t_r, t2);
        rst.value = t.value;
    }
    static Type concat(T &t, T2 &t2) {
        Type rst;
        concat_impl(rst, t, t2);
        return rst;
    }
};

template <typename T>
struct ConcatHelper <NullTuple, T> {
    using Type = T;
    static void concat_impl(Type &rst, NullTuple &t, T &t2) {
        rst = t2;
        return ;
    }
    static Type concat(NullTuple &t, T &t2) {
        Type rst;
        concat_impl(rst, t, t2);
        return rst;
    }
};

}

namespace common::tuple {

template <typename T, typename T2>
using ConcatT = impl::ConcatHelper<T, T2>::Type;

template <typename T, typename T2>
auto concat(T &t, T2 &t2) {
    return impl::ConcatHelper<T, T2>::concat(t, t2);
}

}

namespace common::tuple::impl {

template <typename T, ptrdiff_t n>
struct DropNHelper {
    using Type = typename DropNHelper<typename T::RightType, n - 1>::Type;
};

template <typename T>
struct DropNHelper<T, 0> {
    using Type = T;
};

}

namespace common::tuple {

template <typename T, ptrdiff_t n>
using DropN = impl::DropNHelper<T, n>::Type;

}

namespace common::tuple::impl {

template <typename T, typename A>
struct ReverseHelper {
    using Type = typename ReverseHelper<typename T::RightType, Pair<typename T::LeftType, A>>::Type;
};

template <typename A>
struct ReverseHelper<NullTuple, A> {
    using Type = A;
};

}

namespace common::tuple {

template <typename T>
using Reverse = impl::ReverseHelper<T, impl::NullTuple>::Type;

}

namespace common::tuple::impl {

template <typename T, typename T2, ptrdiff_t idx, ptrdiff_t idx2>
struct ReverHelper2 {
    static void reverse_impl(T &rst, T2 &src) {
        get<idx>(rst) = get<idx2>(src);
        ReverHelper2<T, T2, idx + 1, idx2 - 1>::reverse_impl(rst, src);
    }
    static T reverse(T2 &t2) {
        T rst;
        reverse_impl(rst, t2);
        return rst;
    }
};

template <typename T, typename T2, ptrdiff_t idx>
struct ReverHelper2 <T, T2, idx, 0> {
    static void reverse_impl(T &rst, T2 &src) {
        get<idx>(rst) = get<0>(src);
    }
    static T reverse(T2 &t2) {
        T rst;
        reverse_impl(rst, t2);
        return rst;
    }
};

}

namespace common::tuple {

template <typename T>
auto reverse(T &t) {
    return impl::ReverHelper2<Reverse<T>, T, 0, T::size - 1>::reverse(t);
}

}