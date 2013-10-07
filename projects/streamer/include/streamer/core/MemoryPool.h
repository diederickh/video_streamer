#ifndef ROXLU_MEMORY_POOL_H
#define ROXLU_MEMORY_POOL_H
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

  
  # MemoryPool

  This "MemoryPool" is used to preallocate N-video/audio frames. Because 
  we cannot allocate to much memory while running we need to preallocate a 
  bunch of frames and reuse this. 

  The memory pool works closely together with the AVPackets. A AVPacket is 
  passed into a encoder thread. When the encoder is ready with encoding the 
  packet, it will call "release()" which decreases the refcount. Multiple encoder
  threads can share the same AVPacket: BUT YOU NEED TO CALL `addRef` FOR EACH 
  ENCODER THREAD YOURSELF BEFORE ADDING TO THE STREAMER/ENCODER INSTANCE. USE
  `getFreeVideo/AudioPacket()` TO GET A FREE PACKET. THE RETURNED PACKET WILL
  HAVE A REFCOUNT OF 1 BY DEFAULT. IF YOU WANT TO PASS THIS PACKET TO MULTIPLE
  ENCODER THREADS YOU NEED TO CALL `addRef()` FOR EACH OF THE OTHER ENCODER
  THREADS. SO IF YOU HAVE 5 ENCODER THREAD, YOU USE `getFreeVideo/AudioPacket()`
  WHICH RETURNS AN `AVPacket` WITH REFCOUNT 1, SO YOU NEED TO CALL `addRef()` 
  4 TIMES!!

*/

extern "C" {
#  include <uv.h>
}

#include <stdint.h>
#include <streamer/core/EncoderTypes.h>
#include <vector>

class MemoryPool {
 public:
  MemoryPool();
  ~MemoryPool();

  AVPacket* getFreeVideoPacket(); /* get a free video packet from the pool: IMPORTANT: refcount will be set to 1 for you! If you need to share it with multiple encoder call: pkt->addRef(NUMBER_OF_SHARED_ENCODERS) */
  AVPacket* getFreeAudioPacket();
  
  bool allocateVideoFrames(size_t nframes, uint32_t nbytes); /* allocate nframes which will contain nbytes of video data */
  bool allocateAudioFrames(size_t nframes, uint32_t nbytes); /* allocate nframes which will contain nbytes of audio data */

  /* lock the appropriate mutexes */
  void lockVideo();
  void unlockVideo();
  void lockAudio();
  void unlockAudio();

  /* ref counting for the AVPackets */
  void release(AVPacket* pkt);
  void addRef(AVPacket* pkt, int count = 1); /* you can add multiple refcounts by change the count, this is handy when you want to share a AVPacket with multiple encoder threads */

 private:
  AVPacket* getFreePacket(uint8_t type);

 private: 
  uv_mutex_t audio_mutex;
  uv_mutex_t video_mutex;
  std::vector<AVPacket*> video_packets;
  std::vector<AVPacket*> audio_packets;
};

inline void MemoryPool::lockVideo() {
  uv_mutex_lock(&video_mutex);
}

inline void MemoryPool::unlockVideo() {
  uv_mutex_unlock(&video_mutex);
}

inline void MemoryPool::lockAudio() {
  uv_mutex_lock(&audio_mutex);
}

inline void MemoryPool::unlockAudio() {
  uv_mutex_unlock(&audio_mutex);
}

inline void MemoryPool::release(AVPacket* pkt) {

  if(pkt->type == AV_TYPE_VIDEO) {
    lockVideo();

#if !defined(NDEBUG)
    if(pkt->refcount == 0) {
      printf("error: trying to release a packet which refcount is zero already! \n");
      ::exit(EXIT_FAILURE);
    }
#endif
    pkt->refcount--;

    unlockVideo();
  }
  else if(pkt->type == AV_TYPE_AUDIO) {

    lockAudio();
#if !defined(NDEBUG)
    if(pkt->refcount == 0) {
      printf("error: trying to release a packet which refcount is zero already!\n");
      ::exit(EXIT_FAILURE);
    }
#endif    

    pkt->refcount--;

    unlockAudio();
  }
}

inline void MemoryPool::addRef(AVPacket* pkt, int count) {

  if(pkt->type == AV_TYPE_VIDEO) {

    lockVideo();
      pkt->refcount += count;
    unlockVideo();

  }
  else if(pkt->type == AV_TYPE_AUDIO) {

    lockAudio();
      pkt->refcount += count;
    unlockAudio();

  }
}

inline AVPacket* MemoryPool::getFreeAudioPacket() {
  return getFreePacket(AV_TYPE_AUDIO);
}

inline AVPacket* MemoryPool::getFreeVideoPacket() {
  return getFreePacket(AV_TYPE_VIDEO);
}

/* 
   Get a free AVPacket for the given type. 

   IMPORTANT: when we find a free packet will set the 
   refcount to 1 so you don't need to call `AVPacket::addRef()`
   anymore. We need to do this because there might be some other
   thread that uses this getFreePacket() and it might return the
   same packet when we did not set the refcount to 1.


 */
inline AVPacket* MemoryPool::getFreePacket(uint8_t type) {

  AVPacket* result = NULL;

  if(type == AV_TYPE_VIDEO) {
    lockVideo();
    {
      for(std::vector<AVPacket*>::iterator it = video_packets.begin(); it != video_packets.end(); ++it) {
        AVPacket* pkt = *it;
        if(pkt->refcount == 0) {
          result = pkt;
          result->refcount = 1;  // WE ADD A REFCOUNT FOR YOU!!
          break;
        }
      }
    }
    unlockVideo();
  }
  else if(type == AV_TYPE_AUDIO) {
    lockAudio();
    {
      for(std::vector<AVPacket*>::iterator it = audio_packets.begin(); it != audio_packets.end(); ++it) {
        AVPacket* pkt = *it;
        if(pkt->refcount == 0) {
          result = pkt;
          result->refcount = 1;  // WE ADD A REFCOUNT FOR YOU!!
          break;
        }
      }
    }
    unlockAudio();
  }

  return result;
}


#endif
