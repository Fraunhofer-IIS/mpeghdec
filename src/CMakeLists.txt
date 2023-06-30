set(all_libraries
  MpeghDec
  MpeghUIMan
  MpegTPDec
  PCMutils
  IGFdec
  ArithCoding
  FormatConverter
  gVBAPRenderer
  DRCdec
  UIManager
  FDK
  SYS
)

set(lib_prefix "lib")

set(available_libraries)
foreach(library IN LISTS all_libraries)
  if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${lib_prefix}${library}/")
    list(APPEND available_libraries ${library})
  else()
    message(WARNING "Library ${library} does not exist")
  endif()
endforeach()

foreach(library IN LISTS available_libraries)
  file(GLOB headers "${CMAKE_CURRENT_SOURCE_DIR}/${lib_prefix}${library}/include/*.h")
  file(GLOB sources "${CMAKE_CURRENT_SOURCE_DIR}/${lib_prefix}${library}/src/*.cpp")
  file(GLOB internal_headers "${CMAKE_CURRENT_SOURCE_DIR}/${lib_prefix}${library}/src/*.h")
  add_library(${library} STATIC
    ${headers}
    ${sources}
    ${internal_headers}
  )
  target_include_directories(${library}
    PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/${lib_prefix}${library}/include"
    PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/${lib_prefix}${library}/src"
  )
  include_directories("${CMAKE_CURRENT_SOURCE_DIR}/${lib_prefix}${library}/include")
endforeach(library)

FetchContent_MakeAvailable(ilo mmtisobmff)
target_link_libraries(mmtisobmff)

# Target for grouping all libraries belonging to the UI manager
add_library(mpeghUImanagerLibs INTERFACE)
target_link_libraries(mpeghUImanagerLibs INTERFACE MpeghUIMan UIManager MpegTPDec FDK SYS)

add_library(mpeghDecoderLibs INTERFACE)
target_link_libraries(mpeghDecoderLibs INTERFACE MpeghDec MpegTPDec PCMutils IGFdec ArithCoding FormatConverter gVBAPRenderer DRCdec UIManager FDK SYS)
