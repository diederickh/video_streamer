#include <cmath>
#include <assert.h>
#include <streamer/core/TestPattern.h>

TestPattern::TestPattern() 
  :w(0)
  ,h(0)
  ,frame_num(0)
  ,duration(0)
  ,samplerate(44100)
  ,sample_num(0)
  ,video_timeout(0)
  ,video_delay(0)
  ,audio_timeout(0)
  ,audio_delay(0)
  ,audio_samples(1024)
  ,time_started(0)
  ,prev_video_timeout(0)
  ,video_diff(0)
  ,video_diff_ravg(0)
  ,timestamp(0)
{
}

TestPattern::~TestPattern() {

  if(ofs.is_open()) {
    ofs.close();
  }

}

bool TestPattern::setup(int width, int height, int framespersec, int samprate) {
  assert(width > 0);
  assert(height > 0);
  assert(framespersec > 0);
  w = width;
  h = height;
  fps = framespersec;
  samplerate = samprate;

  video_delay = (1.0f/framespersec) * 1000 * 1000 * 1000; 
  audio_delay = (audio_samples * (1.0f/samplerate)) * 1000 * 1000 * 1000;

  
  return true;
}

void TestPattern::start() {
  video_timeout = uv_hrtime() + video_delay;
  audio_timeout = uv_hrtime() + audio_delay;
  time_started = uv_hrtime() / 1000000;
}

void TestPattern::update() {
  duration = 0.001 * ((uv_hrtime()/1000000.0f) - time_started);
}

void TestPattern::generateVideoFrame(std::vector<uint8_t>& result, uint8_t* planes[], uint32_t* strides) {
  int y_bytes = w * h;
  int uv_bytes = w * h / 4;
  if(result.size() < (uv_bytes+y_bytes)) {
    result.assign( y_bytes + uv_bytes + uv_bytes, 0x00 );
  }

  // set planes
  planes[0] = &result[0];
  planes[1] = &result[y_bytes]; 
  planes[2] = &result[y_bytes + uv_bytes];

  // set the strides
  strides[0] = w;
  strides[1] = w >> 1;
  strides[2] = w >> 1;

  // Y channel
  int POT = 3;
  for(int i = 0; i < w; ++i) {
    for(int j = 0; j < h; ++j) {
      int dx = j * w + i;
      result[dx] = (((i ^ j) >> POT) & 1) - 1; // checkerbord
    }
  }

  // U channel
  float t = 0.5 + 0.5 * sin(duration * 3.14159);
  if(t < 0.5) {
    t = 0.0f;
  }
  else {
    t = 1.0f;
  }

  int offset = y_bytes;

  for(int i = 0; i < uv_bytes; ++i) {
    int dx = offset + i;
    result[dx] = t * 0xFF;
  }

  // V channel
  offset = y_bytes + uv_bytes;
  for(int i = 0; i < uv_bytes; ++i) {
    int dx = offset + i;
    result[dx] = (1.0 - t) * 0xFF;
  }
}

uint32_t TestPattern::generateAudioFrame(std::vector<uint8_t>& result) {
  uint32_t samples_end = sample_num + audio_samples;
  float t = sin(3.14159 * duration);
  float base_vol = 0.3;
  if(t <= 0.0 ) {
    t = 0;
    base_vol = 0.0f;
  }
  else {
    t = 1;
  }
  float volume = base_vol + t * 0.2; // 1.0 = loudest
  
  float freq = 240.0;
  size_t dx = 0;
  for(uint32_t i = sample_num; i < samples_end; ++i) {
    float a = (2 * 3.14159) * i / (samplerate / freq);
    int16_t v = (sin(a) * volume) * 65535;
    uint8_t* ptr = (uint8_t*)&v;
    
    // channel 0
    result[dx++] = ptr[0];
    result[dx++] = ptr[1];

    // channel 1
    result[dx++] = ptr[0];
    result[dx++] = ptr[1];
  }
  sample_num += audio_samples;
  return audio_samples;
}

bool TestPattern::openFile(std::string path) {
  ofs.open(path.c_str(), std::ios::out | std::ios::binary);
  if(!ofs.is_open()) {
    printf("error: cannot open: %s\n", path.c_str());
    return false;
  }
  return true;
}

bool TestPattern::writeFrameToFile(std::vector<uint8_t>& result) {

  if(!ofs.is_open()) {
    printf("error: the file is not opened! call openFile first.\n");
    return false;
  }

  ofs.write((char*)&result.front(), result.size());
  return true;
}

