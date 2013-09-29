cmake_minimum_required(VERSION 2.8)

include(${CMAKE_CURRENT_LIST_DIR}/../../../../tools/cmake/Triplet.cmake)  # include triplet

set(tiny_base_dir ${CMAKE_CURRENT_LIST_DIR}/../../)

set(tiny_inc_dir ${tiny_base_dir}/include/)

include_directories(${tiny_inc_dir})

set(CMAKE_INSTALL_PREFIX ${tiny_base_dir}/../../install/${tri_triplet}/)

install(DIRECTORY ${tiny_inc_dir} DESTINATION include) 