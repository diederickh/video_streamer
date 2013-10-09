#include <stdio.h>
#include <streamer/core/EncoderTypes.h>
#include <streamer/core/MemoryPool.h>
#include <streamer/core/Log.h>

// -----------------------------------------

MultiAVPacketInfo::MultiAVPacketInfo() {
  planes[0] = planes[1] = planes[2] = NULL;
  strides[0] = strides[1] = strides[2] = 0;
}

// -----------------------------------------

AVPacket::AVPacket(MemoryPool* mp) 
  :type(AV_TYPE_UNKNOWN)
  ,timestamp(0)
  ,memory_pool(mp)
  ,is_multi(false)
{
  planes[0] = planes[1] = planes[2] = NULL;
  strides[0] = strides[1] = strides[2] = 0;
}

void AVPacket::print() {
  STREAMER_VERBOSE("AVPacket.strides[0]: %d\n", strides[0]);
  STREAMER_VERBOSE("AVPacket.strides[1]: %d\n", strides[1]);
  STREAMER_VERBOSE("AVPacket.strides[2]: %d\n", strides[2]);
}

AVPacket::~AVPacket() {

  printf("AVPacket::~AVPacket()\n");

  type = AV_TYPE_UNKNOWN;
  timestamp = 0;
  planes[0] = planes[1] = planes[2] = NULL;
  strides[0] = strides[1] = strides[2] = 0;
  is_multi = false;
  multi_info.clear();
}

void AVPacket::allocate(size_t nbytes) {
  data.assign(nbytes, 0x00);
}

void AVPacket::addRef(int count) {

  if(!memory_pool) {
    STREAMER_WARNING("warning: trying to refcount an AVPacket which does not have a memory pool! - maybe a stop packet?\n");
    return;
  }

  memory_pool->addRef(this, count);
}

void AVPacket::release() {

  // @todo - when a AVPacket is not part of a memory pool we need to have a way to cleanly free the memory ..
  // @todo - ... this is e.g. used in the encoder thread when we add a stop packet, this stop packets needs to be deleted which we do herea
  if(!memory_pool) {
    STREAMER_WARNING("warning: trying to refcount an AVPacket which does not have a memory pool! - maybe a stop packet?\n");
    STREAMER_WARNING("warning: we are going to delete ourself!\n");
    delete this;
    return;
  }

  memory_pool->release(this);
}

// -----------------------------------------

VideoSettings::VideoSettings()
  :width(0)
  ,height(0)
  ,fps(0.0)
  ,bitrate(0)
  ,threads(1)
  ,vbv_buffer_size(-1)
  ,vbv_max_bitrate(-1)
  ,keyint_max(-1)
  ,bframe(-1)
  ,level_idc(-1)
{

}

bool VideoSettings::validate() {

  if(!width || !height || !fps) {
    STREAMER_ERROR("videosettings: invalid settings. width = %d, height = %d, fps = %d\n", width, height, fps);
    return false;
  }
  
  if(!bitrate) {
    STREAMER_ERROR("videosettings: no bitrate set.\n");
    return false;
  }

  return true;
}

void VideoSettings::print() {
  STREAMER_VERBOSE("video_settings.width :%d\n", width);
  STREAMER_VERBOSE("video_settings.height :%d\n", height);
  STREAMER_VERBOSE("video_settings.fps :%d\n", fps);
}

// -----------------------------------------

AudioSettings::AudioSettings()
  :codec_id(AV_AUDIO_CODEC_UNKNOWN)
  ,samplerate(AV_AUDIO_SAMPLERATE_UNKNOWN)
  ,mode(AV_AUDIO_MODE_UNKNOWN)
  ,bitsize(AV_AUDIO_BITSIZE_UNKNOWN)
  ,quality(5) 
  ,bitrate(0)
  ,in_bitsize(AV_AUDIO_BITSIZE_UNKNOWN)
  ,in_interleaved(true)
{
  STREAMER_VERBOSE("AudioSettings.quality: not used atm\n");
}

void AudioSettings::print() {
  STREAMER_VERBOSE("audio_settings.samplerate: %d\n", samplerate);
  STREAMER_VERBOSE("audio_settings.mode: %d\n", mode);
  STREAMER_VERBOSE("audio_settings.bitsize: %d\n", bitsize);
  STREAMER_VERBOSE("audio_settings.quality: %d\n", quality);
  STREAMER_VERBOSE("audio_settings.bitrate: %d\n", bitrate);
  STREAMER_VERBOSE("audio_settings.in_bitsize: %d\n", in_bitsize);
  STREAMER_VERBOSE("audio_settings.in_interleaved: %d\n", in_interleaved);
}

bool AudioSettings::validate() {
  if(samplerate == AV_AUDIO_SAMPLERATE_UNKNOWN) {
    STREAMER_ERROR("audiosettings: no samplerate set.\n");
    return false;
  }

  if(mode == AV_AUDIO_MODE_UNKNOWN) {
    STREAMER_ERROR("audiosettings: no audio mode set.\n");
    return false;
  }

  if(bitsize == AV_AUDIO_BITSIZE_UNKNOWN) {
    STREAMER_ERROR("audiosettings: no bitsize set.\n");
    return false;
  }

  if(codec_id == AV_AUDIO_CODEC_UNKNOWN) {
    STREAMER_ERROR("audiosettings: no codec id set.\n");
    return false;
  }

  if(!bitrate) {
    STREAMER_ERROR("audiosettings: no bitrate set.\n");
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
    STREAMER_ERROR("serversettings: no url set.\n");
    return false;
  }
  return true;
}

void ServerSettings::print() {
  STREAMER_VERBOSE("server_settings.url: %s\n", url.c_str());
}

// -----------------------------------------
