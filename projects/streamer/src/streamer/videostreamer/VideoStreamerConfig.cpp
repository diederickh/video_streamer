#include <streamer/videostreamer/VideoStreamerConfig.h>
#include <xmlconfig/Config.h>
#include <iostream>

// ---------------------------------------
StreamerConfiguration::StreamerConfiguration() 
  :id(0)
{
}

bool StreamerConfiguration::validate() {
  // @todo - use the same logic as in DaemonConfig
  return video.validate() && audio.validate() && server.validate();
}

void StreamerConfiguration::print() {
  video.print();
  audio.print();
}

bool StreamerConfiguration::hasAudio() {
  return audio.samplerate || audio.bitsize || audio.quality;
}

bool StreamerConfiguration::hasVideo() {
  return video.width || video.height || video.fps;
}

bool StreamerConfiguration::hasServer() {
  return server.url.size();
}

// ---------------------------------------

VideoStreamerConfig::VideoStreamerConfig() 
  :default_stream_id(0)  
{
}

VideoStreamerConfig::~VideoStreamerConfig() {
  for(std::vector<StreamerConfiguration*>::iterator it = configs.begin(); it != configs.end(); ++it) {
    delete *it;
  }
  configs.clear();
}

bool VideoStreamerConfig::load(std::string filepath) {

  Config conf;
  if(!conf.load(filepath)) {
    printf("error: cannot load the configuration for the VideoStreamer.\n");
    return false;
  }

  StreamerConfiguration* sc = NULL;

  try {
    xml_node<>* settings = conf.getNode("videostreamer/settings");
    default_stream_id = conf.readU32(settings, "default_stream_id");
    
    xml_node<>* streams = conf.getNode("videostreamer/streams");
    for(xml_node<>* stream = streams->first_node(); stream; stream = stream->next_sibling()) {
      sc = new StreamerConfiguration();
      sc->id                    = conf.readU16(stream, "id");
      sc->video.width           = conf.readU16(stream, "video/width");
      sc->video.height          = conf.readU16(stream, "video/height");
      sc->video.fps             = conf.readU16(stream, "video/fps");
      sc->video.bitrate         = conf.readU16(stream, "video/bitrate");
      sc->video.threads         = conf.readU16(stream, "video/threads");
      sc->video.preset          = conf.readString(stream, "video/preset", "veryfast");
      sc->video.tune            = conf.readString(stream, "video/tune", "zerolatency");
      sc->video.profile         = conf.readString(stream, "video/profile", "baseline");
      sc->video.vbv_buffer_size = conf.readS32(stream, "video/vbv_buffer_size", -1);
      sc->video.vbv_max_bitrate = conf.readS32(stream, "video/vbv_max_bitrate", -1);
      sc->video.keyint_max      = conf.readS32(stream, "video/keyint_max", -1);
      sc->video.bframe          = conf.readS32(stream, "video/bframe", -1);
      sc->video.level_idc       = conf.readS16(stream, "video/level_idc", -1);

      if(conf.doesNodeExists(stream, "audio")) {
        sc->audio.codec_id         = conf.readU16(stream, "audio/codec", 1);
        sc->audio.samplerate       = conf.readU32(stream, "audio/samplerate");
        sc->audio.bitsize          = conf.readU8(stream, "audio/bitsize");
        sc->audio.quality          = conf.readU8(stream, "audio/quality");
        sc->audio.bitrate          = conf.readU32(stream, "audio/bitrate");
        sc->audio.mode             = conf.readU32(stream, "audio/mode");
        sc->audio.in_bitsize       = conf.readU8(stream, "audio/in_bitsize");
        sc->audio.in_interleaved = conf.readU8(stream, "audio/in_interleaved");
      }

      sc->server.url = conf.readString(stream, "server/url");
      
      if(conf.doesNodeExists(stream, "server/username")) {
        sc->server.username = conf.readString(stream, "server/username");
      }

      if(conf.doesNodeExists(stream, "server/password")) {
        sc->server.password = conf.readString(stream, "server/password");
      }

      

      configs.push_back(sc);
      sc = NULL;
    }
  }
  catch(ConfigException ex) {
    printf("error: %s\n", ex.what());
    if(sc) {
      delete sc;
      sc = NULL;
    }
    return false;
  }
  
  return true;
}

