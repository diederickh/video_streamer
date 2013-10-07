/*

---------------------------------------------------------------------------------
      
                                               oooo              
                                               `888              
                oooo d8b  .ooooo.  oooo    ooo  888  oooo  oooo  
                `888""8P d88' `88b  `88b..8P'   888  `888  `888  
                 888     888   888    Y888'     888   888   888  
                 888     888   888  .o8"'88b    888   888   888  
                d888b    `Y8bod8P' o88'   888o o888o  `V88V"V8P' 
                                                                 
                                                  www.roxlu.com
                                             www.apollomedia.nl  
                                          www.twitter.com/roxlu
              
---------------------------------------------------------------------------------

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

  <settings>
    <default_stream_id>0</default_stream_id>  
  </settings>

  <streams>                                  <!-- contains all the streams -->
    <stream>                                 <!-- each separate video stream is contained in it's own <stream> block -->
      <id>0</id>                             <!-- ID, must be unique and is used internally when you have multiple quality streams -->
      <server>
        <username></username>                <!-- when necessary set the username -->
        <password></password>                <!-- when necessary set the password -->
        <url>rtmp://192.168.0.188/flvplayback/livestream</url>
      </server>
      <video>
        <width>320</width>                    <!-- width of the output video -->
        <height>240</height>                  <!-- height of the output video -->
        <fps>15</fps>                         <!-- desired framerate to encode --> 
        <bitrate>400</bitrate>                <!-- the bitrate you want to use for the video, in kbps --> 
        <threads>4</threads>                  <!-- the number of x264 encoder threads, set this to the number of logical cores -->
      </video>
      <audio>
        <samplerate>0</samplerate>            <!-- samplerate: 44100, 22050, 11025 -->
        <mode>0</mode>                        <!-- mode: mono = 1, stereo = 2 -->
        <bitsize>0</bitsize>                  <!-- bitsize: S8 = 0, S16 = 2, F32 = 3 -->
        <bitrate>0</bitrate>                  <!-- in kbps -->
        <quality>0</quality>                  <!-- quality to use, value between 0 and 9, 0 = best (slow), 9 = worst (fast), 5 is ok -->
        <in_bitsize>0</in_bitsize>            <!-- we have basic support for conversion, set this to the input  bitsize, see bitsize above for the values you can use  -->
        <in_interleaved>0</in_interleaved>    <!-- 0 = not using interleaved audio, 1 = using interleaved audio -->
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
  StreamerConfiguration* getDefault(); /* returns the default configuration object, which can be set in the main settings with <default_stream_id></default_stream_id> */
  StreamerConfiguration* getByID(uint32_t id); /* each stream must have a unique id, you can use this function to get the configuration for the given ID, if not found we return NULL */
  size_t size(); /* returns the number of stream configs */
  StreamerConfiguration* operator[](size_t dx);
 public:
  std::vector<StreamerConfiguration*> configs; /* VideoStreamerConfig takes ownership of all configs and will destory them when the class is destroyed */
  uint32_t default_stream_id; /* in the xml you can define the <default_stream_id></default_stream_id> that is selected when you load a settings file using VideoStreamer::loadSettings() */
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

inline StreamerConfiguration* VideoStreamerConfig::getDefault() {
  return getByID(default_stream_id);
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
