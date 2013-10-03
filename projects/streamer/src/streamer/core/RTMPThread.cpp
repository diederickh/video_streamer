#if !defined(_WIN32)
#  include <unistd.h>
#endif
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

#if 0
  uint32_t time_diff; /* the total millis of packets we have */
  uint32_t time_min = 0;
  uint32_t time_max = 0;
  uint64_t timeout = 0;
  uint64_t delay_us = 2 * 10 * 1000;
  uint64_t time_started = 0;
  uint64_t time_now = 0;
  uint64_t time_d = 0; 

  uint64_t buffer_time = 4500;
  bool buffer_empty = true;
#endif

  uint64_t packet_time_max = 0;
  uint64_t bytes_written = 0; // just showing some debug info

  while(!rtmp.must_stop) {

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

#if defined(USE_GRAPH)
      network_graph["rtmp"] += pkt->data.size();
#endif

      if(!pkt->data.size()) {
        printf("error: zero sized packed!\n");
        ::exit(EXIT_FAILURE);
      }

      packet_time_max = pkt->timestamp;

      static int i = 0;
      ++i;
      if(i > 50) {
        printf("rtmp mbytes written: %f\n", double(bytes_written/(1024.0 * 1024.0)));
        i = 0;
      }

      bytes_written += pkt->data.size(); // just some debug info
      rtmp_writer.write(&pkt->data.front(), pkt->data.size());
      delete pkt;
      pkt = NULL;
      it = todo.erase(it);
    }

  }

  rtmp.state = RTMP_STATE_NONE;
  printf("::::::::::::::::::::::: RTMP STATE RESET ! ::::::::::::::::n\n");
  printf("rtmp bytes written: %lld\n", bytes_written);
  printf("rtmp work: %ld\n", rtmp.work.size());
}

// ---------------------------------------------------

RTMPThread::RTMPThread(FLVWriter& flv, RTMPWriter& rtmp) 
  :flv(flv)
  ,rtmp_writer(rtmp)
  ,thread(NULL)
  ,must_stop(true)
  ,state(RTMP_STATE_NONE)
{
  uv_mutex_init(&mutex);
  uv_cond_init(&cv);
  flv.addListener(this);
}

RTMPThread::~RTMPThread() {

  if(!must_stop) {
    stop();
  }

  uv_mutex_destroy(&mutex);
  uv_cond_destroy(&cv);
  must_stop = true;
}

bool RTMPThread::start() {

  printf(">>>>>> START RTMP THREAD <<<<<\n");
  if(state == RTMP_STATE_STARTED) {
    printf("error: canot start the rtmp thread because we're already running.\n");
    return false;
  }

  if(!must_stop) {
    printf("error: seems like the rtmp thread is already running.\n");
    return false;
  }

  state = RTMP_STATE_STARTED;

  must_stop = false;

  uv_thread_create(&thread, rtmp_thread_func, this);
  return true;
}

bool RTMPThread::stop() {

  if(state != RTMP_STATE_STARTED) {
    printf("error: cannot stop the rtmp state because we're not running.\n");
    return false;
  }

  if(must_stop) {
    printf("error: seems that we've already stoppped the encoder thread.\n");
    return false;
  }

  must_stop = true;

  // trigger the thread loop/condvar
  RTMPData* pkt = new RTMPData();
  addPacket(pkt);

  uv_thread_join(&thread);

  return true;
}

// we take ownership of the packet and we will delete delete it!
void RTMPThread::addPacket(RTMPData* pkt) {

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
  // bs.clear();
}

void RTMPThread::onTag(BitStream& bs, FLVTag& tag) {

  // @todo - we need to handle each packet else the header won't be sent
  /*
  if(state == RTMP_STATE_NONE) {
    printf("error: we're not handling a tag becasue we're not started.\n");
  }
  */

  RTMPData* pkt = new RTMPData();
  pkt->setTimeStamp(tag.timestamp);
  pkt->putBytes(bs.getPtr(), bs.size());
  addPacket(pkt);

  // and flush..
  // bs.clear();
}

