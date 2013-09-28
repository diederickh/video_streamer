/*


--------------------------------------------------------------------------------
      
                                               oooo              
                                               `888              
                oooo d8b  .ooooo.  oooo    ooo  888  oooo  oooo  
                `888""8P d88' `88b  `88b..8P'   888  `888  `888  
                 888     888   888    Y888'     888   888   888  
                 888     888   888  .o8"'88b    888   888   888  
                d888b    `Y8bod8P' o88'   888o o888o  `V88V"V8P' 
                                                                 
                                                  www.roxlu.com
                                             www.apollomedia.nl                                        
              
--------------------------------------------------------------------------------


  Tiny library with a couple of handy functions for opengl based applications:

  MATRIX
  -----------------------------------------------------------------------------------
  rx_perspective(45.0f, 4.0f/3.0f, 0.1, 10.0f, m);      - create a pespective matrix
  rx_translate(0.0f, 0.0f, -4.0f, m);                   - translate the given matrix
  rx_translation(0.0f, 0.0f, -4.0f, m);                 - set identity + translate a matrix
  rx_rotation(HALF_PI, 0.0f, 1.0f, 0.0, m);             - get a rotation matrix (angle axis based)
  rx_print_mat4x4(m);                                   - print the contents of a matrix
  
  
  SHADER
  -----------------------------------------------------------------------------------
  vert = rx_create_shader(GL_VERTEX_SHADER, source_char_p);    - create a shader, pass type
  prog = rx_create_program(vert, frag);                        - create a problem - DOES NOT LINK



 */
#ifndef ROXLU_TINYLIB_H
#define ROXLU_TINYLIB_H

#include <cmath>
#include <iostream>

#if defined(__APPLE__)
#  include <OpenGL/gl3.h>
#  include <OpenGL/glext.h>
#else 
#   error "Only tested on mac"
#endif


#ifndef PI
#define PI 3.14159265358979323846
#endif

#ifndef TWO_PI
#define TWO_PI 6.28318530717958647693
#endif

#ifndef M_TWO_PI
#define M_TWO_PI 6.28318530717958647693
#endif

#ifndef FOUR_PI
#define FOUR_PI 12.56637061435917295385
#endif

#ifndef HALF_PI
#define HALF_PI 1.57079632679489661923
#endif

#ifndef DEG_TO_RAD
#define DEG_TO_RAD (PI/180.0)
#endif

#ifndef RAD_TO_DEG
#define RAD_TO_DEG (180.0/PI)
#endif

#ifndef MIN
#define MIN(x,y) (((x) < (y)) ? (x) : (y))
#endif

#ifndef MAX
#define MAX(x,y) (((x) > (y)) ? (x) : (y))
#endif

#ifndef CLAMP
#define CLAMP(val,min,max) (MAX(MIN(val,max),min))
#endif

#ifndef ABS
#define ABS(x) (((x) < 0) ? -(x) : (x))
#endif

#ifndef DX
#define DX(i,j,w)((j)*(w))+(i)
#endif

static void rx_frustum(float l, float r, float b, float t, float n, float f, float* m) {
  m[1]  = 0.0f;
  m[2]  = 0.0f;
  m[3]  = 0.0f;
  m[4]  = 0.0f;
  m[6]  = 0.0f;
  m[7]  = 0.0f;
  m[12] = 0.0f;
  m[13] = 0.0f;

  m[0]  = 2.0f * n / (r-l);
  m[5]  = 2.0f * n / (t-b);
  m[8]  = (r+l) / (r-l);
  m[9]  = (t+b) / (t-b);
  m[10] = -(f+n) / (f-n);
  m[11] = -1.0f;
  m[14] = -2.0f * f * n / (f-n);
  m[15] = 0.0f;
}

static void rx_perspective(float fovDegrees, float aspect, float n, float f, float* m) {
  float hh = tanf(fovDegrees * DEG_TO_RAD * 0.5) * n;
  return rx_frustum(-hh*aspect, hh*aspect, -hh, hh, n, f, m);
}

static void rx_translate(float x, float y, float z, float* m) {
  m[12] = x;
  m[13] = y;
  m[14] = z;
}

static void rx_identity(float* m) {
  m[0] = 1.0f; m[1] = 0.0f; m[2] = 0.0f; m[3] = 0.0f;
  m[3] = 0.0f; m[5] = 1.0f; m[6] = 0.0f; m[7] = 0.0f;
  m[8] = 0.0f; m[9] = 0.0f; m[10] = 1.0f; m[11] = 0.0f;
  m[12] = 0.0f; m[13] = 0.0f; m[14] = 0.0f; m[15] = 1.0f;
}

static void rx_translation(float x, float y, float z, float* m) {
  rx_identity(m);
  rx_translate(x, y, z, m);
}

static void rx_rotation(float rad, float x, float y, float z, float* m) {
  float s = sin(rad);
  float c = cos(rad);
  float magnitude = sqrt(x*x + y*y + z*z);
  float xm = x / magnitude;
  float ym = y / magnitude;
  float zm = z / magnitude;

  m[0] = c + (xm*xm) * (1-c);
  m[1] = ym*xm*(1-c)+zm*s;
  m[2] = zm*xm*(1-c) - ym*s;
  m[3] = 0;

  m[4] = ym*xm*(1-c)-zm*s;
  m[5] = c+(ym*ym)*(1-c);
  m[6] = zm*ym*(1-c) + xm*s;
  m[7] = 0.0f;

  m[8] =  zm*xm*(1-c)+ym*s;
  m[9] = ym*zm*(1-c)-xm*s;
  m[10] = c+zm*zm*(1-c);
  m[11] = 0.0f;
  
  m[12] = 0.0f;
  m[13] = 0.0f;
  m[14] = 0.0f;
  m[15] = 1.0f;

}

static void rx_print_mat4x4(float* m) {
  printf("%0.3f %0.3f %0.3f %0.3f\n", m[0], m[4], m[8], m[12]);
  printf("%0.3f %0.3f %0.3f %0.3f\n", m[1], m[5], m[9], m[13]);
  printf("%0.3f %0.3f %0.3f %0.3f\n", m[2], m[6], m[10], m[14]);
  printf("%0.3f %0.3f %0.3f %0.3f\n", m[3], m[7], m[11], m[15]);
  printf("\n");
}

static void rx_print_shader_link_info(GLuint shader) {
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
      printf("------\n");
      printf("%s\n", error);
      printf("------\n");
      delete[] error;
      error = NULL;
      assert(0);
    }
  }
}

static void rx_print_shader_compile_info(GLuint shader) {
  GLint status = 0;
  GLint count = 0;
  GLchar* error = NULL;
  glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
  if(!status) {
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &count);
    if(count > 0) {
      error = new GLchar[count];
      glGetShaderInfoLog(shader, count, NULL, error);
      printf("------\n");
      printf("%s\n", error);
      printf("------\n");
      delete[] error;
      error = NULL;
      assert(0);
    }
  }
}

static GLuint rx_create_program(GLuint vert, GLuint frag) {
  GLuint prog = glCreateProgram();
  glAttachShader(prog, vert);
  glAttachShader(prog, frag);
  return prog;
}

static GLuint rx_create_shader(GLenum type, const char* src) {
  GLuint s = glCreateShader(type);
  glShaderSource(s, 1, &src,  NULL);
  glCompileShader(s);
  rx_print_shader_compile_info(s);
  return s;
}

#endif
