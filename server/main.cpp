#include <iostream>
#include <string>
#include <iterator>
#include <set>
#include <nlohmann/json.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/core/core.hpp>
#include <nadjieb/mjpeg_streamer.hpp>
#include "include/network/websocket/WebSocketServer.h"
#include "include/video/H264Encoder.h"

using namespace nlohmann;
using namespace std::chrono_literals;
using MJPEGStreamer = nadjieb::MJPEGStreamer;

int main(int argc, char** argv) {
  cv::VideoCapture capture;  // used to get the test video, could be camera too
  if (!capture.open(0)) {  // open default camera
  // if (!capture.open("../server/videos/test1.mp4")) {
    std::cout << "Unable to open video!" << std::endl;
    return -1;
  }
  // capture.set(cv::CAP_PROP_FRAME_HEIGHT, 640);
  // capture.set(cv::CAP_PROP_FRAME_HEIGHT, 480);
  capture.set(cv::CAP_PROP_FPS, 30);
  int fps = static_cast<int>(capture.get(cv::CAP_PROP_FPS));
  // int width = static_cast<int>(capture.get(cv::CAP_PROP_FRAME_WIDTH));
  // int height = static_cast<int>(capture.get(cv::CAP_PROP_FRAME_HEIGHT));
  int width = 426;
  int height = 240;

  /**
   * 480p: 854x480
   * 360p: 640x360
   * 240p: 426x240
  */
  std::cout << width << " " << height << " " << fps << std::endl;
  cv::Mat frame;  // stores frames from capture
  auto sleepUntil = std::chrono::steady_clock::now();
  std::multiset<int> frame_sizes;
  long total_frame_sizes = 0;
  
  auto start = std::chrono::high_resolution_clock::now();

  // h264 encoding
  std::cout << "h264 encoding.";
  // Encoder enc(width, height, width, height, fps);
  video::H264Encoder encoder(fps);
  auto img = reinterpret_cast<unsigned char *>(malloc(width * height * 3));

  bool clientConnected = false;
  std::set<std::string> cameras;
  net::websocket::SingleClientWSServer server("test", 3001);
  std::unique_ptr<net::websocket::WebSocketProtocol> protocol = std::make_unique<net::websocket::WebSocketProtocol>("/mission-control");
  protocol->addConnectionHandler([&]() {
    // client connects, signal that
    clientConnected = true;
  });
  protocol->addDisconnectionHandler([&]() {
    auto stop = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);
    clientConnected = false;
    // client disocnnects, signal that
  });
  protocol->addMessageHandler("cameraStreamOpenRequest", [&](json j) {
    cameras.insert(j["camera"]);
    std::cout << "new camera: " << j["camera"] << std::endl;
  });
  protocol->addMessageHandler("cameraStreamCloseRequest", [&](json j) {
    cameras.erase(j["camera"]);
    std::cout << "removing camera: " << j["camera"] << std::endl;
  });
  server.addProtocol(std::move(protocol));
  server.start();
  while(!clientConnected) {
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }
  // wait for signal that client has conected
  bool flag = true;
  start = std::chrono::high_resolution_clock::now();
  while (true) {
    if (clientConnected) {
      capture >> frame;
      if (frame.empty()) {
        break;
      }

      cv::Mat out;
      cv::resize(frame, out, cv::Size(width, height), cv::INTER_AREA);  // resize frame
      frame = out;

      // convert frame to encoded data and send it
      auto data_vector = encoder.encode_frame(frame);
      // std::cout << data_vector.size() << " packets are to be sent." << std::endl;
      for (auto camera : cameras) {
        for (auto data_string : data_vector) { // for each encoded peice of data, send it.
          json json_data;
          json_data["type"] = "cameraStreamReport";
          json_data["camera"] = camera;
          json_data["data"] = data_string;
          // std::cout << "sending data to client" << std::endl;
          server.sendJSON("/mission-control", json_data);
        }
      }
      // std::cout << "finished processing frame" << std::endl;
    }
  }
  server.stop();
  std::cout << "Done streaming.";
  return 0;
}