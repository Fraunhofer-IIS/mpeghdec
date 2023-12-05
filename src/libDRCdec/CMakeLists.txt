set(headers
  "include/FDK_drcDecLib.h"
)

set(srcs
  "src/FDK_drcDecLib.cpp"
  "src/drcDec_gainDecoder.cpp"
  "src/drcDec_gainDecoder.h"
  "src/drcDec_reader.cpp"
  "src/drcDec_reader.h"
  "src/drcDec_rom.cpp"
  "src/drcDec_rom.h"
  "src/drcDec_selectionProcess.cpp"
  "src/drcDec_selectionProcess.h"
  "src/drcDec_tools.cpp"
  "src/drcDec_tools.h"
  "src/drcDec_types.h"
  "src/drcDecoder.h"
  "src/drcGainDec_init.cpp"
  "src/drcGainDec_init.h"
  "src/drcGainDec_multiband.cpp"
  "src/drcGainDec_multiband.h"
  "src/drcGainDec_preprocess.cpp"
  "src/drcGainDec_preprocess.h"
  "src/drcGainDec_process.cpp"
  "src/drcGainDec_process.h"
)

target_sources(mpeghdec PRIVATE ${headers} ${srcs})
target_include_directories(mpeghdec PRIVATE include)
