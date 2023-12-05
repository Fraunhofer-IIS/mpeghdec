function(ADD_FDK_LIB SUBDIR)
  add_subdirectory(${SUBDIR})
  source_group("${SUBDIR}\\Source Files" REGULAR_EXPRESSION "${SUBDIR}/src")
  source_group("${SUBDIR}\\Header Files" REGULAR_EXPRESSION "${SUBDIR}/include")
  target_sources(mpeghdec PRIVATE "${SUBDIR}/CMakeLists.txt")
  source_group("${SUBDIR}" "${SUBDIR}/CMakeLists.txt")
endfunction()


add_library(mpeghdec STATIC)

target_include_directories(mpeghdec PUBLIC "${PROJECT_SOURCE_DIR}/include")
target_include_directories(mpeghdec PRIVATE "${PROJECT_SOURCE_DIR}/include/sys")

if(${mpeghdec_BUILD_DECODER})
  target_sources(mpeghdec PUBLIC "${PROJECT_SOURCE_DIR}/include/mpeghdecoder.h")
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