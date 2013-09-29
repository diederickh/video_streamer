# The DeckLink SDK uses .idl files on Windows which are meta files which
# in this case generate a .h and .c file of the decklink api. By including
# the .idl file into the sources, it seems that it gets compiled automagically.
# though, we do need to add the current compile dir to the include paths.

cmake_minimum_required(VERSION 2.8)
project(videostreamer_decklink)

if(WIN32)
  option(USE_64BIT "Use 64bit (not supported on windows)" OFF)
else()
  option(USE_64BIT "Use 64bit (not supported on windows)" ON)
endif()

set(vd_app_name "decklink")
set(vd_lib_name "libdecklink")
set(vd_base_dir ${CMAKE_CURRENT_LIST_DIR}/../../)
set(vd_src_dir ${vd_base_dir}/src/)
set(vd_inc_dir ${vd_base_dir}/include)
set(vd_install_dir ${vd_base_dir}/../../install/)
set(vd_sdk_dir ${vd_base_dir}/sdk/)
set(vd_extern_dir ${vd_base_dir}/../../extern/)

include_directories(
  ${vd_inc_dir}
  ${vd_sdk_dir}
  ${vd_idl_out_dir}
)

# midl.exe only works when the output directory (-out) actually exists
add_custom_target(DeckLinkAPI
  COMMAND midl.exe -nologo -W1 -char signed -env win32 -out "${vd_sdk_dir}" -h DeckLinkAPI.h /iid DeckLinkAPI_i.c ${vd_sdk_dir}/idl/DeckLinkAPI.idl
  )
set_source_files_properties(${vd_sdk_dir}/DeckLinkAPI_i.c PROPERTIES GENERATED TRUE)

set(vd_source_files
  ${vd_src_dir}/decklink/DeckLink.cpp
  ${vd_src_dir}/decklink/DeckLinkCard.cpp
  ${vd_src_dir}/decklink/DeckLinkCaptureCallback.cpp
  ${vd_sdk_dir}/DeckLinkAPI_i.c                            # note that we have set that this is a generated file (see above)
)

set(vd_lib_source_files ${vd_source_files})

list(APPEND vd_source_files ${vd_src_dir}/decklink/main.cpp)

set(vd_libs
  comsuppw.lib  # for <comutil.h> which is used for BSTR > char* conversion (_bstr_t)
)

set(vd_arch "i386")

if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
  set(vd_compiler "clang")
elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
  set(vd_compiler "gcc")
elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
  if(MSVC10)
    set(vd_compiler "vs2010")
  else()
    set(vd_compiler "vs2012")
  endif()
endif()
 
if(APPLE)
  set(vd_platform "mac")
elseif(WIN32)
  set(vd_platform "win")
else()
  set(vd_platform "unix")
endif()

set(vd_triplet "${vd_platform}-${vd_compiler}-${vd_arch}")

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  set(vd_triplet "${vd_triplet}d")
  set(vd_app_name "${vd_app_name}_debug")
endif()

set(vd_extern_inc_dir ${vd_extern_dir}/${vd_triplet}/include/)
set(vd_extern_lib_dir ${vd_extern_dir}/${vd_triplet}/lib/)
set(vd_extern_bin_dir ${vd_extern_dir}/${vd_triplet}/bin/)

list(APPEND vd_libs 
  ${vd_extern_lib_dir}/libuv.lib
  ${vd_extern_lib_dir}/libyuv.lib
)

include_directories(
  ${vd_extern_inc_dir}
  ${vd_decklink_output_dir}
)

set(CMAKE_INSTALL_PREFIX "${vd_install_dir}/${vd_triplet}/")

if(NOT TARGET ${vd_app_name})
  add_executable(${vd_app_name} ${vd_source_files})
  target_link_libraries(${vd_app_name} ${vd_libs})
  install(TARGETS ${vd_app_name} DESTINATION bin)
endif()

if(NOT TARGET ${vd_lib_name})
  add_library(${vd_lib_name} ${vd_lib_source_files})
  
endif()

if(USE_CUSTOM_IDL)
  add_dependencies(${vd_app_name} DeckLinkAPI)
endif()

install(TARGETS ${vd_lib_name} DESTINATION lib)

install(DIRECTORY ${vd_inc_dir}/ DESTINATION include)

install(FILES ${vd_sdk_dir}/DeckLinkAPI.h DESTINATION include)

install(FILES ${vd_sdk_dir}/bin/DeckLinkAPI.dll DESTINATION bin)

if(WIN32) 
#  install(FILES ${vd_extern_bin_dir}/libuv.dll DESTINATION bin)
endif()

