#ifndef BITPACK_PAIR_INCLUDE_GUARD
#define BITPACK_PAIR_INCLUDE_GUARD

#include <bitpack/bits.hpp>
#include <bitpack/workaround.hpp>

#include <type_traits>

namespace bitpack {
/**
 * A pair packed into a specified UInt type.
 */
template<class X, class Y, class UInt, int low_bit_count_ = bits::bit_sizeof<Y>>
class UInt_pair {
  static_assert(std::is_unsigned_v<UInt>);

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
    static_assert(i == 0 || i == 1, "That index is out of bounds.");
    if constexpr(i == 0)
      return pair.x();
    else if(i == 1)
      return pair.y();
  }

  template<class T>
  static constexpr auto get(UInt_pair const pair) noexcept {
    constexpr bool isX = std::is_same_v<T, X>;
    constexpr bool isY = std::is_same_v<T, Y>;
    static_assert(isX || isY, "That is not a type in this pair.");
    if constexpr(isX)
      return pair.x();
    else if(isY)
      return pair.y();
  }

  friend auto to_std_pair(UInt_pair const self) noexcept {
    return std::pair(self.x(), self.y());
  }
  explicit operator std::pair<X, Y>() const { return to_std_pair(*this); }
  // explicit because silently converting to std::pair can result in trying to
  // grab pointers to a temporary (pr value)
  //
  // consider the following
  // get_if has no overload that takes UInt_pair
  // Imagine we had implicit conversion.
  // If user mistakenly calls it on UInt_pair, it implicitly converts to
  // std::pair<X,Y> (a temporary) and then get_if returns a pointer to the value
  // the temporary held. This is never what the user intended.

 private:
  UInt y_ : low_bit_count; // little endian : low = lsb = first(lowest address)
  UInt x_ : high_bit_count;
};

#define BITPACK_DEF_COMPARE(op)                                                \
  template<class A0,                                                           \
           class A1,                                                           \
           class AUint,                                                        \
           auto Anum,                                                          \
           class B0,                                                           \
           class B1,                                                           \
           class BUint,                                                        \
           auto Bnum>                                                          \
  auto operator op(const UInt_pair<A0, A1, AUint, Anum>& a,                    \
                   const UInt_pair<B0, B1, BUint, Bnum>& b) {                  \
    return to_std_pair(a) op to_std_pair(b);                                   \
  }
BITPACK_DEF_COMPARE(==)
BITPACK_DEF_COMPARE(<=>)

template<class X, class Y, int low_bit_count = bits::bit_sizeof<Y>>
using uintptr_pair = UInt_pair<X, Y, uintptr_t, low_bit_count>;
template<class X, class Y, int low_bit_count = bits::bit_sizeof<Y>>
constexpr auto make_uintptr_pair(X x, Y y)
    BITPACK_NOEXCEPT_WRAP(uintptr_pair<X, Y, low_bit_count>(x, y));
template<int N>
constexpr auto make_uintptr_pair(auto x, auto y)
    BITPACK_NOEXCEPT_WRAP(make_uintptr_pair<decltype(x), decltype(y), N>(x, y));

} // namespace bitpack

#endif // BITPACK_PAIR_INCLUDE_GUARD
