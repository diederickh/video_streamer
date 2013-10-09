/*
  # Multi Video Streamer 
  
  This application will test multiple quality video and audio streams 
  using the MultiVideoStreamer.

 */

#if defined(GLFW_INCLUDE_GLCOREARB)
#  undef GLFW_INCLUDE_GLCOREARB
#  define GFLFW_INCLUDE_NONE
#endif

extern "C" {
#  include <uv.h>
#  include <GLXW/glxw.h>
#  include <GLFW/glfw3.h>
}

#include <signal.h>
#include <iostream>
#include <string>
#include <tinylib/tinylib.h>
#include <streamer/videostreamer/VideoStreamer.h>
#include <streamer/videostreamer/MultiVideoStreamer.h>
#include <streamer/core/TestPattern.h>
#include <streamer/core/MemoryPool.h>
#include <streamer/core/Log.h>
#include <hwscale/opengl/YUV420PGrabber.h>
#include "Animation.h"

#define USE_AUDIO 1

#if USE_AUDIO
   #include <portaudio/PAudio.h>
   void on_audio(const void* input, unsigned long nframes, void* user);
   PAudio audio; 
#endif

MultiVideoStreamer mvs;
MemoryPool mempool;
YUV420PGrabber grabber;
GLFWwindow* win = NULL;
bool must_run = false;

void sighandler(int signum);
void key_callback(GLFWwindow* win, int key, int scancode, int action, int mods);
void error_callback(int err, const char* desc);
void resize_callback(GLFWwindow* window, int width, int height);

int main() {

  // MULTI STREAMER SETUP
  // ----------------------------------------
  std::string settings_file = rx_get_exe_path() +"test_multi_streamer.xml";
  if(!mvs.loadSettings(settings_file)) {
    STREAMER_ERROR("error: cannot find the connection_test.xml file: %s.", settings_file.c_str());
    ::exit(EXIT_FAILURE);
  }

  mvs.print();

  if(!mvs.setup()) {
    STREAMER_ERROR("Cannot setup() the MultiVideoStreamer.\n");
    ::exit(EXIT_FAILURE);
  }

  if(!mvs.start()) {
    STREAMER_ERROR("Cannot start the MultiVideoStreamer.\n");
    ::exit(EXIT_FAILURE);
  }

  signal(SIGINT, sighandler);

  must_run = true;

#if USE_AUDIO

  // AUDIO SETUP
  // ----------------------------------------
  audio.listDevices();
  if(!audio.openInputStream(audio.getDefaultInputDevice(), 2, paInt16, 44100, 1024)) {
    STREAMER_ERROR("error: cannot set port audio.");
    ::exit(EXIT_FAILURE);
  }

  STREAMER_VERBOSE("Using input audio device: %d.", audio.getDefaultInputDevice());
  audio.setCallback(on_audio, NULL);
  
#endif

  // GL SETUP
  // ----------------------------------------
  glfwSetErrorCallback(error_callback);

  if(!glfwInit()) {
    STREAMER_ERROR("error: cannot setup glfw.\n");
    return false;
  }

  glfwWindowHint(GLFW_SAMPLES, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  

  int w = 1280;
  int h = 720;

  win = glfwCreateWindow(w, h, "Multi Video Streamer", NULL, NULL);
  if(!win) {
    glfwTerminate();
    exit(EXIT_FAILURE);
  }

  glfwSetFramebufferSizeCallback(win, resize_callback);
  glfwSetKeyCallback(win, key_callback);
  glfwMakeContextCurrent(win);
  glfwSwapInterval(1);

  if(glxwInit() != 0) {
    STREAMER_ERROR("error: cannot initialize glxw.\n");
    ::exit(EXIT_FAILURE);
  }

  // ANIMATION SETUP
  // -------------------------------------------
  Animation anim;
  if(!anim.setup(w, h)) {
    STREAMER_ERROR("Cannot setup the animation.\n");
    ::exit(EXIT_FAILURE);
  }

  // GRABBER SETUP
  // -------------------------------------------
 
  for(size_t i = 0; i < mvs.size(); ++i) {

    MultiStreamerInfo* msi = mvs[i];
    if(!msi) {
      STREAMER_ERROR("MultiVideoStreamer returned invalid info. Stopping now\n");
      ::exit(EXIT_FAILURE);
    }

    if(!grabber.addSize(msi->id, msi->streamer->getVideoWidth(), msi->streamer->getVideoHeight())) {
      STREAMER_ERROR("Error while adding a size to the YUV420PGrabber. Stopping now.\n");
      ::exit(EXIT_FAILURE);
    }
             
    STREAMER_WARNING("%d %d %d\n", msi->id, msi->streamer->getVideoWidth(), msi->streamer->getVideoHeight());
  }

  if(!grabber.setup(w, h, 15)) {
    STREAMER_ERROR("Cannot setup the YUV420PGrabber. Stopping now.\n");
    ::exit(EXIT_FAILURE);
  }

  if(!mempool.allocateVideoFrames(10, grabber.getNumBytes())) {
    STREAMER_ERROR("Cannot allocate the video frames for the memory pool. Stopping now\n");
    ::exit(EXIT_FAILURE);
  }

  grabber.start();

#if USE_AUDIO
  size_t nbytes_audio = 2 * sizeof(short int) * 512; 
  if(!mempool.allocateAudioFrames(128, nbytes_audio)) {
     STREAMER_ERROR("Cannot allocate the audio frames for the memory pool. Stopping now.\n");
     ::exit(EXIT_FAILURE);
  }
  audio.start();
#endif

  glEnable(GL_CULL_FACE);

  while(!glfwWindowShouldClose(win)) {
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // make sure that we reconnect when we get disconnected
    mvs.update();

    if(grabber.hasNewFrame()) {
      grabber.beginGrab();
        anim.draw();
      grabber.endGrab();
      grabber.downloadTextures();

      AVPacket* vid = mempool.getFreeVideoPacket();
      if(!vid) {
        STREAMER_ERROR("error: cannot get a free video packet, try to increase the pool size (only when you get this in release mode!)\n");
      }
      else {

        grabber.assignPixels(vid->data);

        vid->clearMulti();
        vid->makeMulti();

        // set the correct strides and plane pointers for each of the streams
        for(size_t i = 0; i < mvs.size(); ++i) {
          MultiStreamerInfo* msi = mvs[i];
          MultiAVPacketInfo info;
          grabber.assignPlanes(msi->id, vid->data, info.planes);
          grabber.assignStrides(msi->id, info.strides);
          vid->addMulti(msi->id, info);
        }

        vid->makeVideoPacket();
        vid->setTimeStamp(grabber.getTimeStamp());
        mvs.addVideo(vid);

      }
    }

    anim.draw();

    glfwSwapBuffers(win);
    glfwPollEvents();


  }

#if USE_AUDIO
  audio.stop();
#endif

  mvs.stop();
  mvs.shutdown();
  
  glfwTerminate();

  return EXIT_SUCCESS;
}


void sighandler(int signum) {
  STREAMER_WARNING("\nStop!\n");
  must_run = false;
  glfwSetWindowShouldClose(win, GL_TRUE);
}

void error_callback(int err, const char* desc) {
  printf("glfw error: %s (%d)\n", desc, err);
}

void key_callback(GLFWwindow* win, int key, int scancode, int action, int mods) {
  
  if(action != GLFW_PRESS) {
    return;
  }

  switch(key) {
    case GLFW_KEY_LEFT: {
      break;
    }
    case GLFW_KEY_RIGHT: {
      break;
    }
    case GLFW_KEY_ESCAPE: {
      glfwSetWindowShouldClose(win, GL_TRUE);
      break;
    }
  };
  
}

void resize_callback(GLFWwindow* window, int width, int height) {
}

#if USE_AUDIO
void on_audio(const void* input, unsigned long nframes, void* user) {

  if(nframes != 1024) {
    STREAMER_ERROR("The receive audio buffer does not contain the size we need.\n");
    ::exit(EXIT_FAILURE);
  }

  AVPacket* pkt = mempool.getFreeAudioPacket();                                                                    
  if(!pkt) {
    STREAMER_ERROR("Cannot get a free audio packet!\n");
    return;
  }

  uint8_t* ptr = (uint8_t*)input;
  size_t nbytes = nframes * 2 * sizeof(short int);
  pkt->data.assign(ptr, ptr + nbytes);
  pkt->setTimeStamp(grabber.getTimeStamp());
  mvs.addAudio(pkt);
}
#endif
