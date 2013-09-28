/*
  
  # RTMP Thread

  The RTMP Thread is responsible for sending FLVTags to the RTMP server
  as confiruged in the RTMPWriter object you pass into the constructor. 
  
 */
#ifndef ROXLU_RTMP_THREAD_H
#define ROXLU_RTMP_THREAD_H

extern "C" {
#  include <uv.h>
}

#include <vector>
#include <streamer/flv/FLVListener.h>

// @todo - remove graph 
#if defined(USE_GRAPH)
#  include <streamer/utils/Graph.h>
#endif

// ---------------------------------------------------

void rtmp_thread_func(void* user);

struct RTMPData;
class RTMPWriter;
class FLVWriter;

// ---------------------------------------------------

class RTMPThread : public FLVListener {
 public:
  RTMPThread(FLVWriter& flv, RTMPWriter& rtmpWriter);
  ~RTMPThread();
  bool start();
  bool stop();
  void addPacket(RTMPData* pkt);

  /* FLVListener */
  void onTag(BitStream& bs, FLVTag& tag);
  void onSignature(BitStream& bs); 

 public:
  FLVWriter& flv;
  RTMPWriter& rtmp_writer;
  volatile bool must_stop;
  uv_thread_t thread;
  uv_cond_t cv;
  uv_mutex_t mutex;
  std::vector<RTMPData*> work;
};

// ---------------------------------------------------

#endif
