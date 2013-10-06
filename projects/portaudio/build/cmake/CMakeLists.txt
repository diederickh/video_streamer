cmake_minimum_required(VERSION 2.8)

# ${portaudio_include_dirs}    -  include directories, add these to you include dirs
# ${portaudio_source_files}    -  source files of hwscale, compile these
# ${portaudio_libraries}       -  link with these libraries
# ${portaudio_source_dirs}     -  use this with install hen you want to package hwscaler, see the openframeworks addon

include(${CMAKE_CURRENT_LIST_DIR}/../../../../tools/cmake/Triplet.cmake)  # include triplet

set(portaudio_include_dirs ${CMAKE_CURRENT_LIST_DIR}/../../include/)

set(portaudio_source_dir ${CMAKE_CURRENT_LIST_DIR}/../../src)

set(portaudio_source_dirs ${portaudio_source_dir}/portaudio/)

set(portaudio_source_files
  ${portaudio_source_dir}/portaudio/PAudio.cpp
)

if(APPLE)
  find_library(fr_audio CoreAudio)
  find_library(fr_audio_unit AudioUnit)
  find_library(fr_audio_toolbox AudioToolbox)
  find_library(fr_core_services CoreServices)
  
  set(portaudio_libraries
   ${fr_audio}
   ${fr_audio_unit}
   ${fr_audio_toolbox}
   ${fr_core_services}
   ${extern_dir}/lib/libportaudio.a
   )

endif()

