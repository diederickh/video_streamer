#ifndef ROXLU_OFXVIDEOSTREAMER_SCREEN_CAPTURE_H
#define ROXLU_OFXVIDEOSTREAMER_SCREEN_CAPTURE_H

#include "ofMain.h"

#include <streamer/videostreamer/VideoStreamer.h>
#include <streamer/core/MemoryPool.h>
#include <hwscale/opengl/YUV420PGrabber.h>
#include <string>

class ofxVideoStreamerScreenCapture {
 public:
  ofxVideoStreamerScreenCapture();
  ~ofxVideoStreamerScreenCapture();

  bool setup(std::string settingsFile, int winW, int winH, int vidW, int vidH);
  bool start();

  bool hasNewFrame();
  void beginGrab();
  void endGrab();
  void draw();

 private:
  VideoStreamer streamer;
  YUV420PGrabber grabber;
  MemoryPool memory_pool;
  bool has_new_frame;
};

inline bool ofxVideoStreamerScreenCapture::hasNewFrame() {
  return grabber.hasNewFrame();
}
#endif
