cmake_minimum_required(VERSION 2.8)

project(ConnectionTest)

if(APPLE)
  set(USE_64BIT ON)
endif()

include(${CMAKE_CURRENT_LIST_DIR}/../../../../tools/cmake/Triplet.cmake)  # include triplet
include(${CMAKE_CURRENT_LIST_DIR}/../../../../projects/streamer/build/cmake/CMakeLists.txt) # streamer
include(${CMAKE_CURRENT_LIST_DIR}/../../../../projects/xmlconfig/build/cmake/CMakeLists.txt) # xmlconfig
include(${CMAKE_CURRENT_LIST_DIR}/../../../../projects/decklink/build/cmake/CMakeLists.txt) # decklink
include(${CMAKE_CURRENT_LIST_DIR}/../../../../projects/tinylib/build/cmake/CMakeLists.txt) # tinylib
include(${CMAKE_CURRENT_LIST_DIR}/../../../../projects/hwscale/build/cmake/CMakeLists.txt) # hwscale
include(${CMAKE_CURRENT_LIST_DIR}/../../../../projects/portaudio/build/cmake/CMakeLists.txt) # portaudio

set(app_source_dir ${CMAKE_CURRENT_LIST_DIR}/../../src/)
set(app_name "test_multi_streamer")

if(CMAKE_BUILD_TYPE STREQUAL Debug)
  set(app_name "${app_name}_debug")
endif()

set(app_source_files
  ${streamer_source_files}
  ${xmlconfig_source_files}
  ${decklink_source_files}
  ${hwscale_source_files}
  ${portaudio_source_files}
  ${app_source_dir}/main.cpp
  ${app_source_dir}/Animation.cpp
  ${extern_dir}/src/GLXW/glxw.c
)

find_package(OpenGL REQUIRED)

set(app_libraries 
  ${streamer_libraries}
  ${decklink_libraries}
  ${hwscale_libraries}
  ${portaudio_libraries}
  ${tinylib_libraries}
  ${OPENGL_gl_LIBRARY}
)

if(WIN32)
  list(APPEND app_libraries
    ${extern_dir}/lib/glfw3.lib
    ${extern_dir}/lib/libyuv.lib
    )
endif()

if(APPLE)
  list(APPEND app_libraries
    ${extern_dir}/lib/libglfw3.a
    ${extern_dir}/lib/libyuv.a
    )
endif()


set(app_install_files
  ${streamer_install_files}
  ${CMAKE_CURRENT_LIST_DIR}/test_multi_streamer.xml
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
  ${portaudio_include_dirs}
)

add_executable(${app_name} ${app_source_files})

add_dependencies(${app_name} ${decklink_dependencies})

target_link_libraries(${app_name} ${app_libraries})

install(TARGETS ${app_name} DESTINATION bin)

install(FILES ${app_install_files} DESTINATION bin)
