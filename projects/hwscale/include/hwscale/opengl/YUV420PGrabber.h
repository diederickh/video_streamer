/*

  # YUV420PGrabber

  This class will grab the draw commands between `beginGrab()` and `endGrab()`, and
  it will convert the grabbed scene to YUV420P using a shader. This class is a lot
  faster then libswscale (www.libav.org), for both color conversion and resizing.

  You can define a cppflag  `-DUSE_APPLE_VAO=1` if you want to use the 
  glGenVertexArraysAPPLE() functions.

 */
#ifndef ROXLU_YUV420P_GRABBER_H
#define ROXLU_YUV420P_GRABBER_H


#if defined(__APPLE__)
#  include <OpenGL/gl3.h>
#  include <OpenGL/glext.h>
#else 
#  include <GLXW/glxw.h>
#endif

extern "C" {
#  include <uv.h>  
}

// -----------------------------------------------------
#if YUV420P_GRABBER_GLSL_VERSION == 120
static const char* YUV420P_Y_VS = ""
  "#version 120\n"
  "#extension GL_EXT_gpu_shader4 : require\n" // we want gl_VertexID
  "const vec2 vert_data[4] = vec2[]("
  "  vec2(-1.0, 1.0),"
  "  vec2(-1.0, -1.0),"
  "  vec2(1.0, 1.0),"
  "  vec2(1.0, -1.0)"
  ");"

  "const vec2 tex_data[4] = vec2[]("
  "  vec2(0.0, 1.0), "
  "  vec2(0.0, 0.0), "
  "  vec2(1.0, 1.0), "
  "  vec2(1.0, 0.0)  "
  ");"

  "varying vec2 v_tex; "

  "void main() { "
  "  gl_Position = vec4(vert_data[gl_VertexID], 0.0, 1.0);"
  "  v_tex = tex_data[gl_VertexID]; "
  "}"
  ;

static const char* YUV420P_Y_FS = ""
  "#version 120\n"
  "uniform sampler2D u_tex;"
  "varying vec2 v_tex;"
  
  "void main() {"
  "  vec3 tc = texture2D(u_tex, v_tex).rgb; "
  "  gl_FragColor.rgba = vec4(1.0);"           // we need to set all channels
  "  gl_FragColor.r = (tc.r * 0.257) + (tc.g * 0.504) + (tc.b * 0.098) + 0.0625;"
  "}"
  ;

static const char* YUV420P_UV_FS = ""
  "#version 120\n"
  "uniform sampler2D u_tex;"
  "varying vec2 v_tex;"
  "void main() {"
  " vec3 tc = texture2D(u_tex, v_tex).rgb; "
  " gl_FragData[0].r = -(tc.r * 0.148) - (tc.g * 0.291) + (tc.b * 0.439) + 0.5; "
  " gl_FragData[1].r =  (tc.r * 0.439) - (tc.g * 0.368) - (tc.b * 0.071) + 0.5; "
  "}"
  ;

static const char* YUV420P_PT_FS = ""
  "#version 120\n"
  "uniform sampler2D u_tex;\n"
  "varying vec2 v_tex; " 
  "void main() {"
  " gl_FragColor = vec4(texture2D(u_tex, v_tex).rgb, 1.0);"
  "}"
  ;
#elif YUV420P_GRABBER_GLSL_VERSION == 150


static const char* YUV420P_Y_VS = ""
  "#version 150\n"
  "const vec2 vert_data[4] = vec2[]("
  "  vec2(-1.0, 1.0),"
  "  vec2(-1.0, -1.0),"
  "  vec2(1.0, 1.0),"
  "  vec2(1.0, -1.0)"
  ");"

  "const vec2 tex_data[4] = vec2[]("
  "  vec2(0.0, 1.0), "
  "  vec2(0.0, 0.0), "
  "  vec2(1.0, 1.0), "
  "  vec2(1.0, 0.0)  "
  ");"

  "out vec2 v_tex; "

  "void main() { "
  "  gl_Position = vec4(vert_data[gl_VertexID], 0.0, 1.0);"
  "  v_tex = tex_data[gl_VertexID]; "
  "}"
  ;

// conversion shader
static const char* YUV420P_Y_FS = ""
  "#version 150\n"
  "uniform sampler2D u_tex;"
  "in vec2 v_tex;"
  "out vec4 fragcol;"
  "void main() {"
  "  vec3 tc = texture(u_tex, v_tex).rgb; "
  "  fragcol.rgba = vec4(1.0);"           // we need to set all channels
  "  fragcol.r = (tc.r * 0.257) + (tc.g * 0.504) + (tc.b * 0.098) + 0.0625;"
  "}"
  ;

static const char* YUV420P_UV_FS = ""
  "#version 150\n"
  "uniform sampler2D u_tex;"
  "in vec2 v_tex;"
  "out vec4 u_channel;"
  "out vec4 v_channel;"
  "void main() {"
  "  vec3 tc = texture(u_tex, v_tex).rgb; "
    "  u_channel = vec4(1.0);"
    "  v_channel = vec4(1.0);"
    "  u_channel.r = -(tc.r * 0.148) - (tc.g * 0.291) + (tc.b * 0.439) + 0.5; "
    "  v_channel.r =  (tc.r * 0.439) - (tc.g * 0.368) - (tc.b * 0.071) + 0.5; "
  "}"
  ;

// pass through shader
static const char* YUV420P_PT_FS = ""
  "#version 150\n"
  "uniform sampler2D u_tex;\n"
  "in vec2 v_tex; " 
  "out vec4 fragcol; "
  "void main() {"
  " fragcol = vec4(texture(u_tex, v_tex).rgb, 1.0);"
  "}"
  ;

#else 
#  error "Unsupported YUV420P_GRABBER_GLSL_VERSION"
#endif
// -----------------------------------------------------

class YUV420PGrabber {
 public:
  YUV420PGrabber();
  ~YUV420PGrabber();
  bool setup(int winW, int winH, int videoW, int videoH, int fps);
  void beginGrab();
  void endGrab();
  void draw();
  void print(); /* print some debug info */
  void start(); /* start capturing; sets the frame timeout */
  bool hasNewFrame();
  void downloadTextures(); /* downloads the YUV420p into the planes you can get with getPlanes() */
  unsigned char* getPlaneY();
  unsigned char* getPlaneU();
  unsigned char* getPlaneV();
  int getWidth(); /* returns video output width, same as y_plane dimensions*/
  int getHeight();  /* returns video output height, same as y_plane dimensions */
  int getWidthUV(); /* returns the width for the u and v planes */
  int getHeightUV(); /* returns the height for the u and v planes */
  uint32_t getTimeStamp(); /* get the timestamp for the current video frame in millis */
  size_t getNumBytes(); /* get num bytes in the image (for all planes) */ 
  GLuint getYTex(); /* get the texture that stores the Y plane */
  GLuint getUTex(); /* get the texture that stores the U plane */
  GLuint getVTex(); /* get the texture that stores the V plane */
  GLuint getFBO(); /* returns the id of the fbo */
 private:
  bool setupFBO();
  bool setupVAO();
  bool setupTextures();
  bool setupShaders();
  void setTextureParameters();
  void printShaderCompileInfo(GLuint shader);
  void printShaderLinkInfo(GLuint shader);
  void bindVAO();
  void unbindVAO();
  void deleteTexture(GLuint& tex);
  void deleteShader(GLuint& sh);
  void deleteProgram(GLuint& p);
 private:
  int win_w;
  int win_h;
  int vid_w;
  int vid_h;
  int uv_w;
  int uv_h;
  int fps;

  GLuint y_tex;
  GLuint u_tex;
  GLuint v_tex;
  GLuint vao; 
  GLuint scene_fbo;
  GLuint scene_depth;
  GLuint scene_tex;

  /* Y-plane */
  GLuint prog_y;
  GLuint vert_y;
  GLuint frag_y;

  /* U & V planes */
  GLuint prog_uv;
  GLuint vert_uv;
  GLuint frag_uv;

  /* passthrough */
  GLuint prog_pt;
  GLuint frag_pt;

  unsigned char* image;
  unsigned char* y_plane;
  unsigned char* u_plane;
  unsigned char* v_plane;
  
  /* framerate management */
  int64_t frame_timeout; /* when we should grab a new frame */
  int64_t frame_prev_timeout; /* previous frame timeout, used to calc diff */
  int64_t frame_delay;  /* delay between frames */
  int64_t frame_delay_adjusted; /* when the fps drops or raises to much we slightly adjust the delay so the avarage fps will be constant */
  int64_t frame_diff; /* different between two successive frames */
  int64_t frame_diff_avg; /* running avarage of `frame_time_diff` */
  uint32_t time_started; /* time we start() was called */
  uint32_t timestamp; /* timestamp of last frame, set in hasNewFrame() */
};

inline void YUV420PGrabber::setTextureParameters() {
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
}

inline void YUV420PGrabber::deleteTexture(GLuint& tex) {
  if(tex) {
    glDeleteTextures(1, &tex);
    tex = 0;
  }
}

inline void YUV420PGrabber::deleteShader(GLuint& sh) {
  if(sh) {
    glDeleteShader(sh);
    sh = 0;
  }
}

inline void YUV420PGrabber::deleteProgram(GLuint& p) {
  if(p) {
    glDeleteProgram(p);
    p = 0;
  }
}

inline unsigned char* YUV420PGrabber::getPlaneY() {
  return y_plane;
}

inline unsigned char* YUV420PGrabber::getPlaneU() {
  return u_plane;
}

inline unsigned char* YUV420PGrabber::getPlaneV() {
  return v_plane;
}

inline int YUV420PGrabber::getWidth() {
  return vid_w;
}

inline int YUV420PGrabber::getHeight() {
  return vid_h;
}

inline int YUV420PGrabber::getWidthUV() {
  return uv_w;
}

inline int YUV420PGrabber::getHeightUV() {
  return uv_h;
}

inline bool YUV420PGrabber::hasNewFrame() {
  bool wants = (frame_timeout && uv_hrtime() >= frame_timeout);
  if(wants) {
    frame_prev_timeout = frame_timeout;
    frame_diff = (frame_prev_timeout - uv_hrtime());
    frame_diff_avg = frame_diff_avg * 0.9 + frame_diff * 0.1;
    frame_delay_adjusted = frame_delay + frame_diff_avg;
    frame_timeout = uv_hrtime() + frame_delay_adjusted;
    timestamp = (uv_hrtime() / 1000000) - time_started;
  }
  return wants;
}

inline uint32_t YUV420PGrabber::getTimeStamp() {
  return timestamp;
}

inline size_t YUV420PGrabber::getNumBytes() {
  return vid_w * vid_h + (uv_w * uv_h * 2);
}

inline GLuint YUV420PGrabber::getYTex() {
  return y_tex;
}

inline GLuint YUV420PGrabber::getUTex() {
  return u_tex;
}

inline GLuint YUV420PGrabber::getVTex() {
  return v_tex;
}

inline GLuint YUV420PGrabber::getFBO() {
  return scene_fbo;
}

#endif

