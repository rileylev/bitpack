#ifndef BITPACK_MACROS_INCLUDE_GUARD
#define BITPACK_MACROS_INCLUDE_GUARD

#if defined(__GNUC__)
#  define BITPACK_FORCEINLINE __attribute__((always_inline))
#else
#  define BITPACK_FORCEINLINE
#endif

#if defined(BITPACK_ASSERT)
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

#define BITPACK_NOEXCEPT_WRAP(...)                                             \
  noexcept(noexcept(__VA_ARGS__)) { return __VA_ARGS__; }
#define BITPACK_WRAP_STATIC(template_type, name)                               \
  template<template_type X>                                                    \
  inline constexpr auto name(auto const self)                                  \
      BITPACK_NOEXCEPT_WRAP(decltype(self)::template name<X>(self))

namespace bitpack { namespace impl {
bool constexpr is_assert_off = !BITPACK_ENABLE_ASSERT;
}} // namespace bitpack::impl

#endif
