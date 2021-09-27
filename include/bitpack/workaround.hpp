#ifndef BITPACK_WORKAROUND_INCLUDE_GUARD
#define BITPACK_WORKAROUND_INCLUDE_GUARD

// gcc had trouble with hidden friend templates so I made them static and wrote
// free/namespace level wrappers to approximate that same interface
#include "macros.hpp"
#define BITPACK_WRAP_STATIC(template_type, name)                               \
  template<template_type X>                                                    \
  inline constexpr auto name(auto const self)                                  \
      BITPACK_EXPR_BODY(decltype(self)::template name<X>(self))

namespace bitpack {
BITPACK_WRAP_STATIC(class, get)
BITPACK_WRAP_STATIC(auto, get)
BITPACK_WRAP_STATIC(class, holds_alternative)

template<class R>
inline constexpr auto visit(auto visitor, auto const self)
    BITPACK_EXPR_BODY(decltype(self)::template visit<R>(visitor, self))
inline constexpr auto visit(auto visitor, auto const self)
    BITPACK_EXPR_BODY(decltype(self)::visit(visitor, self));

} // namespace bitpack

#endif // BITPACK_WORKAROUND_INCLUDE_GUARD
