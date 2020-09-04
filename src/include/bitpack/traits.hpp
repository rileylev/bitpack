#ifndef BITPACK_TRAITS_INCLUDE_GUARD
#define BITPACK_TRAITS_INCLUDE_GUARD

#include <type_traits>
namespace bitpack { namespace traits {
// dereference of void* is undefined, but lets just say it's void since that is
// close enough. maybe unpointer is a more accurate name?
template<class T>
using deref_t = std::conditional_t<std::is_same_v<std::decay_t<T>, void*>,
                                   void,
                                   decltype(*std::declval<T>())>;
}} // namespace bitpack::traits

#endif // BITPACK_TRAITS_INCLUDE_GUARD
