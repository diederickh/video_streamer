cmake_minimum_required(VERSION 2.8)

# ${xmlconfig_source_dir}    - path to source dir
# ${xmlconfig_source_dirs}   - all the source dirs that contains source files for xmlconfig (use this when you want to package something, see the openFrameworks addon)
# ${xmlconfig_souce_files}   - compile these source files
# ${xmlconfig_include_dirs}  - add these to your include directories

include(${CMAKE_CURRENT_LIST_DIR}/../../../../tools/cmake/Triplet.cmake)

set(xmlconfig_source_dir ${CMAKE_CURRENT_LIST_DIR}/../../src/xmlconfig)

set(xmlconfig_source_dirs ${xmlconfig_source_dir}/)

set(xmlconfig_include_dirs ${CMAKE_CURRENT_LIST_DIR}/../../include/)

set(xmlconfig_source_files  ${xmlconfig_source_dir}/Config.cpp)

