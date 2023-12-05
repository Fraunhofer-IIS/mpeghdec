set(headers
  "include/ac_arith_coder.h"
)

set(srcs
  "src/ac_arith_coder.cpp"
)

target_sources(mpeghdec PRIVATE ${headers} ${srcs})
target_include_directories(mpeghdec PRIVATE include)
