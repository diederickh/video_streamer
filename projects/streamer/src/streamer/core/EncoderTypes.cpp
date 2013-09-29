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
    printf("videosettings: invalid settings. width = %d, height = %d, fps = %d\n", width, height, fps);
    return false;
  }
  return true;
}

void VideoSettings::print() {
  printf("video_settings.width :%d\n", width);
  printf("video_settings.height :%d\n", height);
  printf("video_settings.fps :%d\n", fps);
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

void AudioSettings::print() {
  printf("audio_settings.samplerate: %d\n", samplerate);
  printf("audio_settings.mode: %d\n", mode);
  printf("audio_settings.bitsize: %d\n", bitsize);
  printf("audio_settings.quality: %d\n", quality);
  printf("audio_settings.bitrate: %d\n", bitrate);
  printf("audio_settings.in_bitsize: %d\n", in_bitsize);
  printf("audio_settings.in_interleaved: %d\n", in_interleaved);
}

bool AudioSettings::validate() {
  if(samplerate == AV_AUDIO_SAMPLERATE_UNKNOWN) {
    printf("audiosettings: no samplerate set.\n");
    return false;
  }

  if(mode == AV_AUDIO_MODE_UNKNOWN) {
    printf("audiosettings: no audio mode set.\n");
    return false;
  }

  if(bitsize == AV_AUDIO_BITSIZE_UNKNOWN) {
    printf("audiosettings: no bitsize set.\n");
    return false;
  }

  if(!bitrate) {
    printf("audiosettings: no bitrate set.\n");
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
    printf("serversettings: no url set.\n");
    return false;
  }
  return true;
}

void ServerSettings::print() {
  printf("server_settings.url: %s\n", url.c_str());
}

// -----------------------------------------
