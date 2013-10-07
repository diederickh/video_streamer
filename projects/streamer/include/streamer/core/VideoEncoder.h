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

// --------------------------------------------------

void videoencoder_x264_log(void* param, int level, const char* fmt, va_list arg);

// --------------------------------------------------

class VideoEncoder {
 public:
  VideoEncoder();
  ~VideoEncoder();
  bool setup(VideoSettings settings); /* set state that is used in initialize() */
  void setStreamID(int32_t id); /* set the id of the stream for which this VideoEncoder is used. When doing multiple video streams (different qualities per stream), each stream uses an unique ID. */
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
  x264_t* encoder; /* our x264 encoder instance ... */
  x264_param_t params; /* parameters that we pass into the x264 encoder */
  x264_picture_t pic_in; /* passed -into- the encoder */
  x264_picture_t pic_out; /* contains an encoded picture */
  VideoSettings settings; /* the video settings like width, height, bitrate etc.. */
  bool vflip; /* flip video, defaults to true */
  uint32_t frame_num; /* current encoded frame number */
  int32_t stream_id; /* an AVPacket might contain video data for multiple quality streams (multi streams). Each stream uses its own VideoEncoder instance with the correct parameters. By setting the stream id to a value >= 0 (see setStreamID()), we will select the correct strides and planes from the AVPacket data */
  std::ofstream ofs; /* only used when writing to a file */
};

inline uint8_t VideoEncoder::getFPS() {
  return settings.fps;
}

inline void VideoEncoder::setStreamID(int32_t id) {
  stream_id = id;
}

#endif
