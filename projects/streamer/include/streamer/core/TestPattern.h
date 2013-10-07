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


  # Test Pattern

  Generates a test YUV/I420p pattern and audio pattern

  When you call `openFile()` and `writeFrameToFile()` you can use avconv to 
  convert the raw yuv file into a video using:

  ````sh
  ./avconv -f rawvideo  -s 320x240 -i raw.yuv -r 10 -vcodec h264 out.mov
  ````

  Simple usage (@todo - this needs to be updated slightly) : 
  ````c++
  #include <streamer/core/TestPattern.h>

  TestPattern tp;
  if(!tp.setup(320, 240, 25, 44100)) {
     printf("error: cannot setup.\n");
     ::exit(EXIT_FAILURE);
  }

  tp.start();
  
  while(true) {

    tp.update();

    // timestamp not tested in this pseude code
    if(tp.hasVideoFrame()) {
       AVPacket* vid_pkt = new AVPacket();
       vid_pkt->allocate(320 * 240 + (320 * 0.5 * 240 * 0.5) * 2);      
       tp.generateVideoFrame(vid_pkt->data);
       vid_pkt->makeVideoPacket();
       vid_pkt->setTimeStamp(tp.timestamp);
       streamer.addVideo(vid_pkt);
    }

    // timestamp not tested in this pseude code
    if(tp.hasAudioFrame()) {
        AVPacket* audio_pkt = new AVPacket();
        tp.generateAudioFrame(audio_pkt->data);
        audio_pkt->makeAudioPacket();
        audio_pkt->setTimeStamp(tp.timestamp);
        streamer.addAudio(audio_pkt);
    }
  }
  ````
 */
#ifndef ROXLU_TEST_PATTERN_H
#define ROXLU_TEST_PATTERN_H

extern "C" {
#  include <uv.h>
}

#include <stdint.h>
#include <vector>
#include <fstream>
#include <streamer/core/EncoderTypes.h>

class TestPattern {

 public:
  TestPattern();
  ~TestPattern();

  bool setup(int width, int height, int fps, int samplerate);

  void start();  /* start the generator; sets the delays + timeouts */

  bool hasVideoFrame(); /* returns true when you should call generateVideoFrame() */
  bool hasAudioFrame(); /* returns true when you should call generateAudioFrame() */ 

  void update();
  void generateVideoFrame(std::vector<uint8_t>& result, uint8_t* planes[], uint32_t* strides);
  uint32_t generateAudioFrame(std::vector<uint8_t>& result);  // generates a stereo audio signal

  bool openFile(std::string file);
  bool writeFrameToFile(std::vector<uint8_t>& result);
  
  uint32_t getTimeStamp();
  uint32_t getNumAudioBytes(); /* get the number of bytes that will be generated for each generate audio frame call */
  uint32_t getNumVideoBytes(); /* returns the number of bytes that we need for the Y, U and V planes */

 public:
  int w;
  int h;
  int fps;
  unsigned long frame_num;
  std::ofstream ofs;
  float duration;

  uint32_t samplerate;
  uint32_t sample_num;

  int64_t prev_video_timeout;
  int64_t video_timeout; /* the timeout when we need to call hasVideoFrame().  hasVideoFrame() will return true if timeout has been reached; and will set a new timeout */
  int64_t video_diff; /* difference between two frames that is used to anticipate on cpu delays */
  int64_t video_diff_ravg; /* "" avarage */
  int64_t video_delay; 
  uint64_t audio_samples; /* how many samples do you want per "sample time"..  this is used to set the audio delay .. defaults to 1024 */
  uint64_t audio_timeout; /* the timeout when we need to call generateAudioFrame(). hasAudioFrame() will return true if time has been reached and will set a new timeout */
  uint64_t audio_delay; /* time which is necessary for 1024 samples */
  uint64_t time_started; /* time in millis when we started */
  uint32_t timestamp;
};

inline uint32_t TestPattern::getTimeStamp() {
  return timestamp;
}

inline uint32_t TestPattern::getNumVideoBytes() {

  if(!w || !h) {
    printf("error: cannot calculate number of bytes; no width or height set.\n");
    ::exit(EXIT_FAILURE);
  }

  return w * h * 2;
}

inline uint32_t TestPattern::getNumAudioBytes() {
  return audio_samples * 2  * sizeof(int16_t); 
}

inline bool TestPattern::hasVideoFrame() {
  bool wants = video_timeout && uv_hrtime() >= video_timeout;
  if(wants) {
    timestamp = (uint32_t)((uv_hrtime()/1000000-time_started));
    prev_video_timeout = video_timeout;

    if(prev_video_timeout) {
      video_diff = (prev_video_timeout - uv_hrtime());
      video_diff_ravg = 0.9 * video_diff_ravg + 0.1 * video_diff;
      video_delay = ((1.0f/fps) * 1000 * 1000 * 1000) + video_diff_ravg;
      //printf("diff: %lld, ravg: %lld, new_delay: %lld\n", video_diff, video_diff_ravg, video_delay);
  
    }
    video_timeout = uv_hrtime() + video_delay;
    frame_num++;
  }
  return wants;
}

inline bool TestPattern::hasAudioFrame() {
  bool wants = audio_timeout && uv_hrtime() >= audio_timeout;
  if(wants) {
    timestamp = (uint32_t)((uv_hrtime()/1000000-time_started));
    audio_timeout = uv_hrtime() + audio_delay;
  }
  return wants;
}
#endif
