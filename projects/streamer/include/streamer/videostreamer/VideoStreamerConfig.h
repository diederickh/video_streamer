/*

  # StreamerConfiguration

  This class is used to load an xml with configuration for a VideoStreamer instance. 
  @todo we also have a DaemonConfig which must be replaced by this class!

  _Usage_
  ''''c++
  VideoStreamerConfig vs_conf;
  if(!vs_conf.load("config.xml")) {
     printf("error loading config.xml.\n");
     ::exit(EXIT_FAILURE)
  }

  StreamerConfiguration* sc = vs_conf.getByID(0);
  if(!sc) {
    printf("error: cannot find config by id 0.\n");
    ::exit(EXIT_FAILURE);
  }

  // use sc->audio, sc->video, sc->server
  ''''
  
  _Example of XML_

 ''''xml

<?xml version="1.0" encoding="UTF-8"?>
<videostreamer>
  <streams>
    <stream>
      <id>0</id>
      <server>
        <rtmp_url>rtmp://192.168.0.188/flvplayback/livestream</rtmp_url>
      </server>
      <video>
        <width>320</width>
        <height>240</height>
        <fps>15</fps>
      </video>
      <audio>
        <samplerate>0</samplerate>
        <mode>0</mode>
        <bitsize>0</bitsize>
        <bitrate>0</bitrate> <!-- in kbps -->
        <quality>0</quality>
        <in_bitsize>0</in_bitsize>
        <in_interleaved>0</in_interleaved>
      </audio>
    </stream>
  </streams>
</videostreamer>

 ''''
 */
#ifndef ROXLU_VIDEOSTREAMER_STREAMER_CONFIG_H
#define ROXLU_VIDEOSTREAMER_STREAMER_CONFIG_H

#include <iostream>
#include <string>
#include <vector>
#include <stdint.h>

#include <streamer/core/EncoderTypes.h>

// -------------------------------------------------

struct StreamerConfiguration {
  StreamerConfiguration();
  bool validate(); /* validate -all- settings */
  void print(); /* print some debug info */
  bool hasVideo(); /* when one of the video settings has been set this will return true; this does not validate the settings. */
  bool hasAudio(); /* when one of the audio settings has been set this will return true; this does not validate the settings. */
  bool hasServer(); /* when one of the server settings has been set this will return true; this does not validate the settings. */

  uint32_t id;
  AudioSettings audio;
  VideoSettings video;
  ServerSettings server;
};

// -------------------------------------------------

class VideoStreamerConfig {
 public:
  VideoStreamerConfig();
  ~VideoStreamerConfig();
  bool load(std::string filepath); /* load the xml from a full path */
  StreamerConfiguration* getByID(uint32_t id); /* each stream must have a unique id, you can use this function to get the configuration for the given ID, if not found we return NULL */
  size_t size(); /* returns the number of stream configs */
  StreamerConfiguration* operator[](size_t dx);
 public:
  std::vector<StreamerConfiguration*> configs; /* VideoStreamerConfig takes ownership of all configs and will destory them when the class is destroyed */
};

inline StreamerConfiguration* VideoStreamerConfig::getByID(uint32_t id) {
  for(std::vector<StreamerConfiguration*>::iterator it = configs.begin(); it != configs.end(); ++it) {
    StreamerConfiguration* sc = *it;
    if(sc->id == id) {
      return sc;
    }
  }
  return NULL;
}

inline StreamerConfiguration* VideoStreamerConfig::operator[](size_t dx) {
  if(dx >= configs.size()) {
    printf("error: trying to get a configuration which does not exist.\n");
    return NULL;
  }
  return configs[dx];
}

inline size_t VideoStreamerConfig::size() {
  return configs.size();
}

// -------------------------------------------------

#endif
