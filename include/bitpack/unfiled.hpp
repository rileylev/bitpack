#ifndef __UNFILED_H_
#define __UNFILED_H_

template<size_t I, class... Args>
struct variant_alternative<I, bitpack::variant_ptr<Args...>> {
  using type = std::optional<std::remove_reference_t<decltype(get<n>(variant))>>
};

#endif // __UNFILED_H_
