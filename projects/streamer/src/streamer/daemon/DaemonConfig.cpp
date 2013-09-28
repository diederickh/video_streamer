#include <streamer/daemon/DaemonConfig.h>

// ----------------------------------------------

StreamConfig::StreamConfig() {
  reset();
}

StreamConfig::~StreamConfig() {
  reset();
}

void StreamConfig::reset() {
  id = -1;
  rtmp_url.clear();
  address.clear();
  width = 0;
  height = 0;
  fps = 0;
  samplerate = 0;
  bitrate = 0;
  bitsize = 0;
  quality = 0;
  in_bitsize = 0;
  in_interleaved = 0;
  mode = 0;
}

bool StreamConfig::validate() {

  if(!usesVideo()) {
    printf("error: no video settings defined.\n");
    return false;
  }

  if(!width) {
    printf("error: no width set.\n");
    return false;
  }
  if(!height) {
    printf("error: no height set.\n");
    return false;
  }
  if(!fps) {
    printf("error: no fps set.\n");
    return false;
  }

  if(samplerate || bitsize || in_bitsize || in_interleaved || quality || bitrate || mode) {
    if(!samplerate) {
      printf("error: samplerate not set.\n");
      return false;
    }
    if(!bitsize) {
      printf("error: bitsize not set.\n");
      return false;
    }
    if(!bitrate) {
      printf("error: bitrate not set.\n");
      return false;
    }
    if(!mode) {
      printf("error: mode not set.\n");
      return false;
    }
    if(!in_bitsize) {
      in_bitsize = bitsize;
    }
    if(!quality) {
      quality = 6; // @todo set default quality
    }
  }

  if(!rtmp_url.size()) {
    printf("error: rtmp_url not set.\n");
    return false;
  }

  if(id < 0) {
    printf("error: no id set.\n");
    return false;
  }

  if(!address.size()) {
    printf("error: no ipc address set; tcp:// inproc:// ipc://\n");
    return false;
  }

  return true;
    
}

void StreamConfig::print() {
  printf("config.id: %lld\n", id);
  printf("config.address: %s\n", address.c_str());
  printf("config.rtmp_url: %s\n", rtmp_url.c_str());
  printf("config.width: %d\n", width);
  printf("config.height: %d\n", height);
  printf("config.fps: %d\n", fps);
  printf("config.samplerate: %d\n", samplerate);
  printf("config.bitsize: %d\n", bitsize);
  printf("config.quality: %d\n", quality);
  printf("config.in_bitsize: %d\n", in_bitsize);
  printf("config.in_interleaved: %d\n", in_interleaved);
}

// ----------------------------------------------

DaemonConfig::DaemonConfig() {
}

DaemonConfig::~DaemonConfig() {
  for(std::map<uint32_t, StreamConfig*>::iterator it = configs.begin(); it != configs.end(); ++it) {
    delete it->second;
  }
  configs.clear();
}

bool DaemonConfig::load(std::string filepath) {

  if(!conf.load(filepath)) {
    return false;
  }

  StreamConfig* sc = NULL;

  try {
    xml_node<>* streams = conf.getNode("videostreamer/streams");
    for(xml_node<>* stream = streams->first_node(); stream; stream = stream->next_sibling()) {
      sc = new StreamConfig();
      sc->id = conf.readU32(stream, "id");
      sc->address = conf.readString(stream, "address");
      sc->width = conf.readU16(stream, "video/width");
      sc->height = conf.readU16(stream, "video/height");
      sc->fps = conf.readU16(stream, "video/fps");
      sc->samplerate = conf.readU32(stream, "audio/samplerate");
      sc->bitsize = conf.readU8(stream, "audio/bitsize");
      sc->quality = conf.readU8(stream, "audio/quality");
      sc->bitrate = conf.readU32(stream, "audio/bitrate");
      sc->mode = conf.readU32(stream, "audio/mode");
      sc->in_bitsize = conf.readU8(stream, "audio/in_bitsize");
      sc->in_interleaved = conf.readU8(stream, "audio/in_interleaved");
      sc->rtmp_url = conf.readString(stream, "server/rtmp_url");

      if(!sc->validate()) {
        sc->print();
        ::exit(EXIT_FAILURE);
      }

      configs[sc->id] = sc;
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

