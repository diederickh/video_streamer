#include <stdio.h>
#include <streamer/core/EncoderTypes.h>

// -----------------------------------------

AVPacket::AVPacket() 
  :type(AV_TYPE_UNKNOWN)
  ,timestamp(0)
{
}

void AVPacket::allocate(size_t nbytes) {
  data.assign(nbytes, 0x00);
}

// -----------------------------------------

VideoSettings::VideoSettings()
  :width(0)
  ,height(0)
  ,fps(0.0)
{

}

bool VideoSettings::validate() {
  if(!width || !height || !fps) {
#if !defined(NDEBUG)
    printf("videosettings: invalid settings. width = %d, height = %d, fps = %d\n", width, height, fps);
#endif
    return false;
  }
  return true;
}

// -----------------------------------------

AudioSettings::AudioSettings()
  :samplerate(AV_AUDIO_SAMPLERATE_UNKNOWN)
  ,mode(AV_AUDIO_MODE_UNKNOWN)
  ,bitsize(AV_AUDIO_BITSIZE_UNKNOWN)
  ,quality(5) 
  ,bitrate(0)
  ,in_bitsize(AV_AUDIO_BITSIZE_UNKNOWN)
  ,in_interleaved(true)
{
  printf("AudioSettings.quality: not used atm\n");
}

bool AudioSettings::validate() {
  if(samplerate == AV_AUDIO_SAMPLERATE_UNKNOWN) {
#if !defined(NDEBUG)
    printf("audiosettings: no samplerate set.\n");
#endif
    return false;
  }

  if(mode == AV_AUDIO_MODE_UNKNOWN) {
#if !defined(NDEBUG)
    printf("audiosettings: no audio mode set.\n");
    return false;
#endif    
  }

  if(bitsize == AV_AUDIO_BITSIZE_UNKNOWN) {
#if !defined(NDEBUG)
    printf("audiosettings: no bitsize set.\n");
    return false;
#endif
  }

  if(!bitrate) {
#if !defined(NDEBUG)
    printf("audiosettings: no bitrate set.\n");
#endif    
    return false;
  }

  if(in_bitsize == AV_AUDIO_BITSIZE_UNKNOWN) {
    in_bitsize = bitsize; 
  }

  return true;

}

// -----------------------------------------

ServerSettings::ServerSettings() {
}

bool ServerSettings::validate() {
  if(!url.size()) {
#if !defined(NDEBUG)
    printf("serversettings: no url set.\n");
    return false;
#endif    
  }
    return true;
}
// -----------------------------------------
