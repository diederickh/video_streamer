cmake_minimum_required(VERSION 2.8)

# ${xmlconfig_source_dir}    - path to source dir
# ${xmlconfig_souce_files}   - compile these source files
# ${xmlconfig_include_dirs}  - add these to your include directories

include(${CMAKE_CURRENT_LIST_DIR}/../../../../tools/cmake/Triplet.cmake)

set(xmlconfig_source_dir ${CMAKE_CURRENT_LIST_DIR}/../../src/xmlconfig)

set(xmlconfig_include_dirs ${CMAKE_CURRENT_LIST_DIR}/../../include/)

set(xmlconfig_source_files  ${xmlconfig_source_dir}/Config.cpp)

