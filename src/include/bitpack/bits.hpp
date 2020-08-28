#ifndef BITPACK_BITS_INCLUDE_GUARD
#define BITPACK_BITS_INCLUDE_GUARD

#include <array>
#include <cstring>
#include <bit>

#include <bitpack/macros.hpp>

namespace bitpack{
namespace impl {

// from Guidelines Support Library
// isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#gslutil-utilities
template<class T>
inline constexpr T narrow(auto const x) //
    noexcept(impl::is_assert_off) {
  T casted = static_cast<T>(x);
  BITPACK_ASSERT(static_cast<decltype(x)>(casted) == x);
  return casted;
}

// polyfill: i don't have std::bit_cast yet
template<class To, class From>
inline constexpr To bit_cast(From const x) noexcept {
  static_assert(sizeof(To) == sizeof(From));
  To temp;
  std::memcpy(&temp, &x, sizeof(To));
  return temp;
}

/**
 * Given an object of type T, return a std::array of (a copy of) the underlying
 * bytes.
 */
template<class T>
inline constexpr auto bytes_of(T const x) noexcept {
  return bit_cast<std::array<std::byte, sizeof(x)>>(x);
}

// polyfill: I don't have std::to_integer yet
template<class T>
inline constexpr T to_integer(auto const x) noexcept {
  return static_cast<T>(x);
}

/**
 * Given a `T`, return (a copy of) its underlying bytes as a `UInt`
 */
template<class UInt, class T>
inline constexpr auto as_UInt(T const x) noexcept {
  auto const bytes = bytes_of(x);
  UInt acc{};
  // The inliner should see through this for little endian.
  for(auto i = 0u; i < bytes.size(); ++i) {
    auto const this_byte = to_integer<UInt>(bytes[i]);
    acc |= (this_byte << (i * 8u));
  }
  return acc;
}

/**
 * Unpack the underlying bits in a `UInt` back to `To`
 */
template<class To, class From>
inline constexpr auto from_UInt(From const from) noexcept {
  std::array<std::byte, sizeof(To)> bytes;
  // The inliner should see through this for little endian?
  for(auto i = 0u; i < bytes.size(); ++i)
    bytes[i] = narrow<std::byte>((from >> (i * 8u)) & 0xFFu);
  return bit_cast<To>(bytes);
}

inline constexpr auto as_uintptr_t(auto const x) noexcept {
  return as_UInt<uintptr_t>(x);
}

template<class To>
inline constexpr auto from_uintptr_t(uintptr_t const x) noexcept {
  return from_UInt<To>(x);
}

} // namespace impl
}

#endif
