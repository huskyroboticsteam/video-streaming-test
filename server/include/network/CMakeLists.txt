add_library(websocket_utils STATIC
  websocket/WebSocketServer.cpp
  websocket/WebSocketProtocol.cpp)
target_link_libraries(websocket_utils
  ${BOOST_LIBRARIES}
  websocketpp::websocketpp
  nlohmann_json::nlohmann_json)