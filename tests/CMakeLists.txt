# Bare-bones test definition.
# It only define the logical connection between components.
#
# This file does not assume any kind of package manager.
# It assumes dependencies will be found by find_package.
project(bitpack_test)


add_executable(tester test.cpp)
add_test(NAME bitpack.test COMMAND tester )
find_package(Catch2 REQUIRED)

set(CMAKE_BUILD_TYPE Debug)

target_link_libraries(tester
  PRIVATE
  bitpack::bitpack
  Catch2::Catch2)

include(CTest)
include(Catch)
catch_discover_tests(tester)
