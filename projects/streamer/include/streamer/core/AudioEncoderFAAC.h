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

  # AudioEncoderFAAC

  Simple wrapper around libfaac to support AAC bsed audio encoding for the streamer.
  This project started out using MP3 for the audio encoding, but it turns out that
  HTTP Live Streamer (HLS) does not work with MP3 on iOS devices. 

  This encoder API work similar to the others, you first make sure that all necessary
  settings are set using setup() (or other functions which might be added later). Then 
  you call initialize() to actually create an encoder instance using all the provided
  settings. Then start feeding audio packets and when ready you call shutdown() to free 
  up all the used memory.

  IMPORTANT: 
  You can use AV_AUDIO_BITSIZE_S16 or AV_AUDIO_BITSIZE_F32. When using F32, we expect
  the values to be scaled into a range of -32768.0f to 32768.0f. If your values are 
  -1.0f to 1.0f, just loop over them and scale them by 32768.0f. Or use SSE

*/


#ifndef ROXLU_VIDEOSTREAMER_AUDIOENCODER_FAAC_H
#define ROXLU_VIDEOSTREAMER_AUDIOENCODER_FAAC_H

#include <streamer/core/Log.h>
#include <streamer/core/EncoderTypes.h>
#include <streamer/core/AudioEncoder.h>
#include <streamer/flv/FLVTypes.h>
#include <faac.h>
#include <string>
#include <stdint.h>
#include <fstream>
#include <vector>

#define FAAC_ENCODER_MEASURE_BITRATE 1  /* when set to 1 we will measure how many kbps the faac encoder produces */

std::string faac_debug_mpeg_version_to_string(int v);
std::string faac_debug_object_type_to_string(int v);
std::string faac_debug_input_format_to_string(int v);
std::string faac_debug_output_format_to_string(int v);

class AudioEncoderFAAC : public AudioEncoder {
 public:
  AudioEncoderFAAC();
  ~AudioEncoderFAAC();
  void setOutputFile(std::string filepath);       /* when you want to write the encoded data to a file, set the output file. */
  bool setup(AudioSettings s);                    /* provide the AudioEncoderFAAC encoder with all the necessary settings; it will return when the settings are invalid OR when libfaac cannot handle them */
  bool initialize();                              /* after specifying all the settings, call initialize to create an instance of the encoder; this allocates memory which should be freed by shutdown, which closes the encoder */
  bool shutdown();                                /* frees all allocated memory */
  void print();                                   /* provide us with some debug info */
  bool encodePacket(AVPacket* p, FLVTag& tag);    /* encode the given data into "tag". When you're using AV_AUDIO_BITSIZE_F32 and your audio is in a range of -1.0f, 1.0f, you need to upscale if by 32768.0f (just a simple loop) */
  unsigned long getSamplesNeededForEncoding();    /* (call this after you've called initialize())  get the number of framers we need to pass into encodePacket(). You should use this value for your audio input callback, so you get this number of samples everytime you want to encode something; or you need to keep a custom buffer and make sure it has enough data in it before encoding it.  */
  std::vector<uint8_t> getAudioSpecificConfig();  /* return the AAC audio specific config. This is e.g. used in the FLV stream, we simple return the result from `faacEncGetDecoderSpecificInfo()` */
 private:
  bool validateSettings(AudioSettings& s);        /* validates the given audio settings so we're sure that libfaac can use them */
  uint8_t getNumChannels();                       /* determines the number of channels that we need to use, base on the settings.mode member. only detect 1 or 2 channels  */
 private:
  unsigned long nsamples_needed;                  /* the number of samples that the faac encoder needs for each encode() call */
  unsigned long nbytes_out;                       /* the total number of bytes that can be produces by a encoder() call */
  unsigned char* faac_buffer;                     /* the buffer into which we write the faac data, is created in initialize(), freed in shutdown() */
  faacEncHandle encoder;
  AudioSettings settings;
  std::string output_file;                        /* when setOutputFile() is called this is set to that value */
  std::ofstream ofs;                              /* used for debugging purposes, when setOutputFile() is used we write the encoded data to this file. */
  std::vector<uint8_t> audio_specific_config;     /* the AAC specific config, used in the FLV bitstream when AAC is used. */

#if FAAC_ENCODER_MEASURE_BITRATE 
  uint64_t kbps_timeout;      /* the timeout - when we will check the current kpbs */
  uint64_t kbps_delay;        /* the time between each interval that we check the current bitrate, in nano sec */
  uint64_t kbps_nbytes;       /* the total bytes generated */
  uint64_t kbps_time_started; /* when we started encoding, first packet */
  double kbps;                /* the actual kbps value */
#endif

};

inline uint8_t AudioEncoderFAAC::getNumChannels() {

#if !defined(NDEBUG)
  if(settings.mode == AV_AUDIO_MODE_UNKNOWN) {
    STREAMER_ERROR("Cannot determine the number of channels in AudioEncoderFAAC because the AudioSettings.mode is set to AV_AUDIO_MODE_UNKNOWN. Did you call setup()?");
    ::exit(EXIT_FAILURE);
  }
#endif

  return (settings.mode ==  AV_AUDIO_MODE_MONO) ? 1 : 2;
}

inline unsigned long AudioEncoderFAAC::getSamplesNeededForEncoding() {
  return nsamples_needed;
}

inline void AudioEncoderFAAC::setOutputFile(std::string filepath) {
  output_file = filepath;
}

inline std::vector<uint8_t> AudioEncoderFAAC::getAudioSpecificConfig() {
  return audio_specific_config;
}

#endif
