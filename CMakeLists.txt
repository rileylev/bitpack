# Bare-bones project definition.
# It only define the logical connection between components.
#
# This file does not assume any kind of package manager.
# It assumes dependencies will be found by find_package.
cmake_minimum_required(VERSION 3.17)

include(early_hooks.cmake OPTIONAL)

project(bitpack
  DESCRIPTION "Utilities for packing bits together (small pairs, tagged pointers)."
  VERSION 0.0.0
  LANGUAGES CXX)

add_library(bitpack INTERFACE)
add_library(bitpack::bitpack ALIAS bitpack)
target_include_directories(bitpack
  INTERFACE
  include)

target_compile_features(bitpack INTERFACE cxx_std_20)
