// @todo - cleanup the encoder thread
#include <iostream>
#include <algorithm>
#include <iterator>
#include <streamer/flv/FLVWriter.h>
#include <streamer/core/VideoEncoder.h>
#include <streamer/core/AudioEncoder.h>
#include <streamer/core/EncoderThread.h>

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

      while(enc.work.size() == 0) {
        uv_cond_wait(&enc.cv, &enc.mutex);
      }

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
    }

    todo.clear();
  }

  printf("todo size: %ld\n", todo.size());
  printf("work size: %ld\n", enc.work.size());
  printf("++++++++++++++++++++++++++++++++++++++++++++++++++++++\n");
  printf("frames created: %lld\n", nframes);
  printf("++++++++++++++++++++++++++++++++++++++++++++++++++++++\n");

  // reset state when thread stops
  uv_mutex_lock(&enc.mutex);
    enc.work.clear();
    enc.state = ENCT_STATE_NONE; 
    enc.flv.close();
  uv_mutex_unlock(&enc.mutex);

  enc.audio_encoder.shutdown();
  enc.video_encoder.shutdown();
}

// -------------------------------------------------

EncoderThread::EncoderThread(FLVWriter& flv, VideoEncoder& venc, AudioEncoder& aenc) 
  :flv(flv)
  ,audio_encoder(aenc)
  ,video_encoder(venc)
  ,thread(NULL)
  ,must_stop(true)
  ,state(ENCT_STATE_NONE)
{
  uv_mutex_init(&mutex);
  uv_cond_init(&cv);
}

EncoderThread::~EncoderThread() {

  if(!must_stop) {
    stop();
  }
 
  // cleanup
  uv_thread_join(&thread);
  uv_mutex_destroy(&mutex);
  uv_cond_destroy(&cv);

  must_stop = true;      

  state = ENCT_STATE_NONE;
}

bool EncoderThread::start() {
  
  if(state == ENCT_STATE_STARTED) {
    printf("error: already started! first stop().\n");
    return false;
  }

  if(!must_stop) {
    printf("error: seems like the encoder thread is already/still running.\n");
    return false;
  }

  must_stop = false;

  uv_thread_create(&thread, encoder_thread_func, this);

  state = ENCT_STATE_STARTED;

  return true;
}

bool EncoderThread::stop() {

  if(must_stop) {
    printf("error: seems that we've already stoppped the encoder thread.\n");
    return false;
  }

  must_stop = true;

  // we must trigger the thread conditional loop 
  AVPacket* stop_pkt = new AVPacket(NULL);
  addPacket(stop_pkt);

  uv_thread_join(&thread);  

  return true;
}

// we take ownership of the packet and we will delete delete it!
void EncoderThread::addPacket(AVPacket* pkt) {

  if(state == ENCT_STATE_NONE) {
    printf("warning: not adding a packet to the encoder thread because the thread is not running.\n");
    return;
  }

  uv_mutex_lock(&mutex);
  {
    work.push_back(pkt);
  }
  uv_cond_signal(&cv);

  uv_mutex_unlock(&mutex);
}
