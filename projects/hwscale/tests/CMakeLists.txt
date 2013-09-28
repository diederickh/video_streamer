cmake_minimum_required(VERSION 2.8)

include(${CMAKE_CURRENT_LIST_DIR}/../../grapher/build/cmake/CMakeLists.txt) # grapher
include(${CMAKE_CURRENT_LIST_DIR}/../../tinylib/build/cmake/CMakeLists.txt) # tinylib

set(hws_test_name "yuv420")

set(hws_base_dir ${hws_tests_dir}/opengl_yuv420p)

set(hws_test_source_files 
  ${hws_source_files} 
  ${hws_base_dir}/main.cpp
  ${hws_base_dir}/Animation.cpp
)

add_executable(${hws_test_name} ${hws_test_source_files})

target_link_libraries(${hws_test_name} ${hws_libs} ${gr_lib_name})

install(TARGETS ${hws_test_name} RUNTIME DESTINATION bin)

add_dependencies(${hws_test_name} ${gr_lib_name})  # gr_lib_name is defined in the cmake of grapher
  