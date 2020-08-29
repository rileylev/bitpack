#ifndef BITPACK_PAIR_INCLUDE_GUARD
#define BITPACK_PAIR_INCLUDE_GUARD

#include <bitpack/bits.hpp>
#include <bitpack/workaround.hpp>

namespace bitpack{
/**
 * A pair packed into a specified UInt type.
 */
template<class X, class Y, class UInt, int low_bit_count_ = sizeof(Y) * 8>
class UInt_pair {
  // I have to think about this limitation more
  // is making a small encoding class the right way to put "big" types in here?
  static_assert(sizeof(X) <= sizeof(UInt));
  static_assert(sizeof(Y) <= sizeof(UInt));

 public:
  static constexpr auto low_bit_count = low_bit_count_;
  static constexpr auto high_bit_count = sizeof(UInt) * 8 - low_bit_count;
  constexpr UInt_pair() = default;
  explicit constexpr UInt_pair(X const x, Y const y) //
      noexcept(impl::is_assert_off)
      : x_{bits::as_UInt<UInt>(x)}, y_{bits::as_UInt<UInt>(y)} {
    // postcondition
    BITPACK_ASSERT(this->x() == x);
    BITPACK_ASSERT(this->y() == y);
  }

  constexpr X x() const noexcept { return bits::from_UInt<X>(x_); }
  constexpr Y y() const noexcept { return bits::from_UInt<Y>(y_); }

  template<int i>
  static constexpr auto get(UInt_pair const pair) noexcept {
    static_assert(i == 0 || i == 1);
    if constexpr(i == 0)
      return pair.x();
    else if(i == 1)
      return pair.y();
  }

 private:
  UInt y_ : low_bit_count; // little endian : low = lsb = first(lowest address)
  UInt x_ : high_bit_count;
};

template<class X, class Y, int low_bit_count = sizeof(Y) * 8>
using uintptr_pair = UInt_pair<X, Y, uintptr_t, low_bit_count>;
template<class X, class Y, int low_bit_count = sizeof(Y) * 8>
auto make_uintptr_pair(X x, Y y) {
  return uintptr_pair<X, Y, low_bit_count>(x, y);
}
template<int N>
auto make_uintptr_pair(auto x, auto y) {
  return make_uintptr_pair<decltype(x), decltype(y), N>(x, y);
}
}

#endif // BITPACK_PAIR_INCLUDE_GUARD
