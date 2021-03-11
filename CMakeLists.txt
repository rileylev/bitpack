cmake_minimum_required(VERSION 3.17)

project(bitpack
  DESCRIPTION "Utilities for packing bits together (small pairs, tagged pointers)."
  VERSION 0.0.0
  LANGUAGES CXX)

include(early_hook.cmake OPTIONAL)

add_library(bitpack INTERFACE)
add_library(bitpack::bitpack ALIAS bitpack)
target_include_directories(bitpack
  INTERFACE
  include)

target_compile_features(bitpack INTERFACE cxx_std_20)


add_subdirectory(tests)
