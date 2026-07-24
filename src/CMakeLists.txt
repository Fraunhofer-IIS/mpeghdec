function(ADD_FDK_LIB SUBDIR)
  add_subdirectory(${SUBDIR})
  source_group("${SUBDIR}\\Source Files" REGULAR_EXPRESSION "${SUBDIR}/src")
  source_group("${SUBDIR}\\Header Files" REGULAR_EXPRESSION "${SUBDIR}/include")
  target_sources(mpeghdec PRIVATE "${SUBDIR}/CMakeLists.txt")
  source_group("${SUBDIR}" "${SUBDIR}/CMakeLists.txt")
endfunction()


add_library(mpeghdec)
install(TARGETS mpeghdec)

# ---------------------------------------------------------------------------
# Force-include the symbol-prefix header (see mpeghdec_SYMBOL_PREFIX in the root CMakeLists.txt file).

if(mpeghdec_SYMBOL_PREFIX)
  if(NOT EXISTS "${mpeghdec_SYMBOL_PREFIX_HEADER}")
    message(FATAL_ERROR
      "mpeghdec_SYMBOL_PREFIX is ON but the header was not found:\n"
      "  ${mpeghdec_SYMBOL_PREFIX_HEADER}\n")
  endif()
  if(MSVC)
    target_compile_options(mpeghdec PRIVATE "/FI${mpeghdec_SYMBOL_PREFIX_HEADER}")
  else()
    target_compile_options(mpeghdec PRIVATE "-include" "${mpeghdec_SYMBOL_PREFIX_HEADER}")
  endif()
  message(STATUS "mpeghdec: symbol prefixing enabled via ${mpeghdec_SYMBOL_PREFIX_HEADER}")
endif()

target_include_directories(mpeghdec PUBLIC "${PROJECT_SOURCE_DIR}/include")
target_include_directories(mpeghdec PRIVATE "${PROJECT_SOURCE_DIR}/include/sys")

target_compile_definitions(mpeghdec PRIVATE MPEGHDEC_INTERNAL=1)
get_target_property(MPEGHDEC_TYPE mpeghdec TYPE)
if("${MPEGHDEC_TYPE}" STREQUAL "STATIC_LIBRARY")
  target_compile_definitions(mpeghdec PUBLIC MPEGHDEC_STATIC=1)
endif()
install(FILES "${PROJECT_SOURCE_DIR}/include/mpeghexport.h" DESTINATION "include/mpeghdec")

if(${mpeghdec_BUILD_DECODER})
  target_sources(mpeghdec PUBLIC "${PROJECT_SOURCE_DIR}/include/mpeghdecoder.h")
  install(FILES "${PROJECT_SOURCE_DIR}/include/mpeghdecoder.h" DESTINATION "include/mpeghdec")
  add_fdk_lib(libMpeghDec)
  add_fdk_lib(libDRCdec)
  add_fdk_lib(libPCMutils)
  add_fdk_lib(libIGFdec)
  add_fdk_lib(libArithCoding)
  add_fdk_lib(libFormatConverter)
  add_fdk_lib(libgVBAPRenderer)
endif()

if(${mpeghdec_BUILD_UIMANAGER})
  target_sources(mpeghdec PUBLIC "${PROJECT_SOURCE_DIR}/include/mpeghUIManager.h")
  install(FILES "${PROJECT_SOURCE_DIR}/include/mpeghUIManager.h" DESTINATION "include/mpeghdec")
  add_fdk_lib(libMpeghUIMan)
endif()

add_fdk_lib(libMpegTPDec)
add_fdk_lib(libUIManager)
add_fdk_lib(libFDK)
add_fdk_lib(libSYS)

set(sys_headers
  "${PROJECT_SOURCE_DIR}/include/sys/cmdl_parser.h"
  "${PROJECT_SOURCE_DIR}/include/sys/genericStds.h"
  "${PROJECT_SOURCE_DIR}/include/sys/machine_type.h"
  "${PROJECT_SOURCE_DIR}/include/sys/wav_file.h"
)
target_sources(mpeghdec PUBLIC ${sys_headers})
source_group("Header Files\\sys" REGULAR_EXPRESSION "/include/sys/")
