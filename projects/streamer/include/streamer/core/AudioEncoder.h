/* 

   # AudioEncoder

   Encodes raw audio data using liblamemp3. We can either ingest 
   interleaved float or interleaved signed shorts. 

   ````c++
   AudioEncoder ae;

   my_settings.samplerate = AV_AUDIO_SAMPLERATE_44100;
   my_settings.mode = AV_AUDIO_MODE_STEREO;
   my_settings.bitsize = AV_AUDIO_BITSIZE_S16;
   my_settings.bitrate = 96; // 96 kbps
   my_settings.quality = 6;
   my_settings.in_bitsize = AV_AUDIO_BITSIZE_F32; 
   my_settings.in_interleaved = true.

   ae.setup(my_settings);
   ae.initialize();
   
   for(int i = 0; i < 50; ++i) {
      ...
      ae.encodePacket(some_packet, tag);
      ...
   }
   ````
   

 */
#ifndef ROXLU_AUDIO_ENCODER_H
#define ROXLU_AUDIO_ENCODER_H

#include <stdint.h>

extern "C" {
#  include <lame/lame.h>
}

#include <fstream>
#include <streamer/flv/FLVTypes.h>
#include <streamer/core/EncoderTypes.h>

#define AUDIO_ENCODER_BUFFER_SIZE 168384

class AudioEncoder {
 public:
  AudioEncoder();
  ~AudioEncoder();
  bool setup(AudioSettings settings);
  bool initialize();
  bool encodePacket(AVPacket* p, FLVTag& result);
 private:
  int nchannels;
  AudioSettings settings;
  int samplerate;
  MPEG_mode mode;
  lame_global_flags* lame_flags;
  uint8_t mp3_buffer[AUDIO_ENCODER_BUFFER_SIZE];
};

#endif
