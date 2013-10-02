cmake_minimum_required(VERSION 2.8)

# ${hwscale_definitions}     -  add these to your preprocessor definitions
# ${hwscale_include_dirs}    -  include directories, add these to you include dirs
# ${hwscale_source_files}    -  source files of hwscale, compile these
# ${hwscale_libraries}       -  link with these libraries
# ${hwscale_source_dirs}     -  use this with install hen you want to package hwscaler, see the openframeworks addon

include(${CMAKE_CURRENT_LIST_DIR}/../../../../tools/cmake/Triplet.cmake)  # include triplet

set(hwscale_include_dirs ${CMAKE_CURRENT_LIST_DIR}/../../include/)

set(hwscale_source_dir ${CMAKE_CURRENT_LIST_DIR}/../../src)

set(hwscale_source_dirs ${hwscale_source_dir}/hwscale/)

set(hwscale_source_files
  ${hwscale_source_dir}/hwscale/opengl/ResizerFBO.cpp
  ${hwscale_source_dir}/hwscale/opengl/YUV420PGrabber.cpp
)

set(hwscale_definitions
#  -DGLFW_INCLUDE_GLCOREARB   # tmp disabled for the decklink streamer on mac @todo can we leave it out?
  -DYUV420P_GRABBER_GLSL_VERSION=150
)

if(APPLE)
  find_library(fr_opengl OpenGL)
  find_library(fr_iokit IOKit)
  find_library(fr_cf CoreFoundation)
  find_library(fr_cocoa Cocoa)

  set(hwscale_libraries
    ${extern_dir}/lib/libglfw3.a
    ${extern_dir}/lib/libuv.a
    ${extern_dir}/lib/libnanomsg.a
    ${fr_opengl} 
    ${fr_iokit} 
    ${fr_cf}
    ${fr_cocoa}
#    ${fr_opengl}/Headers/
    )
endif()

