cmake_minimum_required(VERSION 2.8)

# ${tinylib_include_dirs}   - Include directories for tinylib, add these to your include dirs
# ${tinylib_libraries}      - The libraries to link with.

include(${CMAKE_CURRENT_LIST_DIR}/../../../../tools/cmake/Triplet.cmake)  # include triplet

set(tinylib_include_dirs ${CMAKE_CURRENT_LIST_DIR}/../../include/)

set(tinylib_libraries
  ${extern_dir}/lib/libpng.a
)
