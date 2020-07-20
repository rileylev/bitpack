# Bare-bones project definition.
# It only define the logical connection between components.
#
# This file does not assume any kind of package manager.
# It assumes dependencies will be found by find_package.
cmake_minimum_required(VERSION 3.17)

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

# install(TARGETS bitpack EXPORT bitpack_targets)
# install(EXPORT bitpack_targets
#   FILE bitpack_targets.cmake
#   NAMESPACE bitpack::bitpack
#   # should this be set in stone?
#   DESTINATION lib/cmake/bitpack)

# include(CMakePackageConfigHelpers)
# write_basic_package_version_file("bitpackConfigVersion.cmake"
#   VERSION ${bitpack_version}
#   COMPATIBILITY SameMajorVersion)
# install(FILES "bitpackConfig.cmake" "bitpackConfigVersion.cmake"
#   DESTINATION lib/cmake/bitpack)
