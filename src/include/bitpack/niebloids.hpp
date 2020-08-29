#ifndef BITPACK_NIEBLOIDS_INCLUDE_GUARD
#define BITPACK_NIEBLOIDS_INCLUDE_GUARD

#include <bitpack/macros.hpp>

#include <variant>

namespace bitpack {
#define BITPACK_FWD(x) std::forward<decltype(x)>(x)
#include <variant>
namespace niebloids {
#define BITPACK_NIEBLOID_(template_type, name, fn_name, struct_name)           \
  namespace impl {                                                             \
  using std::fn_name;                                                          \
  template<template_type T>                                                    \
  struct struct_name {                                                         \
    template<class... Args>                                                    \
    constexpr decltype(auto) operator()(Args&&... args)                        \
        BITPACK_NOEXCEPT_WRAP(fn_name<T>(BITPACK_FWD(args)...));               \
  };                                                                           \
  }                                                                            \
  template<template_type T>                                                    \
  constexpr auto name = impl::struct_name<T>{};
#define BITPACK_NIEBLOID(template_type, name, fn_name)                         \
  BITPACK_NIEBLOID_(template_type, name, fn_name, name##struct)
BITPACK_NIEBLOID(class, get, get)
BITPACK_NIEBLOID(auto, get_n, get)
BITPACK_NIEBLOID(class, get_if, get_if)
BITPACK_NIEBLOID(auto, get_if_n, get_if)
BITPACK_NIEBLOID(class, holds_alternative, holds_alternative)

namespace impl {
using std::visit;
struct visit_struct {
  template<class... Args>
  constexpr decltype(auto) operator()(Args&&... args)
      BITPACK_NOEXCEPT_WRAP(visit(BITPACK_FWD(args)...));
};
} // namespace impl
[[maybe_unused]] constexpr impl::visit_struct visit{};
} // namespace niebloids
} // namespace bitpack

#endif
