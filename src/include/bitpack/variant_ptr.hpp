#ifndef BITPACK_VARIANT_PTR_INCLUDE_GUARD
#define BITPACK_VARIANT_PTR_INCLUDE_GUARD

#include <bitpack/macros.hpp>
#include <bitpack/traits.hpp>
#include <bitpack/tagged_ptr.hpp>
#include <bitpack/workaround.hpp>

#include <tuple>

namespace bitpack {
namespace impl {

template<class T>
constexpr T garbage_value() noexcept {
  // if this ever needs to handle a type with no default constructor, I think
  // this works. But disabling warnings is a pain

  // return *static_cast<T*>(nullptr);
  return {};
}
/**
 * This implements typelists and related functionality as needed for
 * variant_ptr
 */
template<class... Ts>
struct typelist {
 private:
  static_assert((... && !std::is_reference_v<Ts>),
                "You can't put references in a variant_ptr");
  using type_tuple = std::tuple<Ts...>;

 public:
  static constexpr unsigned size = sizeof...(Ts);

  template<int N>
  using nth = std::tuple_element_t<N, type_tuple>;

 private:
  template<class T, int n>
  static constexpr bool is_T_nth = std::is_same_v<T, nth<n>>;

  template<class T, int N>
  static constexpr int find_looper() noexcept {
    if constexpr(N >= size) return N;
    // separate case to prevent instantiating is_T_nth with big N. The user gets
    // our error instead of std::tuple's
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

/**
 * Use this to check if calling func on any of the types in a typelist can
 * throw an exception (and if asserts are off)
 */
template<class variant_ptr, class func, class seq>
struct is_visit_noexcept_by_seq;

template<class variant_ptr, class func, auto... i>
struct is_visit_noexcept_by_seq<variant_ptr, func, std::index_sequence<i...>> {
  static constexpr bool value =
      impl::is_assert_off
      && (noexcept(std::declval<func>()(get<i>(std::declval<variant_ptr>())))
          && ...);
};

template<class variant_ptr, class Func, class seq>
struct visit_common_type_by_seq;

template<class variant_ptr, class Func, auto... i>
struct visit_common_type_by_seq<variant_ptr, Func, std::index_sequence<i...>> {
  using type = std::common_type_t<decltype(
      std::declval<Func>()(get<i>(std::declval<variant_ptr>())))...>;
};

} // namespace impl

// for now, we force the Ts to be raw pointers, but they could be something else
template<class... Ts>
class variant_ptr {
  using types = impl::typelist<Ts...>;
  static constexpr auto tag_bits = std::bit_width(types::size - 1);

 public:
  static constexpr auto size = types::size;
  using Tag = int;
  static constexpr Tag index(variant_ptr const self) noexcept {
    auto const ptr = self.ptr_;
    return decltype(ptr)::tag(ptr);
  }
  constexpr Tag index() const noexcept { return decltype(ptr_)::tag(ptr_); }

  constexpr variant_ptr() = default;
  template<class T>
  explicit(alignof(traits::unptr_t<T>) <= tag_bits)
      // If alignment<= number. of tag bits, then inserting it into the variant
      // risks clobbering meaningful low bits of the address. So we require it
      // be done explicitly.
      constexpr variant_ptr(T ptr) noexcept(impl::is_assert_off)
      : ptr_{ptr, types::template find<T>} {}

  explicit constexpr variant_ptr(std::nullptr_t) noexcept
      : variant_ptr(typename types::template nth<0>{nullptr}) {}

  // gcc 10 had a hidden friend template bug so these are static.
  // workaround.hpp defines free functions in namespace bitpack that forwards to
  // these. niebloids.hpp defines niebloids that will forward to them too.
  template<Tag N>
  static constexpr auto
      get(variant_ptr const self) noexcept(impl::is_assert_off) ->
      typename types::template nth<N> {
    static_assert(0 <= N && N < size, "The variant index is out of bounds");
    using T = typename types::template nth<N>;
    return get<T>(self);
  }
  template<class T>
  static constexpr auto
      get(variant_ptr const self) noexcept(impl::is_assert_off) -> T {
    static_assert(types::template has<T>, "That type is not in this variant");
    BITPACK_ASSERT(holds_alternative<T>(self));
    return static_cast<T>(void_star(self));
  }

  template<class T>
  static constexpr bool holds_alternative(variant_ptr const self) noexcept {
    return variant_ptr::index(self) == types::template find<T>;
  }

  constexpr friend bool operator==(variant_ptr const p, std::nullptr_t) {
    return p.ptr_ == nullptr;
  }
  constexpr friend bool operator==(std::nullptr_t, variant_ptr const p) {
    return p == nullptr;
  }
  constexpr operator bool() { return *this == nullptr; }

 private:
  static constexpr void* void_star(variant_ptr const self) noexcept {
    return self.ptr_.get();
  }

  template<class Func>
  static constexpr bool is_visit_noexcept =
      impl::is_visit_noexcept_by_seq<variant_ptr,
                                     Func,
                                     std::make_index_sequence<size>>::value;

  template<class Func>
  using visit_common_type = typename impl::visit_common_type_by_seq<
      variant_ptr,
      Func,
      std::make_index_sequence<size>>::type;

  template<class R, auto N, class Func>
  BITPACK_FORCEINLINE // help the compiler convert it to a switch?
      static R
      visit_nth(variant_ptr const self,
                Func visitor,
                Tag const tag) noexcept(is_visit_noexcept<Func>) {
    if constexpr(N >= size) {
      BITPACK_ASSERT(false);
      // just need the type. This is out of contract
      // conjure up a null deref in case we don't have a default constructor
      return impl::garbage_value<decltype(visitor(get<0>(self)))>();
    } else {
      if(tag == N)
        return visitor(get<N>(self));
      else
        return visit_nth<R, N + 1>(self, visitor, tag);
    }
  }

  tagged_ptr<void*, Tag, tag_bits> ptr_;

 public:
  template<class R, class Func>
  static constexpr R visit(Func visitor, variant_ptr const self)
      noexcept(is_visit_noexcept<Func>) {
    auto const tag = variant_ptr::index(self);
    BITPACK_ASSERT(0 <= tag);
    BITPACK_ASSERT(tag < size);
    return visit_nth<R, 0>(self, visitor, tag);
  }

  template<class Func>
  static constexpr auto visit(Func visitor, variant_ptr const self)
      BITPACK_EXPR_BODY(visit<visit_common_type<Func>, Func>(visitor, self))
};

// possible future directions:
// - derived_variant_ptr. Put the rtti into a tag
// - unique_variant?
// - variant_ptr etc that can hold pointer-like things?

} // namespace bitpack

#endif // BITPACK_VARIANT_PTR_INCLUDE_GUARD
