#include <iostream>
#include <string>
#include <nlohmann/json.hpp>
#include <opencv2/videoio.hpp>
#include "include/network/websocket/WebSocketServer.h"
#include "include/encoding/encoder.hpp"
using namespace nlohmann;
using namespace std::chrono_literals;
// assuming they're the same size
void matToCharArray(cv::Mat, unsigned char *, int, int);

int main() {
  cv::VideoCapture capture;  // used to get the test video, could be camera too
  // capture.open(0);  // open default camera
  if (!capture.open("../server/videos/test1.mp4")) {
    std::cout << "Unable to open video!" << std::endl;
    return -1;
  }
  float fps = static_cast<float>(capture.get(cv::CAP_PROP_FPS));
  int height = static_cast<int>(capture.get(cv::CAP_PROP_FRAME_HEIGHT));
  int width = static_cast<int>(capture.get(cv::CAP_PROP_FRAME_WIDTH));
  cv::Mat frame;  // stores frames from capture

  Encoder enc(width, height, width, height, fps);
  auto img = reinterpret_cast<unsigned char *>(malloc(width * height * 3));

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
  bool flag = true;
  auto sleepUntil = std::chrono::steady_clock::now();
  int total = 0.0;
  int counts = 0;
  while (true) {
    if (clientConnected) {
      capture >> frame;
      if (frame.empty()) {
        break;
      }
      int frame_size = enc.encode(frame.data, &flag);
      for (auto i = 0; i  < enc.num_nals; i++) {
        json json_data = {
          {"data", std::basic_string<uint8_t>(enc.nals[i].p_payload, enc.nals[i].i_payload)}
        };
        counts++;
        total += enc.nals[i].i_payload;
        server.sendJSON("/videostream", json_data);
      }
    }
    sleepUntil += 50ms;
    std::this_thread::sleep_until(sleepUntil);
    // std::this_thread::sleep_for(std::chrono::milliseconds(50));
  }
  server.stop();
  std::cout << "average payload size: " << total / counts << std::endl;
  return 0;
}