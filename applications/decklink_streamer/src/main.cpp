extern "C" {
#  include <uv.h>

#  define GLFW_INCLUDE_NONE
#  include <GLXW/glxw.h>
#  include <GLFW/glfw3.h>
}

#include <signal.h>
#include <fstream>
#include <iostream>
#include <streamer/videostreamer/VideoStreamer.h>
#include <streamer/videostreamer/VideoStreamerConfig.h>
#include <streamer/core/MemoryPool.h>
#include <decklink/DeckLink.h>
#include <libyuv/libyuv.h>
#include <tinylib/tinylib.h>

#include <decklink_streamer/FastI420Upload.h>
#include <hwscale/opengl/YUV420PGrabber.h>

#define WRITE_INPUT_TO_FILE 0
#define WRITE_YUV420_TO_FILE 0

static const char* DEBUG_VS = ""
  "#version 150\n"
  "const vec2 verts[4] = vec2[] ("
  "  vec2(-1.0, 1.0), "
  "  vec2(-1.0, -1.0), "
  "  vec2(1.0, 1.0), "
  "  vec2(1.0, -1.0) "
  ");"
  "const vec2 texcoords[4] = vec2[] ("
  "  vec2(0.0, 0.0), "
  "  vec2(0.0, 1.0), "
  "  vec2(1.0, 0.0), "
  "  vec2(1.0, 1.0)  "
  ");"
  "out vec2 v_texcoord;"
  "void main() {"                               
  "  gl_Position = vec4(verts[gl_VertexID], 0.0, 1.0);"
  "  v_texcoord = texcoords[gl_VertexID];"
  "}"
  ;

static const char* DEBUG_FS = "" 
  "#version 150\n"
  "uniform sampler2D y_tex;"
  "uniform sampler2D u_tex;"
  "uniform sampler2D v_tex;"
  "in vec2 v_texcoord;"
  "out vec4 fragcolor;"
  "const vec3 R_cf = vec3(1.164383,  0.000000,  1.596027);"
  "const vec3 G_cf = vec3(1.164383, -0.391762, -0.812968);"
  "const vec3 B_cf = vec3(1.164383,  2.017232,  0.000000);"
  "const vec3 offset = vec3(-0.0625, -0.5, -0.5);"
  "void main() {"
  "  float y = texture(y_tex, v_texcoord).r;"
  "  float u = texture(u_tex, v_texcoord).r;"
  "  float v = texture(v_tex, v_texcoord).r;"
  "  fragcolor.a = 1.0; "

#if 1  
  "  vec3 yuv = vec3(y,u,v);"
  "  yuv += offset;"
  "  fragcolor = vec4(0.0, 0.0, 0.0, 1.0);"
  "  fragcolor.r = dot(yuv, R_cf);"
  "  fragcolor.g = dot(yuv, G_cf);"
  "  fragcolor.b = dot(yuv, B_cf);"
#else
  // source: http://www.fourcc.org/source/YUV420P-OpenGL-GLSLang.c
  "  y = 1.1643 * (y-0.0625);"
  "  u = u - 0.5; "
  "  v = v - 0.5;"
  "  fragcolor.r = y + 1.5958 * v;"
  "  fragcolor.g = y - 0.39173 * u - 0.81290 * v;"
  "  fragcolor.b = y + 2.017 * u;"
#endif

  "}";
 
void key_callback(GLFWwindow* win, int key, int scancode, int action, int mods);
void error_callback(int err, const char* desc);
void resize_callback(GLFWwindow* window, int width, int height);
void frame_callback(IDeckLinkVideoInputFrame* vframe, IDeckLinkAudioInputPacket* aframe, void* user);

#define INPUT_MODE 1 
#if INPUT_MODE == 0
#  define INPUT_WIDTH 1920
#  define INPUT_HEIGHT 1080
#  define INPUT_NBYTES 3029400
#  define INPUT_NAME bmdModeHD1080p24
#elif INPUT_MODE == 1
#  define INPUT_WIDTH 720
#  define INPUT_HEIGHT 576
#  define INPUT_NBYTES 829440
#  define INPUT_NAME bmdModePAL
#else
#   error "Undefined size."
#endif

VideoStreamer streamer;
YUV420PGrabber grabber;
FastI420Upload fast_upload;

uint8_t* yuv420p = NULL;
uv_mutex_t frame_mutex;
bool new_frame;

GLuint vao;
GLuint vert;
GLuint frag;
GLuint prog;

std::ofstream ofs_raw; // yuv to write the data we get from decklink
std::ofstream ofs_yuv; // used to write the converted  yuv

int main() {


  printf("DeckLink Stream.\n");

  if(!glfwInit()) {
    printf("error: cannot setup glfw.\n");
    return false;
  }

  new_frame = false;
  DeckLink dl;
  dl.printDevices();

  if(!dl.setup(0)) {
    ::exit(EXIT_FAILURE);
  }

  if(!dl.setCallback(frame_callback, NULL)) {
    ::exit(EXIT_FAILURE);
  }

  if(!dl.setVideoMode(INPUT_NAME, bmdFormat8BitYUV)) {
    ::exit(EXIT_FAILURE);
  }
 
  int yuv420_nbytes = INPUT_WIDTH * INPUT_HEIGHT * 2;
  printf("yuv420_nbytes: %d\n", yuv420_nbytes);
  yuv420p = new uint8_t[yuv420_nbytes];
  if(!yuv420p) {
    printf("error: cannot allocated the buffer to hold the yuv420 data.\n");
    ::exit(EXIT_FAILURE);
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

  // before calling any other GL function you need to initialize!
  if(glxwInit() != 0) {
    printf("error: cannot initialize glxw.\n");
    ::exit(EXIT_FAILURE);
  }
 
  glDisable(GL_DEPTH_TEST);
  glEnable(GL_MULTISAMPLE);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

#if WRITE_INPUT_TO_FILE
  std::string inputpath = rx_get_exe_path() +"input.raw";
  ofs_raw.open(inputpath.c_str(), std::ios::out | std::ios::binary);
  if(!ofs_raw.is_open()) {
    printf("error: cannot open output file.\n");
    ::exit(EXIT_FAILURE);
  }
#endif

#if WRITE_YUV420_TO_FILE
  std::string yuvpath = rx_get_exe_path() +"yuv420p.raw";
  ofs_yuv.open(yuvpath.c_str(), std::ios::out | std::ios::binary);
  if(!ofs_yuv.is_open()) {
    printf("error: cannot open output file for yuv420p.\n");
    ::exit(EXIT_FAILURE);
  }
#endif 

  grabber.addSize(0, w >> 2, h >> 2);
  if(!grabber.setup(w, h, 25)) {
    //  if(!grabber.setup(w, h, streamer.getVideoWidth(), streamer.getVideoHeight(), 25)) {
    printf("error: cannot setup the yuv grabber.\n");
    ::exit(EXIT_FAILURE);
  }
  // load settings.
  std::string config_path = rx_get_exe_path() +"decklink_streamer.xml";
  if(!streamer.loadSettings(config_path)) {
    printf("error: could not load the settings for the streamer.\n");
    ::exit(EXIT_FAILURE);
  }

  YUV420PSize size = grabber.getSize(0);
  streamer.setStrides(size.strides[0], size.strides[1], size.strides[2]);
  streamer.setVideoWidth(size.yw);
  streamer.setVideoHeight(size.yh);
  printf("video output: %d x %d\n", size.yw, size.yh);

  if(!streamer.setup()) {
    printf("error: could not setup the streamer.\n");
    ::exit(EXIT_FAILURE);
  }
  if(!streamer.start()) {
    printf("error: cannot start streamer.\n");
    ::exit(EXIT_FAILURE);
  }

  uv_mutex_init(&frame_mutex);

  if(!fast_upload.setup(INPUT_WIDTH, INPUT_HEIGHT)) {
    ::exit(EXIT_FAILURE);
  }

  if(!dl.start()) {
    ::exit(EXIT_FAILURE);
  }

  uint32_t offset_u = INPUT_WIDTH * INPUT_HEIGHT;
  uint32_t offset_v = offset_u + (INPUT_WIDTH * 0.5) * (INPUT_HEIGHT * 0.5);
  uint8_t* dest_y = yuv420p;
  uint8_t* dest_u = &yuv420p[offset_u];
  uint8_t* dest_v = &yuv420p[offset_v];
  printf("offset_u: %d, offset_v: %d\n", offset_u, offset_v);

  glGenVertexArrays(1, &vao);
  glBindVertexArray(vao);
  vert = rx_create_shader(GL_VERTEX_SHADER, DEBUG_VS);
  frag = rx_create_shader(GL_FRAGMENT_SHADER, DEBUG_FS);
  prog = rx_create_program(vert, frag);
  glLinkProgram(prog);
  glUseProgram(prog);

  glUniform1i(glGetUniformLocation(prog, "y_tex"), 0);
  glUniform1i(glGetUniformLocation(prog, "u_tex"), 1);
  glUniform1i(glGetUniformLocation(prog, "v_tex"), 2);

  grabber.start();

  MemoryPool memory_pool; // we use a memory pool that provides us with AVPackets
  memory_pool.allocateVideoFrames(10, grabber.getNumBytes());

  uint32_t start_time = grabber.getTimeStamp();

  while(!glfwWindowShouldClose(win)) {
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    //glGetIntegerv(GL_READ_BUFFER, &buf);
    //printf("BUF: %d, GL_BACK_LEFT: %d, GL_BACK_RIGHT: %d, GL_FRONT: %d, GL_BACK: %d\n", buf, GL_BACK_LEFT, GL_BACK_RIGHT, GL_FRONT, GL_BACK);
    uv_mutex_lock(&frame_mutex);
    {
      if(new_frame) {
        fast_upload.update(dest_y, dest_u, dest_v);
        new_frame = false;
      }
    }
    uv_mutex_unlock(&frame_mutex);

    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    if(grabber.hasNewFrame()) {

      grabber.beginGrab();
      
      glBindVertexArray(vao);
      glUseProgram(prog);

      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_2D, fast_upload.tex_y);

      glActiveTexture(GL_TEXTURE1);
      glBindTexture(GL_TEXTURE_2D, fast_upload.tex_u);

      glActiveTexture(GL_TEXTURE2);
      glBindTexture(GL_TEXTURE_2D, fast_upload.tex_v);

      glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

      grabber.endGrab();

      grabber.downloadTextures();

#if 1
      uint32_t timestamp = streamer.getTimeStamp();
      uint32_t timediff = timestamp - start_time;

      AVPacket* pkt = memory_pool.getFreeVideoPacket();
      if(pkt) {
        pkt->setTimeStamp(grabber.getTimeStamp());
        pkt->makeVideoPacket();
        pkt->data.assign(grabber.getPtr(), grabber.getPtr() + grabber.getNumBytes());

        YUV420PSize size = grabber.getSize(0);
        pkt->y_offset = size.y_offset;
        pkt->u_offset = size.u_offset;
        pkt->v_offset = size.v_offset;
        if(!streamer.addVideo(pkt)) {
          printf("warning: cannot add a video packet .. probably disconnect...\n");
          pkt->release();
        }
      }
      else {
        printf("warning: cannot get a free video packet. add more to the pool and make sure that you run this in release mode.\n");
      }

#endif
      //      grabber.draw();
    }
    grabber.draw();
    //else
#if 0
    glBindVertexArray(vao);
    glUseProgram(prog);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, fast_upload.tex_y);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, fast_upload.tex_u);

    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, fast_upload.tex_v);

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
#endif
    //    }

    glfwSwapBuffers(win);
    glfwPollEvents();
  }

  dl.stop();

  uv_mutex_destroy(&frame_mutex);

#if WRITE_YUV_TO_FILE
  if(ofs.is_open()) {
    ofs.close();
  }
#endif

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

void frame_callback(IDeckLinkVideoInputFrame* vframe, IDeckLinkAudioInputPacket* aframe, void* user) {

  if(!vframe) {
    printf("-- invalid vframe.\n");
    return;
  }

  uint32_t w = vframe->GetWidth();
  uint32_t h = vframe->GetHeight();
  uint32_t stride = vframe->GetRowBytes();
  uint8_t* uyvy422 = NULL;

  if(w != INPUT_WIDTH || h != INPUT_HEIGHT) {
    printf("error: the input size is not the same as the one defined.\n");
    ::exit(EXIT_FAILURE);
  }

  HRESULT r = vframe->GetBytes((void**)&uyvy422);
  if(r != S_OK) {
    printf("error: cannot get yuv bytes.\n");
    return ;
  }

#if WRITE_INPUT_TO_FILE
  if(ofs_raw.is_open()) {
    ofs_raw.write((char*)uyvy422, stride * h);
  }
#endif

  uv_mutex_lock(&frame_mutex);
  {
    new_frame = true;
    uint32_t offset_u = INPUT_WIDTH * INPUT_HEIGHT;
    uint32_t offset_v = offset_u + (INPUT_WIDTH * 0.5) * (INPUT_HEIGHT * 0.5);
    uint8_t* dest_y = yuv420p;
    uint8_t* dest_u = &yuv420p[offset_u];
    uint8_t* dest_v = &yuv420p[offset_v];

    uint64_t n = uv_hrtime();
    libyuv::UYVYToI420(uyvy422, stride,
                       dest_y, w, 
                       dest_u, w >> 1,
                       dest_v, w >> 1,
                       w, h);
    uint64_t d = uv_hrtime() - n;

#if WRITE_YUV420_TO_FILE
    int nbytes = INPUT_WIDTH * INPUT_HEIGHT * 2;
    ofs_yuv.write((char*)yuv420p, nbytes);
#endif    
    
  }
  uv_mutex_unlock(&frame_mutex);

}

