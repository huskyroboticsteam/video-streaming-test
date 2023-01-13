cmake_minimum_required(VERSION 3.13)

PROJECT(test LANGUAGES C CXX)

find_package(Boost REQUIRED COMPONENTS system)
find_package(OpenCV 4.2 REQUIRED)
find_package(nadjieb_mjpeg_streamer REQUIRED)

add_executable(main server/main.cpp)
target_link_libraries(main PRIVATE nadjieb_mjpeg_streamer::nadjieb_mjpeg_streamer ${OpenCV_LIBS})
include_directories(${OpenCV_INCLUDE_DIRS})
target_compile_features(main PRIVATE cxx_std_11)