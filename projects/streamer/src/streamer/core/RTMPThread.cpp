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

  uint32_t time_diff; /* the total millis of packets we have */
  uint32_t time_min = 0;
  uint32_t time_max = 0;
  uint64_t timeout = 0;
  uint64_t delay_us = 2 * 10 * 1000;
  uint64_t time_started = 0;
  uint64_t time_now = 0;
  uint64_t time_d = 0; 
  uint64_t packet_time_max = 0;
  uint64_t buffer_time = 4500;
  bool buffer_empty = true;

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
#if 0
    for(std::vector<RTMPData*>::iterator it = todo.begin(); it != todo.end(); ++it) {
      RTMPData* pkt = *it;
      if(!time_min) {
        time_min = pkt->timestamp;
      }
      if(!time_max) {
        time_max = pkt->timestamp;
      }
      if(pkt->timestamp < time_min) {
        time_min = pkt->timestamp;
      }
      if(pkt->timestamp > time_max){ 
        time_max = pkt->timestamp;
      }
    }

    time_diff = time_max - time_min;
    if(buffer_empty && time_diff >= buffer_time) {
      continue;
    }
    buffer_empty = false;
    
    printf("time min: %d, time max: %d, time_diff: %d\n", time_min, time_max, time_diff);
    if(!time_started) {
      time_started = uv_hrtime() / 1000000;
    }
#endif

    // for(std::vector<RTMPData*>::iterator it = todo.begin(); it != todo.end(); ++it) {
    std::vector<RTMPData*>::iterator it = todo.begin();
    while(it != todo.end()) {
      RTMPData* pkt = *it;

#if 0      
      time_d = (uv_hrtime() / 1000000) - time_started;
      
      if(pkt->timestamp > time_d) {
        // time_max = pkt->timestamp;
        break;
      }
#endif

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

      //printf("--------------------->>>>> $$$\n");
      //rtmp_writer.read();
      //printf("<<<<---------------------- $$$\n");
    }

#if 0
    printf(".. %lld, done: %lld, time_started: %lld, time_max: %d, buffer_time: %lld, time_diff: %lld \n", packet_time_max, ((packet_time_max - time_d)), time_d, time_max, buffer_time, time_diff);
    // todo.clear();
    time_min = time_max; // packet_time_max;
#endif

  }

  printf("rtmp bytes written: %lld\n", bytes_written);
  printf("rtmp work: %ld\n", rtmp.work.size());
}

// ---------------------------------------------------

RTMPThread::RTMPThread(FLVWriter& flv, RTMPWriter& rtmp) 
  :flv(flv)
  ,rtmp_writer(rtmp)
  ,thread(NULL)
  ,must_stop(true)
{
  uv_mutex_init(&mutex);
  uv_cond_init(&cv);
  flv.addListener(this);
}

RTMPThread::~RTMPThread() {

  if(!must_stop) {
    stop();
  }

  // trigger the thread loop/condvar
  RTMPData* pkt = new RTMPData();
  addPacket(pkt);

  uv_thread_join(&thread);
  uv_mutex_destroy(&mutex);
  uv_cond_destroy(&cv);
  must_stop = true;
}

bool RTMPThread::start() {

  if(!must_stop) {
    printf("error: seems like the rtmp thread is already running.\n");
    return false;
  }

  must_stop = false;
  uv_thread_create(&thread, rtmp_thread_func, this);
  return true;
}

bool RTMPThread::stop() {

  if(must_stop) {
    printf("error: seems that we've already stoppped the encoder thread.\n");
    return false;
  }

  must_stop = true;
  return true;
}

// we take ownership of the packet and we will delete delete it!
void RTMPThread::addPacket(RTMPData* pkt) {
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

  RTMPData* pkt = new RTMPData();
  pkt->setTimeStamp(tag.timestamp);
  pkt->putBytes(bs.getPtr(), bs.size());
  addPacket(pkt);

  // and flush..
  // bs.clear();
}

