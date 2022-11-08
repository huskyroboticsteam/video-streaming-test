#include <iostream>
#include <nlohmann/json.hpp>
#include "include/network/websocket/WebSocketServer.h"

int main() {
  bool clientConnected = false;
  net::websocket::SingleClientWSServer server("test", 3001);
  std::unique_ptr<net::websocket::WebSocketProtocol> protocol = std::make_unique<net::websocket::WebSocketProtocol>("/videostream");
  protocol->addConnectionHandler([&]() {
    // client connects, signal that
    clientConnected = true;
  });
  protocol->addDisconnectionHandler([&]() {
    clientConnected = false;
    // client disocnnects, signal that
  });
  server.addProtocol(std::move(protocol));
  server.start();
  while(!clientConnected) {
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }

  // wait for signal that client has conected

  while (true) {
    if (clientConnected) {
      nlohmann::json json_data = nlohmann::json::parse(R"(
        {
          "data": "base64string"
        }
      )");
      server.sendJSON("/videostream", json_data);
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }
  server.stop();
  return 0;
}