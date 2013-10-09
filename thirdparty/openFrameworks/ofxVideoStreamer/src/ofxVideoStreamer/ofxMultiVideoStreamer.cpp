#include "ofMain.h"
#include <ofxVideoStreamer/ofxMultiVideoStreamer.h>

ofxMultiVideoStreamer::ofxMultiVideoStreamer() 
  :has_allocated_audio_pool(false)
{
}

ofxMultiVideoStreamer::~ofxMultiVideoStreamer() {
}

bool ofxMultiVideoStreamer::setup(std::string filename, int winW, int winH, int fps) {
  
  if(!mvs.loadSettings(ofToDataPath(filename))) {
    return false;
  }

  if(!mvs.setup()) {
    return false;
  }

  for(size_t i = 0; i < mvs.size(); ++i) {

    MultiStreamerInfo* msi = mvs[i];
    if(!msi) {
      printf("MultiVideoStreamer returned invalid info. Stopping now\n");
      ::exit(EXIT_FAILURE);
    }
 
    if(!grabber.addSize(msi->id, msi->streamer->getVideoWidth(), msi->streamer->getVideoHeight())) {
      printf("Error while adding a size to the YUV420PGrabber. Stopping now.\n");
      ::exit(EXIT_FAILURE);
    }
             
    printf("[%d] %d %d\n", msi->id, msi->streamer->getVideoWidth(), msi->streamer->getVideoHeight());
  }

  if(!grabber.setup(winW, winH, fps)) {
    printf("Error while trying to setup the grabber.\n");
    return false;
  } 

  printf("setup: %s\n", filename.c_str());

  if(!memory_pool.allocateVideoFrames(10, grabber.getNumBytes())) {
    printf("error: cannot allocate the video bytes. maybe out of memory?\n");
    return false;
  }

  return true;
}

bool ofxMultiVideoStreamer::start() {

  if(!mvs.size()) {
    printf("Error, cannot start ofxMultiVideoStreamer because we did not find any streams. Did you call setup() and is your xml correct?.\n");
    return false;
  }

  if(!mvs.start()) {
    return false;
  }

  grabber.start();

  return true;

}

void ofxMultiVideoStreamer::beginGrab() {
  grabber.beginGrab();
}

void ofxMultiVideoStreamer::endGrab() {
  grabber.endGrab();
  grabber.downloadTextures();

  AVPacket* vid = memory_pool.getFreeVideoPacket();
  if(!vid) {
    printf("error: cannot get a free memory packet. this often means that you're encoding too many streams or that you cpu isn't fast enough to keep up with the encoding. try reducing the bitrate, fps, video size.\n");
  }
  else {
    grabber.assignPixels(vid->data);

    vid->clearMulti();
    vid->makeMulti();

    // set the correct strides and plane pointers for each of the streams
    for(size_t i = 0; i < mvs.size(); ++i) {
      MultiStreamerInfo* msi = mvs[i];
      MultiAVPacketInfo info;
      grabber.assignPlanes(msi->id, vid->data, info.planes);
      grabber.assignStrides(msi->id, info.strides);
      vid->addMulti(msi->id, info);
    }

    vid->makeVideoPacket();
    vid->setTimeStamp(grabber.getTimeStamp());

    mvs.addVideo(vid);
  }
}


void ofxMultiVideoStreamer::addAudio(float* input, int nsize, int nchannels) {
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
  mvs.addAudio(pkt);
}
