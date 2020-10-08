# This defines the development build process (for me)
# This will set up CMAKE to work with conan without leaking the
# conan-related details into the core project definition.
#
# This file also sets up testing and options relevant for development
cmake_minimum_required(VERSION 3.17)

set(CMAKE_WARN_DEPRICATED ON)
set(CMAKE_FIND_PACKAGE_PREFER_CONFIG TRUE)

include(${CMAKE_BINARY_DIR}/conan_paths.cmake)

enable_testing()
add_subdirectory(tests)

find_program(CCACHE ccache)
if(CCACHE)
  message(STATUS "Found ccache")
  set(CMAKE_CXX_COMPILER_LAUNCHER ${CCACHE})
else()
  message(STATUS "Couldn't find ccache")
endif()

find_program(CLANGTIDY clang-tidy)
set(CMAKE_CXX_CLANG_TIDY ${CLANGTIDY} -extra-arg=-Wno-unknown-warning-option)

if (CMAKE_GENERATOR MATCHES "Visual Studio")
else()
add_compile_options(
  -fsanitize=address
  -Werror
  -Wall
  -Wextra
  -Wshadow
  -Wnon-virtual-dtor
  -Wold-style-cast
  -Wunused
  -Wpedantic
  -Wconversion
  -Wsign-conversion
  -Wnull-dereference
  -Wdouble-promotion
  )
endif()
