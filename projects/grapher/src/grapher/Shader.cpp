#include <assert.h>
#include <grapher/Shader.h>
#include <iostream>

Shader::Shader() 
  :prog(0)
  ,vert(0)
  ,frag(0)
  ,u_pm(-1)
{

}

Shader::~Shader() {
  printf("error: free Gl.\n");
}

bool Shader::setup() {

  vert = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vert, 1, &GR_VS, NULL);
  glCompileShader(vert); 
  printShaderCompileInfo(vert);

  frag = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(frag, 1, &GR_FS, NULL);
  glCompileShader(frag);
  printShaderCompileInfo(frag);

  prog = glCreateProgram();
  glAttachShader(prog, vert);
  glAttachShader(prog, frag);
  glBindAttribLocation(prog, 0, "a_pos");
  glBindAttribLocation(prog, 1, "a_col");
  glLinkProgram(prog);
  printShaderLinkInfo(prog);
  
  u_pm = glGetUniformLocation(prog, "u_pm");
  if(u_pm < 0) {
    printf("error: u_pm not found.\n");
    return false;
  }

  return true;
}

void Shader::use() {
  glUseProgram(prog);
}

void Shader::setPM(const float* pm) {
  glUniformMatrix4fv(u_pm, 1, GL_FALSE, pm);
}

void Shader::printShaderCompileInfo(GLuint shader) {
  GLint status = 0;
  GLint count = 0;
  GLchar* error = NULL;
  glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
  if(!status) {
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &count);
    if(count > 0) {
      error = new GLchar[count];
      glGetShaderInfoLog(shader, count, NULL, error);
      printf("%s\n", error);
      delete[] error;
      error = NULL;
      assert(0);
    }
  }
}

void Shader::printShaderLinkInfo(GLuint shader) {
  GLint status = 0;
  GLint count = 0;
  GLchar* error = NULL;
  GLsizei nchars = 0;
  glGetProgramiv(shader, GL_LINK_STATUS, &status);
  if(!status) {
    glGetProgramiv(shader, GL_INFO_LOG_LENGTH, &count);
    if(count > 0) {
      error = new GLchar[count];
      glGetProgramInfoLog(shader, 2048, &nchars, error);
      printf("%s\n", error);
      delete[] error;
      error = NULL;
      assert(0);
    }
  }
}
