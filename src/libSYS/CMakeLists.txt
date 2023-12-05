set(headers
  "include/FDK_audio.h"
)

set(srcs
  "src/genericStds.cpp"
  "src/cmdl_parser.cpp"
  "src/wav_file.cpp"
)

target_sources(mpeghdec PRIVATE ${headers} ${srcs})
target_include_directories(mpeghdec PRIVATE include)