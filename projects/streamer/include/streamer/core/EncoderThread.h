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


  # EncoderThread

  Accepts raw YUV420 and raw audio buffers as input which is will encode 
  into FLVTags. A listener will be called from this thread when 
  a new FLVTag has been generated; this thread creates a FLV bitstream.

 */

#ifndef ROXLU_ENCODER_THREAD_H
#define ROXLU_ENCODER_THREAD_H

extern "C" {
#  include <uv.h>
}

#include <vector>
#include <streamer/core/EncoderTypes.h>

class FLVWriter;
class VideoEncoder;
class AudioEncoder;

void encoder_congestion_thread_func(void* user);
void encoder_thread_func(void* user);

// ------------------------------------------------------------------

#define ENCT_STATE_NONE 0
#define ENCT_STATE_STARTED 1

class EncoderThread {
 public:
  EncoderThread(FLVWriter& flv, VideoEncoder& ve, AudioEncoder& ae);
  ~EncoderThread();
  bool start();
  bool stop(); /* stops the thread and joins it - this blocks */
  void addPacket(AVPacket* pkt); /* add a packet that needs to be encoded */
 public:
  FLVWriter& flv;
  AudioEncoder& audio_encoder;
  VideoEncoder& video_encoder;
  volatile bool must_stop;
  uv_thread_t thread;
  uv_cond_t cv;
  uv_mutex_t mutex;
  std::vector<AVPacket*> work;
  int state;
};


#endif
