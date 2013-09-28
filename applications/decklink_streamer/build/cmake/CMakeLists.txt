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
#include(${dls_root_dir}/projects/streamer/build/cmake/CMakeLists.txt) # streamer -> cannot use because somewhere WIN32_LEAN_AND_MEAN is defined which mixes up the .idl compilation
include(${dls_root_dir}/projects/tinylib/build/cmake/CMakeLists.txt) # tinylib 
include(${dls_root_dir}/projects/decklink/build/cmake/CMakeLists.txt) # decklink

include_directories(
  ${dls_inc_dir}
  ${dls_install_dir}/include
  ${dls_root_dir}/extern/${tri_triplet}/include
)

set(dls_source_files 
  ${dls_src_dir}/main.cpp
)

if(CMAKE_BUILD_TYPE STREQUAL Debug)
  set(dls_name "${dls_name}_debug")
endif()

add_executable(${dls_name} ${dls_source_files})

target_link_libraries(${dls_name}
  ${dls_install_dir}/lib/${tri_triplet}/streamer.lib
  ${dls_install_dir}/lib/${tri_triplet}/decklink.lib
  comsuppw.lib
)

install(TARGETS ${dls_name} DESTINATION bin) 

add_dependencies(${dls_name} ${vd_lib_name}) # ${vd_lib_name} is included form the decklink cmake