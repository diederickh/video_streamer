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

*/
#ifndef ROXLU_ENCODER_TYPES_H
#define ROXLU_ENCODER_TYPES_H

#define AV_TYPE_UNKNOWN 0
#define AV_TYPE_VIDEO 1
#define AV_TYPE_AUDIO 2

#define AV_AUDIO_SAMPLERATE_UNKNOWN 0
#define AV_AUDIO_SAMPLERATE_11025 11025
#define AV_AUDIO_SAMPLERATE_22050 22050
#define AV_AUDIO_SAMPLERATE_44100 44100

#define AV_AUDIO_MODE_UNKNOWN 0
#define AV_AUDIO_MODE_MONO 1
#define AV_AUDIO_MODE_STEREO 2

#define AV_AUDIO_BITSIZE_UNKNOWN 0
#define AV_AUDIO_BITSIZE_S8 1         
#define AV_AUDIO_BITSIZE_S16 2
#define AV_AUDIO_BITSIZE_F32 3 

#include <stdint.h>
#include <iterator>
#include <algorithm>
#include <vector>
#include <string>

// -----------------------------------------

class MemoryPool;

struct AVPacket {
  AVPacket(MemoryPool* mp);     /* An AVPacket can be part of a MemoryPool, which is used to preallocate frames and reuse frames that are not used anymore but this is not necessary, pass NULL when you don't want ot use  a memory pool */
  ~AVPacket();                  /* cleans up ;-) */
  void print();                 /* print some debug info */
  void makeVideoPacket();
  void makeAudioPacket();
  void setTimeStamp(uint32_t ts);
  
  void addRef(); /* call addRef when you want to hold on to this data for a while, when ready call Release */
  void release();  /* call Release when you don't use this packet anymore, so the memory pool can reuse  it */
  
  void copy(uint8_t* buf, size_t nbytes); /* copy the given bytes to `data` */

  void allocate(size_t nbytes);  /* make sure that the data member can hold `nbytes` of data */
  uint8_t type;                  /* either AV_TYPE_VIDEO or AV_TYPE_AUDIO */
  uint32_t timestamp;            /* timestamp that will be used by the FLVTag; this is the timestamp on which the data for this packet was genearted in millis, started with 0 */
  std::vector<uint8_t> data;     /* the actual RAW video or audio data that will be encoded */
  uint8_t* planes[3];            /* pointer to the planes in `data` */
  uint32_t strides[3];           /* strides of the Y,U,V planes in `data` */
  MemoryPool* memory_pool;       /* the memory pool to which this packet belongs */
  uint32_t refcount;             /* when addRef() is called this gets incremented, release() decrements it  (through memory pool) */
};

// -----------------------------------------

struct VideoSettings {
  VideoSettings();
  void print();           /* print some debug info */
  bool validate();        /* returns if all settings have been set correctly */
  uint16_t width;         /* width of the incoming frames */
  uint16_t height;        /* height of the incoming frames */
  uint8_t fps;            /* framerate, e.g. 60 */
  uint32_t bitrate;       /* preferred bitrate in kbps, setting vbv_buffer_size as bitrate control  */
  uint16_t threads;       /* number of encoding threads - sets the i_threads x264 parameter */
};

// -----------------------------------------

struct AudioSettings {
  AudioSettings();
  void print();            /* print some debug info */
  bool validate();         /* validates the settings; if false is returned we cannot use them */
  uint32_t samplerate;     /* e.g. AV_AUDIO_SAMPLERATE_44100 */
  uint8_t mode;            /* e.g. AV_AUDIO_MODE_STEREO */
  uint8_t bitsize;         /* e.g. AV_AUDIO_BITSIZE_S16, the output format as supported by flash */
  uint8_t quality;         /* quality to use, value between 0 and 9, 0 = best (slow), 9 = worst (fast), 5 is ok */
  uint16_t bitrate;        /* bitrate in kilobits */
  uint8_t in_bitsize;      /* e.g. AV_AUDIO_BITSIZE_S16, the format of the input data as passed to the audio encoder, when not specified we will use `bitsize`  */
  bool in_interleaved;     /* set to true when the input data is interleaved. defaults to true*/
};

// -----------------------------------------

struct ServerSettings {
  ServerSettings();
  bool validate();     /* returns true when all members have been set correctly */
  void print();        /* print debug info */
  std::string url;
  std::string username; /* the username you can use for protected write streams */
  std::string password; /* the password you can use for protected write streams */
};

// -----------------------------------------

inline void AVPacket::makeAudioPacket() {
  type = AV_TYPE_AUDIO;
}

inline void AVPacket::makeVideoPacket() {
  type = AV_TYPE_VIDEO;
}

inline void AVPacket::setTimeStamp(uint32_t ts) {
  timestamp = ts;
}

inline void AVPacket::copy(uint8_t* buf, size_t nbytes) {
  std::copy(buf, buf+nbytes, std::back_inserter(data));
}



#endif
