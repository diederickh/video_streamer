/*

  # YUV420PGrabber

  This class will grab the draw commands between `beginGrab()` and `endGrab()`, and
  it will convert the grabbed scene to YUV420P using a shader. This class is a lot
  faster then libswscale (www.libav.org), for both color conversion and resizing.

  See: http://www.flickr.com/photos/diederick/10027076563/  for how different
       video streams are packet together into one buffer. 
       IMPORTANT: the memory is flipped .. so the smallest buffer is at index 0.


 */
#ifndef ROXLU_YUV420P_GRABBER_H
#define ROXLU_YUV420P_GRABBER_H

extern "C" {
#  include <uv.h>  
}

#if defined(__APPLE__)
#  if !defined(GL_TRUE)
#    include <OpenGL/gl3.h>
#    include <OpenGL/glext.h>
#  endif

#else 
#  include <GLXW/glxw.h>
#endif

#include <assert.h>
#include <vector>
#include <fstream>

#define YUV420P_NUM_PBO 3

// -----------------------------------------------------
#if YUV420P_GRABBER_GLSL_VERSION == 120
static const char* YUV420P_YUV_VS = ""
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

static const char* YUV420P_U_FS = ""
  "#version 120\n"
  "uniform sampler2D u_tex;"
  "varying vec2 v_tex;"
  "void main() {"
  "  vec3 tc = texture2D(u_tex, v_tex).rgb; "
  "  gl_FragColor = vec4(1.0);"
  "  gl_FragColor.r = -(tc.r * 0.148) - (tc.g * 0.291) + (tc.b * 0.439) + 0.5; "
  "}"
  ;

static const char* YUV420P_V_FS = ""
  "#version 120\n"
  "uniform sampler2D u_tex;"
  "varying vec2 v_tex;"
  "void main() {"
  "  vec3 tc = texture2D(u_tex, v_tex).rgb; "
  "  gl_FragColor = vec4(1.0);"
  "  gl_FragColor.r =  (tc.r * 0.439) - (tc.g * 0.368) - (tc.b * 0.071) + 0.5; "
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

//#error "Need to update the YUV420P grabber for GLSL 120"

#elif YUV420P_GRABBER_GLSL_VERSION == 150


static const char* YUV420P_YUV_VS = ""
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

static const char* YUV420P_U_FS = ""
  "#version 150\n"
  "uniform sampler2D u_tex;"
  "in vec2 v_tex;"
  "out vec4 u_channel;"
  "void main() {"
  "  vec3 tc = texture(u_tex, v_tex).rgb; "
  "  u_channel = vec4(1.0);"
  "  u_channel.r = -(tc.r * 0.148) - (tc.g * 0.291) + (tc.b * 0.439) + 0.5; "
  "}"
  ;

static const char* YUV420P_V_FS = ""
  "#version 150\n"
  "uniform sampler2D u_tex;"
  "in vec2 v_tex;"
  "out vec4 v_channel;"
  "void main() {"
    "  vec3 tc = texture(u_tex, v_tex).rgb; "
    "  v_channel = vec4(1.0);"
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

struct YUV420PSize {
  YUV420PSize();
  bool operator()(const YUV420PSize& a, const YUV420PSize b) { return a.yw > b.yw; } /* sorts from BIG to SMALL */
  void print();

  /* ... for fetching from the buffer */
  uint32_t y_offset; /* first y-pixel in buffer */
  uint32_t u_offset; /* first u-pixel in buffer */
  uint32_t v_offset; /* first v-pixel in buffer */
  uint32_t y_stride; /* y-plane stride in image buffer */
  uint32_t u_stride; /* u-plane stride in image buffer */
  uint32_t v_stride; /* v-plane stride in image buffer */

  int yw;  /* y-plane width */
  int yh;  /* y-plane height */
  int uvw;  /* u/v-plane width */
  int uvh;  /* u/v-plane height */
  int y_viewport_x; /* set viewport to this when rendering/grabbing */
  int y_viewport_y; /* set viewport to this when rendering/grabbing  */
  int u_viewport_x; /* set viewport to this when rendering/grabbing */
  int u_viewport_y; /* set viewport to this when rendering/grabbing */
  int v_viewport_x; /* set viewport to this when rendering/grabbing */
  int v_viewport_y; /* set viewport to this when rendering/grabbing */

  int id; /* the ID of this size, can be used with getImage(ID, ...) */
  uint8_t* planes[3]; /* pointers into YUV420PGrabber::image */
  uint32_t strides[3]; 
};

class YUV420PGrabber {
 public:
  YUV420PGrabber();
  ~YUV420PGrabber();
  void addSize(int id, int w, int h); /* the id identifies this size and this id can be used with getImage(..) to retrieve pointers to this image */
  YUV420PSize getSize(int id); /* get the size info object for the given image ID, make sure you have called setup() fist! */
  bool setup(int winW, int winH, int fps); /* , int videoW, int videoH, int fps); */
  void beginGrab();
  void endGrab();
  void draw();
  void print(); /* print some debug info */
  void start(); /* start capturing; sets the frame timeout */
  bool hasNewFrame();
  void downloadTextures(); /* downloads the YUV420p into the planes you can get with getPlanes(), when setOutput() has been called it will write the data to a raw yuv 420p file*/

  /* getting the frame data */
  void assignFrame(size_t id, std::vector<uint8_t>& pixels, uint8_t* planes[], uint32_t* strides); /* moves over the current frame into the given vector and sets the plane pointers + strides. pixels MUST be big enough to handle the data! */
  void assignPlanes(size_t id, std::vector<uint8_t>& pixels, uint8_t* planes[]); /* this function will assign the plane pointers into the given planes array. The pointers will point to the data in the given vector and for the given size ID. This is used to set the planes member of a AVPacket */
  void assignStrides(size_t id, uint32_t* strides); /* this function will assign the strides for the given size, this is used to setup the  strides member of the AVPacket.*/
  unsigned char* getPtr(); /* get ptr to the image data */

  int getWidth(); /* returns video output width, same as y_plane dimensions*/
  int getHeight();  /* returns video output height, same as y_plane dimensions */
  int getWidthUV(); /* returns the width for the u and v planes */
  int getHeightUV(); /* returns the height for the u and v planes */
  uint32_t getTimeStamp(); /* get the timestamp for the current video frame in millis */
  size_t getNumBytes(); /* get num bytes in the image (for all planes) */ 
  GLuint getFBO(); /* returns the id of the fbo */
  bool getImage(int id, uint8_t* planes, uint32_t* strides); /* return y,u,v planes and strides, both arrays are assumed to have a size of 3 */

  bool setOutput(int id, std::string filepath); /* when set, we will write the given size (id) to a raw yuv file. */

 private:
  bool setupFBO();
  bool setupVAO();
  bool setupSizes(); /* calculates the viewport positions for the added sizes and sets the texture width/height into which we grab the frame */
  bool setupTextures();
  bool setupShaders();
  bool setupPBO();
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

  /* the w and h of the texture into which we grab all the added sizes */
  int tex_w;
  int tex_h;

  GLuint yuv_tex;
  GLuint vao; 
  GLuint scene_fbo;
  GLuint scene_depth;
  GLuint scene_tex;

  /* Y-plane */
  GLuint prog_y;
  GLuint frag_y;

  /* U & V planes */
  GLuint vert_yuv;
  GLuint prog_u;
  GLuint prog_v;
  GLuint frag_v;
  GLuint frag_u;

  /* passthrough */
  GLuint prog_pt;
  GLuint frag_pt;

  GLuint pbo[YUV420P_NUM_PBO];
  int pbo_write_dx;
  int pbo_read_dx;

  unsigned char* image;
  
  /* framerate management */
  int64_t frame_timeout; /* when we should grab a new frame */
  int64_t frame_prev_timeout; /* previous frame timeout, used to calc diff */
  int64_t frame_delay;  /* delay between frames */
  int64_t frame_delay_adjusted; /* when the fps drops or raises to much we slightly adjust the delay so the avarage fps will be constant */
  int64_t frame_diff; /* different between two successive frames */
  int64_t frame_diff_avg; /* running avarage of `frame_time_diff` */
  uint32_t time_started; /* time we start() was called */
  uint32_t timestamp; /* timestamp of last frame, set in hasNewFrame() */
  std::vector<YUV420PSize> sizes; /* the sizes into which we need to rescale the grabber frames */

  /* debug */
  std::ofstream ofs; /* the outfile stream, see setOutput(), which initializes these members; and writes the given size id to a file */
  int outfile_size_id; /* id of of a YUV420PSize that can be found in the `sizes` member, see setOutput() */
  bool outfile_set; /* is set to true when we need to write to an output file */
};

inline unsigned char* YUV420PGrabber::getPtr() {
  return image;
}

inline void YUV420PGrabber::addSize(int id, int w, int h) {
  assert(w && h);
  printf("- w: %d, h: %d\n", w, h);
  YUV420PSize s;
  s.yw = w;
  s.yh = h;
  s.uvw = w >> 1;
  s.uvh = h >> 1;

  s.id = id;
  sizes.push_back(s);
}

inline YUV420PSize YUV420PGrabber::getSize(int id) {
  for(std::vector<YUV420PSize>::iterator it = sizes.begin(); it != sizes.end(); ++it) {
    YUV420PSize& s = *it;
    if(s.id == id) {
      return s;
    }
  }
  printf("error: size with id `%d` not found.\n", id);
  ::exit(EXIT_FAILURE);
}

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
  return tex_w * tex_h; 
}

inline GLuint YUV420PGrabber::getFBO() {
  return scene_fbo;
}

#endif

