#include <iostream>
#include <ofxVideoStreamer/ofxVideoStreamerScreenCapture.h>

ofxVideoStreamerScreenCapture::ofxVideoStreamerScreenCapture() 
  :has_new_frame(false)
{
  printf("screen grabber\n");
}

ofxVideoStreamerScreenCapture::~ofxVideoStreamerScreenCapture() {

}

bool ofxVideoStreamerScreenCapture::setup(std::string filename, 
                                          int winW, int winH, 
                                          int vidW, int vidH) 
{

  if(!streamer.loadSettings(ofToDataPath(filename))) {
    printf("error: cannot load the streamer settings.\n");
    return false;
  }

  grabber.addSize(0, vidW, vidH);

  if(!grabber.setup(winW, winH, 25)) {
    printf("error: cannot setup the grabber.\n");
    return false;
  }

  YUV420PSize size = grabber.getSize(0);
  //streamer.setStrides(size.strides[0], size.strides[1], size.strides[2]);
  streamer.setVideoWidth(size.yw);
  streamer.setVideoHeight(size.yh);

  if(!streamer.setup()) {
    printf("error: cannot setup the streamer.\n");
    return false;
  }

  if(!memory_pool.allocateVideoFrames(10, grabber.getNumBytes())) {
    printf("error: cannot setup the memory pool. maybe out of memory?\n");
    return false;
  }

  return true;
}

bool ofxVideoStreamerScreenCapture::start() {

  if(!streamer.start()) {
    printf("error: cannot start the streamer, did you call setup()?.\n");
    return false;
  }

  grabber.start();

  return true;
}

void ofxVideoStreamerScreenCapture::beginGrab() {
  grabber.beginGrab();
}

void ofxVideoStreamerScreenCapture::endGrab() {
  grabber.endGrab();
  grabber.downloadTextures();

  AVPacket* vid = memory_pool.getFreeVideoPacket();
  if(!vid) {
    printf("error: cannot get a free video packet, try to increase the pool size\n");
  }
  else {
    vid->makeVideoPacket();
    vid->setTimeStamp(grabber.getTimeStamp());
    grabber.assignFrame(0, vid->data, vid->planes, vid->strides);
    /*
    vid->data.assign(grabber.getPtr(), grabber.getPtr() + grabber.getNumBytes()); // takes roughly ~0.7-0.9ms for a video texture of 1900800 bytes (1.8mb)

    YUV420PSize size = grabber.getSize(0);
    vid->y_offset = size.y_offset;
    vid->u_offset = size.u_offset;
    vid->v_offset = size.v_offset;
    */
    streamer.addVideo(vid);
  }
}

void ofxVideoStreamerScreenCapture::draw() {
  grabber.draw();
}
                                                                                                                   
