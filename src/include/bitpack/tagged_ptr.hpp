#ifndef BITPACK_TAGGED_PTR_INCLUDE_GUARD
#define BITPACK_TAGGED_PTR_INCLUDE_GUARD

#include <bitpack/traits.hpp>
#include <bitpack/pair.hpp>

#include <cstddef>
#include <bit>
#include <algorithm>
#include <concepts>

namespace bitpack {

/**
 * Holds a pointer(`T*`) and puts a tag(`Tag`) in the low bits(the number
 * `tag_size` of lowest bits). Optionally, use `ptr_replacement_bits` to fill
 * the low bits of the pointer back in. Provide a smart pointer-- like
 * interface to get at the underlying pointer.
 *
 * Ptr = the pointer type to hold
 * Tag = the tag type to hold (probably an enum or small number)
 * tag_bits_ = the number of bits needed to store the tag
 * ptr_replacement_bits = if the low bits of the pointer aren't 0, what should
 * they be filled in with?
 */
template<class Ptr,
         class Tag,
         uintptr_t tag_bits_ = std::bit_width(alignof(traits::unptr_t<Ptr>) - 1),
         uintptr_t ptr_replacement_bits = 0u>
class tagged_ptr {
 public:
  static constexpr uintptr_t tag_bits =
      std::max<uintptr_t>(tag_bits_, 1); // can't have 0 length bitfields :C
  constexpr tagged_ptr() = default;
  /**
   * ptr = the pointer to store
   * tag = the tag to store
   */
  explicit constexpr tagged_ptr(Ptr const ptr,
                                Tag const tag) noexcept(impl::is_assert_off)
      : pair_{bits::bit_cast<uintptr_t>(ptr) >> tag_bits, tag} {
    BITPACK_ASSERT(this->tag() == tag);
    BITPACK_ASSERT(this->ptr() == ptr);
  }
  constexpr static Ptr ptr(tagged_ptr const self) noexcept {
    auto const pair = self.pair_;
    return bits::bit_cast<Ptr>((decltype(pair)::x(pair) << tag_bits)
                               | ptr_replacement_bits);
  }
  constexpr Ptr ptr() const noexcept { return ptr(*this); }
  constexpr static Tag tag(tagged_ptr const self) noexcept {
    auto const pair = self.pair_;
    return decltype(pair)::y(pair);
  }
  constexpr Tag tag() const noexcept { return tag(*this); }
  friend constexpr traits::unptr_t<Ptr>&
      operator*(tagged_ptr const self) noexcept
      requires(!std::is_void_v<traits::unptr_t<Ptr>>) {
    return *ptr(self);
  }
  constexpr Ptr operator->() const noexcept { return ptr(); }
  /**
   * returns the pointer stored
   */
  constexpr Ptr get() const noexcept { return ptr(); }

  friend constexpr bool operator==(tagged_ptr const p, std::nullptr_t) {
    return ptr(p) == nullptr;
  }
  friend constexpr bool operator==(std::nullptr_t, tagged_ptr const p) {
    return p == nullptr;
  }
  constexpr operator bool() { return *this == nullptr; }

 private:
  uintptr_pair<uintptr_t, Tag, tag_bits> pair_;
};
} // namespace bitpack
#endif // BITPACK_TAGGED_PTR_INCLUDE_GUARD
