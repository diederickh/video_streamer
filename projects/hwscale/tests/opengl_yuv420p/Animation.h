#ifndef ROXLU_ANIMATION_H
#define ROXLU_ANIMATION_H

#include <hwscale/opengl/YUV420PGrabber.h> /* using this for the correct GL includes */

#if YUV420P_GRABBER_GLSL_VERSION == 120
#  error "The animation class does not yet have support for GLSL 120"
#elif YUV420P_GRABBER_GLSL_VERSION == 150
static const char* ANIM_VS = ""
  "#version 150\n"
  "uniform mat4 u_pm;"
  "uniform mat4 u_mm;"
  "in vec4 a_pos;"
  "in vec3 a_norm;"
  "out vec3 v_norm;"
  "void main() { "
  "  gl_Position = u_pm * u_mm * a_pos;"
  "  v_norm = a_norm; "
  "}"
  ;

static const char* ANIM_FS = ""
  "#version 150\n"
  "out vec4 fragcolor; "
  "in vec3 v_norm;"
  "void main() {"
  "  fragcolor.rgba = vec4(1.0, 0.0, 1.0, 1.0);"
  "  fragcolor.rgb = 0.5 + 0.5 * v_norm;"
  "}"
  ;
#endif


class Animation {
 public:
  Animation();
  ~Animation();

  bool setup(int w, int h);
  void draw();

 public:
  GLuint prog;
  GLuint vert;
  GLuint frag;
  GLuint vao;
  GLuint vbo;
  float pm[16];
  float mm[16];
  GLint u_mm;
  int nvertices;
};

#endif
