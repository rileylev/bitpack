#ifndef BITPACK_MACROS_INCLUDE_GUARD
#define BITPACK_MACROS_INCLUDE_GUARD

// Customizable assert macro
// If you define BITPACK_ENABLE_ASSERT, it will pull in the standard assert.h
// mechanism. If you define BITPACK_ASSERT before #including this library, it
// will use that. If you do nothing, as of now there are no asserts.
//
// Most of bitpack is a good candidate for inlining, so assertions might be
// relatively costly, hence the extra indirection
#if defined(BITPACK_ASSERT)
#  if defined(BITPACK_ENABLE_ASSERT)
#    undef BITPACK_ENABLE_ASSERT
#  endif
#  define BITPACK_ENABLE_ASSERT true
#else
#  if !defined(BITPACK_ENABLE_ASSERT)
#    define BITPACK_ENABLE_ASSERT false
#  endif
#  if BITPACK_ENABLE_ASSERT
#    include <assert.h>
#    define BITPACK_ASSERT(...) assert(__VA_ARGS__)
#  else
#    define BITPACK_ASSERT(...) ((void)0)
#  endif
#endif

#define BITPACK_FWD(x) std::forward<decltype(x)>(x)

// for expression bodies!
// use this where you want to define a function whose whole body is one
// expression. this macro wil automatically propogate the return type and the
// noexcept-ness of that expression, which helps a little with the keyword soup.
// This also prevents the inevitable subtle noexcept/decltype typos :C
#define BITPACK_EXPR_BODY(...)                                                 \
  noexcept(noexcept(__VA_ARGS__))->decltype(__VA_ARGS__) { return __VA_ARGS__; }

namespace bitpack { namespace impl {
inline bool constexpr is_assert_off = !BITPACK_ENABLE_ASSERT;
}} // namespace bitpack::impl

// To locally turn of warnings that some paths don't return
// in particular, in variant_ptr::visit, index must be in the range [0,size).
// So correct code can assume as much.
// But the compiler warnings do not know that.
//
// the code is based on
// https://www.fluentcpp.com/2019/08/30/how-to-disable-a-warning-in-cpp/
#if defined(__GNUC__) || defined(__clang__)
#  define BITPACK_WRETURN_OFF                                                  \
    _Pragma("GCC diagnostic push")                                             \
        _Pragma("GCC diagnostic ignored \"-Wreturn-type\"")
#  define BITPACK_DIAGNOSTIC_POP _Pragma("GCC diagnostic pop")
#elif defined(_MSC_VER)
#  define BITPACK_WRETURN_OFF                                                  \
    __pragma(warning(push)) __pragma(warning(disable : 4715))
#  define BITPACK_DIAGNOSTIC_POP __pragma(warning(pop))
#else
#  define BITPACK_WRETURN_OFF
#  define BITPACK_DIAGNOSTIC_POP
#endif

// here we have preprocessor looping constructs. These seem semi-standard.
// Boost::preprocessor discusses + implements more generic facilities like these.
//
// concatenate two tokens *after* expanding them
#define BITPACK_CAT(x, y)  BITPACK_CAT_(x, y)
#define BITPACK_CAT_(x, y) x##y

// call macro(i) for i in [0,n)
#define BITPACK_REPEAT(macro, n) BITPACK_CAT(BITPACK_REPEAT_, n)(macro)
#define BITPACK_REPEAT_0(macro)
// #define BITPACK_REPEAT_n+1 BITPACK_REPEAT_n BITPACK_REPEAT_n
#define BITPACK_REPEAT_1(macro) BITPACK_REPEAT_0(macro) macro(0)
#define BITPACK_REPEAT_2(macro) BITPACK_REPEAT_1(macro) macro(1)
#define BITPACK_REPEAT_3(macro) BITPACK_REPEAT_2(macro) macro(2)
#define BITPACK_REPEAT_4(macro) BITPACK_REPEAT_3(macro) macro(3)
#define BITPACK_REPEAT_5(macro) BITPACK_REPEAT_4(macro) macro(4)
#define BITPACK_REPEAT_6(macro) BITPACK_REPEAT_5(macro) macro(5)
#define BITPACK_REPEAT_7(macro) BITPACK_REPEAT_6(macro) macro(6)
#define BITPACK_REPEAT_8(macro) BITPACK_REPEAT_7(macro) macro(7)
#define BITPACK_REPEAT_9(macro) BITPACK_REPEAT_8(macro) macro(8)

// macros are not re-entrant, so to have nested repetition, we need a second copy
#define BITPACK_REPEAT_OUTER(macro, n)                                         \
  BITPACK_CAT(BITPACK_REPEAT_OUTER_, n)(macro)
#define BITPACK_REPEAT_OUTER_0(macro)
// #define BITPACK_REPEAT_OUTER_n+1 BITPACK_REPEAT_OUTER_n BITPACK_REPEAT_OUTER_n
#define BITPACK_REPEAT_OUTER_1(macro) BITPACK_REPEAT_OUTER_0(macro) macro(0)
#define BITPACK_REPEAT_OUTER_2(macro) BITPACK_REPEAT_OUTER_1(macro) macro(1)
#define BITPACK_REPEAT_OUTER_3(macro) BITPACK_REPEAT_OUTER_2(macro) macro(2)
#define BITPACK_REPEAT_OUTER_4(macro) BITPACK_REPEAT_OUTER_3(macro) macro(3)
#define BITPACK_REPEAT_OUTER_5(macro) BITPACK_REPEAT_OUTER_4(macro) macro(4)
#define BITPACK_REPEAT_OUTER_6(macro) BITPACK_REPEAT_OUTER_5(macro) macro(5)
#define BITPACK_REPEAT_OUTER_7(macro) BITPACK_REPEAT_OUTER_6(macro) macro(6)
#define BITPACK_REPEAT_OUTER_8(macro) BITPACK_REPEAT_OUTER_7(macro) macro(7)
#define BITPACK_REPEAT_OUTER_9(macro) BITPACK_REPEAT_OUTER_8(macro) macro(8)

#endif // BITPACK_MACROS_INCLUDE_GUARD
