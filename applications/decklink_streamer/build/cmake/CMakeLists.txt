cmake_minimum_required(VERSION 2.8)

project(DeckLinkStreamer)

set(dls_root_dir ${CMAKE_CURRENT_LIST_DIR}/../../../../)
set(dls_install_dir ${dls_root_dir}/install)
set(dls_tools_dir ${dls_root_dir}/tools)
set(dls_name "decklink_streamer")
set(dls_base_dir ${CMAKE_CURRENT_LIST_DIR}/../../)
set(dls_src_dir ${dls_base_dir}/src/)
set(dls_inc_dir ${dls_base_dir}/include/)
set(dls_extern_dir ${dls_base_dir}/../../extern/)
set(CMAKE_INSTALL_PREFIX ${dls_install_dir})

include(${dls_tools_dir}/cmake/Triplet.cmake)  # include triplet
include(${dls_root_dir}/projects/tinylib/build/cmake/CMakeLists.txt) # tinylib 
include(${dls_root_dir}/projects/decklink/build/cmake/CMakeLists.txt) # decklink
include(${dls_root_dir}/projects/hwscale/build/cmake/CMakeLists.txt) # hwscale

find_package(OpenGL REQUIRED)

add_definitions(-DYUV420P_GRABBER_GLSL_VERSION=150)

include_directories(
  ${dls_inc_dir}
  ${dls_install_dir}/include
  ${dls_root_dir}/extern/${tri_triplet}/include
)

set(dls_source_files 
  ${dls_src_dir}/main.cpp
  ${dls_src_dir}/FastI420Upload.cpp
  ${dls_root_dir}/extern/${tri_triplet}/src/GLXW/glxw.c
)

if(CMAKE_BUILD_TYPE STREQUAL Debug)
  set(dls_name "${dls_name}_debug")
endif()

add_executable(${dls_name} ${dls_source_files})

target_link_libraries(${dls_name}
  ${dls_install_dir}/lib/${tri_triplet}/streamer.lib
  ${dls_install_dir}/lib/${tri_triplet}/libdecklink.lib
  ${dls_install_dir}/lib/${tri_triplet}/libhwscale.lib
  ${dls_extern_dir}/${tri_triplet}/lib/glfw3.lib
  ${dls_extern_dir}/${tri_triplet}/lib/libyuv.lib
  ${dls_extern_dir}/${tri_triplet}/lib/libuv.lib
  comsuppw.lib
  ${OPENGL_gl_LIBRARY}
)

install(TARGETS ${dls_name} DESTINATION bin) 

add_dependencies(${dls_name} ${vd_lib_name}) # ${vd_lib_name} is included form the decklink cmake
add_dependencies(${dls_name} libhwscale) # hw scaler

#include(${dls_root_dir}/projects/streamer/build/cmake/CMakeLists.txt) # streamer -> cannot use because somewhere WIN32_LEAN_AND_MEAN is defined which mixes up the .idl compilation