cmake_minimum_required(VERSION 2.8)

project(VideoStreamer)

option(USE_GRAPH "Generate graphs to inspect the encoder (audio/video) and network i/o" ON)

if(WIN32)
  option(USE_64BIT "Compile a 64bit version (else we build 32bit)" OFF)
else()
  option(USE_64BIT "Compile a 64bit version (else we build 32bit)" ON)
endif()

if(USE_64BIT)
  set(vs_arch "x86_64")
else()
  set(vs_arch "i386")
endif()

set(vs_app_name video_streamer)
set(vs_lib_name streamer)
set(vs_base_dir ${CMAKE_CURRENT_LIST_DIR}/../../)
set(vs_src_dir ${vs_base_dir}/src/streamer/)
set(vs_inc_dir ${vs_base_dir}/include)
set(vs_extern_dir ${vs_base_dir}/../../extern)

set(CMAKE_INSTALL_PREFIX ${vs_base_dir}/../../install/)
set(CMAKE_OSX_ARCHITECTURES ${vs_arch})

include(${vs_base_dir}/../xmlconfig/build/cmake/CMakeLists.txt) # XMLConfig

include_directories(${vs_base_dir}/include)

add_definitions(
  -DVIDEO_STREAMER_LITTLE_ENDIAN
)

if(USE_GRAPH)
  add_definitions(-DUSE_GRAPH)
endif()

set(vs_source_files
  ${vs_src_dir}/amf/AMF0.cpp
  ${vs_src_dir}/amf/types/AMFType.cpp
  ${vs_src_dir}/amf/types/AMF0EcmaArray.cpp
  ${vs_src_dir}/amf/types/AMF0Number.cpp
  ${vs_src_dir}/amf/types/AMF0Boolean.cpp
  ${vs_src_dir}/amf/types/AMF0String.cpp
  ${vs_src_dir}/amf/types/AMF0Property.cpp
  ${vs_src_dir}/amf/types/AMF0Object.cpp
  ${vs_src_dir}/flv/FLV.cpp
  ${vs_src_dir}/flv/FLVReader.cpp
  ${vs_src_dir}/flv/FLVWriter.cpp
  ${vs_src_dir}/flv/FLVFileWriter.cpp
  ${vs_src_dir}/flv/FLVTypes.cpp
  ${vs_src_dir}/core/BitStream.cpp
  ${vs_src_dir}/core/Debug.cpp
  ${vs_src_dir}/core/VideoEncoder.cpp
  ${vs_src_dir}/core/AudioEncoder.cpp
  ${vs_src_dir}/core/EncoderTypes.cpp
  ${vs_src_dir}/core/TestPattern.cpp
  ${vs_src_dir}/core/H264Parser.cpp
  ${vs_src_dir}/core/RTMPWriter.cpp
  ${vs_src_dir}/core/EncoderThread.cpp
  ${vs_src_dir}/core/RTMPThread.cpp
  ${vs_src_dir}/daemon/Daemon.cpp
  ${vs_src_dir}/daemon/DaemonConfig.cpp
  ${vs_src_dir}/daemon/Runner.cpp
  ${vs_src_dir}/daemon/Channel.cpp
  ${vs_src_dir}/videostreamer/VideoStreamer.cpp
)

if(USE_GRAPH)
  list(APPEND vs_source_files "${vs_src_dir}/utils/Graph.cpp")
endif()

if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
  set(vs_compiler "clang")
elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
  set(vs_compiler "gcc")
elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
  if(MSVC10)
    set(vs_compiler "vs2010")
  else()
    set(vs_compiler "vs2012")
  endif()
endif()

if(APPLE)
  set(vs_platform "mac")
elseif(WIN32)
  set(vs_platform "win")
else()
  set(vs_platform "unix")
endif()

set(vs_triplet "${vs_platform}-${vs_compiler}-${vs_arch}")

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  set(vs_triplet "${vs_triplet}d")
  set(vs_app_name "video_streamer_debug")
endif()

set(vs_extern_include_dir ${vs_extern_dir}/${vs_triplet}/include/)
set(vs_extern_lib_dir ${vs_extern_dir}/${vs_triplet}/lib/)
set(vs_extern_bin_dir ${vs_extern_dir}/${vs_triplet}/bin/)

include_directories(${vs_extern_include_dir})

add_executable(${vs_app_name} ${vs_src_dir}/main.cpp ${vs_source_files})

add_library(${vs_lib_name} STATIC ${vs_source_files})

add_dependencies(${vs_lib_name} xmlconfig)

# libraries
if(APPLE)
  find_library(fr_foundation CoreFoundation)
  find_library(fr_cs CoreServices)
  set(vs_libs
    ${vs_extern_lib_dir}/libx264.a
    ${vs_extern_lib_dir}/libmp3lame.a
    ${vs_extern_lib_dir}/librtmp.a
    ${vs_extern_lib_dir}/libssl.a
    ${vs_extern_lib_dir}/libcrypto.a
    ${vs_extern_lib_dir}/libz.a
    ${vs_extern_lib_dir}/libuv.a
    ${vs_extern_lib_dir}/libnanomsg.a
    ${fr_foundation}
    ${fr_cs}
    )
endif()

if(WIN32)
  add_definitions(-DWIN32_LEAN_AND_MEAN)
  set(vs_libs
    ${vs_extern_lib_dir}/libx264.dll.a
    ${vs_extern_lib_dir}/libuv.lib
    ${vs_extern_lib_dir}/zdll.lib
    ${vs_extern_lib_dir}/libmp3lame.lib
    ${vs_extern_lib_dir}/librtmp.lib
    ${vs_extern_lib_dir}/libeay32.lib
    ${vs_extern_lib_dir}/ssleay32.lib
    ${vs_extern_lib_dir}/nanomsg.lib
    ws2_32.lib
    psapi.lib
    iphlpapi.lib
    Winmm.lib  # for librtmp timeGetTime
    ${vs_base_dir}/../../install/lib/${vs_triplet}/xmlconfig.lib
    )

  set(vs_extern_install_files
    ${vs_extern_bin_dir}/libeay32.dll
    ${vs_extern_bin_dir}/libmp3lame.dll
    ${vs_extern_bin_dir}/libuv.dll
    ${vs_extern_bin_dir}/libx264-138.dll
    ${vs_extern_bin_dir}/nanomsg.dll
    ${vs_extern_bin_dir}/ssleay32.dll
    ${vs_extern_bin_dir}/zlib1.dll
    )

  install(FILES ${vs_extern_install_files} DESTINATION bin)

endif()

# targets
target_link_libraries(${vs_app_name} ${vs_libs})

install(TARGETS ${vs_app_name} RUNTIME DESTINATION bin  BUNDLE DESTINATION bin  )

install(TARGETS ${vs_lib_name} ARCHIVE DESTINATION lib/${vs_triplet}/)

install(DIRECTORY ${vs_inc_dir}/ DESTINATION include)
