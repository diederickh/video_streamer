/*

  # Test Pattern

  Generates a test YUV/I420p pattern and audio pattern

  When you call `openFile()` and `writeFrameToFile()` you can use avconv to 
  convert the raw yuv file into a video using:

  ````sh
  ./avconv -f rawvideo  -s 320x240 -i raw.yuv -r 10 -vcodec h264 out.mov
  ````

  Simple usage: 
  ````c++
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

class TestPattern {

 public:
  TestPattern();
  ~TestPattern();

  bool setup(int width, int height, int fps, int samplerate);

  void start();  /* start the generator; sets the delays + timeouts */
  bool hasVideoFrame(); /* returns true when you should call generateVideoFrame() */
  bool hasAudioFrame(); /* returns true when you should call generateAudioFrame() */ 

  void update();
  void generateVideoFrame(std::vector<uint8_t>& result);
  uint32_t generateAudioFrame(std::vector<uint8_t>& result);  // generates a stereo audio signal

  bool openFile(std::string file);
  bool writeFrameToFile(std::vector<uint8_t>& result);

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
