cmake_minimum_required(VERSION 2.8)

include(${CMAKE_CURRENT_LIST_DIR}/../../../../../tools/cmake/Triplet.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/../../../../../projects/hwscale/build/cmake/CMakeLists.txt)    # hwscale 
include(${CMAKE_CURRENT_LIST_DIR}/../../../../../projects/streamer/build/cmake/CMakeLists.txt)   # streamer 
include(${CMAKE_CURRENT_LIST_DIR}/../../../../../projects/xmlconfig/build/cmake/CMakeLists.txt)  # xmlconfig

set(ofx_source_dir ${CMAKE_CURRENT_LIST_DIR}/../../src/ofxVideoStreamer/)

set(ofx_source_files
  ${ofx_source_dir}/ofxVideoStreamerScreenCapture.cpp
)

set(CMAKE_INSTALL_PREFIX ${CMAKE_CURRENT_LIST_DIR}/../../../../../install/${tri_triplet}/thirdparty/openframeworks/ofxVideoStreamer/)

install(FILES ${ofx_source_files} DESTINATION src)

install(DIRECTORY ${xmlconfig_source_dirs} DESTINATION src/xmlconfig)

install(DIRECTORY ${streamer_source_dirs} DESTINATION src/streamer)

install(DIRECTORY ${hwscale_source_dirs} DESTINATION src/hwscale)

install(DIRECTORY ${streamer_include_dirs} DESTINATION include/)

install(DIRECTORY ${xmlconfig_include_dirs} DESTINATION include/)

install(DIRECTORY ${hwscale_include_dirs} DESTINATION include/)

install(DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/../../include/ DESTINATION include)

# extern files
set(extern_dirs
  ${extern_dir}/include/lame
  ${extern_dir}/include/librtmp
  ${extern_dir}/include/nanomsg
  ${extern_dir}/include/openssl
)

file(GLOB extern_header_files ${extern_dir}/include/*.h )

file(GLOB extern_hpp_files ${extern_dir}/include/*.hpp)

install(DIRECTORY ${extern_dirs} DESTINATION include)

install(FILES ${extern_hpp_files} DESTINATION include)

install(FILES ${extern_header_files} DESTINATION include)

# install streamer libraries
foreach(lib ${streamer_libraries})
  if(NOT IS_DIRECTORY ${lib})
    install(FILES ${lib} DESTINATION lib)
  endif()
endforeach()

# install hwscale libraries
foreach(lib ${hwscale_libraries})
  if(NOT IS_DIRECTORY ${lib})
    install(FILES ${lib} DESTINATION lib)
  endif()
endforeach()
