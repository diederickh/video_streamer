#include <stdio.h>
#include <streamer/core/AudioEncoder.h>
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
{
}

AudioEncoder::~AudioEncoder() {
  printf("AudioEncoder::~AudioEncoder() - cleanup.\n");
}

bool AudioEncoder::setup(AudioSettings s) {
  settings = s;
  return true;
}

bool AudioEncoder::initialize() {

  if(lame_flags) {
    printf("error: cannot initialize AudioEncoder because lame_flags has been setup already.\n");
    return false;
  }
  
  if(settings.bitsize == AV_AUDIO_BITSIZE_UNKNOWN) {
    printf("error: cannot initialize AudioEncoder because the set bitsize is invalid.\n");
    return false;
  }

  if(settings.in_bitsize == AV_AUDIO_BITSIZE_UNKNOWN) {
    settings.in_bitsize = settings.bitsize;
  }

  if(settings.mode == AV_AUDIO_MODE_UNKNOWN) {
    printf("error: cannot initialize AudioEncoder because the mode (mono/stereo) is invalid.\n");
    return false;
  }

  if(settings.samplerate == AV_AUDIO_SAMPLERATE_UNKNOWN) {
    printf("error: cannot intialize the AudioEncoder because the samplerate is invalid.\n");
    return false;
  }

  if(!settings.bitrate) {
    printf("error: cannot initialize the AudioEncoder because the bitrate was not set.\n");
    return false;
  }

  if(settings.samplerate == AV_AUDIO_SAMPLERATE_44100) {
    samplerate = 44100;
  }
  else if(settings.samplerate == AV_AUDIO_SAMPLERATE_22050) {
    samplerate = 22050;
  }
  else {
    printf("error: invalid samplerate given for the AudioEncoder.\n");
    return false;
  }
  
  if(settings.mode == AV_AUDIO_MODE_STEREO) {
    mode = STEREO;
    nchannels = 2;
  }
  else if(settings.mode == AV_AUDIO_MODE_MONO) {
    mode = MONO;
    nchannels = 1;
    printf("error: for now we only implement stereo audio.\n");
    return false;
  }
  else {
    printf("error: invalid mode given for the AudioEncoder.\n");
    return false;
  }

  if(settings.quality > 9) {
    printf("error: invalid quality given for the AudioEncoder, use a value between 0 (best) and 9 (worst).\n");
    return false;
  }

  lame_flags = lame_init();
  if(!lame_flags) {
    printf("error: cannot initalize lame.\n");
    return false;
  }

  lame_set_num_channels(lame_flags, (mode == STEREO) ? 2 : 1);
  lame_set_in_samplerate(lame_flags, samplerate);
  lame_set_brate(lame_flags, settings.bitrate);
  lame_set_VBR_min_bitrate_kbps(lame_flags, settings.bitrate);
  lame_set_mode(lame_flags, mode);
  lame_set_quality(lame_flags, settings.quality);

  if(lame_init_params(lame_flags) < 0) {
    printf("error: cannot initialize the lame parameters.\n");
    return false;
  }

#if !defined(NDEBUG)
  lame_print_config(lame_flags);
  lame_print_internals(lame_flags);
  printf("lame: bitrate: %d\n", lame_get_brate(lame_flags));
#endif

  return true;
}

bool AudioEncoder::shutdown() {
  // @todo - cleanup audio encoder here
  printf("ERROR: WE STILL NEED TO IMPLEMENT SHUTDOWN FOR AUDIOENCODER.\n");
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
    written = lame_encode_buffer_interleaved(lame_flags, (short int*)&p->data.front(), nsamples, mp3_buffer, AUDIO_ENCODER_BUFFER_SIZE);
  }
  else if(settings.in_bitsize == AV_AUDIO_BITSIZE_F32) {
    nsamples = p->data.size() / (sizeof(float) * nchannels);
    written = lame_encode_buffer_interleaved_ieee_float(lame_flags, (const float*)&p->data.front(), nsamples, mp3_buffer, AUDIO_ENCODER_BUFFER_SIZE);
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
