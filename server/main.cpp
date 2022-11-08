#include <iostream>
#include "include/network/websocket/WebSocketServer.h"

int main() {
  net::websocket::SingleClientWSServer* server = new net::websocket::SingleClientWSServer("test server", 3001);
  server->start();
  int n = -1;
  while (true) {
    if (n == 1) break;
    std::cout << "1 to stop" << std::endl;
    std::cin >> n;
  }
  server->stop();
  return 0;
}