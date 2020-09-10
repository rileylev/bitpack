#ifndef MAYBE_GET_INCLUDE_GUARD
#define MAYBE_GET_INCLUDE_GUARD

#include <bitpack/macros.hpp>

// we can't implement get_if for variant_ptr because we don't store objects in a
// way we can return a pointer to them. So instead, we implement a get that
// returns an optional
#include <optional>
namespace bitpack {
template<class T>
constexpr auto maybe_get(auto const variant)
    BITPACK_EXPR_BODY((holds_alternative<T>(variant))
                          ? std::optional{get<T>(variant)}
                          : std::nullopt);

template<auto n>
constexpr auto maybe_get(auto const variant)
    BITPACK_EXPR_BODY((variant.index() == n) ? std::optional{get<n>(variant)}
                                             : std::nullopt);
} // namespace bitpack

#endif // MAYBE_GET_INCLUDE_GUARD
