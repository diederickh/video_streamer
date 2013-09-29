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

VideoStreamerConfig::VideoStreamerConfig() {
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
    xml_node<>* streams = conf.getNode("videostreamer/streams");
    for(xml_node<>* stream = streams->first_node(); stream; stream = stream->next_sibling()) {
      sc = new StreamerConfiguration();
      sc->id = conf.readU16(stream, "id");
      sc->video.width = conf.readU16(stream, "video/width");
      sc->video.height = conf.readU16(stream, "video/height");
      sc->video.fps = conf.readU16(stream, "video/fps");
      sc->audio.samplerate = conf.readU32(stream, "audio/samplerate");
      sc->audio.bitsize = conf.readU8(stream, "audio/bitsize");
      sc->audio.quality = conf.readU8(stream, "audio/quality");
      sc->audio.bitrate = conf.readU32(stream, "audio/bitrate");
      sc->audio.mode = conf.readU32(stream, "audio/mode");
      sc->audio.in_bitsize = conf.readU8(stream, "audio/in_bitsize");
      sc->audio.in_interleaved = conf.readU8(stream, "audio/in_interleaved");
      sc->server.url = conf.readString(stream, "server/url");

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

