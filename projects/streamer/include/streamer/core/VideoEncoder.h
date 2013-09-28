/*

  # VideoEncoder

  Encodes YUV420p/I420 frames using libx264.

  _Example_
  ```c++
  VideoEncoder ve;
  ve.setup(my_settings);
  ve.initialize();

  for(int i = 0; i < 50; ++i) {
     ...
     ve.encodePacket(some_packet, tag);
     ...
  }
  ```

 */
#ifndef ROXLU_VIDEO_ENCODER_H
#define ROXLU_VIDEO_ENCODER_H

#include <stdint.h>

extern "C" {
#  include <x264.h>  
}

#include <fstream>
#include <streamer/flv/FLVTypes.h>
#include <streamer/core/EncoderTypes.h>

class VideoEncoder {
 public:
  VideoEncoder();
  ~VideoEncoder();
  bool setup(VideoSettings settings); /* set state that is used in initialize() */
  bool initialize(); /* initialize all members, first set all state (e.g. call setup()) */
  bool createDecoderConfigurationRecord(AVCDecoderConfigurationRecord& rec); /* generates the SPS and PPS nal units */
  bool encodePacket(AVPacket* p, FLVTag& result);

  bool openFile(std::string filepath);
  bool writeTagToFile(FLVTag& tag);
  bool closeFile();

  uint8_t getFPS();

 private:
  bool initializePic();
  bool initializeX264();
 private:
  x264_t* encoder;
  x264_param_t params;
  x264_picture_t pic_in;
  x264_picture_t pic_out;
  VideoSettings settings;
  bool vflip; /* flip video, defaults to true */
  uint32_t frame_num;
  std::ofstream ofs; /* only used when writing to a file */
};

inline uint8_t VideoEncoder::getFPS() {
  return settings.fps;
}

#endif
