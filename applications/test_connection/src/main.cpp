/*

  # Connection Test
  
  Basic example that shows how to stream video to a Flash Media Server or alike, 
  using the `TestPattern` generator. This `TestPattern` generates a simple red/yellow,
  blue checkerboard pattern and a simple sine wave audio stream.

  See the connection_test.xml file in the install directory where you can set the 
  url of the media server and widht/height etc.. 

 */

#define MIC_IN 1 /* do you want to use microphone input? Set to 1 or 0  */
#define USE_AUDIO 1 /* set to 1 if you want audio */

#include <signal.h>
#include <iostream>
#include <string>
#include <tinylib/tinylib.h>
#include <streamer/videostreamer/VideoStreamer.h>
#include <streamer/core/TestPattern.h>
#include <streamer/core/MemoryPool.h>
#include <streamer/core/Log.h>

#if MIC_IN
#  include <portaudio/PAudio.h>
   void on_audio_in(const void* input, unsigned long nframes, void* user);
#endif

MemoryPool mempool;
VideoStreamer vs;
TestPattern tp;

bool must_run = false;

void sighandler(int signum);

int main() {

  std::string settings_file = rx_get_exe_path() +"connection_test.xml";
  if(!vs.loadSettings(settings_file)) {
    STREAMER_ERROR("error: cannot find the connection_test.xml file: %s.", settings_file.c_str());
    ::exit(EXIT_FAILURE);
  }

#if MIC_IN

  AudioSettings audio_settings;
  audio_settings.samplerate = AV_AUDIO_SAMPLERATE_44100;
  audio_settings.mode = AV_AUDIO_MODE_STEREO;
  audio_settings.bitsize = AV_AUDIO_BITSIZE_S16;
  audio_settings.quality = 6;
  audio_settings.bitrate = 64;
  audio_settings.in_bitsize = AV_AUDIO_BITSIZE_S16;
  audio_settings.in_interleaved = true;

  PAudio paudio;
  paudio.listDevices();
  if(!paudio.openInputStream(paudio.getDefaultInputDevice(), 2, paInt16, 44100, 512)) {
    STREAMER_ERROR("error: cannot set port audio.");
    ::exit(EXIT_FAILURE);
  }
  STREAMER_VERBOSE("Using input audio device: %d.", paudio.getDefaultInputDevice());

  paudio.setCallback(on_audio_in, NULL);

  vs.setAudioSettings(audio_settings);

  //std::string output_file = rx_get_exe_path() +"test.flv";
  //vs.setOutputFile(output_file);
#endif


  if(!tp.setup(vs.getVideoWidth(), vs.getVideoHeight(), vs.getFrameRate(), vs.getSampleRate())) {
    STREAMER_ERROR("error: cannot setup the test pattern.");
    ::exit(EXIT_FAILURE);
  }

  STREAMER_VERBOSE("Loaded streamer with: %d x %d @ %d, samplerate: %d.", vs.getVideoWidth(), vs.getVideoHeight(), vs.getFrameRate(), vs.getSampleRate());
  
  mempool.allocateVideoFrames(10, tp.getNumVideoBytes());
  mempool.allocateAudioFrames(512, tp.getNumAudioBytes());

  signal(SIGINT, sighandler);

  if(!vs.setup()) {
    ::exit(EXIT_FAILURE);
  }

  if(!vs.start()) {
    ::exit(EXIT_FAILURE);
  }

#if MIC_IN
  paudio.start();
#endif

  tp.start();

  int nbytes_video = vs.getVideoWidth() * vs.getVideoHeight() + (2 * (vs.getVideoWidth() * 0.5) * (vs.getVideoHeight() * 0.5));

  must_run = true;

  while(must_run) {

    tp.update();

    if(tp.hasVideoFrame()) {
      AVPacket* vid_pkt = mempool.getFreeVideoPacket(); // packet is owned by memory pool and released by 
      if(vid_pkt) {
        tp.generateVideoFrame(vid_pkt->data, vid_pkt->planes, vid_pkt->strides);
        vid_pkt->makeVideoPacket();
        vid_pkt->setTimeStamp(tp.getTimeStamp());
        vs.addVideo(vid_pkt);
      }
    }
    
#if MIC_IN == 0 && USE_AUDIO == 1
    if(tp.hasAudioFrame()) {
      AVPacket* au_pkt = mempool.getFreeAudioPacket();
      if(au_pkt) {
        tp.generateAudioFrame(au_pkt->data);
        au_pkt->setTimeStamp(tp.getTimeStamp());
        vs.addAudio(au_pkt);
      }
      else {
        STREAMER_ERROR("error: cannot get new audio frame.");
      }
    }
#endif
  }



#if MIC_IN
  paudio.stop();
#endif

  return EXIT_SUCCESS;
}


void sighandler(int signum) {
  STREAMER_WARNING("\nStop!\n");
  must_run = false;
}

void on_audio_in(const void* input, unsigned long nframes, void* user) {
#if USE_AUDIO  
  AVPacket* au_pkt = mempool.getFreeAudioPacket();
  if(au_pkt) {
    uint8_t* ptr = (uint8_t*)input;
    size_t nbytes = nframes * sizeof(short int) * 2;
    au_pkt->data.assign(ptr, ptr + nbytes);
    au_pkt->setTimeStamp(tp.getTimeStamp());
    vs.addAudio(au_pkt);
  }
#endif
}
