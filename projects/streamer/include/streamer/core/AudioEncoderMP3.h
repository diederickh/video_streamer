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

   # AudioEncoderMP3

   Encodes raw audio data using liblamemp3. We can either ingest 
   interleaved float or interleaved signed shorts. 

   ````c++
   AudioEncoderMP3 ae;

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

#ifndef ROXLU_AUDIO_ENCODER_MP3_H
#define ROXLU_AUDIO_ENCODER_MP3_H

#include <stdint.h>

extern "C" {
#  include <lame/lame.h>
#  include <uv.h>
}

#include <fstream>
#include <streamer/flv/FLVTypes.h>
#include <streamer/core/EncoderTypes.h>
#include <streamer/core/AudioEncoder.h>

#define AUDIO_ENCODER_BUFFER_SIZE 168384

class AudioEncoderMP3 : public AudioEncoder {
 public:
  AudioEncoderMP3();
  ~AudioEncoderMP3();
  bool setup(AudioSettings settings);
  bool initialize();
  bool encodePacket(AVPacket* p, FLVTag& result);
  bool shutdown();
 private:
  int nchannels;
  AudioSettings settings;
  int samplerate;
  MPEG_mode mode;
  lame_global_flags* lame_flags;
  uint8_t mp3_buffer[AUDIO_ENCODER_BUFFER_SIZE];

  /* used to monitor bitrate */
  uint64_t bitrate_time_started; /* when we started with encoding, in nanosec. we've put this in setup() because we don't want to add a check in encodePacket(), this result in a bit less accurate value for the first run. */
  uint64_t bitrate_timeout;  /* when we should calculate the current bitrate again */
  uint64_t bitrate_delay; /* the time between bitrate measurements, in nanosec */
  double bitrate_in_kbps; /* the current bitrate we measured last delay */
  double bitrate_nbytes; /* the total amount of transferred bytes */
  
};

#endif
