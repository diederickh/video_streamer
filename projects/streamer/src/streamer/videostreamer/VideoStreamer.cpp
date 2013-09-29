#include <assert.h>
#include <iostream>
#include <streamer/videostreamer/VideoStreamer.h>
#include <streamer/videostreamer/VideoStreamerConfig.h>

VideoStreamer::VideoStreamer() 
  :flv_writer(flv_bitstream)
  ,rtmp_thread(flv_writer, rtmp_writer)
  ,enc_thread(flv_writer, video_enc, audio_enc)
  ,state(VS_STATE_NONE)
  ,video_timeout(0)
  ,video_delay(0)
  ,time_started(0)
  ,flv_file_writer(NULL)
{
  printf("~VideoStreamer\n");
}

VideoStreamer::~VideoStreamer() {

  if(state == VS_STATE_STARTED) {
    stop();
  }

  if(flv_file_writer) {
    delete flv_file_writer;
    flv_file_writer = NULL;
  }

  time_started = 0;
}

bool VideoStreamer::loadSettings(std::string filepath) {

  if(!filepath.size()) {
    printf("error: invalid settings filepath: '%s'\n", filepath.c_str());
    return false;
  }

  VideoStreamerConfig cfg;
  if(!cfg.load(filepath)) {
    return false;
  }

  if(!cfg.size()) {
    printf("error: no configurations found.\n");
    return false;
  }

  StreamerConfiguration* sc = cfg[0]; // we just pick the first found config

  if(sc->hasAudio()) {
    setAudioSettings(sc->audio);
  }

  if(sc->hasVideo()) {
    setVideoSettings(sc->video);
  }

  if(sc->hasServer()) {
    setServerSettings(sc->server);
  }

  return true;
}

bool VideoStreamer::setup() {

  if(state != VS_STATE_NONE) {
    printf("error: cannot setup the videostreamer because the current state is not VS_STATE_NONE, which means we're probably already setup.\n");
    return false;
  }

  if(!usesVideo() && !usesAudio()) {
    printf("error: no settings for audio or video found.\n");
    return false;
  }

  if(!video_settings.validate()) {
    return false;
  }

  if(!server_settings.validate()) {
    return false;
  }

  if(usesVideo()) {
    if(!video_enc.setup(video_settings)) {
      return false;
    }
    flv_writer.setVideoCodec(FLV_VIDEOCODEC_AVC);
    flv_writer.setWidth(video_settings.width);
    flv_writer.setHeight(video_settings.height);
    flv_writer.setFrameRate(video_settings.fps);
    flv_writer.setCanSeekToEnd(false);

    video_delay = (1.0/video_settings.fps) * 1000 * 1000 * 1000;
  }

  if(usesAudio()) {
    if(!audio_enc.setup(audio_settings)) {
      return false;
    }
    flv_writer.setAudioCodec(FLV_SOUNDFORMAT_MP3);
    flv_writer.setAudioDataRate(audio_settings.bitrate);
    flv_writer.setAudioSampleRate(encoder_samplerate_to_flv(audio_settings.samplerate));
    flv_writer.setAudioSize(encoder_audio_bitsize_to_flv(audio_settings.bitsize));
    flv_writer.setAudioType(encoder_audio_mode_to_flv(audio_settings.mode));
  }
  
  rtmp_writer.setURL(server_settings.url);

  state = VS_STATE_SETUP;

  return true;
}


bool VideoStreamer::start() {

  if(state != VS_STATE_SETUP) {
    printf("error: cannot initialize the videostreamer because the current state is not VS_STATE_SETUP, which means we're probably already initialized or we're not yet setup.\n");
    return false;
  }
  
  if(usesVideo()) {
    if(!video_enc.initialize()) {
      return false;
    }

    AVCDecoderConfigurationRecord avc;
    video_enc.createDecoderConfigurationRecord(avc);
    flv_writer.setDecoderConfigurationRecord(avc);
  }

  if(usesAudio()) {
    if(!audio_enc.initialize()) {
      return false;
    }
  }

  if(output_file.size() && !flv_file_writer) {
    flv_file_writer = new FLVFileWriter(output_file);
    flv_writer.addListener(flv_file_writer);
  }

  if(!flv_writer.open()) {
    return false;
  }

  if(!rtmp_writer.initialize()) {
    return false;
  }


  enc_thread.start();
  rtmp_thread.start();

  state = VS_STATE_STARTED;

  video_timeout = uv_hrtime() + video_delay;
  time_started = uv_hrtime();
  return true;
}

bool VideoStreamer::stop() {

  if(state != VS_STATE_STARTED) {
    printf("error: cannot stop the videostreamer when it's not yet started.\n");
    return false;
  }

  enc_thread.stop();
  rtmp_thread.stop();
  flv_writer.close();
  video_timeout = 0;

  if(flv_file_writer) {
    delete flv_file_writer;
    flv_file_writer = NULL;
  }

  state = VS_STATE_SETUP;

  return true;
}

void VideoStreamer::addAudio(AVPacket* pkt) {
  assert(state == VS_STATE_STARTED);
  enc_thread.addPacket(pkt);
}

void VideoStreamer::addVideo(AVPacket* pkt) {
  assert(state == VS_STATE_STARTED);
  enc_thread.addPacket(pkt);
}

// -----------------------------------------

uint8_t encoder_samplerate_to_flv(uint32_t v) {
  switch(v) {
    case AV_AUDIO_SAMPLERATE_11025: { return FLV_SOUNDRATE_11KHZ; }
    case AV_AUDIO_SAMPLERATE_22050: { return FLV_SOUNDRATE_22KHZ; }
    case AV_AUDIO_SAMPLERATE_44100: { return FLV_SOUNDRATE_44KHZ; }
    default: {
      printf("error: cannot find the samplerate: %d\n", v);
      return FLV_SOUNDRATE_UNKNOWN;
    }
  }
}

uint8_t encoder_audio_mode_to_flv(uint8_t v) {
  switch(v) {
    case AV_AUDIO_MODE_MONO:   { return FLV_SOUNDTYPE_MONO;   }
    case AV_AUDIO_MODE_STEREO: { return FLV_SOUNDTYPE_STEREO; }
    default: {
      printf("error: cannot find a valid audio mode for flv: %d\n", v);
      return FLV_SOUNDTYPE_UNKNOWN;
    };
  }
}

uint8_t encoder_audio_bitsize_to_flv(uint8_t v) {
  switch(v) {
    case AV_AUDIO_BITSIZE_S8:  { return FLV_SOUNDSIZE_8BIT;  }  
    case AV_AUDIO_BITSIZE_S16: { return FLV_SOUNDSIZE_16BIT; }
    default: {
      printf("error: cannot convert the bitsize for flv: %d\n", v);
      return FLV_SOUNDSIZE_UNKNOWN;
    }
  }
}
