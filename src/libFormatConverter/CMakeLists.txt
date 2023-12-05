set(headers
  "include/FDK_dmxMatrixParser.h"
  "include/FDK_formatConverter.h"
  "include/FDK_formatConverterLib.h"
  "include/FDK_formatConverter_api.h"
  "include/FDK_formatConverter_constants.h"
  "include/FDK_formatConverter_data.h"
  "include/FDK_formatConverter_init.h"
  "include/FDK_formatConverter_intern.h"
  "include/FDK_formatConverter_process.h"
)

set(srcs
  "src/FDK_dmxMatrixParser.cpp"
  "src/FDK_formatConverter.cpp"
  "src/FDK_formatConverterLib.cpp"
  "src/FDK_formatConverter_activeDmx_stft.cpp"
  "src/FDK_formatConverter_activeDmx_stft.h"
  "src/FDK_formatConverter_constants.cpp"
  "src/FDK_formatConverter_constants_stft.cpp"
  "src/FDK_formatConverter_constants_stft.h"
  "src/FDK_formatConverter_data.cpp"
  "src/FDK_formatConverter_init.cpp"
  "src/FDK_formatConverter_process.cpp"
)

target_sources(mpeghdec PRIVATE ${headers} ${srcs})
target_include_directories(mpeghdec PRIVATE include)