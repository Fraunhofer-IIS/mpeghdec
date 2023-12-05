set(headers
  "include/tp_data.h"
  "include/tpdec_lib.h"
)

set(srcs
  "src/tp_version.h"
  "src/tpdec_asc.cpp"
  "src/tpdec_asi.cpp"
  "src/tpdec_lib.cpp"
)

target_sources(mpeghdec PRIVATE ${headers} ${srcs})
target_include_directories(mpeghdec PRIVATE include)