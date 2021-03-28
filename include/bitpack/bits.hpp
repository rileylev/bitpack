#ifndef BITPACK_BITS_INCLUDE_GUARD
#define BITPACK_BITS_INCLUDE_GUARD

#include <bitpack/macros.hpp>

#include <climits>
#include <array>
#include <cstring>
#include <bit>
#include <concepts>

namespace bitpack { namespace bits {

template<class T> constexpr auto bit_sizeof = sizeof(T) * CHAR_BIT;

// from Guidelines Support Library
// isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#gslutil-utilities
template<class T>
inline constexpr T narrow(auto const x) noexcept(impl::is_assert_off) {
  T casted = static_cast<T>(x);
  BITPACK_ASSERT(static_cast<decltype(x)>(casted) == x);
  return casted;
}

#if true
// polyfill: i don't have std::bit_cast yet
// unfortunately, this isn't truly constexpr until I get std::bit_cast
template<class To, class From>
inline constexpr To bit_cast(From const z) noexcept {
  static_assert(sizeof(To) == sizeof(From));
#  if defined(__cpp_lib_bit_cast)
  return std::bit_cast<To>(z);
#  elif defined(__has_builtin) && __has_builtin(__builtin_bit_cast)
  return __builtin_bit_cast(To, z);
#  else
  To temp;
  std::memcpy(&temp, &z, sizeof(To));
  return temp;
#  endif
}
#endif

/**
 * Given an object of type T, return a std::array of (a copy of) the underlying
 * bytes.
 */
template<class T> inline constexpr auto bytes_of(T const x) noexcept {
  return bit_cast<std::array<std::byte, sizeof(x)>>(x);
}

// polyfill: I don't have std::to_integer yet
template<std::integral T> inline constexpr T to_integer(auto const x) noexcept {
  return static_cast<T>(x);
}

/**
 * Given a `T`, return (a copy of) its underlying bytes as a `UInt`
 */
// ENDIAN??????
template<std::unsigned_integral UInt,
         class T,
         std::endian endian = std::endian::native>
inline constexpr UInt as_UInt(T const x) noexcept {
  static_assert(endian == std::endian::little || endian == std::endian::big);
  auto const bytes = bytes_of(x);
  UInt       acc{};
  auto const size = bytes.size();
  for(auto i = 0u; i < size; ++i) {
    auto const lookup_idx =
        (endian == std::endian::little) ? i : (size - i - 1);
    auto const this_byte = to_integer<UInt>(bytes[lookup_idx]);
    acc |= (this_byte << (i * 8u));
  }
  return acc;
}

/**
 * Unpack the underlying bits in a `UInt` back to `To`
 */
template<class To,
         std::unsigned_integral From,
         std::endian            endian = std::endian::native>
inline constexpr auto from_UInt(From const from) noexcept {
  std::array<std::byte, sizeof(To)> bytes;
  auto const                        size = bytes.size();
  for(auto i = 0u; i < size; ++i) {
    auto const byte_idx = (endian == std::endian::little) ? i : (size - i - 1);
    bytes[byte_idx]     = narrow<std::byte>((from >> (i * 8u)) & 0xFFu);
  }
  return bit_cast<To>(bytes);
}

inline constexpr auto as_uintptr_t(auto const x) noexcept {
  return as_UInt<std::uintptr_t>(x);
}

template<class To>
inline constexpr auto from_uintptr_t(uintptr_t const x) noexcept {
  return from_UInt<To>(x);
}

}} // namespace bitpack::bits

#endif
