#ifndef BITPACK_VARIANT_PTR_INCLUDE_GUARD
#define BITPACK_VARIANT_PTR_INCLUDE_GUARD

#include <bitpack/macros.hpp>
#include <bitpack/traits.hpp>
#include <bitpack/tagged_ptr.hpp>
#include <bitpack/workaround.hpp>

#include <tuple>

namespace bitpack {
namespace impl {
/**
 * This implements typelists and related functionality as needed for
 * variant_ptr
 */
template<class... Ts> struct typelist {
 private:
  static_assert((... && !std::is_reference_v<Ts>),
                "You can't put references in a variant_ptr");
  using type_tuple = std::tuple<Ts...>;

 public:
  static constexpr unsigned size = sizeof...(Ts);

  template<size_t N> using nth = std::tuple_element_t<N, type_tuple>;

 private:
  template<class T, size_t n>
  static constexpr bool is_T_nth = std::is_same_v<T, nth<n>>;

  // helper for finding the index of a given type. If its not found, return one
  // past the end, like the STL convention
  template<class T, int N> static constexpr int find_looper() noexcept {
    // separate case to prevent instantiating is_T_nth with big N. The user gets
    // our error instead of std::tuple's
    if constexpr(N >= size) return N;
    else if(is_T_nth<T, N>)
      return N;
    else
      return find_looper<T, N + 1>();
  }

 public:
  template<class T>
  static constexpr unsigned unguarded_find = find_looper<T, 0>();

  /**
   * Is the type T contained in this list?
   */
  template<class T> static constexpr bool has = (unguarded_find<T> < size);

  /**
   * Find the index of a type T. Static_assert if the type T is not in this
   * list.
   */
  template<class T>
  static constexpr unsigned find = [] {
    static_assert(has<T>, "Type not in variant");
    return unguarded_find<T>;
  }();
};

// these assume an index sequence will be supplied
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

/**
 * What type will a visitor return?
 */
template<class variant_ptr, class Func, class seq>
struct visit_common_type_by_seq;

template<class variant_ptr, class Func, auto... i>
struct visit_common_type_by_seq<variant_ptr, Func, std::index_sequence<i...>> {
  using type = std::common_type_t<decltype(std::declval<Func>()(
      get<i>(std::declval<variant_ptr>())))...>;
};

} // namespace impl

#if true
/**
 * A variant that can only store pointers. It stores the type index inline in
 * the low bits of the pointer itself.
 *
 * The alignemnt of all the pointers must be big enough to hold the type tag.
 * That's the sharp edge of this tool. You can only implicitly construct if the
 * alignment is sufficiently large. Otherwise, you have to allocate carefully or
 * use a different tool.
 *
 * Ts = the pointer types your variant_ptr can hold.
 */
template<class... Ts> class variant_ptr {
  using types = impl::typelist<Ts...>;
  static constexpr auto tag_bits = std::bit_width(types::size - 1);

 public:
  static constexpr auto size = types::size;
  using Tag = int;
  static constexpr Tag index(variant_ptr const self) noexcept {
    auto const ptr = self.ptr_;
    return decltype(ptr)::tag(ptr);
  }
  constexpr Tag index() const noexcept { return index(*this); }

  constexpr variant_ptr() = default;
  template<class T>
  explicit(alignof(traits::unptr_t<T>) <= tag_bits)
      // If alignment<= number. of tag bits, then inserting it into the variant
      // risks clobbering meaningful low bits of the address. So we require it
      // be done explicitly.
      constexpr variant_ptr(T ptr) noexcept(impl::is_assert_off)
      : ptr_{ptr, types::template find<T>} {}

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

  // generic case visitor that uses template recursion
  template<class R, Tag N, class Func>
  static R visit_nth(variant_ptr const self,
                     Func visitor,
                     Tag const tag) noexcept(is_visit_noexcept<Func>) {
    static_assert(0 <= N && N < size);
    auto const invoke_it = [&] { return std::invoke(visitor, get<N>(self)); };
    if constexpr(N == size - 1) {
      return invoke_it();
    } else {
      if(tag == N)
        return invoke_it();
      else
        return visit_nth<R, N + 1>(self, visitor, tag);
    }
  }

  tagged_ptr<void*, Tag, tag_bits> ptr_;

 public:
  // unrolled/optimized visit implementation.
  // Implement visit as a switch on the index.
  // because there is no way (afaik) to generate the cases from a parameter
  // pack, I do this with macros (up to a finite limit)

  // this is the finite unrolling limit
  // note: to raise this, you might have to implement more cases for
  // BITPACK_REPEAT
#  ifndef BITPACK_UNROLL_VISIT_LIMIT
#    define BITPACK_UNROLL_VISIT_LIMIT 6
#  endif
#  define BITPACK_VISIT_CASE(n)                                                \
    case n: return std::invoke(visitor, get<n>(self));
  // this macro generates the visit implementation (via switch(index) ) for
  // a variant that can take on n types
#  define BITPACK_UNROLL_VISIT(n)                                              \
    template<class R, class Func>                                              \
    requires(size == n) static constexpr R                                     \
        visit(Func visitor_,                                                   \
              variant_ptr const self) noexcept(is_visit_noexcept<Func>) {      \
      auto const tag = index(self);                                            \
      /* idk clang thinks im not using this */                                 \
      [[maybe_unused]] auto const visitor = visitor_;                          \
      BITPACK_ASSERT(0 <= tag && tag < bits::narrow<int>(size));               \
      switch(tag) { BITPACK_REPEAT(BITPACK_VISIT_CASE, n) }                    \
    }
  // index can only be in [0, size), but the compiler does not realize this.
  // We don't need a default in the switch. Asserts + static_asserts enforce
  // this invariant.
  BITPACK_WRETURN_OFF
    BITPACK_REPEAT_OUTER(BITPACK_UNROLL_VISIT, BITPACK_UNROLL_VISIT_LIMIT)
  BITPACK_DIAGNOSTIC_POP

  // fallback visit implementation
  template<class R, class Func>
  requires(size >= BITPACK_UNROLL_VISIT_LIMIT) //
      static constexpr R
      visit(Func visitor,
            variant_ptr const self) noexcept(is_visit_noexcept<Func>) {
    auto const tag = index(self);
    BITPACK_ASSERT(0 <= tag && tag < bits::narrow<int>(size));
    return visit_nth<R, 0>(self, visitor, tag);
  }

  template<class Func>
  static constexpr auto visit(Func visitor, variant_ptr const self)
      BITPACK_EXPR_BODY(visit<visit_common_type<Func>, Func>(visitor, self))
};
#endif

// possible future directions:
// - derived_variant_ptr. Put the rtti into a tag
// - unique_variant?
// - variant_ptr etc that can hold pointer-like things?

} // namespace bitpack

#endif // BITPACK_VARIANT_PTR_INCLUDE_GUARD
