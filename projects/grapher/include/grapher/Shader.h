#ifndef ROXLU_GRAPHER_SHADER_H
#define ROXLU_GRAPHER_SHADER_H

#include <GLFW/glfw3.h>

static const char* GR_VS = ""
  "#version 150\n"
  "uniform mat4 u_pm;"
  "in vec4 a_pos;"
  "in vec3 a_col;"
  "out vec3 v_col;"
  
  "void main() {"
  "  gl_Position = u_pm * a_pos;"
  "  v_col = a_col;"
  "}"
  ;

static const char* GR_FS = ""
  "#version 150\n"
  "in vec3 v_col;"
  "out vec4 fragcol; "
  "void main() {"
  "  fragcol = vec4(1.0, 0.0, 1.0, 1.0);"
  "  fragcol.rgb = v_col;"
  "}"
  ;

class Shader {
 public:
  Shader();
  ~Shader();
  bool setup();
  void use();
  void setPM(const float* pm);
 private:
  void printShaderCompileInfo(GLuint shader);
  void printShaderLinkInfo(GLuint shader);
 public:
  GLuint prog;
  GLuint vert;
  GLuint frag;
  GLint u_pm;

};

#endif
