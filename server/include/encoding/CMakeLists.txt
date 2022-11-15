add_library(encoder STATIC
  encoder.cpp)
# find_package(x264 REQUIRED)
target_link_libraries(encoder x264 avutil avcodec avformat swscale)
