#ifndef BITPACK_INCLUDE_GUARD
#define BITPACK_INCLUDE_GUARD

#include <utility>
#include <array>
#include <cstring>
#include <cstdint>
#include <bit>
#include <tuple>
#include <assert.h>

#ifdef __GNUC__
#  define BITPACK_FORCEINLINE __attribute__((always_inline))
#else
#  define BITPACK_FORCEINLINE
#endif

#ifndef BITPACK_ASSERT
#  define BITPACK_ASSERT(...) assert(__VA_ARGS__)
#endif
#ifndef BITPACK_ENABLE_SLOW_ASSERT
#  define BITPACK_ENABLE_SLOW_ASSERT false
#endif
#ifndef BITPACK_SLOW_ASSERT
#  if BITPACK_ENABLE_SLOW_ASSERT
#    define BITPACK_SLOW_ASSERT(...) BITPACK_ASSERT(__VA_ARGS__)
#  else
#    define BITPACK_SLOW_ASSERT(...)                                           \
      do {                                                                     \
      } while(false)
#  endif
#endif

namespace bitpack {
namespace impl {
// from Guidelines Support Library
// isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#gslutil-utilities
template<class T>
inline constexpr T
    narrow_cast(auto const x) noexcept(!BITPACK_ENABLE_SLOW_ASSERT) {
  T casted = static_cast<T>(x);
  BITPACK_SLOW_ASSERT(static_cast<decltype(x)>(casted) == x);
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
  for(auto i = 0u; i < bytes.size(); ++i) {
    auto const this_byte = to_integer<UInt>(bytes[i]);
    acc |= (this_byte << (i * 8u));
  }
  return acc;
}

/**
 * Unpack the underlying bits in a `UInt` back to `To`
 */
template<class To, class UInt>
inline constexpr auto from_UInt(UInt const from) noexcept {
  std::array<std::byte, sizeof(To)> bytes;
  for(auto i = 0u; i < bytes.size(); ++i)
    bytes[i] = narrow_cast<std::byte>((from >> (i * 8u)) & 0xFFu);
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

/**
 * A pair packed into a specified UInt type.
 */
template<class X, class Y, class UInt, int low_bit_count_ = sizeof(Y) * 8>
class UInt_pair {
  static_assert(sizeof(X) <= sizeof(UInt));
  static_assert(sizeof(Y) <= sizeof(UInt));

 public:
  static constexpr auto low_bit_count = low_bit_count_;
  static constexpr auto high_bit_count = sizeof(UInt) * 8 - low_bit_count;
  constexpr UInt_pair() = default;
  explicit constexpr UInt_pair(X const x,
                               Y const y) noexcept(!BITPACK_ENABLE_SLOW_ASSERT)
      : x_{impl::as_UInt<UInt>(x)}, y_{impl::as_UInt<UInt>(y)} {
    BITPACK_SLOW_ASSERT(this->x() == x);
    BITPACK_SLOW_ASSERT(this->y() == y);
  }

  constexpr X x() const noexcept { return impl::from_UInt<X>(x_); }
  constexpr Y y() const noexcept { return impl::from_UInt<Y>(y_); }

  template<int i>
  friend constexpr auto get(UInt_pair const pair) noexcept {
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

/**
 * Holds a pointer(`T*`) and puts a tag(`Tag`) in the low bits(the number
 * `tag_size` of lowest bits). Optionally, use `ptr_replacement_bits` to fill
 * the low bits of the pointer back in. Provide a smart pointer-- like
 * interface to get at the underlying pointer.
 */
template<class T,
         class Tag,
         uintptr_t tag_bits_ = std::bit_width(alignof(T) - 1),
         uintptr_t ptr_replacement_bits = 0u>
class tagged_ptr {
 public:
  static constexpr uintptr_t tag_bits =
      std::max<uintptr_t>(tag_bits_, 1); // can't have 0 length bitfields :C
  constexpr tagged_ptr() = default;
  explicit constexpr tagged_ptr(T* const ptr, Tag const tag) noexcept(
      !BITPACK_ENABLE_SLOW_ASSERT)
      : pair_{impl::bit_cast<uintptr_t>(ptr) >> tag_bits, tag} {
    BITPACK_SLOW_ASSERT(this->tag() == tag);
    BITPACK_SLOW_ASSERT(this->ptr() == ptr);
  }
  constexpr T* ptr() const noexcept {
    return impl::bit_cast<T*>((pair_.x() << tag_bits) | ptr_replacement_bits);
  }
  constexpr Tag tag() const noexcept { return pair_.y(); }
  constexpr auto& operator*() const noexcept { return *ptr(); }
  constexpr auto operator->() const noexcept { return ptr(); }
  constexpr auto get() const noexcept { return ptr(); }

  friend constexpr bool operator==(tagged_ptr const p, std::nullptr_t) {
    return p.get() == nullptr;
  }
  friend constexpr bool operator==(std::nullptr_t, tagged_ptr const p) {
    return p == nullptr;
  }
  constexpr operator bool() { return *this == nullptr; }

 private:
  uintptr_pair<uintptr_t, Tag, tag_bits> pair_;
};

namespace impl {

/**
 * This implements typelists and related functionality as needed for
 * variant_ptr
 */
template<class T>
using ensure_pointer_t = std::conditional_t<std::is_pointer_v<T>, T, T*>;
template<class... Ts>
struct typelist {
 private:
  static_assert((... && !std::is_reference_v<Ts>),
                "You can't put references in a variant_ptr");
  using type_tuple = std::tuple<ensure_pointer_t<Ts>...>;

 public:
  static constexpr unsigned size = sizeof...(Ts);

  template<int N>
  using nth = std::tuple_element_t<N, type_tuple>;

 private:
  template<class T, int n>
  static constexpr bool is_T_nth = std::is_same_v<ensure_pointer_t<T>, nth<n>>;

  template<class T, int N>
  static constexpr int find_looper() noexcept {
    if constexpr(N >= size) return N;
    // separate case to prevent istantiation with big N. The user gets our
    // error instead of std::tuple's
    else if(is_T_nth<T, N>)
      return N;
    else
      return find_looper<T, N + 1>();
  }

 public:
  template<class T>
  static constexpr unsigned unguarded_find = find_looper<T, 0>();

  template<class T>
  static constexpr bool has = (unguarded_find<T> < size);

  template<class T>
  static constexpr unsigned find = [] {
    static_assert(has<T>, "Type not in variant");
    return unguarded_find<T>;
  }();
};
} // namespace impl

template<class... Ts>
class variant_ptr {
  using types = impl::typelist<Ts...>;
  static constexpr auto tag_bits = std::bit_width(types::size - 1);

 public:
  static constexpr auto size = types::size;
  using Tag = int;
  constexpr Tag index() const noexcept { return ptr_.tag(); }

  constexpr variant_ptr() = default;
  template<class T>
  explicit(alignof(T) < tag_bits) // If the alignment is small, YOU have to
                                  // guarantee the pointer's low bits are empty
      constexpr variant_ptr(T* ptr) noexcept(!BITPACK_ENABLE_SLOW_ASSERT)
      : ptr_{ptr, types::template find<T>} {}

  template<class Func>
  constexpr friend auto visit(variant_ptr const self, Func visitor) {
    return visit_nth<0>(self, visitor, self.index());
  }

  template<Tag N>
  constexpr friend auto
      get(variant_ptr const self) noexcept(!BITPACK_ENABLE_SLOW_ASSERT) {
    static_assert(0 <= N && N < size, "Variant index out of bounds");
    using T = typename types::template nth<N>;
    return get<T>(self);
  }
  template<class T>
  constexpr friend auto
      get(variant_ptr const self) noexcept(!BITPACK_ENABLE_SLOW_ASSERT) {
    static_assert(types::template has<T>, "That type is not in this variant");
    BITPACK_SLOW_ASSERT(holds_alternative<T>(self));
    return static_cast<impl::ensure_pointer_t<T>>(void_star(self));
  }
  template<int N>
  constexpr friend auto
      get_if(variant_ptr const self) noexcept(!BITPACK_ENABLE_SLOW_ASSERT) {
    return (self.index() == N) ? get<N>(self) : nullptr;
  }
  template<class T>
  constexpr friend auto
      get_if(variant_ptr const self) noexcept(!BITPACK_ENABLE_SLOW_ASSERT) {
    return get_if<types::template find<T>>(self);
  }

  template<class T>
  constexpr friend bool holds_alternative(variant_ptr const self) noexcept {
    return self.index() == types::template find<T>;
  }

  constexpr friend bool operator==(variant_ptr const p, std::nullptr_t) {
    return p.ptr_ == nullptr;
  }
  constexpr friend bool operator==(std::nullptr_t, variant_ptr const p) {
    return p == nullptr;
  }
  constexpr operator bool() { return *this == nullptr; }

 private:
  constexpr friend void* void_star(variant_ptr const self) noexcept {
    return self.ptr_.get();
  }

  template<auto N, class Func>
  friend auto BITPACK_FORCEINLINE // help the compiler convert it to a switch?
      visit_nth(variant_ptr const self, Func visitor, Tag const tag) {
    if constexpr(N >= size) {
      BITPACK_ASSERT(N < size); // should this be changed to SLOW_ASSERT?
      // just need the type. This is out of contract
      return decltype(visitor(get<0>(self))){};
    } else {
      if(tag == N)
        return visitor(get<N>(self));
      else
        return visit_nth<N + 1>(self, visitor, tag);
    }
  }
  tagged_ptr<void, Tag, tag_bits> ptr_;
};

// possible future direction: derived_variant_ptr. Put the rtti into the
// pointers
} // namespace bitpack

#endif // BITPACK_INCLUDE_GUARD
