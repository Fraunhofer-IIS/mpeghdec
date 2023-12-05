set(headers
  "include/aacdecoder_lib.h"
)

set(srcs
  "src/FDK_delay.cpp"
  "src/FDK_delay.h"
  "src/aac_ram.cpp"
  "src/aac_ram.h"
  "src/aac_rom.cpp"
  "src/aac_rom.h"
  "src/aacdec_tns.cpp"
  "src/aacdec_tns.h"
  "src/aacdecoder.cpp"
  "src/aacdecoder.h"
  "src/aacdecoder_lib.cpp"
  "src/block.cpp"
  "src/block.h"
  "src/channel.cpp"
  "src/channel.h"
  "src/channelinfo.cpp"
  "src/channelinfo.h"
  "src/conceal.cpp"
  "src/conceal.h"
  "src/conceal_types.h"
  "src/deque.cpp"
  "src/deque.h"
  "src/fdp.cpp"
  "src/fdp.h"
  "src/ltp_post.cpp"
  "src/ltp_post.h"
  "src/mct.cpp"
  "src/mct.h"
  "src/mpeghdecoder.cpp"
  "src/overlapadd.h"
  "src/stereo.cpp"
  "src/stereo.h"
  "src/ui.cpp"
  "src/ui.h"
  "src/usacdec_const.h"
)

target_sources(mpeghdec PRIVATE ${headers} ${srcs})
target_include_directories(mpeghdec PRIVATE include)