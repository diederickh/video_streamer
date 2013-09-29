extern "C" {
#  include <uv.h>
}
#include <iostream>
#include <decklink_streamer/FastI420Upload.h>

FastI420Upload::FastI420Upload() 
  :tex_y(0)
  ,tex_u(0)
  ,tex_v(0)
  ,w(0)
  ,h(0)
  ,uv_w(0)
  ,uv_h(0)
  ,read_dx(0)
  ,write_dx(0)
  ,must_unmap(false)
{
  memset((char*)pbo_y, 0x00, sizeof(pbo_y));
  memset((char*)pbo_u, 0x00, sizeof(pbo_u));
  memset((char*)pbo_v, 0x00, sizeof(pbo_v));
}

FastI420Upload::~FastI420Upload() {
}

bool FastI420Upload::setup(int width, int height) {

  if(!width || !height) {
    printf("error: invalid width or height given. %d x %d\n", width, height);
    return false;
  }

  w = width;
  h = height;
  uv_w = w >> 1;
  uv_h = h >> 1;
  glGenBuffers(FI_NUM_BUFFERS, pbo_y);
  glGenBuffers(FI_NUM_BUFFERS, pbo_u);
  glGenBuffers(FI_NUM_BUFFERS, pbo_v);

  for(int i = 0; i < FI_NUM_BUFFERS; ++i) {
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pbo_y[i]);
    glBufferData(GL_PIXEL_UNPACK_BUFFER, w * h, NULL, GL_STREAM_DRAW);
    
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pbo_u[i]);
    glBufferData(GL_PIXEL_UNPACK_BUFFER, (uv_w * uv_w), NULL, GL_STREAM_DRAW);

    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pbo_v[i]);
    glBufferData(GL_PIXEL_UNPACK_BUFFER, (uv_w * uv_w), NULL, GL_STREAM_DRAW);
  }

  write_dx = FI_NUM_BUFFERS-1;
  read_dx = 0;

  glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);

  tex_y = createTexture(w, h);
  tex_u = createTexture(uv_w, uv_h);
  tex_v = createTexture(uv_w, uv_h);
  
  glBindTexture(GL_TEXTURE_2D, 0);

  printf("tex_y: %d, tex_u: %d, tex_v: %d\n", tex_y, tex_u, tex_v);
  return true;
}

GLuint FastI420Upload::createTexture(int width, int height) {
  GLuint tex = 0;
  glGenTextures(1, &tex);
  glBindTexture(GL_TEXTURE_2D, tex);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, width, height, 0, GL_RED, GL_UNSIGNED_BYTE, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  return tex;
}

void FastI420Upload::update(uint8_t* y, uint8_t* u, uint8_t* v) {

  // Y-plane
  {
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pbo_y[write_dx]);
    char* data = (char*)glMapBuffer(GL_PIXEL_UNPACK_BUFFER, GL_WRITE_ONLY);
    if(data) {
      memcpy(data, (char*)y, w * h);
      glUnmapBuffer(GL_PIXEL_UNPACK_BUFFER);
    }
  
    glBindTexture(GL_TEXTURE_2D, tex_y);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pbo_y[read_dx]);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, w, h, GL_RED, GL_UNSIGNED_BYTE, 0);
  }
  
  // U-plane
  {
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pbo_u[write_dx]);
    char* data = (char*)glMapBuffer(GL_PIXEL_UNPACK_BUFFER, GL_WRITE_ONLY);
    if(data) {
      memcpy(data, (char*)u, uv_w * uv_h);
      glUnmapBuffer(GL_PIXEL_UNPACK_BUFFER);
    }
  
    glBindTexture(GL_TEXTURE_2D, tex_u);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pbo_u[read_dx]);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, uv_w, uv_h, GL_RED, GL_UNSIGNED_BYTE, 0);
  }

  // V-plane
  {
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pbo_v[write_dx]);
    char* data = (char*)glMapBuffer(GL_PIXEL_UNPACK_BUFFER, GL_WRITE_ONLY);
    if(data) {
      memcpy(data, (char*)v, uv_w * uv_h);
      glUnmapBuffer(GL_PIXEL_UNPACK_BUFFER);
    }
  
    glBindTexture(GL_TEXTURE_2D, tex_v);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pbo_v[read_dx]);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, uv_w, uv_h, GL_RED, GL_UNSIGNED_BYTE, 0);
  }

  glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);

  read_dx = (read_dx + 1) % FI_NUM_BUFFERS;
  write_dx = (write_dx + 1) % FI_NUM_BUFFERS;
}

//char* data = (char*)glMapBufferRange(GL_PIXEL_UNPACK_BUFFER, 0, w*h, GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);
