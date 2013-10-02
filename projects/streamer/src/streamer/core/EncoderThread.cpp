// @todo - cleanup the encoder thread
#include <iostream>
#include <algorithm>
#include <iterator>
#include <streamer/flv/FLVWriter.h>
#include <streamer/core/VideoEncoder.h>
#include <streamer/core/AudioEncoder.h>
#include <streamer/core/EncoderThread.h>

#define USE_CV 1  // use condition var for threading 
// -------------------------------------------------
// @todo - remove / cleanup!!
void encoder_congestion_thread_func(void* user) {
  return;
  EncoderThread* enc_ptr = static_cast<EncoderThread*>(user);
  EncoderThread& enc = *enc_ptr;
  VideoEncoder& vid_enc = enc.video_encoder;

  uint64_t delay = enc.video_encoder.getFPS() * 1000 * 1000; 
  uint64_t timeout = uv_hrtime() + delay;
  uint64_t now = 0;
  uint64_t nframes = 0;

  while(!enc.must_stop) {
    now = uv_hrtime();
    if(now >= timeout) {
      timeout = uv_hrtime() + delay;
      nframes++;
      printf("-------------------------------------------> FRAME: %d <---------------------------------  \n", enc.video_encoder.getFPS());
    }
  }

  printf("==================================================\n");
  printf("frames created: %lld\n", nframes);
  printf("==================================================\n");

}

// -------------------------------------------------

void encoder_thread_func(void* user) {
  EncoderThread* enc_ptr = static_cast<EncoderThread*>(user);
  EncoderThread& enc = *enc_ptr;
  VideoEncoder& vid_enc = enc.video_encoder;
  AudioEncoder& audio_enc = enc.audio_encoder;
  FLVWriter& flv = enc.flv;
  FLVTag flv_tag;

  uint64_t last_timestamp = 0;
  uint64_t nframes = 0;
  std::vector<AVPacket*> todo;
  
  while(!enc.must_stop) {

    // get work to process
    uv_mutex_lock(&enc.mutex);
    {
#if USE_CV      
      while(enc.work.size() == 0) {
        uv_cond_wait(&enc.cv, &enc.mutex);
      }
#endif
      std::copy(enc.work.begin(), enc.work.end(), std::back_inserter(todo));
      enc.work.clear();
    }
    uv_mutex_unlock(&enc.mutex);
    // process the new work
    for(std::vector<AVPacket*>::iterator it = todo.begin(); it != todo.end(); ++it) {
      AVPacket& pkt = **it;

      // packets must be 100% ascending (cannot sort because "older" packets might be added after the current "todo" buffer)
      if(pkt.timestamp <= last_timestamp) {
        pkt.release();
        //delete* it;
        continue;
      }

      if(pkt.type == AV_TYPE_VIDEO) {
        if(vid_enc.encodePacket(&pkt, flv_tag)) {
          nframes++;
          flv.writeVideoTag(flv_tag);
        }
      }
      else if(pkt.type == AV_TYPE_AUDIO) {
        if(audio_enc.encodePacket(&pkt, flv_tag)) { 
          flv.writeAudioTag(flv_tag);
        }
      }
      else {
        printf("- error: EncoderThread cannot handle a AVPacket with type: %d\n", pkt.type);
      }
      last_timestamp = pkt.timestamp;

      pkt.release();
      //delete *it; 
    }

    todo.clear();
  }

  printf("todo size: %ld\n", todo.size());
  printf("work size: %ld\n", enc.work.size());
  printf("++++++++++++++++++++++++++++++++++++++++++++++++++++++\n");
  printf("frames created: %lld\n", nframes);
  printf("++++++++++++++++++++++++++++++++++++++++++++++++++++++\n");

  uv_mutex_lock(&enc.mutex);
    enc.work.clear();
    enc.video_packets.clear();
  uv_mutex_unlock(&enc.mutex);
}


// -------------------------------------------------

EncoderThread::EncoderThread(FLVWriter& flv, VideoEncoder& venc, AudioEncoder& aenc) 
  :flv(flv)
  ,audio_encoder(aenc)
  ,video_encoder(venc)
  ,thread(NULL)
  ,must_stop(true)
{
  uv_mutex_init(&mutex);
  uv_mutex_init(&congest_mutex);
#if USE_CV
  uv_cond_init(&cv);
#endif
}

EncoderThread::~EncoderThread() {

  if(!must_stop) {
    stop();
  }

  // we must trigger the thread conditional loop 
  AVPacket* stop_pkt = new AVPacket(NULL);
  addPacket(stop_pkt);
  
  // cleanup
  uv_thread_join(&thread);
  uv_mutex_destroy(&mutex);
#if USE_CV
  uv_cond_destroy(&cv);
#endif
  must_stop = true;                       
}

void EncoderThread::join() {
  uv_thread_join(&thread);  
}

bool EncoderThread::start() {

  if(!must_stop) {
    printf("error: seems like the encoder thread is already/still running.\n");
    return false;
  }

  must_stop = false;
  uv_thread_create(&thread, encoder_thread_func, this);
  uv_thread_create(&congest_thread, encoder_congestion_thread_func, this);
  return true;
}

bool EncoderThread::stop() {

  if(must_stop) {
    printf("error: seems that we've already stoppped the encoder thread.\n");
    return false;
  }

  must_stop = true;
  return true;
}

// we take ownership of the packet and we will delete delete it!
void EncoderThread::addPacket(AVPacket* pkt) {
#if USE_CONGEST
  if(pkt->type == AV_TYPE_VIDEO) {
    uv_mutex_lock(&congest_mutex);
    video_packets.push_back(pkt);
    uv_mutex_unlock(&congest_mutex);
    return;
  }
#endif

  uv_mutex_lock(&mutex);
  {
    work.push_back(pkt);
  }

#if USE_CV
  uv_cond_signal(&cv);
#endif

  uv_mutex_unlock(&mutex);
}
