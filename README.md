# video-streaming-test
`sudo apt-get install -y build-essential cmake ffmpeg x264 libopencv-dev`
```
git clone https://github.com/nadjieb/cpp-mjpeg-streamer.git;
cd cpp-mjpeg-streamer;
mkdir build && cd build;
cmake ../;
make;
sudo make install;
```
H264 Encoder:
https://github.com/cbachhuber/CppVideoStreamer/tree/master/src

JMuxer:
https://github.com/samirkumardas/jmuxer

Use Chromium for lowest latency