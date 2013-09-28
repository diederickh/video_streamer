/*

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

class EncoderThread {
 public:
  EncoderThread(FLVWriter& flv, VideoEncoder& ve, AudioEncoder& ae);
  ~EncoderThread();
  bool start();
  bool stop();
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
  std::vector<AVPacket*> video_packets;

  uv_thread_t congest_thread;
  uv_mutex_t congest_mutex;
};

#endif
