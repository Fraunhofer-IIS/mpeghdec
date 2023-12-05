set(headers
)

set(srcs
  "src/mpeghUIManager.cpp"
)

target_sources(mpeghdec PRIVATE ${headers} ${srcs})
target_include_directories(mpeghdec PRIVATE include)