/*

  # VideoStreamer 

  The VideoStreamer class is the main interface for all underlying 
  parts of this library. It will startup all the video and audio encoding, 
  manage the rtmp connection and encoder/networking threads.

  _Usage_
  ````
  - first call all of sevaral of:
    - setServerSettings()
    - setVideoSettings()
    - setAudioSettings()
  - after setting the correct settings, call:
    - setup()
    - initialize()

  - then call addVideo(...) and addAudio(...) repeatedly 

  ````
 */

#ifndef ROXLU_VIDEOSTREAMER_H
#define ROXLU_VIDEOSTREAMER_H

extern "C" {
#  include <uv.h>
}

#include <streamer/flv/FLVWriter.h>
#include <streamer/flv/FLVFileWriter.h>
#include <streamer/core/BitStream.h>
#include <streamer/core/EncoderTypes.h>
#include <streamer/core/VideoEncoder.h>
#include <streamer/core/AudioEncoder.h>
#include <streamer/core/RTMPWriter.h>
#include <streamer/core/RTMPThread.h>
#include <streamer/core/EncoderThread.h>

// ---------------------------------

#define VS_STATE_NONE 0
#define VS_STATE_STARTED 1
#define VS_STATE_SETUP 2

// ---------------------------------
uint8_t encoder_samplerate_to_flv(uint32_t v);
uint8_t encoder_audio_mode_to_flv(uint8_t v);
uint8_t encoder_audio_bitsize_to_flv(uint8_t v);
// ---------------------------------

class VideoStreamer {
 public:
  VideoStreamer();
  ~VideoStreamer();

  /* these functions must be called before you call setup() ! */
  bool loadSettings(std::string filepath); /* instead of passing the settings to setServerSettings(), setVideoSettings() and setAudioSettings(), you can load them from a xml file. */
  void setServerSettings(ServerSettings ss);
  void setVideoSettings(VideoSettings vs);
  void setAudioSettings(AudioSettings as);
  void setOutputFile(std::string filepath);
  void setStrides(uint32_t strideY, uint32_t strideU, uint32_t strideV); /* (call before setup()) - sets the strides for the y,u,v planes on the video encoder */
  void setVideoWidth(uint16_t w); /* change the width of the video settings */
  void setVideoHeight(uint16_t h); /* change the height of the video settings */

  bool setup(); /* setup all the used members, after you've called `setServerSettings()`, `setAudioSettings()`, `setVideoSettings()` */
  bool start();
  bool isStarted(); 
  bool stop();

  bool wantsVideo(); /* returns true when we need a new video frame, used in "direct" mode, w/o the daemon */

  void addVideo(AVPacket* pkt); /* add a new packet to encode - see the MemoryPool description in the header for some info about refcounts */
  void addAudio(AVPacket* pkt); /* add a new packet to encode - see the MemoryPool description in the header for some info about refcounts */

  uint32_t getTimeStamp(); /* get the current timestamp in mills, relative from the 'start' timestamp */
  uint16_t getVideoWidth(); /* get the width of the output video, as defined in the settings - width used by encoder */
  uint16_t getVideoHeight(); /* get the height of the output video, as defined in the settings - height used by encoder */
  uint8_t getFrameRate(); /* returns the video framerate */
  uint16_t getSampleRate(); /* get audio sample rate as defined in the audio settings */

 private:
  bool usesVideo(); /* returns true when video encoding is used */
  bool usesAudio(); /* returns true when audio encoding is  used */

 private:
  BitStream flv_bitstream;         /* the flv bitstream, used in the encoder thread */
  FLVWriter flv_writer;            /* the flv muxer, which creates the flv bitstream */
  FLVFileWriter* flv_file_writer;  /* when `setOutputFile()` is called we will dump the generated FLV to a file */
  RTMPWriter rtmp_writer;          /* used by the rtmp_thread to stream rtmp packets */
  VideoEncoder video_enc;          /* the video encoder, used in the encoder thread */
  AudioEncoder audio_enc;          /* the audio encoder, used in the encoder thread */
  EncoderThread enc_thread;        /* encoder thread, encodes audio and video */
  RTMPThread rtmp_thread;          /* thread that is writing rtmp packets to a server */
  VideoSettings video_settings;    /* used by the video encoder */
  AudioSettings audio_settings;    /* used by the audio encoder */
  ServerSettings server_settings;  /* rtmp server settings */
  uint8_t state;                   /* used to manage state of the VideoStreamer */
  uint64_t video_timeout;          /* timeout when we need a new video frame, based on the framerate */
  uint64_t video_delay;            /* delay between frames, in ns */
  uint64_t time_started;           /* time when you called `start()` in ns */
  std::string output_file;         /* set by setOutputFile(), will save the generated flv to a file */
};

// ---------------------------------
inline void VideoStreamer::setVideoWidth(uint16_t w) {
  video_settings.width = w;
}

inline void VideoStreamer::setVideoHeight(uint16_t h) {
  video_settings.height = h;
}

inline void VideoStreamer::setServerSettings(ServerSettings ss) {
  server_settings = ss;
}

inline void VideoStreamer::setVideoSettings(VideoSettings vs) {
  video_settings = vs;
}

inline void VideoStreamer::setAudioSettings(AudioSettings as) {
  audio_settings = as;
}

inline void VideoStreamer::setOutputFile(std::string path) {
  output_file = path;
}

inline bool VideoStreamer::usesAudio() {
  return audio_settings.samplerate > 0 && audio_settings.bitrate > 0;
}

inline bool VideoStreamer::usesVideo() {
  return video_settings.width > 0 && video_settings.height > 0; 
}

inline bool VideoStreamer::wantsVideo() {
  bool wants = video_timeout && uv_hrtime() >= video_timeout;
  if(wants) {
    video_timeout = uv_hrtime() + video_delay;
  }
  return wants;
}

inline bool VideoStreamer::isStarted() {
  return state == VS_STATE_STARTED;
}

inline uint32_t VideoStreamer::getTimeStamp() {
  return (uv_hrtime() - time_started) / 1000000;
}

inline uint16_t VideoStreamer::getVideoWidth() {
  return video_settings.width;
}

inline uint16_t VideoStreamer::getVideoHeight() {
  return video_settings.height;
}

inline uint8_t VideoStreamer::getFrameRate() {
  return video_settings.fps;
}

inline uint16_t VideoStreamer::getSampleRate() {
  return audio_settings.samplerate;
}

inline void VideoStreamer::setStrides(uint32_t strideY, uint32_t strideU, uint32_t strideV) {
  video_enc.setStrides(strideY, strideU, strideV);
}

#endif
