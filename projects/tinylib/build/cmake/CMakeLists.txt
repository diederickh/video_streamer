cmake_minimum_required(VERSION 2.8)

set(tiny_base_dir ${CMAKE_CURRENT_LIST_DIR}/../../)
set(tiny_inc_dir ${tiny_base_dir}/include/)

include_directories(${tiny_inc_dir})

install(DIRECTORY ${tiny_inc_dir}/ DESTINATION include) 