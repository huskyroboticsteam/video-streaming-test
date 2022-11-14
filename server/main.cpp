#include <iostream>
#include <string>
#include <nlohmann/json.hpp>
#include <opencv2/videoio.hpp>
#include "include/network/websocket/WebSocketServer.h"

int main() {
  cv::VideoCapture capture;  // used to get the test video, could be camera too
  if (!capture.open("../server/videos/test1.mp4")) {
    std::cout << "Unable to open video!" << std::endl;
    return -1;
  }
  int fourcc = cv::VideoWriter::fourcc('H', '2', '6', '4');  // video format,  avc1(mp4/mkv)/H264(mkv)/X264(mkv)
  double fps = capture.get(cv::CAP_PROP_FPS);
  cv::Size frameSize(capture.get(cv::CAP_PROP_FRAME_HEIGHT), capture.get(cv::CAP_PROP_FRAME_WIDTH));
  cv::Mat frame;  // stores frames from capture
  const std::string name("test.mkv");
  cv::VideoWriter writer;  // used turn cv::mat into compressed data
  writer.open(name, fourcc, fps, frameSize);
  // capture.open(0);  // open default camera

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
    capture >> frame;
    // do something!!!
    if (clientConnected) {
      nlohmann::json json_data = nlohmann::json::parse(R"(
        {
          "data": ""
        }
      )");
      server.sendJSON("/videostream", json_data);
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }
  server.stop();
  return 0;
}