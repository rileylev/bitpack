#define CATCH_CONFIG_MAIN

#include <string>
#include <exception>

// I think exceptions gave me clearer catch2 error messages compared to assert.h
// This also lets us test assertions are actually fired
struct assert_exception : std::exception {
  std::string message;
  assert_exception(std::string file, int line)
      : message{file + ":" + std::to_string(line)} {}
  const char* what() const noexcept override { return message.c_str(); }
};

#define BITPACK_ASSERT(...)                                                    \
  [&]{if(!(__VA_ARGS__)) (throw assert_exception(__FILE__, __LINE__));}()
#include <bitpack/bitpack.hpp>
#include <bitpack/niebloids.hpp>

#include <catch2/catch.hpp>

TEST_CASE("The asserts are on"){
  STATIC_REQUIRE(BITPACK_ENABLE_ASSERT);
  REQUIRE_THROWS(BITPACK_ASSERT(false));
}

TEST_CASE("from_uintptr and as_uintptr are inverses"){
  int x =15124;
  using namespace bitpack::bits;
  REQUIRE(from_uintptr_t<int>(as_uintptr_t(x))==x);

  uintptr_t y = 1340918;
  REQUIRE(as_uintptr_t(from_uintptr_t<intptr_t>(y))==y);
}

// pair
TEST_CASE("A UInt_pair<X,Y,T>'s size and alignment match those of T") {
  STATIC_REQUIRE(sizeof(bitpack::UInt_pair<int, int, uintptr_t>)
                 == sizeof(uintptr_t));
  STATIC_REQUIRE(alignof(bitpack::UInt_pair<int, int, uintptr_t>)
                 == alignof(uintptr_t));
  STATIC_REQUIRE(sizeof(bitpack::UInt_pair<char, char, unsigned char, 2>)
                 == sizeof(unsigned char));
  STATIC_REQUIRE(alignof(bitpack::UInt_pair<char, char, unsigned char, 2>)
                 == alignof(unsigned char));
  STATIC_REQUIRE(sizeof(bitpack::UInt_pair<char, char, unsigned int, 2>)
                 == sizeof(unsigned int));
  STATIC_REQUIRE(alignof(bitpack::UInt_pair<char, char, unsigned int, 2>)
                 == alignof(unsigned int));
}

TEST_CASE("A uintptr_pair's elements are accessed in the same order as "
          "construction") {
  auto const elt0 = 32;
  auto const elt1 = 'c';
  auto const pair = bitpack::make_uintptr_pair(elt0, elt1);
  REQUIRE(get<0>(pair) == elt0);
  REQUIRE(get<1>(pair) == elt1);
}

TEST_CASE("UInt_pairs are lexicographically compared") {
  REQUIRE(bitpack::make_uintptr_pair(0, 5) < bitpack::make_uintptr_pair(2, 0));
  REQUIRE(bitpack::make_uintptr_pair(1, 5) < bitpack::make_uintptr_pair(1, 6));
}

TEST_CASE("UInt_pair's == is defined by elementwise == ") {
  REQUIRE(bitpack::make_uintptr_pair(1, 5) == bitpack::make_uintptr_pair(1, 5));
  REQUIRE(bitpack::make_uintptr_pair(2, 5) != bitpack::make_uintptr_pair(1, 5));
  REQUIRE(bitpack::make_uintptr_pair(1, 5) != bitpack::make_uintptr_pair(1, 0));
  REQUIRE(bitpack::make_uintptr_pair(5, 4) != bitpack::make_uintptr_pair(1, 0));
}

// tagged ptr
TEST_CASE(
    "A tagged_ptr is == to nullptr exactly when the pointer it holds is null") {
  bitpack::tagged_ptr<int*, int> p{nullptr, 0};
  REQUIRE(p == nullptr);
  REQUIRE(nullptr == p);

  p = bitpack::tagged_ptr<int*, int>{nullptr, 3};
  REQUIRE(p == nullptr);
  REQUIRE(nullptr == p);

  int x = 5;
  p = bitpack::tagged_ptr<int*, int>{&x, 0};
  REQUIRE(p != nullptr);
  REQUIRE(nullptr != p);

  p = bitpack::tagged_ptr<int*, int>{&x, 3};
  REQUIRE(p != nullptr);
  REQUIRE(nullptr != p);
}

TEST_CASE("tagged_ptr supports dereference operators"){
  SECTION("operator*"){
  int x = 3;
  bitpack::tagged_ptr p{&x,0};
  REQUIRE(*p==3);}

  SECTION("operator->"){
  struct {int x;} box{2};
  bitpack::tagged_ptr p{&box,3};
  REQUIRE(p->x==2);
  }
}

TEST_CASE("maybe_get returns nullopt if its argument does not hold the given "
          "type or index") {
  int x;
  std::variant<int*, long*> std_variant = &x;
  bitpack::variant_ptr<int*, long*> bpk_variant = &x;
  REQUIRE(bitpack::maybe_get<long*>(std_variant) == std::nullopt);
  REQUIRE(bitpack::maybe_get<long*>(bpk_variant) == std::nullopt);
  REQUIRE(bitpack::maybe_get<1>(std_variant) == std::nullopt);
  REQUIRE(bitpack::maybe_get<1>(bpk_variant) == std::nullopt);

  long y;
  std_variant = &y;
  bpk_variant = &y;
  REQUIRE(bitpack::maybe_get<int*>(std_variant) == std::nullopt);
  REQUIRE(bitpack::maybe_get<int*>(bpk_variant) == std::nullopt);
  REQUIRE(bitpack::maybe_get<0>(std_variant) == std::nullopt);
  REQUIRE(bitpack::maybe_get<0>(bpk_variant) == std::nullopt);
}

TEST_CASE("maybe_get returns optional of its contents when it does hold that "
          "type or index") {
  int x;
  std::variant<int*, long*> std_variant = &x;
  bitpack::variant_ptr<int*, long*> bpk_variant = &x;
  REQUIRE(bitpack::maybe_get<int*>(std_variant) == &x);
  REQUIRE(bitpack::maybe_get<int*>(bpk_variant) == &x);
  REQUIRE(bitpack::maybe_get<0>(std_variant) == &x);
  REQUIRE(bitpack::maybe_get<0>(bpk_variant) == &x);

  long y;
  std_variant = &y;
  bpk_variant = &y;
  REQUIRE(bitpack::maybe_get<long*>(std_variant) == &y);
  REQUIRE(bitpack::maybe_get<long*>(bpk_variant) == &y);
  REQUIRE(bitpack::maybe_get<1>(std_variant) == &y);
  REQUIRE(bitpack::maybe_get<1>(bpk_variant) == &y);
}

template<class... Fs>
struct overload : Fs... {
  using Fs::operator()...;
};
template<class... Fs>
overload(Fs...) -> overload<Fs...>;

namespace niebloids = bitpack::niebloids;
TEST_CASE("Niebloids give == comparable values on bitpack containers and std "
          "containers") {
  SECTION("Pairs") {
    auto const std_pair = std::pair{'a', 2};
    auto const bpk_pair = bitpack::make_uintptr_pair('a', 2);

    SECTION("get_n") {
      REQUIRE(niebloids::get_n<0>(std_pair) == niebloids::get_n<0>(bpk_pair));
      REQUIRE(niebloids::get_n<1>(std_pair) == niebloids::get_n<1>(bpk_pair));
    }
    SECTION("get_t") {
      REQUIRE(niebloids::get_t<char>(std_pair)
              == niebloids::get_t<char>(bpk_pair));
      REQUIRE(niebloids::get_t<int>(std_pair)
              == niebloids::get_t<int>(bpk_pair));
    }
  }
  SECTION("variant") {
    using BpkVariant = bitpack::variant_ptr<int*, float*, std::string*>;
    using StdVariant = std::variant<int*, float*, std::string*>;

    int x = 3;
    BpkVariant bpk_variant = &x;
    StdVariant std_variant = &x;
    SECTION("get_n") {
      REQUIRE(niebloids::get_n<0>(std_variant)
              == niebloids::get_n<0>(bpk_variant));
    }
    SECTION("get_t") {
      REQUIRE(niebloids::get_t<int*>(std_variant)
              == niebloids::get_t<int*>(bpk_variant));
    }
    SECTION("holds_alternative") {
      REQUIRE(niebloids::holds_alternative<int*>(std_variant)
              == niebloids::holds_alternative<int*>(bpk_variant));
      REQUIRE(niebloids::holds_alternative<float*>(std_variant)
              == niebloids::holds_alternative<float*>(bpk_variant));
      REQUIRE(niebloids::holds_alternative<std::string*>(std_variant)
              == niebloids::holds_alternative<std::string*>(bpk_variant));
    }
    using namespace std::literals;
    SECTION("visit") {
      auto const visitor = overload{[](int*) { return "int*"s; },
                                    [](float*) { return "float*"s; },
                                    [](std::string*) { return "std::string*"s; }};

      REQUIRE(niebloids::visit(visitor, bpk_variant)
              == niebloids::visit(visitor, std_variant));

      float y;
      bpk_variant =&y;
      std_variant =&y;
      REQUIRE(niebloids::visit(visitor, bpk_variant)
              == niebloids::visit(visitor, std_variant));

      std::string z;
      bpk_variant =&z;
      std_variant =&z;
      REQUIRE(niebloids::visit(visitor, bpk_variant)
              == niebloids::visit(visitor, std_variant));
    }
  }
}
