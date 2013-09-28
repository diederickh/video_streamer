#include <hwscale/opengl/ResizerFBO.h>
#include <iostream>

ResizerFBO::ResizerFBO(GLuint ytex, GLuint utex, GLuint vtex)
  :in_width(0)
  ,in_height(0)
  ,y_tex(ytex)
  ,u_tex(utex)
  ,v_tex(vtex)
  ,fbo(0)
{
  width[0] = width[1] = 0;
  height[0] = height[1] = 0;
  resized_y_tex[0] = resized_y_tex[1] = 0;
  resized_u_tex[0] = resized_u_tex[1] = 0;
  resized_v_tex[0] = resized_v_tex[1] = 0;
}


ResizerFBO::~ResizerFBO() {
  printf("error: @todo cleanup GL in ResizerFBO.\n");
}

bool ResizerFBO::setup(int inWidth, int inHeight, int w0, int h0, int w1, int h1) {

  if(!inWidth || !inHeight) {
    printf("error: invalid iWidth or inHeight: %d x %d\n", inWidth, inHeight);
    return false;
  }
  in_width = inWidth;
  in_height = inHeight;

  if(!w0 || !h0) {
    printf("error: invalid w0 or h0: %d x %d\n", w0, h0);
    return false;
  }

  width[0] = w0;
  height[0] = h0;

  if(w1 && h1) {
    width[1] = w1;
    height[1] = h1;
  }

  // create fbo
  glGenFramebuffers(1, &fbo);
  glBindFramebuffer(GL_FRAMEBUFFER, fbo);

  // @todo - cleanup ResizerFBO - we don't need this depth buffer
  /*
  glGenRenderbuffers(1, &depth);
  glBindRenderbuffer(GL_RENDERBUFFER, depth);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT32, in_width, in_height);
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depth);
  */

  // create textures for first resized version
  if(width[0] && height[0]) {
    resized_y_tex[0] = createTexture(width[0], height[0]);
    resized_u_tex[0] = createTexture(width[0] * 0.5, height[0] * 0.5);
    resized_v_tex[0] = createTexture(width[0] * 0.5, height[0] * 0.5);
    printf("Created resizer textures: y: %d, u: %d, v: %d\n", resized_y_tex[0], resized_u_tex[0], resized_v_tex[0]);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, resized_y_tex[0], 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, resized_u_tex[0], 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, resized_v_tex[0], 0);
  }

  GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
  if(status != GL_FRAMEBUFFER_COMPLETE) {
    printf("error: framebuffer of resizer is not complete!\n");
    return false;
  }
  
  return true;
}

GLuint ResizerFBO::createTexture(int w, int h) {

  if(!w || !h) {
    printf("error: invalid width/height for createTexture: %d x %d\n", w, h);
    ::exit(EXIT_FAILURE);
  }

  printf("create texture: %d, %d\n", w, h);

  GLuint tex;
  glGenTextures(1, &tex);
  glBindTexture(GL_TEXTURE_2D, tex);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, w, h, 0, GL_RED, GL_UNSIGNED_BYTE, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  return tex;
}

void ResizerFBO::resize(YUV420PGrabber& grabber) {
  int uv_width = in_width * 0.5;
  int uv_height = in_height * 0.5;
  int scaled_width = width[0] * 0.5;
  int scaled_height = height[0] * 0.5;

  glBindFramebuffer(GL_READ_FRAMEBUFFER, grabber.getFBO());
  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);

  // u-plane 
  {
    glReadBuffer(GL_COLOR_ATTACHMENT2); 

    GLenum draw_bufs[] = { GL_COLOR_ATTACHMENT1 };
    glDrawBuffers(1, draw_bufs);
    glClear(GL_COLOR_BUFFER_BIT);

    glBlitFramebuffer(0, 0, uv_width, uv_height, 
                      0, 0, scaled_width, scaled_height, 
                      GL_COLOR_BUFFER_BIT, GL_LINEAR);

    //printf("uv: %d x %d, out: %d x %d\n", uv_width, uv_height, scaled_width, scaled_height);
  }

  // v-plane 
  {
    glReadBuffer(GL_COLOR_ATTACHMENT3); 

    GLenum draw_bufs[] = { GL_COLOR_ATTACHMENT2 };
    glDrawBuffers(1, draw_bufs);
    glClear(GL_COLOR_BUFFER_BIT);

    glBlitFramebuffer(0, 0, uv_width, uv_height, 
                      0, 0, scaled_width, scaled_height, 
                      GL_COLOR_BUFFER_BIT, GL_LINEAR);

    //printf("uv: %d x %d, out: %d x %d\n", uv_width, uv_height, scaled_width, scaled_height);
  }



  // y-plane
  {
    glReadBuffer(GL_COLOR_ATTACHMENT1); 

    GLenum draw_bufs[] = { GL_COLOR_ATTACHMENT0 }; 
    glDrawBuffers(1, draw_bufs);

    glClear(GL_COLOR_BUFFER_BIT);

    glBlitFramebuffer(0, 0, in_width, in_height, 
                      0, 0, width[0], height[0], 
                      GL_COLOR_BUFFER_BIT, GL_LINEAR);
  }



  glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glReadBuffer(GL_BACK_LEFT);
}
