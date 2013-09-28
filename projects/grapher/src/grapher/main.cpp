#include <iostream>
#include <GLFW/glfw3.h>
#include <glext.h>
#include <roxlu/core/Utils.h>
#include <grapher/GraphDrawer.h>
#include <grapher/Graph.h>
#include <grapher/Shader.h>
#include <grapher/Vertices.h>
#include <grapher/Daemon.h>
#include <grapher/GraphConfigReader.h>


#define USE_TEST_GRAPH 0

void key_callback(GLFWwindow* win, int key, int scancode, int action, int mods);
void error_callback(int err, const char* desc);
void resize_callback(GLFWwindow* window, int width, int height);

Daemon server;
Shader shader;

int main() {

  // get config.
  ServerSettings server_config;
  GraphConfigReader app_config;
  if(!app_config.loadServerConfig("./server.xml", server_config)) {
    printf("error: cannot load the server config.\n");
    ::exit(EXIT_FAILURE);
  }

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
  int w = server_config.width;
  int h = server_config.height;
  win = glfwCreateWindow(w, h, "Grapher", NULL, NULL);
  if(!win) {
    glfwTerminate();
    exit(EXIT_FAILURE);
  }

  glfwSetFramebufferSizeCallback(win, resize_callback);
  glfwSetKeyCallback(win, key_callback);
  glfwMakeContextCurrent(win);
  glfwSwapInterval(1);

  std::string daemon_url = server_config.address;
  server.setup(daemon_url, w, h);
  server.start();

  if(!shader.setup()) {
    printf("error: cannot set the shader.\n");
    ::exit(EXIT_FAILURE);
  }

#if USE_TEST_GRAPH
  Graph g;
  g.load("./client.xml");
  g.start();
#endif

  float pm[16];
  rx_ortho_top_left(w, h, -1.0, 1.0, pm);
  shader.use();
  shader.setPM(pm);

  glDisable(GL_DEPTH_TEST);
  glEnable(GL_MULTISAMPLE_ARB);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  while(!glfwWindowShouldClose(win)) {

#if USE_TEST_GRAPH
    static float i = 0.0;
    i += 0.1;
    g.add(0, "end", sin(i) * 30);
#endif

    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    shader.use();
    server.update();
    server.draw();

    glfwSwapBuffers(win);
    glfwPollEvents();
  }

  server.stop();

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
      server.prev();
      break;
    }
    case GLFW_KEY_RIGHT: {
      server.next();
      break;
    }
    case GLFW_KEY_ESCAPE: {
      glfwSetWindowShouldClose(win, GL_TRUE);
      break;
    }
  };
  
}

void resize_callback(GLFWwindow* window, int width, int height) {
  server.onResize(width, height);

  float pm[16];
  rx_ortho_top_left(width, height, -1.0, 1.0, pm);
  shader.use();
  shader.setPM(pm);
}
