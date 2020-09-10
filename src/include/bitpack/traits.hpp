#ifndef BITPACK_TRAITS_INCLUDE_GUARD
#define BITPACK_TRAITS_INCLUDE_GUARD

#include <concepts>

namespace bitpack { namespace traits {
namespace impl {
template<class T> concept Derefable = requires(T ptr) { *ptr; };
void unptr_dispatch(void*);
auto unptr_dispatch(Derefable auto ptr) { return *ptr; }
} // namespace impl

// unptr dereferences or converts void* -> void
// not called deref_t because dereferencing void is undefined
// not std::remove_pointer_t because what if we want to store smart pointers? idk
template<class T>
using unptr_t = decltype(impl::unptr_dispatch(std::declval<T>()));
}} // namespace bitpack::traits

#endif // BITPACK_TRAITS_INCLUDE_GUARD
