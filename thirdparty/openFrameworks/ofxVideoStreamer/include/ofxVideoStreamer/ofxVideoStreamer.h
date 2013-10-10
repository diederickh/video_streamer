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
 

  # ofxVideoStreamer

  This class is a basic and simple to use wrapper around the 
  Video Streamer library (git@github.com:roxlu/video_streamer.git) for
  openFrameworks. This can be used to stream video + audio to a flash 
  media server or alike (tested with wowza, flash media server and 
  c++ rtmpd server).

  First thing you need to do is create a XML settings file. The 
  file: _streamer/include/streamer/videostreamer/VideoStreamerConfig.h_
  contains a complete description of the settings you can use, or see
  below the one I used while testing with wowza.


  _Example XML (see VideoStreamerConfig.h for the most up to date example)_
  ````xml

     <?xml version="1.0" encoding="UTF-8"?>
     <videostreamer>
     
       <settings>
         <default_stream_id>0</default_stream_id>
       </settings>
     
       <streams>
     
         <stream>
     
           <id>0</id>
           <server>
             <url>rtmp://test.home.roxlu.com/flvplayback/livestream</url>
           </server>
     
           <video>
             <width>640</width>
             <height>360</height>
             <fps>15</fps>
             <bitrate>600</bitrate>
             <threads>4</threads>
           </video>
     
           <audio>
             <samplerate>44100</samplerate>        <!-- samplerate: 44100, 22050, 11025 -->
             <mode>2</mode>                        <!-- mode: mono = 1, stereo = 2 -->
             <bitsize>2</bitsize>                  <!-- bitsize: S8 = 0, S16 = 2, F32 = 3 -->
             <bitrate>64</bitrate>                 <!-- in kbps -->
             <quality>5</quality>                  <!-- quality to use, value between 0 and 9, 0 = best (slow), 9 = worst (fast), 5 is ok -->
             <in_bitsize>3</in_bitsize>            <!-- we have basic support for conversion, set this to the input  bitsize, see bitsize above for the values you can use  -->
             <in_interleaved>1</in_interleaved>    <!-- 0 = not using interleaved audio, 1 = using interleaved audio -->
           </audio>
     
         </stream>
     
       </streams>
     </videostreamer>

  ````
  

  _Example usage_
  ````c++

  void testApp::setup() {
    // STREAMER
    // --------------------------------------------------
    int video_w = 640;
    int video_h = 360;

    if(!streamer.setup("streamer.xml", ofGetWidth(), ofGetHeight(), video_w, video_h)) {
      printf("error: cannot setup the streamer.\n");
      ::exit(EXIT_FAILURE);
    }
    
    if(!streamer.start()) {
      printf("error: cannot start the streamer.\n");
      ::exit(EXIT_FAILURE);
    }

    // our ofSoundStream to get audio!
    sound_stream.listDevices(); 
    sound_stream.setup(this, 0, 2, 44100, 1024, 4);
 }

  void testApp::audioIn(float* input, int nsize, int nchannels) {
    streamer.addAudio(input, nsize, nchannels);
  }

  void testApp::draw(){
    if(streamer.wantsNewFrame()) {
      streamer.beginGrab();
        drawVideo();
      streamer.endGrab();
    }
    drawVideo();
  }
 
  ````
 
  

 */
#ifndef ROXLU_OFXVIDEOSTREAMER_H
#define ROXLU_OFXVIDEOSTREAMER_H

#include "ofMain.h"

#include <streamer/videostreamer/VideoStreamer.h>
#include <streamer/core/MemoryPool.h>
#include <streamer/core/AudioEncoderFAAC.h>
#include <hwscale/opengl/YUV420PGrabber.h>
#include <string>

class ofxVideoStreamer {
 public:
  ofxVideoStreamer();
  ~ofxVideoStreamer();

  bool setup(std::string settingsFile, int winW, int winH, int vidW, int vidH);
  bool start();

  
  bool wantsNewFrame(); 
  void beginGrab();
  void endGrab();
  void update();                                          /* call this often; this will make sure we reconnect when we get disconnected by the remote media server */
  void draw();
  unsigned long getNumSamplesNeededForAudioEncoding();     /* the audio encoder (AAC) needs a fixed set of audio sample for each 'encoding' call. this function returns the number of samples. call this after you've called setup(). Simply divide by the number of channels to get the number of frames per encoding step */
  void addAudio(float* input, int nsize, int nchannels);   /* input is the audio buffer from OF, nsize number of frames and nchannels the number of channels must be 2, when using AAC you must scale this value! see the AAC encoder */

 private:
  AudioEncoderFAAC aac;                                    /* the ofxVideoStreamer only works with the AAC encoder to maximize compatibility (with HLS streaming to ios for example) */
  VideoStreamer streamer;
  YUV420PGrabber grabber;
  MemoryPool memory_pool;
  bool has_new_frame;
  bool has_allocated_audio_pool;                           /* we allocate the audio pool when we receive the first audio frame, @todo move this to setup() */
};

inline bool ofxVideoStreamer::wantsNewFrame() {
  return grabber.hasNewFrame();
}

inline unsigned long ofxVideoStreamer::getNumSamplesNeededForAudioEncoding() {
  return aac.getSamplesNeededForEncoding();
}

inline void ofxVideoStreamer::update() {
  streamer.update();
}

#endif
