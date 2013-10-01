/*

  # YUV420p grabber.
  
  Basic example that shows how to stream video to a Flash Media Server or alike
  We show how to use the YUV420PGrabber which can grab the current framebuffer
  and resize it into a couple of different dimensions so you can stream different
  qualities if you need to support this. 


  IMPORTANT: WE DO NOT YET CREATE MULTIPLE ENCODER INSTANCES - SO NO MULTIPLE VIDEO STREAMS YET

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
#include <streamer/core/TestPattern.h>
#include <streamer/core/MemoryPool.h>
#include <hwscale/opengl/YUV420PGrabber.h>
#include "Animation.h"

bool must_run = false;

void sighandler(int signum);
void key_callback(GLFWwindow* win, int key, int scancode, int action, int mods);
void error_callback(int err, const char* desc);
void resize_callback(GLFWwindow* window, int width, int height);

int main() {

  glfwSetErrorCallback(error_callback);

  if(!glfwInit()) {
    printf("error: cannot setup glfw.\n");
    return false;
  }

  glfwWindowHint(GLFW_SAMPLES, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  
  GLFWwindow* win = NULL;
  int w = 1280;
  int h = 720;

  win = glfwCreateWindow(w, h, "DeckLink Streamer", NULL, NULL);
  if(!win) {
    glfwTerminate();
    exit(EXIT_FAILURE);
  }

  glfwSetFramebufferSizeCallback(win, resize_callback);
  glfwSetKeyCallback(win, key_callback);
  glfwMakeContextCurrent(win);
  glfwSwapInterval(1);

  if(glxwInit() != 0) {
    printf("error: cannot initialize glxw.\n");
    ::exit(EXIT_FAILURE);
  }

  Animation anim;
  if(!anim.setup(w, h)) {
    printf("error: cannot setup animation.\n");
    ::exit(EXIT_FAILURE);
  }

  VideoStreamer streamer;
  if(!streamer.loadSettings(rx_get_exe_path() +"yuv420p_grabber.xml")) {
    ::exit(EXIT_FAILURE);
  }

  YUV420PGrabber grabber;
  int use_size_id = 0;

  grabber.addSize(0, streamer.getVideoWidth(), streamer.getVideoHeight());
  grabber.addSize(1, streamer.getVideoWidth() >> 2, streamer.getVideoHeight() >> 2);
  grabber.addSize(2, streamer.getVideoWidth() >> 3, streamer.getVideoHeight() >> 3);

  if(!grabber.setup(w, h, 25)) {
    printf("error: cannot setup grabber.\n");
    ::exit(EXIT_FAILURE);
  }

  // We get a YUV420PSize object from the grabber which contains all the information about
  // how the frame data is stored in memory. We need to get the strides so the encoder
  // knows how to use the input data we pass into the AVPackets. 
  // The YUV420PGrabber creates a stacked version for each of the different video output
  // sizes you've added (see above addSize). We pack the video data like: 
  // http://www.flickr.com/photos/diederick/10027076563/ (though be aware the memory is upside down! 
  // this is a opengl thing
  YUV420PSize size = grabber.getSize(use_size_id);  
  streamer.setStrides(size.strides[0], size.strides[1], size.strides[2]);
  streamer.setVideoWidth(size.yw);
  streamer.setVideoHeight(size.yh);

  if(!streamer.setup()) {
    ::exit(EXIT_FAILURE);
  }

  if(!streamer.start()) {
    ::exit(EXIT_FAILURE);
  }

  // Create a memory pool for our AVPackets - this preallocates video frames because we don't want to allocate on the fly
  size_t bytes_per_frame = grabber.getNumBytes();
  MemoryPool memory_pool;
  memory_pool.allocateVideoFrames(5, bytes_per_frame);

  printf("we need: %ld bytes per frame.\n", bytes_per_frame);
  
#if 0
  grabber.addSize(0, w, h);
  grabber.addSize(1, w >> 1, h >> 1);
  grabber.addSize(2, w >> 2, h >> 2);
#endif

  printf("size: %d\n", size.id);

  grabber.setOutput(use_size_id, rx_get_exe_path() +"raw.yuv");
  grabber.print();
  grabber.start();
  glEnable(GL_CULL_FACE);

  while(!glfwWindowShouldClose(win)) {
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    if(grabber.hasNewFrame()) {
      grabber.beginGrab();
        anim.draw();
      grabber.endGrab();
      grabber.downloadTextures();

      AVPacket* vid = memory_pool.getFreeVideoPacket();
      if(!vid) {
        printf("error: cannot get a free video packet, try to increase the pool size (only when you get this in release mode!)\n");
      }
      else {
        vid->makeVideoPacket();
        vid->setTimeStamp(grabber.getTimeStamp());
        vid->data.assign(grabber.getPtr(), grabber.getPtr() + grabber.getNumBytes()); // takes roughly ~0.7-0.9ms for a video texture of 1900800 bytes (1.8mb)
      
        YUV420PSize size = grabber.getSize(use_size_id);
        vid->y_offset = size.y_offset;
        vid->u_offset = size.u_offset;
        vid->v_offset = size.v_offset;
        streamer.addVideo(vid);
      }
    }
    anim.draw();

    glfwSwapBuffers(win);
    glfwPollEvents();
  }


  glfwTerminate();
  return EXIT_SUCCESS;
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


void sighandler(int signum) {
  printf("\nStop!\n");
  must_run = false;
}

