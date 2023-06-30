FetchContent_MakeAvailable(ilo mmtisobmff)

add_executable(mpeghDecoder "mpeghDecoder/main_mpeghDecoder.cpp")
target_link_libraries(mpeghDecoder mpeghDecoderLibs SYS mmtisobmff ilo)

add_executable(mpeghUiManager "mpeghUiManager/main_mpeghUiManager.cpp")
target_link_libraries(mpeghUiManager mpeghUImanagerLibs SYS mmtisobmff ilo)
