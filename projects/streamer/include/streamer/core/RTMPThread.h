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

  
  # RTMP Thread

  The RTMP Thread is responsible for sending FLVTags to the RTMP server
  as configured in the RTMPWriter object you pass into the constructor. 
  
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

#define RTMP_STATE_NONE 0
#define RTMP_STATE_STARTED 1

class RTMPThread : public FLVListener {
 public:
  RTMPThread(FLVWriter& flv, RTMPWriter& rtmpWriter);
  ~RTMPThread();
  bool start();
  bool stop(); /* will shutdown the thread and reset state. we join the thread */
  void addPacket(RTMPData* pkt);

  /* FLVListener */
  void onTag(BitStream& bs, FLVTag& tag);
  void onSignature(BitStream& bs); 

 public:
  FLVWriter& flv;
  RTMPWriter& rtmp_writer;
  uv_thread_t thread;
  uv_cond_t cv;
  uv_mutex_t mutex;
  std::vector<RTMPData*> work;
  int state;
};

// ---------------------------------------------------

#endif
