cmake_minimum_required(VERSION 2.8)

project(Grapher)

add_definitions(
  -DROXLU_WITH_OSX_NATIVE
  -DROXLU_GL_CORE3
)

# Grapher depends on the roxlu lib
include(${CMAKE_CURRENT_LIST_DIR}/../../../../../roxlu/lib/build/cmake/CMakeLists.txt)

roxlu_add_addon(BMFont)
roxlu_add_addon(UV)

option(USE_64BIT "Compile a 64bit version" ON)
option(USE_OPENGL_PROFILER "Compiles an openGL 3.x profiler addon" ON)

if(USE_64BIT)
  set(gr_arch "x86_64")
else()
  set(gr_arch "i386")
endif()

set(gr_app_name "grapher")
set(gr_lib_name "grapher")
set(gr_base_dir ${CMAKE_CURRENT_LIST_DIR}/../../)
set(gr_src_dir ${gr_base_dir}/src)
set(gr_inc_dir ${gr_base_dir}/include)
set(gr_ext_dir ${gr_base_dir}/../../extern/)

set(gr_source_files
  ${gr_src_dir}/grapher/main.cpp
  ${gr_src_dir}/grapher/Graph.cpp
  ${gr_src_dir}/grapher/GraphConfigReader.cpp
  ${gr_src_dir}/grapher/Config.cpp
  ${gr_src_dir}/grapher/GraphDrawer.cpp
  ${gr_src_dir}/grapher/Vertices.cpp
  ${gr_src_dir}/grapher/Shader.cpp
  ${gr_src_dir}/grapher/Daemon.cpp
  ${roxlu_lib_source_files}
)

set(gr_lib_source_files 
  ${gr_src_dir}/grapher/Graph.cpp
  ${gr_src_dir}/grapher/GraphConfigReader.cpp
  ${gr_src_dir}/grapher/Config.cpp
  )

  
if(USE_OPENGL_PROFILER)
  add_definitions(-DUSE_OPENGL_PROFILER)
  list(APPEND gr_source_files ${gr_src_dir}/grapher/opengl/OpenGLProfiler.cpp)
  list(APPEND gr_lib_source_files ${gr_src_dir}/grapher/opengl/OpenGLProfiler.cpp)
endif()

set(CMAKE_INSTALL_PREFIX ${gr_base_dir}/../../install/)
set(CMAKE_OSX_ARCHITECTURES ${gr_arch})

if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
  set(gr_compiler "clang")
elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
  set(gr_compiler "gcc")
elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
  if(MSVC10)
    set(gr_compiler "vs2010")
  else()
    set(gr_compiler "vs2012")
  endif()
endif()

if(APPLE)
  set(gr_platform "mac")
elseif(WIN)
  set(gr_platform "win")
else()
  set(gr_platform "unix")
endif()

set(gr_triplet "${gr_platform}-${gr_compiler}-${gr_arch}")

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  set(gr_triplet "${gr_triplet}d")
  set(gr_app_name "grapher_debug")
endif()

set(gr_extern_include_dir ${gr_ext_dir}/${gr_triplet}/include/)
set(gr_extern_lib_dir ${gr_ext_dir}/${gr_triplet}/lib/)

if(APPLE)
  find_library(fr_opengl OpenGL)
  find_library(fr_iokit IOKit)
  find_library(fr_cf CoreFoundation)
  find_library(fr_cocoa Cocoa)
  list(APPEND gr_libs 
    ${gr_extern_lib_dir}/libglfw3.a
    ${fr_opengl} 
    ${fr_iokit} 
    ${fr_cf}
    ${fr_cocoa}
    ${roxlu_extern_lib_dir}/libnanomsg.a
    ) 
endif()

list(APPEND gr_libs
  ${roxlu_libs}

)

add_definitions(
  -DGLFW_INCLUDE_GLCOREARB
)

include_directories(
  ${gr_extern_include_dir} 
  ${gr_inc_dir}
  ${fr_opengl}/Headers/
)

add_executable(${gr_app_name} ${gr_source_files} )

add_library(${gr_lib_name} STATIC ${gr_lib_source_files})

target_link_libraries(${gr_app_name} ${gr_libs})

install(TARGETS ${gr_app_name} RUNTIME DESTINATION bin  BUNDLE DESTINATION bin)

install(TARGETS ${gr_lib_name} ARCHIVE DESTINATION lib/${gr_triplet}/)

install(DIRECTORY ${gr_inc_dir}/ DESTINATION include)
