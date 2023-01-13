# cmake_minimum_required(VERSION 3.13)

# PROJECT(test LANGUAGES C CXX)

# find_package(Boost REQUIRED COMPONENTS system)
# find_package(OpenCV 4.2 REQUIRED)
# find_package(nadjieb_mjpeg_streamer REQUIRED)

# add_executable(main server/main.cpp)
# target_link_libraries(main PRIVATE nadjieb_mjpeg_streamer::nadjieb_mjpeg_streamer ${OpenCV_LIBS})
# include_directories(${OpenCV_INCLUDE_DIRS})
# target_compile_features(main PRIVATE cxx_std_11)

cmake_minimum_required(VERSION 3.13)

PROJECT(test LANGUAGES C CXX)

find_package(websocketpp REQUIRED)
find_package(Boost REQUIRED COMPONENTS system)
find_package(nlohmann_json 3.2.0 REQUIRED)
find_package(OpenCV REQUIRED)

add_subdirectory(server/include/network)
add_subdirectory(server/include/encoding)
add_executable(main server/main.cpp)
target_link_libraries(main websocket_utils encoder ${OpenCV_LIBS})