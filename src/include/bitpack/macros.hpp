#ifndef BITPACK_MACROS_INCLUDE_GUARD
#define BITPACK_MACROS_INCLUDE_GUARD

// to check msvc, _MSC_VER
// to force inline on msvc, __forceinline
// I will not write any msvc specific code until I have a way to test it set up
#if defined(__GNUC__)
#  define BITPACK_FORCEINLINE __attribute__((always_inline))
#else
#  define BITPACK_FORCEINLINE
#endif

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
#    define BITPACK_ASSERT(...) [] {}()
#  endif
#endif

#define BITPACK_FWD(x) std::forward<decltype(x)>(x)

#define BITPACK_EXPR_BODY(...)                                                 \
  noexcept(noexcept(__VA_ARGS__))->decltype(__VA_ARGS__) { return __VA_ARGS__; }

namespace bitpack { namespace impl {
bool constexpr is_assert_off = !BITPACK_ENABLE_ASSERT;
}} // namespace bitpack::impl

#endif
