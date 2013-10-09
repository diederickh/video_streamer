cmake_minimum_required(VERSION 2.8)

project(ConnectionTest)

if(APPLE)
  set(USE_64BIT ON)
endif()

include(${CMAKE_CURRENT_LIST_DIR}/../../../../tools/cmake/Triplet.cmake)  # include triplet
include(${CMAKE_CURRENT_LIST_DIR}/../../../../projects/portaudio/build/cmake/CMakeLists.txt) # portaudio
include(${CMAKE_CURRENT_LIST_DIR}/../../../../projects/streamer/build/cmake/CMakeLists.txt) # streamer
include(${CMAKE_CURRENT_LIST_DIR}/../../../../projects/xmlconfig/build/cmake/CMakeLists.txt) # xmlconfig
include(${CMAKE_CURRENT_LIST_DIR}/../../../../projects/tinylib/build/cmake/CMakeLists.txt) # tinylib

set(app_source_dir ${CMAKE_CURRENT_LIST_DIR}/../../src/)
set(app_name "test_faac")

if(CMAKE_BUILD_TYPE STREQUAL Debug)
  set(app_name "${app_name}_debug")
endif()

set(app_source_files
  ${portaudio_source_files}
  ${streamer_source_files}
  ${xmlconfig_source_files}
  ${app_source_dir}/AudioEncoderFAAC.cpp
  ${app_source_dir}/main.cpp
)

set(app_libraries 
 ${streamer_libraries}
 ${portaudio_libraries}
 ${extern_dir}/lib/libfaac.a
)

include_directories(
  ${CMAKE_CURRENT_LIST_DIR}/../../include/
  ${portaudio_include_dirs} 
  ${streamer_include_dirs}
  ${xmlconfig_include_dirs}
  ${tinylib_include_dirs}
  ${extern_include_dirs}
)

add_definitions(${streamer_definitions})

set(CMAKE_INSTALL_PREFIX ${install_dir})

add_executable(${app_name} ${app_source_files})

target_link_libraries(${app_name} ${app_libraries})

install(TARGETS ${app_name} DESTINATION bin)

install(FILES ${app_install_files} DESTINATION bin)

# start the app after building
add_custom_command(TARGET ${app_name} POST_BUILD COMMAND ${app_name})