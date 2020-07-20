# This defines the development build process (for me)
# This will set up CMAKE to work with conan without leaking the
# conan-related details into the core project definition.
#
# This file also sets up testing and options relevant for development
cmake_minimum_required(VERSION 3.17)

project(bitpack_top)
set(CMAKE_WARN_DEPRICATED ON)
set(CMAKE_FIND_PACKAGE_PREFER_CONFIG TRUE)

include(${CMAKE_BINARY_DIR}/conan_paths.cmake)

enable_testing()


add_subdirectory(src)
add_subdirectory(tests)

add_compile_options(tester PRIVATE
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
