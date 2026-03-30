if(USE_PKGCONFIG_DEPS)
  find_package(PkgConfig)
  pkg_check_modules(ILO REQUIRED IMPORTED_TARGET GLOBAL ilo>=2.0)
  add_library(ilo ALIAS PkgConfig::ILO)
  pkg_check_modules(MMTISOBMFF REQUIRED IMPORTED_TARGET GLOBAL mmtisobmff>=1.0)
  add_library(mmtisobmff ALIAS PkgConfig::MMTISOBMFF)
else()
  FetchContent_MakeAvailable(ilo mmtisobmff)
endif()

if(${mpeghdec_BUILD_DECODER})
  add_executable(mpeghDecoder "mpeghDecoder/main_mpeghDecoder.cpp")
  target_link_libraries(mpeghDecoder mpeghdec mmtisobmff ilo)
  if(${mpeghdec_BUILD_UIMANAGER})
    target_compile_definitions(mpeghDecoder PRIVATE BUILD_UIMANAGER=1)
    target_include_directories(mpeghDecoder PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/mpeghUiManager")
    target_sources(mpeghDecoder PRIVATE
      "mpeghUiManager/interactivityScriptParser.h"
      "mpeghUiManager/mpeghUiManagerProcessor.cpp"
      "mpeghUiManager/mpeghUiManagerProcessor.h"
    )
  endif()
endif()

if(${mpeghdec_BUILD_UIMANAGER})
  add_executable(mpeghUiManager
    "mpeghUiManager/main_mpeghUiManager.cpp"
    "mpeghUiManager/interactivityScriptParser.h"
    "mpeghUiManager/mpeghUiManagerProcessor.cpp"
    "mpeghUiManager/mpeghUiManagerProcessor.h"
  )
  target_link_libraries(mpeghUiManager mpeghdec mmtisobmff ilo)
endif()
