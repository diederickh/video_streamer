#ifndef ROXLU_VIDEOSTREAMER_CONFIG_H
#define ROXLU_VIDEOSTREAMER_CONFIG_H

#include <xmlconfig/Config.h>
#include <map>

// @todo - replace this with VideoStreamerConfig!

// ----------------------------------------------

struct StreamConfig {
  StreamConfig();
  ~StreamConfig();
  void reset();
  void print();
  bool usesVideo();     /* returns true when video is used, must be true for now */
  bool usesAudio();     /* returns true when audio is used */
  bool validate();      /* checks if all video settings are set and, if audio is set it checks these too; returns true when everything is ok */

  int64_t id;

  /* ipc */
  std::string address;  /* socket/ipc address */

  /* server */
  std::string rtmp_url;

  /* video */
  uint16_t width;
  uint16_t height;
  uint8_t fps;

  /* audio */
  uint32_t samplerate;       /* AV_AUDIO_SAMPLERATE_11025, AV_AUDIO_SAMPLERATE_22050, AV_AUDIO_SAMPLERATE__44100 */
  uint32_t bitrate;          /* bitrate in kilobits */
  uint8_t bitsize;           /* AV_AUDIO_BITSIZE_S8, AV_AUDIO_BITSIZE_S16, AV_AUDIO_BITSIZE_F32 */
  uint8_t quality;           /* 0-9, 0 = best, 9 = worst, 5 = ok */
  uint8_t in_bitsize;        /* AV_AUDIO_BITSIZE_S8, AV_AUDIO_BITSIZE_S16, AV_AUDIO_BITSIZE_F32, or nothing, then bitsize is used  */
  uint8_t in_interleaved;    /* 0 = false, 1 = true */
  uint8_t mode;              /* AV_AUDIO_MODE_MONO, AV_AUDIO_MODE_STEREO */
};

// ----------------------------------------------


class DaemonConfig {
 public:
  DaemonConfig();
  ~DaemonConfig();
  typedef std::map<uint32_t, StreamConfig*>::iterator iterator;
  iterator begin();
  iterator end();

  bool load(std::string filepath);

 public:
  Config conf;
  std::map<uint32_t, StreamConfig*> configs;
};

inline DaemonConfig::iterator DaemonConfig::begin() {
  return configs.begin();
}

inline DaemonConfig::iterator DaemonConfig::end() {
  return configs.end();
}

// ----------------------------------------------

inline bool StreamConfig::usesVideo() {
  return width || height || fps;
}

inline bool StreamConfig::usesAudio() {
  return samplerate || bitsize || quality;
}

#endif
