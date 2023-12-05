set(headers
  "include/uiManager.h"
)

set(srcs
  "src/uiManager.cpp"
  "src/uiManagerInternal.h"
  "src/uiPersistence.cpp"
  "src/uiPersistence.h"
  "src/uiXml.cpp"
  "src/uiXml.h"
)

target_sources(mpeghdec PRIVATE ${headers} ${srcs})
target_include_directories(mpeghdec PRIVATE include)