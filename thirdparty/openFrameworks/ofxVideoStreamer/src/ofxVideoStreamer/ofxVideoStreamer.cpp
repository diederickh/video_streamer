#include <iostream>
#include <ofxVideoStreamer/ofxVideoStreamer.h>

ofxVideoStreamer::ofxVideoStreamer() 
  :has_new_frame(false)
  ,has_allocated_audio_pool(false)
  ,streamer(aac)
{
  printf("screen grabber\n");
}

ofxVideoStreamer::~ofxVideoStreamer() {

}

bool ofxVideoStreamer::setup(std::string filename, 
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

bool ofxVideoStreamer::start() {

  if(!streamer.start()) {
    printf("error: cannot start the streamer, did you call setup()?.\n");
    return false;
  }

  grabber.start();

  return true;
}

void ofxVideoStreamer::beginGrab() {
  grabber.beginGrab();
}

void ofxVideoStreamer::endGrab() {
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
    streamer.addVideo(vid);
  }
}

void ofxVideoStreamer::draw() {
  grabber.draw();
}

void ofxVideoStreamer::addAudio(float* input, int nsize, int nchannels) {

  size_t nbytes =  nsize * sizeof(float) * nchannels;

  // @todo - we're delaying allocation here .. we should do this in setup() but then VideoStreamer must be updated.
  if(!has_allocated_audio_pool) {
    if(!memory_pool.allocateAudioFrames(128, nbytes)) {
      printf("error: cannot allocate audio!\n");
      ::exit(EXIT_FAILURE);
    }

    has_allocated_audio_pool = true;
  }

  AVPacket* pkt = memory_pool.getFreeAudioPacket();
  if(!pkt) {
    printf("error: cannot find a free audio packet, make sure that we've allocated enough.\n");
    return ;
  }

  uint8_t* ptr = (uint8_t*)input;
  pkt->data.assign(ptr, ptr + nbytes);
  pkt->setTimeStamp(grabber.getTimeStamp());
  streamer.addAudio(pkt);
}
