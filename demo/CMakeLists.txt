FetchContent_MakeAvailable(ilo mmtisobmff)

if(${mpeghdec_BUILD_DECODER})
  add_executable(mpeghDecoder "mpeghDecoder/main_mpeghDecoder.cpp")
  target_link_libraries(mpeghDecoder mpeghdec mmtisobmff ilo)
endif()

if(${mpeghdec_BUILD_UIMANAGER})
  add_executable(mpeghUiManager
    "mpeghUiManager/main_mpeghUiManager.cpp"
    "mpeghUiManager/interactivityScriptParser.h"
  )
  target_link_libraries(mpeghUiManager mpeghdec mmtisobmff ilo)
endif()
