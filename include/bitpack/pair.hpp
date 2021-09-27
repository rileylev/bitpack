#ifndef BITPACK_PAIR_INCLUDE_GUARD
#define BITPACK_PAIR_INCLUDE_GUARD

#include "bits.hpp"
#include "workaround.hpp"

#include <type_traits>

namespace bitpack {
/**
 * A pair packed into a specified UInt type.
 * X = the type on the "left"
 * Y = the type on the "right"
 * UInt = the unsigned int type to stuff the pair into
 * low_bit_count_ = how many bits of the Y value do we store?
 */
template<class X,
         class Y,
         std::unsigned_integral UInt,
         size_t                 low_bit_count_ = bits::bit_sizeof<Y>>
class UInt_pair {
 public:
  static constexpr auto low_bit_count  = low_bit_count_;
  static constexpr auto high_bit_count = sizeof(UInt) * 8 - low_bit_count;

 private:
  UInt y_ : low_bit_count; // little endian : low = lsb = first(lowest address)
  UInt x_ : high_bit_count;

  template<int i> using nth_t = std::conditional_t<i == 0, X, Y>;

 public:
  constexpr UInt_pair() = default;

  /**
   * x = the "left" value
   * y = the "right" value
   */
  explicit constexpr UInt_pair(X const x,
                               Y const y) noexcept(impl::is_assert_off)
      : y_{bits::as_UInt<UInt>(y)}, x_{bits::as_UInt<UInt>(x)} {
    // postcondition
    BITPACK_ASSERT(this->x() == x);
    BITPACK_ASSERT(this->y() == y);
  }

  constexpr static X x(const UInt_pair self) noexcept {
    return bits::from_UInt<X>(self.x_);
  }
  constexpr static Y y(const UInt_pair self) noexcept {
    return bits::from_UInt<Y>(self.y_);
  }
  constexpr X x() const noexcept { return x(*this); }
  constexpr Y y() const noexcept { return y(*this); }

  /**
   * Return the i-th element of pair (i= 0 or 1). Read-only.
   */
  template<auto i>
  static constexpr nth_t<i> get(UInt_pair const pair) noexcept
      requires(i == 0 || i == 1) {
    if constexpr(i == 0)
      return x(pair);
    else if(i == 1)
      return y(pair);
  }

  /**
   * Return the element of type T. Warning: not a regular function.
   */
  template<class T> static constexpr T get(UInt_pair const pair) noexcept {
    constexpr bool isX = std::is_same_v<T, X>;
    constexpr bool isY = std::is_same_v<T, Y>;
    static_assert(isX || isY, "That type is not in this pair.");
    if constexpr(isX)
      return x(pair);
    else if(isY)
      return y(pair);
  }

  // it's annoying to spell out the exact type
  friend std::pair<X, Y> to_std_pair(UInt_pair const self) noexcept {
    return std::pair(x(self), y(self));
  }
  explicit operator std::pair<X, Y>() const noexcept {
    return to_std_pair(*this);
  }
  // explicit because silently converting to std::pair can result in trying to
  // grab pointers to a temporary (pr value)
  //
  // consider the following
  // get_if has no overload that takes UInt_pair
  // Imagine we had implicit conversion.
  // If user mistakenly calls it on UInt_pair, it implicitly converts to
  // std::pair<X,Y> (a temporary) and then get_if returns a pointer to the value
  // the temporary held. This is never what the user intended.
};

// wrap a binary operator on pair by deferring to std::pair's version
#define BITPACK_DEF_COMPARE(op)                                                \
  template<class A0,                                                           \
           class A1,                                                           \
           class AUint,                                                        \
           auto Anum,                                                          \
           class B0,                                                           \
           class B1,                                                           \
           class BUint,                                                        \
           auto Bnum>                                                          \
  inline auto operator op(const UInt_pair<A0, A1, AUint, Anum>& a,             \
                          const UInt_pair<B0, B1, BUint, Bnum>& b)             \
      BITPACK_EXPR_BODY(to_std_pair(a) op to_std_pair(b));

// these defer to std::pair's relations
// element-wise equality
BITPACK_DEF_COMPARE(==)
// lexicographic comparison
BITPACK_DEF_COMPARE(<=>)
#undef BITPACK_DEF_COMPARE

template<class X, class Y, size_t low_bit_count = bits::bit_sizeof<Y>>
using uintptr_pair = UInt_pair<X, Y, uintptr_t, low_bit_count>;
template<class X, class Y, size_t low_bit_count = bits::bit_sizeof<Y>>
inline constexpr auto make_uintptr_pair(X x, Y y)
    BITPACK_EXPR_BODY(uintptr_pair<X, Y, low_bit_count>(x, y));
template<int N>
inline constexpr auto make_uintptr_pair(auto x, auto y)
    BITPACK_EXPR_BODY(make_uintptr_pair<decltype(x), decltype(y), N>(x, y));

} // namespace bitpack

#endif // BITPACK_PAIR_INCLUDE_GUARD
