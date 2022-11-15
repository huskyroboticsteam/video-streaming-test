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