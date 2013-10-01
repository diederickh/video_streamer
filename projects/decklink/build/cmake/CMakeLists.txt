cmake_minimum_required(VERSION 2.8)

# ${decklink_dependencies}    - VERY IMPORTANT TO ADD! - add this to: add_dependencies(YOUR_APP ${decklink_dependencies})
# ${decklink_source_files}    - compile these sources
# ${decklink_libraries}       - link with these libraries
# ${decklink_include_dirs}    - add this to your include dirs

include(${CMAKE_CURRENT_LIST_DIR}/../../../../tools/cmake/Triplet.cmake)

set(decklink_source_dir ${CMAKE_CURRENT_LIST_DIR}/../../src/)

set(decklink_source_files
  ${decklink_source_dir}/decklink/DeckLink.cpp
  ${decklink_source_dir}/decklink/DeckLinkCard.cpp
  ${decklink_source_dir}/decklink/DeckLinkCaptureCallback.cpp
  )

if(WIN32)

  # midl.exe only works when the output directory (-out) actually exists
  #set(vd_sdk_dir ${CMAKE_CURRENT_BINARY_DIR}/)
  add_custom_target(DeckLinkAPI
    COMMAND midl.exe -nologo -W1 -char signed -env win32 -out "${CMAKE_CURRENT_BINARY_DIR}" -h DeckLinkAPI.h /iid DeckLinkAPI_i.c ${CMAKE_CURRENT_LIST_DIR}/../../sdk/idl/DeckLinkAPI.idl
    )

  set_source_files_properties(${CMAKE_CURRENT_BINARY_DIR}/DeckLinkAPI_i.c PROPERTIES GENERATED TRUE)

  list(APPEND decklink_source_files  ${CMAKE_CURRENT_BINARY_DIR}/DeckLinkAPI_i.c)                            # note that we have set that this is a generated file (see above)

  # for <comutil.h> which is used for BSTR > char* conversion (_bstr_t)
  set(decklink_libraries
    comsuppw.lib  
    )
endif()

if(APPLE)
                                                                               
  list(APPEND decklink_source_files ${CMAKE_CURRENT_LIST_DIR}/../../sdk/include/DeckLinkAPIDispatch.cpp)

endif()


set(decklink_include_dirs 
  ${CMAKE_CURRENT_LIST_DIR}/../../include
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_LIST_DIR}/../../sdk/include/  # unices put the sdk here
  )

set(decklink_dependencies DeckLinkAPI)
