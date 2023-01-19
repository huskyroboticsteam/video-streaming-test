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
#include "include/encoding/encoder.hpp"

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
  float fps = static_cast<float>(capture.get(cv::CAP_PROP_FPS));
  // int width = static_cast<int>(capture.get(cv::CAP_PROP_FRAME_WIDTH));
  // int height = static_cast<int>(capture.get(cv::CAP_PROP_FRAME_HEIGHT));
  int width = 640;
  int height = 480;
  std::cout << width << " " << height << " " << fps << std::endl;
  cv::Mat frame;  // stores frames from capture
  auto sleepUntil = std::chrono::steady_clock::now();

  if (argc > 1) {  // h264 encoding
    std::cout << "h264 encoding.";
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
    while (true) {
      if (clientConnected) {
        auto start = std::chrono::high_resolution_clock::now();
        capture >> frame;
        if (frame.empty()) {
          break;
        }

        cv::Mat out;
        cv::resize(frame, out, cv::Size(width, height), cv::INTER_AREA);  // resize frame
        frame = out;

        int frame_size = enc.encode(frame.data, &flag);
        for (auto i = 0; i  < enc.num_nals; i++) {
          json json_data = {
            {"data", std::basic_string<uint8_t>(enc.nals[i].p_payload, enc.nals[i].i_payload)}
          };
          server.sendJSON("/videostream", json_data);
        }
        auto stop = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);
        std::cout << duration.count() << std::endl;
      }
      // sleepUntil += 25ms;
      // std::this_thread::sleep_until(sleepUntil);
    }
    server.stop();
  } else {  // mjpeg encoding
    std::cout << "MJPEG encoding.";
    std::vector<int> params;
    params.push_back(cv::IMWRITE_JPEG_QUALITY);
    params.push_back(1);
    MJPEGStreamer streamer;
    streamer.start(8000);
    // fetch /shutdown to stop
    while (streamer.isRunning()) {
      capture >> frame;
      if (frame.empty()) {
        std::cout << "No frame to be captured";
        // continue;
        break;
      }
      cv::Mat out;
      cv::resize(frame, out, cv::Size(width, height), cv::INTER_AREA);  // resize frame
      frame = out;
      std::vector<uchar> buff_bgr;
      cv::imencode(".jpg", frame, buff_bgr, params);
      std::string data(std::string(buff_bgr.begin(), buff_bgr.end()));
      streamer.publish("/bgr", data);

      // sleepUntil += 40ms;
      // std::this_thread::sleep_until(sleepUntil);
    }
    streamer.stop();
  }
  return 0;
}