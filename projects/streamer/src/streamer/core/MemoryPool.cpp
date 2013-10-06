#include <streamer/core/MemoryPool.h>
#include <iterator>

MemoryPool::MemoryPool() {
  uv_mutex_init(&video_mutex);
  uv_mutex_init(&audio_mutex);
}

MemoryPool::~MemoryPool() {
  uv_mutex_destroy(&video_mutex);
  uv_mutex_destroy(&audio_mutex);
}

bool MemoryPool::allocateVideoFrames(size_t nframes, uint32_t nbytes) {

  lockVideo();
  {
    for(size_t i = 0; i < nframes; ++i) {
      AVPacket* pkt = new AVPacket(this);
      pkt->allocate(nbytes);
      pkt->makeVideoPacket();
      pkt->refcount = 0; // make sure refcount starts at zero, so it's free
      video_packets.push_back(pkt);
    }
  }
  unlockVideo();

  return true;
}

bool MemoryPool::allocateAudioFrames(size_t nframes, uint32_t nbytes) {

  lockAudio();
  {
    for(size_t i = 0; i < nframes; ++i) {
      AVPacket* pkt = new AVPacket(this);
      pkt->allocate(nbytes);
      pkt->makeAudioPacket();
      pkt->refcount = 0; // make sure refcount starts at zero, so it's free
      audio_packets.push_back(pkt);
    }
  }
  unlockAudio();

  return true;
}

