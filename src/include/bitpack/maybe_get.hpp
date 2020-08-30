#ifndef MAYBE_GET_INCLUDE_GUARD
#define MAYBE_GET_INCLUDE_GUARD

// we can't implement get_if for variant_ptr because we don't store objects in a
// way we can return a pointer to them. So instead, we implement a get that
// returns an optional
#include <optional>
namespace bitpack {
template<class T>
constexpr auto maybe_get(auto variant) -> std::optional<T> {
  if(holds_alternative<T>(variant))
    return std::optional{get<T>(variant)};
  else
    return std::nullopt;
}

template<auto n>
constexpr auto maybe_get(auto variant)
    -> std::optional<std::remove_reference_t<decltype(get<n>(variant))>> {
  if(variant.index() == n)
    return std::optional{get<n>(variant)};
  else
    return std::nullopt;
}
} // namespace bitpack

#endif // MAYBE_GET_INCLUDE_GUARD
