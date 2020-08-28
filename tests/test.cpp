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
  if(!(__VA_ARGS__)) (throw assert_exception(__FILE__, __LINE__));
#include "bitpack/bitpack.hpp"

#include <catch2/catch.hpp>

using namespace bitpack;

TEST_CASE("as_uintptr_t and from_uintptr_t are inverses") {
  using namespace bitpack::impl;
  auto x = 4;
  auto y = from_uintptr_t<long>(as_uintptr_t(long{x}));
  REQUIRE(y == x);
}

TEST_CASE("A uintptr_pair stores its elements within a single uintptr_t") {
  auto p = make_uintptr_pair(1, 3);
  REQUIRE(p.x() == 1);
  REQUIRE(p.y() == 3);
  REQUIRE(sizeof(p) == sizeof(uintptr_t));
}

TEST_CASE("A uintptr_pair's construction asserts when input is too big"
          "(if slow_asserts are enabled).") {
  CHECK_THROWS(make_uintptr_pair<1>(1, 4));
  CHECK_THROWS(uintptr_pair<int, int, sizeof(uintptr_t) * 8 - 1>(4, 1));
}

TEST_CASE("A tagged_ptr will address the same location regardless of tag") {
  int x = 32;
  REQUIRE(tagged_ptr<int, bool>::tag_bits == 2);
  tagged_ptr<int, bool> p{&x, false};
  REQUIRE(p.get() == &x);
}

TEST_CASE("variant_ptr can be constructed implicitly from specified types with "
          "sufficiently large alignment") {
  using test_variant = variant_ptr<int*, float*, std::string*>;
  SECTION("int*") {
    int x = 32;
    test_variant test = &x;
    REQUIRE(test.index() == 0);
    REQUIRE(get<int>(test) == &x);
    REQUIRE(get_if<int>(test) == &x);
  }
  SECTION("float*") {
    float x = 3.14;
    test_variant test = &x;
    REQUIRE(test.index() == 1);
    REQUIRE(get<float>(test) == &x);
    REQUIRE(get_if<float>(test) == &x);
  }
  SECTION("std::string*") {
    std::string x = "hello, world";
    test_variant test = &x;
    REQUIRE(test.index() == 2);
    REQUIRE(get<std::string>(test) == &x);
    REQUIRE(get_if<std::string>(test) == &x);
  }
}

TEST_CASE("get_if returns nullptr when variant_ptr does not contain the "
          "requested type") {
  int x = 3;
  variant_ptr<int*, float*> p = &x;
  REQUIRE(p.index() == 0);
  REQUIRE(get_if<1>(p) == nullptr);
  REQUIRE(get_if<float*>(p) == nullptr);
}

template<class... Ts>
struct overload : Ts... {
  using Ts::operator()...;
};
template<class... Ts>
overload(Ts...) -> overload<Ts...>;

TEST_CASE(
    "Visit dispatches based on the type currently stored in the variant_ptr") {
  using namespace std::string_literals;

  auto visitor = overload{[](int*) { return "int"s; },
                          [](float*) { return "float"s; },
                          [](long*) { return "long"s; },
                          [](std::string*) { return "string"s; }};

  SECTION("A variant with two elements") {
    variant_ptr<int*, float*> p;

    int x = 3;
    p = &x;
    REQUIRE(visit(p, visitor) == "int"s);

    float y = 2.0f;
    p = &y;
    REQUIRE(visit(p, visitor) == "float"s);
  }

  SECTION("A variant with three elements") {
    variant_ptr<int*, float*, long*> p;

    int x = 3;
    p = &x;
    REQUIRE(visit(p, visitor) == "int"s);

    float y = 2.0f;
    p = &y;
    REQUIRE(visit(p, visitor) == "float"s);

    long z = 3;
    p = &z;
    REQUIRE(visit(p, visitor) == "long"s);
  }
  SECTION("A variant with four elements") {
    variant_ptr<int*, float*, long*, std::string*> p;

    int x = 3;
    p = &x;
    REQUIRE(visit(p, visitor) == "int"s);

    float y = 2.0f;
    p = &y;
    REQUIRE(visit(p, visitor) == "float"s);

    long z = 3;
    p = &z;
    REQUIRE(visit(p, visitor) == "long"s);

    std::string s = "hello";
    p = &s;
    REQUIRE(visit(p, visitor) == "string"s);
  }
}

TEST_CASE(
    "variant_ptr SLOW_ASSERTS during creation if the pointer gets clobbered "
    "because alignment is too small") {
  // at least one of these will have bad alignment
  char x = 'a';
  char y = 'b';
  CHECK_THROWS(variant_ptr<char*>(&x), variant_ptr<char*>(&y));
}
