/*

  # VideoEncoder

  Encodes YUV420p/I420 frames using libx264.

  @todo - the video encoder can work with two kinds of inputs now. when you're not 
          specifying any stride (see setStrides)  or AVPacket.y_offset, AVPacket.u_offset, 
          AVPacket.v_offset we assume that you your providing the I420P data as 3 seperate
          buffers. When you specify the setStrides and the AVPacket.{y,u,v}_offset then you
          can basically pass any buffer as long as the AVPacket.data contains all the data.

          We need to to remove the first interface which assumes the 3 planes and start using 
          the setStrides/AVPacket.{y,u,v}_offset. 

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

  void setStrides(uint32_t strideY, uint32_t strideU, uint32_t strideV); /* (call before setup() to override defaults) set the stride of the input image planes */

  bool setup(VideoSettings settings); /* set state that is used in initialize() */
  bool initialize(); /* initialize all members, first set all state (e.g. call setup()) */
  bool createDecoderConfigurationRecord(AVCDecoderConfigurationRecord& rec); /* generates the SPS and PPS nal units */
  bool encodePacket(AVPacket* p, FLVTag& result);
  bool shutdown(); /* deinitialize everything, this is necessary if you want to reinitialize the encoder */

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
  uint32_t strides[3]; /* make sure to call setStrides(..) before you call setup() else we fall back to our defaults. Strides of the input image, when not set we assumes that the input is using 3 separate buffers and using I420P */
  std::ofstream ofs; /* only used when writing to a file */
};

inline uint8_t VideoEncoder::getFPS() {
  return settings.fps;
}

#endif
