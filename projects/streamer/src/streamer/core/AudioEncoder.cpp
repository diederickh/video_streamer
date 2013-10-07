#include <stdio.h>
#include <streamer/core/AudioEncoder.h>
#include <streamer/core/Log.h>
#if defined(USE_GRAPH)
#  include <streamer/utils/Graph.h>

extern "C" {
#  include <uv.h>
}

#endif

#define AUDIO_USE_DATA_PTR 0
#define AUDIO_USE_COPY_DATA 1

AudioEncoder::AudioEncoder() 
  :lame_flags(NULL)
  ,samplerate(0)
  ,nchannels(0)
  ,bitrate_timeout(0)
  ,bitrate_delay(1000 * 1000 * 100)
  ,bitrate_in_kbps(0.0)
  ,bitrate_nbytes(0.0)
  ,bitrate_time_started(0)
{
}

AudioEncoder::~AudioEncoder() {

  shutdown();

  nchannels = 0;
  bitrate_timeout = 0;
  bitrate_delay = 0;
  bitrate_in_kbps = 0;
  bitrate_nbytes = 0;
  
}

bool AudioEncoder::setup(AudioSettings s) {
  settings = s;
  return true;
}

bool AudioEncoder::initialize() {

  if(lame_flags) {
    STREAMER_ERROR("error: cannot initialize AudioEncoder because lame_flags has been setup already.\n");
    return false;
  }
  
  if(settings.bitsize == AV_AUDIO_BITSIZE_UNKNOWN) {
    STREAMER_ERROR("error: cannot initialize AudioEncoder because the set bitsize is invalid.\n");
    return false;
  }

  if(settings.in_bitsize == AV_AUDIO_BITSIZE_UNKNOWN) {
    settings.in_bitsize = settings.bitsize;
  }

  if(settings.mode == AV_AUDIO_MODE_UNKNOWN) {
    STREAMER_ERROR("error: cannot initialize AudioEncoder because the mode (mono/stereo) is invalid.\n");
    return false;
  }

  if(settings.samplerate == AV_AUDIO_SAMPLERATE_UNKNOWN) {
    STREAMER_ERROR("error: cannot intialize the AudioEncoder because the samplerate is invalid.\n");
    return false;
  }

  if(!settings.bitrate) {
    STREAMER_ERROR("error: cannot initialize the AudioEncoder because the bitrate was not set.\n");
    return false;
  }

  if(settings.samplerate == AV_AUDIO_SAMPLERATE_44100) {
    samplerate = 44100;
  }
  else if(settings.samplerate == AV_AUDIO_SAMPLERATE_22050) {
    samplerate = 22050;
  }
  else {
    STREAMER_ERROR("error: invalid samplerate given for the AudioEncoder.\n");
    return false;
  }
  
  if(settings.mode == AV_AUDIO_MODE_STEREO) {
    mode = STEREO;
    nchannels = 2;
  }
  else if(settings.mode == AV_AUDIO_MODE_MONO) {
    mode = MONO;
    nchannels = 1;
    STREAMER_ERROR("error: for now we only implement stereo audio.\n");
    return false;
  }
  else {
    STREAMER_ERROR("error: invalid mode given for the AudioEncoder.\n");
    return false;
  }

  if(settings.quality > 9) {
    STREAMER_ERROR("error: invalid quality given for the AudioEncoder, use a value between 0 (best) and 9 (worst).\n");
    return false;
  }

  lame_flags = lame_init();
  if(!lame_flags) {
    STREAMER_ERROR("error: cannot initalize lame.\n");
    return false;
  }

  lame_set_num_channels(lame_flags, (mode == STEREO) ? 2 : 1);
  lame_set_in_samplerate(lame_flags, samplerate);
  lame_set_brate(lame_flags, settings.bitrate);
  lame_set_VBR(lame_flags, vbr_off);
  lame_set_mode(lame_flags, mode);
  lame_set_quality(lame_flags, settings.quality);

  if(lame_init_params(lame_flags) < 0) {
    STREAMER_ERROR("error: cannot initialize the lame parameters.\n");
    return false;
  }

#if !defined(NDEBUG)
  lame_print_config(lame_flags);
  lame_print_internals(lame_flags);
  STREAMER_VERBOSE("lame: bitrate: %d\n", lame_get_brate(lame_flags));
#endif

  bitrate_time_started = uv_hrtime(); 
  bitrate_timeout = bitrate_time_started + bitrate_delay;

  return true;
}

bool AudioEncoder::shutdown() {

  if(lame_flags) {
    lame_close(lame_flags);
    lame_flags = NULL;
  }

  memset(mp3_buffer, 0x00, AUDIO_ENCODER_BUFFER_SIZE);

  return false;
}

// we expect the AVPacket.data to contain either interleaved S16, or  interleaved F32 audio
bool AudioEncoder::encodePacket(AVPacket* p, FLVTag& tag) {

  assert(lame_flags);
  assert(settings.in_interleaved); /* we only support interleaved audio for now */
 
  int nsamples = 0;
  int written = 0;

#if defined(USE_GRAPH)
  uint64_t enc_start = uv_hrtime() / 1000000;
#endif

  if(settings.in_bitsize == AV_AUDIO_BITSIZE_S16) {
    nsamples = p->data.size() / (sizeof(int16_t) * nchannels);
    //printf("----------------- samples: %d, channels: %d, data.size(): %zu\n", nsamples, nchannels, p->data.size());
    written = lame_encode_buffer_interleaved(lame_flags, (short int*)&p->data.front(), nsamples, mp3_buffer, AUDIO_ENCODER_BUFFER_SIZE);
  }
  else if(settings.in_bitsize == AV_AUDIO_BITSIZE_F32) {
    nsamples = p->data.size() / (sizeof(float) * nchannels);
    written = lame_encode_buffer_interleaved_ieee_float(lame_flags, (const float*)&p->data.front(), nsamples, mp3_buffer, AUDIO_ENCODER_BUFFER_SIZE);
  }


  if(written > 0) {
    bitrate_nbytes += written;
  }

  uint64_t time_now = uv_hrtime();
  if(time_now >= bitrate_timeout) {
    bitrate_timeout = time_now + bitrate_delay;
    double duration = (time_now - bitrate_time_started) / 1000000000.0; // in s.
    bitrate_in_kbps = ((bitrate_nbytes * 8) / 1000) / duration;
    STREAMER_STATUS("audio bitrate: %0.2f kbps\n", bitrate_in_kbps);
  }

#if defined(USE_GRAPH)
  frames_graph["enc_audio"] += ((uv_hrtime()/1000000) - enc_start);
  frames_graph["enc_audio_video"] += ((uv_hrtime()/1000000) - enc_start);
  network_graph["mp3"] += written;
#endif

#if AUDIO_USE_DATA_PTR
  if(written) {
    tag.setData(mp3_buffer, written);
  }
#elif AUDIO_USE_COPY_DATA
  tag.bs.clear();
  if(written) {
    tag.bs.putBytes((uint8_t*)mp3_buffer, written);
    tag.setData(tag.bs.getPtr(), tag.bs.size());
  }
#endif

  tag.makeAudioTag();
  tag.setTimeStamp(p->timestamp);

  return written > 0;
}
