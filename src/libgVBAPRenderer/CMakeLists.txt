set(headers
  "include/cartesianMath.h"
  "include/gVBAPRenderer.h"
  "include/quickHull.h"
  "include/tables.h"
  "include/vbap_core.h"
)

set(srcs
  "src/cartesianMath.cpp"
  "src/gVBAPRenderer.cpp"
  "src/quickHull.cpp"
  "src/tables.cpp"
  "src/vbap_core.cpp"
)

target_sources(mpeghdec PRIVATE ${headers} ${srcs})
target_include_directories(mpeghdec PRIVATE include)