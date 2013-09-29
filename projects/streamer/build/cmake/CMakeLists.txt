cmake_minimum_required(VERSION 2.8)

# ${streamer_source_files}  - compile these source files
# ${streamer_libraries}     - link with these
# ${streamer_include_dirs}  - add these to your include directories
# ${streamer_install_files} - add these to your install command (FILES)
# ${streamer_definitions}   - add these to your preprocessor definitions

include(${CMAKE_CURRENT_LIST_DIR}/../../../../tools/cmake/Triplet.cmake)

set(streamer_source_dir ${CMAKE_CURRENT_LIST_DIR}/../../src/streamer)

set(streamer_source_files
  ${streamer_source_dir}/amf/AMF0.cpp
  ${streamer_source_dir}/amf/types/AMFType.cpp
  ${streamer_source_dir}/amf/types/AMF0EcmaArray.cpp
  ${streamer_source_dir}/amf/types/AMF0Number.cpp
  ${streamer_source_dir}/amf/types/AMF0Boolean.cpp
  ${streamer_source_dir}/amf/types/AMF0String.cpp
  ${streamer_source_dir}/amf/types/AMF0Property.cpp
  ${streamer_source_dir}/amf/types/AMF0Object.cpp
  ${streamer_source_dir}/flv/FLV.cpp
  ${streamer_source_dir}/flv/FLVReader.cpp
  ${streamer_source_dir}/flv/FLVWriter.cpp
  ${streamer_source_dir}/flv/FLVFileWriter.cpp
  ${streamer_source_dir}/flv/FLVTypes.cpp
  ${streamer_source_dir}/core/BitStream.cpp
  ${streamer_source_dir}/core/Debug.cpp
  ${streamer_source_dir}/core/VideoEncoder.cpp
  ${streamer_source_dir}/core/AudioEncoder.cpp
  ${streamer_source_dir}/core/EncoderTypes.cpp
  ${streamer_source_dir}/core/TestPattern.cpp
  ${streamer_source_dir}/core/H264Parser.cpp
  ${streamer_source_dir}/core/RTMPWriter.cpp
  ${streamer_source_dir}/core/EncoderThread.cpp
  ${streamer_source_dir}/core/RTMPThread.cpp
  ${streamer_source_dir}/daemon/Daemon.cpp
  ${streamer_source_dir}/daemon/DaemonConfig.cpp
  ${streamer_source_dir}/daemon/Runner.cpp
  ${streamer_source_dir}/daemon/Channel.cpp
  ${streamer_source_dir}/videostreamer/VideoStreamer.cpp
  ${streamer_source_dir}/videostreamer/VideoStreamerConfig.cpp
)

set(streamer_include_dirs ${CMAKE_CURRENT_LIST_DIR}/../../include/)

set(streamer_definitions -DVIDEO_STREAMER_LITTLE_ENDIAN)

if(APPLE)
  find_library(fr_foundation CoreFoundation)
  find_library(fr_cs CoreServices)
  set(streamer_libraries
    ${extern_dir}/lib/libx264.a
    ${extern_dir}/lib/libmp3lame.a
    ${extern_dir}/lib/librtmp.a
    ${extern_dir}/lib/libssl.a
    ${extern_dir}/lib/libcrypto.a
    ${extern_dir}/lib/libz.a
    ${extern_dir}/lib/libuv.a
    ${extern_dir}/lib/libnanomsg.a
    ${fr_foundation}
    ${fr_cs}
    )
endif()

if(WIN32)

  set(streamer_libraries
    ${extern_dir}/lib/libx264-138.lib
    ${extern_dir}/lib/libuv.lib
    ${extern_dir}/lib/zdll.lib
    ${extern_dir}/lib/libmp3lame.lib
    ${extern_dir}/lib/librtmp.lib
    ${extern_dir}/lib/libeay32.lib
    ${extern_dir}/lib/ssleay32.lib
    ${extern_dir}/lib/nanomsg.lib
    ws2_32.lib
    psapi.lib
    iphlpapi.lib
    Winmm.lib  # for librtmp timeGetTime
    )

  set(streamer_install_files
    ${extern_dir}/bin/libeay32.dll
    ${extern_dir}/bin/libmp3lame.dll
    ${extern_dir}/bin/libuv.dll
    ${extern_dir}/bin/libx264-138.dll
    ${extern_dir}/bin/nanomsg.dll
    ${extern_dir}/bin/ssleay32.dll
    ${extern_dir}/bin/zlib1.dll
    )

endif()

list(APPEND streamer_install_files ${CMAKE_CURRENT_LIST_DIR}/videostreamer_config_example.xml)
