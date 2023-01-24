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
  int height = 360;

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
      // calculate statistics
      // calculate statistics
      auto stop = std::chrono::high_resolution_clock::now();
      auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);
      std::set<int>::iterator it = frame_sizes.begin();
      int largest = *it;
      std::advance(it, frame_sizes.size() / 2);
      std::cout << "Average packet size: " + std::to_string(total_frame_sizes / frame_sizes.size()) + " bytes." << std::endl;
      std::cout << "Median packet size: " + std::to_string(*it) + " bytes." << std::endl;
      it = frame_sizes.end();
      int lowest = *it;
      std::cout << "Largest packet: " << largest << " Smallest packet: " << lowest << std::endl;
      std::cout << "Packet size range: " + std::to_string(largest - lowest) + " bytes" << std::endl;
      it = frame_sizes.begin();
      int total_bytes = 0;
      while (it != frame_sizes.end()) {
        total_bytes += *it;
        it++;
      }
      std::cout << "Total bytes: " << total_bytes << std::endl;
      std::cout << "Average bandwidth (kb/s): " << total_bytes / duration.count() << std::endl;
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

        int frame_size = enc.encode(frame.data, &flag);
        int size_of = 0;
        for (auto i = 0; i  < enc.num_nals; i++) {
          std::basic_string<uint8_t> data = std::basic_string<uint8_t>(enc.nals[i].p_payload, enc.nals[i].i_payload);
          json json_data = {
            {"data", data}
          };
          total_frame_sizes += data.size();
          size_of += data.size();
          server.sendJSON("/videostream", json_data);
        }

        frame_sizes.insert(size_of);
        // auto stop = std::chrono::high_resolution_clock::now();
        // auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);
        // std::cout << duration.count() << std::endl;
      }
      // sleepUntil += 25ms;
      // std::this_thread::sleep_until(sleepUntil);
    }
    server.stop();
  } else {  // mjpeg encoding
    std::cout << "MJPEG encoding.";
    std::vector<int> params;
    params.push_back(cv::IMWRITE_JPEG_QUALITY);
    params.push_back(5); // 0-100; 5 is the lowest where text is still visible
    MJPEGStreamer streamer;
    streamer.start(8000);
    // fetch /shutdown to stop
    start = std::chrono::high_resolution_clock::now();
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
      frame_sizes.insert(data.size());
      total_frame_sizes += data.size();
      streamer.publish("/bgr", data);

      // sleepUntil += 40ms;
      // std::this_thread::sleep_until(sleepUntil);
    }
    streamer.stop();
  }
  // calculate statistics
  auto stop = std::chrono::high_resolution_clock::now();
  auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);
  std::set<int>::iterator it = frame_sizes.begin();
  int largest = *it;
  std::advance(it, frame_sizes.size() / 2);
  std::cout << "Average packet size: " + std::to_string(total_frame_sizes / frame_sizes.size()) + " bytes." << std::endl;
  std::cout << "Median packet size: " + std::to_string(*it) + " bytes." << std::endl;
  it = frame_sizes.end();
  int lowest = *it;
  std::cout << "Largest packet: " << largest << " Smallest packet: " << lowest << std::endl;
  std::cout << "Packet size range: " + std::to_string(largest - lowest) + " bytes" << std::endl;
  it = frame_sizes.begin();
  int total_bytes = 0;
  while (it != frame_sizes.end()) {
    total_bytes += *it;
    it++;
  }
  std::cout << "Total bytes: " << total_bytes << std::endl;
  std::cout << "Average bandwidth (kb/s): " << total_bytes / duration.count() << std::endl;
  return 0;
}