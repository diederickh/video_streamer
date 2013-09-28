cmake_minimum_required(VERSION 2.8)

project(XMLConfig)

set(xc_base_dir ${CMAKE_CURRENT_LIST_DIR}/../../)

include(${xc_base_dir}/../../tools/cmake/Triplet.cmake)  # include triplet

include_directories(
  ${xc_base_dir}/include 
  ${xc_base_dir}/../../install/include
  ${xc_base_dir}/../../extern/${tri_triplet}/include
)

add_library(xmlconfig ${xc_base_dir}/src/xmlconfig/Config.cpp)

set(CMAKE_INSTALL_PREFIX ${xc_base_dir}/../../install)

install(TARGETS xmlconfig DESTINATION lib/${tri_triplet})

install(DIRECTORY ${xc_base_dir}/include/ DESTINATION include)