#if !defined(_WIN32)
#  include <unistd.h>
#endif

extern "C" {
#  include <uv.h>
}

#include <iostream>
#include <streamer/core/RTMPThread.h>
#include <streamer/core/RTMPWriter.h>
#include <streamer/flv/FLVWriter.h>

// ---------------------------------------------------

void rtmp_thread_func(void* user) {

  RTMPThread* rtmp_ptr = static_cast<RTMPThread*>(user);
  RTMPThread& rtmp = *rtmp_ptr;
  RTMPWriter& rtmp_writer = rtmp.rtmp_writer;
  std::vector<RTMPData*> todo;

  uint64_t packet_time_max = 0;
  uint64_t bytes_written = 0; // just showing some debug info
  bool must_stop = false;

  double bitrate_now = 0; /* just the current time */
  double bitrate_delay = 1000 * 1000 * 1000; /* used to give you the bitrate - delay in nanoseconds */
  double bitrate_timeout = uv_hrtime() + bitrate_delay;
  double bitrate_kbps = 0; /* bitrate of the last "bitrate_delay" */
  double bitrate_time_started = uv_hrtime();

  while(!must_stop) {

    // get work to process
    uv_mutex_lock(&rtmp.mutex);
    {
      while(rtmp.work.size() == 0) {
        uv_cond_wait(&rtmp.cv, &rtmp.mutex);
      }
      std::copy(rtmp.work.begin(), rtmp.work.end(), std::back_inserter(todo));
      rtmp.work.clear();
    }
    uv_mutex_unlock(&rtmp.mutex);

    std::vector<RTMPData*>::iterator it = todo.begin();
    while(it != todo.end()) {
      RTMPData* pkt = *it;

      // make sure we stop when we get a stop packet
      if(pkt->type == RTMP_DATA_TYPE_STOP) {
        must_stop = true;
        break;
      }


#if defined(USE_GRAPH)
      network_graph["rtmp"] += pkt->data.size();
#endif

      if(!pkt->data.size()) {
        printf("error: zero sized packed!\n");
        ::exit(EXIT_FAILURE);
      }

      packet_time_max = pkt->timestamp;

      bytes_written += pkt->data.size(); // just some debug info

      rtmp_writer.write(&pkt->data.front(), pkt->data.size());
      
      bitrate_now = uv_hrtime();
      if(bitrate_now > bitrate_timeout) {
        bitrate_kbps = ((bytes_written * 8) / 1000.0) / ((uv_hrtime() - bitrate_time_started) / 1000000000); //  / ((uv_hrtime() - bitrate_time_started)); // in millis
        printf("-- kbps: %0.2f, bytes processed: %f \n", bitrate_kbps, double(bytes_written/(1024.0 * 1024.0)));
        bitrate_timeout = bitrate_now + bitrate_delay;
      }

      delete pkt;
      pkt = NULL;
      it = todo.erase(it);
    }
  }

  rtmp.state = RTMP_STATE_NONE;
}

// ---------------------------------------------------

RTMPThread::RTMPThread(FLVWriter& flv, RTMPWriter& rtmp) 
  :flv(flv)
  ,rtmp_writer(rtmp)
  ,thread(NULL)
  ,state(RTMP_STATE_NONE)
{
  uv_mutex_init(&mutex);
  uv_cond_init(&cv);
  flv.addListener(this);
}

RTMPThread::~RTMPThread() {
  if(state == RTMP_STATE_STARTED) {
    stop();
  }

  uv_mutex_destroy(&mutex);
  uv_cond_destroy(&cv);

  state = RTMP_STATE_NONE;
}

bool RTMPThread::start() {

  if(state == RTMP_STATE_STARTED) {
    printf("error: canot start the rtmp thread because we're already running.\n");
    return false;
  }

  state = RTMP_STATE_STARTED;

  uv_thread_create(&thread, rtmp_thread_func, this);
  return true;
}

bool RTMPThread::stop() {

  if(state != RTMP_STATE_STARTED) {
    printf("error: cannot stop the rtmp state because we're not running.\n");
    return false;
  }

  // trigger the thread loop/condvar
  RTMPData* pkt = new RTMPData();
  pkt->type = RTMP_DATA_TYPE_STOP;
  addPacket(pkt);

  uv_thread_join(&thread);

  return true;
}

// we take ownership of the packet and we will delete delete it!
void RTMPThread::addPacket(RTMPData* pkt) {
  assert(pkt);
  
#if !defined(NDEBUG)
  if(pkt->type == RTMP_DATA_TYPE_NONE) {
    printf("error: the RTMPData packet has an invalid type (RTMP_DATA_TYPE_NONE) (RTMPThread)");
    ::exit(EXIT_FAILURE);
  }
#endif
  
  /* @todo - we need to handle each packet else the header won't be sent
  if(state == RTMP_STATE_NONE) {
    printf("error: we're not handling a packet because we're not started.\n");
    return;
  }
  */

  uv_mutex_lock(&mutex);
  {
    work.push_back(pkt);
  }
  uv_cond_signal(&cv);
  uv_mutex_unlock(&mutex);
}


void RTMPThread::onSignature(BitStream& bs) {
  // rtmp does not want the flv signature
}

void RTMPThread::onTag(BitStream& bs, FLVTag& tag) {

  // @todo - we need to handle each packet else the header won't be sent
  /*
  if(state == RTMP_STATE_NONE) {
    printf("error: we're not handling a tag becasue we're not started.\n");
  }
  */

  RTMPData* pkt = new RTMPData();
  pkt->type = RTMP_DATA_TYPE_AV;
  pkt->setTimeStamp(tag.timestamp);
  pkt->putBytes(bs.getPtr(), bs.size());
  addPacket(pkt);
}

