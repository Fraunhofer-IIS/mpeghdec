set(headers
  "include/limiter.h"
  "include/pcm_utils.h"
)

set(srcs
  "src/limiter.cpp"
  "src/pcm_utils.cpp"
  "src/version.h"
)

target_sources(mpeghdec PRIVATE ${headers} ${srcs})
target_include_directories(mpeghdec PRIVATE include)