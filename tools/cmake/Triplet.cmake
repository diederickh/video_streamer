cmake_minimum_required(VERSION 2.8)

# Creates:
# ${install_dir}          - the install prefix
# ${extern_dir}           - path to the extern libraries (root)
# ${extern_include_dirs}  - include directories in for the extern libraries

if(USE_64BIT)
  set(tri_arch "x86_64")
else()
  set(tri_arch "i386")
endif()

if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
  set(tri_compiler "clang")
elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
  set(tri_compiler "gcc")
elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
  if(MSVC10)
    set(tri_compiler "vs2010")
  else()
    set(tri_compiler "vs2012")
  endif()
endif()

if(APPLE)
  set(tri_platform "mac")
elseif(WIN32)
  set(tri_platform "win")
else()
  set(tri_platform "unix")
endif()

set(tri_triplet "${tri_platform}-${tri_compiler}-${tri_arch}")

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  set(tri_triplet "${tri_triplet}d")
endif()

set(install_dir ${CMAKE_CURRENT_LIST_DIR}/../../install/${tri_triplet})

set(extern_dir ${CMAKE_CURRENT_LIST_DIR}/../../extern/${tri_triplet})

set(extern_include_dirs ${extern_dir}/include/)

set(CMAKE_INSTALL_PREFIX ${install_dir})