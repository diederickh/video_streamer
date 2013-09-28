#include <iostream>
#include <GLFW/glfw3.h>
#include <glext.h>
#include <cmath>
#include <hwscale/opengl/YUV420PGrabber.h>
#include <hwscale/opengl/ResizerFBO.h>
#include "Animation.h"
#include <grapher/opengl/OpenGLProfiler.h>
#include <grapher/Graph.h>

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
  int w = 640;
  int h = 480;
  win = glfwCreateWindow(w, h, "hwscale", NULL, NULL);
  if(!win) {
    glfwTerminate();
    exit(EXIT_FAILURE);
  }

  glfwSetFramebufferSizeCallback(win, resize_callback);
  glfwSetKeyCallback(win, key_callback);
  glfwMakeContextCurrent(win);
  glfwSwapInterval(1);
 
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_MULTISAMPLE_ARB);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  Animation anim;
  if(!anim.setup(w, h)) {
    printf("error: cannot setup the animation.\n");
    ::exit(EXIT_FAILURE);
  }

  YUV420PGrabber grabber;
  if(!grabber.setup(w, h, w, h, 15)) {
    printf("error: cannot setup the YUV420PGrabber.\n");
    ::exit(EXIT_FAILURE);
  }
  grabber.start();

  ResizerFBO resizer(grabber.getYTex(), grabber.getUTex(), grabber.getVTex());
  bool r = resizer.setup(grabber.getWidth(), grabber.getHeight(), 
                         grabber.getWidth() * 0.5, grabber.getHeight() * 0.5,
                         grabber.getWidth() * 0.25, grabber.getHeight() * 0.25
                         );
  if(!r) {
    printf("error: cannot setup the ResizerFBO.\n");
    ::exit(EXIT_FAILURE);
  }
    
  GRAPHER_INIT("yuv_grapher_client.xml");
  GRAPHER_START();
  GRAPHER_ADD_GPU_MARKER(0, "frame_start");
  GRAPHER_ADD_GPU_MARKER(0, "frame_end");

  /*
  Graph graph;
  if(!graph.load("client.xml")) {
    printf("error: cannot load client.xml.\n");
    ::exit(EXIT_FAILURE);
  }
  graph.start();
  */

  while(!glfwWindowShouldClose(win)) {

    double t = glfwGetTime();
    glClearColor(0.5 + 0.5 * sin(t), 0.5 + 0.5 * sin(t * 1.3), 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

#if 1  
    if(grabber.hasNewFrame()) {
      grabber.beginGrab();
        anim.draw();
      grabber.endGrab();
      grabber.draw();
      GPU_MARKER(0, "frame_start");
      resizer.resize(grabber);
      GPU_MARKER(0, "frame_end");
    }
    else {
      anim.draw();
    }
#else 
    anim.draw();
#endif

    GRAPHER_UPDATE();
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
