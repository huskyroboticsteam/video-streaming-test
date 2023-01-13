#include <iostream>
#include <string>
#include <iterator>
#include <set>
#include <opencv2/videoio.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/core/core.hpp>
#include <nadjieb/mjpeg_streamer.hpp>

using namespace std::chrono_literals;
using MJPEGStreamer = nadjieb::MJPEGStreamer;

int main() {
  cv::VideoCapture capture;  // used to get the test video, could be camera too
  // if (!capture.open(0)) {  // open default camera
  if (!capture.open("../server/videos/test1.mp4")) {
    std::cout << "Unable to open video!" << std::endl;
    return -1;
  }
  // capture.set(cv::CAP_PROP_FRAME_HEIGHT, 640);
  // capture.set(cv::CAP_PROP_FRAME_HEIGHT, 480);
  float fps = static_cast<float>(capture.get(cv::CAP_PROP_FPS));
  // int width = static_cast<int>(capture.get(cv::CAP_PROP_FRAME_WIDTH));
  // int height = static_cast<int>(capture.get(cv::CAP_PROP_FRAME_HEIGHT));
  int width = 640;
  int height = 480;
  std::cout << width << " " << height << " " << fps << std::endl;
  cv::Mat frame;  // stores frames from capture

  auto sleepUntil = std::chrono::steady_clock::now();
  int total = 0.0;
  int counts = 0;
  std::multiset<int> median;
  std::vector<int> params = {cv::IMWRITE_JPEG_QUALITY, 1};
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

    sleepUntil += 40ms;
    std::this_thread::sleep_until(sleepUntil);
  }
  streamer.stop();
  std::set<int>::iterator it = median.begin();
  std::advance(it, median.size() / 2);
  std::cout << "Average payload size: " << total / counts << std::endl << "Median payload size: " << *it << std::endl;
  return 0;
}