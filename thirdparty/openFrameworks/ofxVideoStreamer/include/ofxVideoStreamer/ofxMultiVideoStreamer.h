/*

---------------------------------------------------------------------------------
      
                                               oooo              
                                               `888              
                oooo d8b  .ooooo.  oooo    ooo  888  oooo  oooo  
                `888""8P d88' `88b  `88b..8P'   888  `888  `888  
                 888     888   888    Y888'     888   888   888  
                 888     888   888  .o8"'88b    888   888   888  
                d888b    `Y8bod8P' o88'   888o o888o  `V88V"V8P' 
                                                                 
                                                  www.roxlu.com
                                             www.apollomedia.nl  
                                          www.twitter.com/roxlu
              
---------------------------------------------------------------------------------

  # ofxMultiVideoStreamer
 
*/

#ifndef ROXLU_OFX_MULTI_VIDEO_STREAMER_H
#define ROXLU_OFX_MULTI_VIDEO_STREAMER_H

#include <streamer/core/MemoryPool.h>
#include <streamer/core/Log.h>
#include <streamer/videostreamer/MultiVideoStreamer.h>
#include <hwscale/opengl/YUV420PGrabber.h>

class ofxMultiVideoStreamer {
 public:
  ofxMultiVideoStreamer();
  ~ofxMultiVideoStreamer();
  bool setup(std::string filename, int winW, int winH, int fps);
  bool start();
  bool wantsNewFrame();
  void beginGrab();
  void endGrab();
  void addAudio(float* input, int nsize, int nchannels);
  void update(); /* call this often; this makes sure that we reconnect to the remote server when we get disconnected */
 private:
  MultiVideoStreamer mvs;
  YUV420PGrabber grabber;
  MemoryPool memory_pool;
  bool has_allocated_audio_pool; /* we allocate the audio pool when we receive the first audio frame, @todo move this to setup() */
};

inline bool ofxMultiVideoStreamer::wantsNewFrame() {
  return grabber.hasNewFrame();
}

inline void ofxMultiVideoStreamer::update() {
  mvs.update();
}
#endif
