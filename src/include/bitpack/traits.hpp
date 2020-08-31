#ifndef BITPACK_TRAITS_INCLUDE_GUARD
#define BITPACK_TRAITS_INCLUDE_GUARD

#include <type_traits>
namespace bitpack { namespace traits {
template<class T>
using deref_t = decltype(*std::declval<T>());
}} // namespace bitpack::traits

#endif // BITPACK_TRAITS_INCLUDE_GUARD
