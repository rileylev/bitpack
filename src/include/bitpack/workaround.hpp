#ifndef BITPACK_WORKAROUND_INCLUDE_GUARD
#define BITPACK_WORKAROUND_INCLUDE_GUARD

#include<bitpack/macros.hpp>
#define BITPACK_WRAP_STATIC(template_type, name)                               \
  template<template_type X>                                                    \
  inline constexpr auto name(auto const self)                                  \
      BITPACK_NOEXCEPT_WRAP(decltype(self)::template name<X>(self))

namespace bitpack{
BITPACK_WRAP_STATIC(class, get)
BITPACK_WRAP_STATIC(auto, get)
BITPACK_WRAP_STATIC(class, get_if)
BITPACK_WRAP_STATIC(auto, get_if)
BITPACK_WRAP_STATIC(class, holds_alternative)
}

#endif // BITPACK_WORKAROUND_INCLUDE_GUARD
