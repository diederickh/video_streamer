cmake_minimum_required(VERSION 2.8)

project(DeckLinkStreamer)

include(${CMAKE_CURRENT_LIST_DIR}/../../../../tools/cmake/Triplet.cmake)  # include triplet
include(${CMAKE_CURRENT_LIST_DIR}/../../../../projects/streamer/build/cmake/CMakeLists.txt) # streamer
include(${CMAKE_CURRENT_LIST_DIR}/../../../../projects/xmlconfig/build/cmake/CMakeLists.txt) # xmlconfig
include(${CMAKE_CURRENT_LIST_DIR}/../../../../projects/decklink/build/cmake/CMakeLists.txt) # decklink
include(${CMAKE_CURRENT_LIST_DIR}/../../../../projects/tinylib/build/cmake/CMakeLists.txt) # tinylib
include(${CMAKE_CURRENT_LIST_DIR}/../../../../projects/hwscale/build/cmake/CMakeLists.txt) # hwscale

set(dls_source_dir ${CMAKE_CURRENT_LIST_DIR}/../../src/)
set(dls_name "decklink_streamer")

if(CMAKE_BUILD_TYPE STREQUAL Debug)
  set(dls_name "${dls_name}_debug")
endif()

set(dls_source_files
  ${streamer_source_files}
  ${xmlconfig_source_files}
  ${decklink_source_files}
  ${hwscale_source_files}
  ${dls_source_dir}/main.cpp
  ${dls_source_dir}/FastI420Upload.cpp
  ${extern_dir}/src/GLXW/glxw.c
)

find_package(OpenGL REQUIRED)

set(dls_libraries 
  ${streamer_libraries}
  ${decklink_libraries}
  ${hwscale_libraries}
  ${extern_dir}/lib/glfw3.lib
  ${extern_dir}/lib/libyuv.lib
  ${OPENGL_gl_LIBRARY}
)

set(dls_install_files
  ${streamer_install_files}
)

add_definitions(
  ${hwscale_definitions}
  ${streamer_definitions}
)

include_directories(
  ${CMAKE_CURRENT_LIST_DIR}/../../include/
  ${streamer_include_dirs} 
  ${xmlconfig_include_dirs}
  ${decklink_include_dirs}
  ${tinylib_include_dirs}
  ${hwscale_include_dirs}
  ${extern_include_dirs}
)

add_executable(${dls_name} ${dls_source_files})

add_dependencies(${dls_name} ${decklink_dependencies})

target_link_libraries(${dls_name} ${dls_libraries})

install(TARGETS ${dls_name} DESTINATION bin)

install(FILES ${dls_install_files} DESTINATION bin)
