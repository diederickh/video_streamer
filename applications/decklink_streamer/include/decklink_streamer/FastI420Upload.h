#ifndef ROXLU_FAST_I420_UPLOAD_H
#define ROXLU_FAST_I420_UPLOAD_H

#include <GLXW/glxw.h>
#include <stdint.h>

#define FI_NUM_BUFFERS 3

class FastI420Upload {
 public:
  FastI420Upload();
  ~FastI420Upload();
  bool setup(int width, int height);
  void update(uint8_t* y, uint8_t* u, uint8_t* v);
 private: 
  GLuint createTexture(int width, int height);
 public:
  int w;  /* W-plae width */
  int h;  /* Y-plane height */
  int uv_w;
  int uv_h;
  int read_dx;
  int write_dx;
  GLuint tex_y;
  GLuint tex_u;
  GLuint tex_v;
  GLuint pbo_y[FI_NUM_BUFFERS];
  GLuint pbo_u[FI_NUM_BUFFERS];
  GLuint pbo_v[FI_NUM_BUFFERS];
  bool must_unmap;
};

#endif
