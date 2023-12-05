set(headers
  "include/FDK_igfDec.h"
  "include/FDK_igfSCFDecoder.h"
)

set(srcs
  "src/FDK_igfDec.cpp"
  "src/FDK_igfSCFDecoder.cpp"
)

target_sources(mpeghdec PRIVATE ${headers} ${srcs})
target_include_directories(mpeghdec PRIVATE include)