cmake_minimum_required(VERSION 2.8)

project(hwscale)

option(USE_HWSCALE_TEST "Build tests" OFF)
if(WIN32)
  option(USE_64BIT "Compile as 64bit library" OFF)
else()
  option(USE_64BIT "Compile as 64bit library" ON)
endif()

if(USE_64BIT)
  set(hws_arch "x86_64")
else()
  set(hws_arch "i386")
endif()

set(hws_name "hwscale")
set(hws_base_dir ${CMAKE_CURRENT_LIST_DIR}/../../)
set(hws_src_dir ${hws_base_dir}/src/)
set(hws_test_src_dir ${hws_base_dir}/tests/)
set(hws_tests_dir ${hws_base_dir}/tests/)
set(hws_inc_dir ${hws_base_dir}/include)
set(hws_ext_dir ${hws_base_dir}/../../extern/)

set(hws_source_files
#  ${hws_test_src_dir}/opengl_yuv420p/main.cpp
  ${hws_src_dir}/hwscale/opengl/YUV420PGrabber.cpp
  ${hws_src_dir}/hwscale/opengl/ResizerFBO.cpp
)

set(CMAKE_INSTALL_PREFIX ${hws_base_dir}/../../install/)

if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
  set(hws_compiler "clang")
elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
  set(hws_compiler "gcc")
elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
  if(MSVC10)
    set(hws_compiler "vs2010")
  else()
    set(hws_compiler "vs2012")
  endif()
endif()

if(APPLE)
  set(CMAKE_OSX_ARCHITECTURES ${hws_arch})
  set(hws_platform "mac")
elseif(WIN32)
  set(hws_platform "win")
else()
  set(hws_platform "unix")
endif()

set(hws_triplet "${hws_platform}-${hws_compiler}-${hws_arch}")

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  set(hws_triplet "${hws_triplet}d")
  set(hws_name "hwscale_debug")
endif()

set(hws_extern_include_dir ${hws_ext_dir}/${hws_triplet}/include/)
set(hws_extern_lib_dir ${hws_ext_dir}/${hws_triplet}/lib/)

if(APPLE)
  find_library(fr_opengl OpenGL)
  find_library(fr_iokit IOKit)
  find_library(fr_cf CoreFoundation)
  find_library(fr_cocoa Cocoa)
  list(APPEND hws_libs 
    ${hws_extern_lib_dir}/libglfw3.a
    ${hws_extern_lib_dir}/libuv.a
    ${hws_extern_lib_dir}/libnanomsg.a
    ${fr_opengl} 
    ${fr_iokit} 
    ${fr_cf}
    ${fr_cocoa}
    ${fr_opengl}/Headers/
    ) 
elif(WIN32)
  
endif()

add_definitions(
  -DGLFW_INCLUDE_GLCOREARB
  -DYUV420P_GRABBER_GLSL_VERSION=150
)

include_directories(
  ${hws_extern_include_dir} 
  ${hws_inc_dir}
)

# add_executable(${hws_name} ${hws_source_files})

add_library(libhwscale ${hws_source_files})

install(TARGETS libhwscale DESTINATION lib/${hws_triplet}/)

install(DIRECTORY ${hws_inc_dir}/ DESTINATION include)

if(USE_HWSCALE_TESTS) 
  include(${hws_tests_dir}/CMakeLists.txt)
endif()

