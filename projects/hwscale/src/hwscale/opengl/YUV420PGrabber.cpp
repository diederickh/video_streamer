#include <assert.h>
#include <iostream>
#include <hwscale/opengl/YUV420PGrabber.h>

YUV420PGrabber::YUV420PGrabber()
  :win_w(0)
  ,win_h(0)
  ,vid_w(0)
  ,vid_h(0)
  ,uv_h(0)
  ,uv_w(0)
  ,fps(0)
  ,y_tex(0)
  ,u_tex(0)
  ,v_tex(0)
  ,vao(0)
  ,scene_fbo(0)
  ,scene_depth(0)
  ,scene_tex(0)
  ,prog_y(0)
  ,vert_y(0)
  ,frag_y(0)
  ,prog_uv(0)
  ,vert_uv(0)
  ,frag_uv(0)
  ,prog_pt(0)
  ,frag_pt(0)
  ,image(NULL)
  ,y_plane(NULL)
  ,u_plane(NULL)
  ,v_plane(NULL)
  ,frame_timeout(0)
  ,frame_prev_timeout(0)
  ,frame_delay(0)
  ,frame_delay_adjusted(0)
  ,frame_diff(0)
  ,frame_diff_avg(0)
{
}

YUV420PGrabber::~YUV420PGrabber() {
  printf("error: ~YUV420PGrabber(), free all GL-objects.\n");
  deleteShader(vert_y);
  deleteShader(frag_y);
  deleteShader(vert_uv);
  deleteShader(frag_uv);
  deleteShader(frag_pt);
  deleteProgram(prog_y);
  deleteProgram(prog_uv);
  deleteProgram(prog_pt);
  deleteTexture(y_tex);
  deleteTexture(u_tex);
  deleteTexture(v_tex);
  deleteTexture(scene_tex);

  if(scene_fbo) {
    glDeleteFramebuffers(1, &scene_fbo);
  }

  if(scene_depth) {
    glDeleteRenderbuffers(1, &scene_depth);
  }

  if(vao) {
#if defined(__APPLE__)
    glDeleteVertexArraysAPPLE(1, &vao);
#else
    glDeleteVertexArrays(1, &vao);
#endif  
  }

  if(image) {
    delete[] image;
  }

  win_w = 0;
  win_h = 0;
  vid_w = 0;
  vid_h = 0;
  uv_w = 0;
  uv_h = 0;
  y_tex = 0;
  u_tex = 0;
  v_tex = 0;
  scene_fbo = 0;
  scene_depth = 0;
  scene_tex = 0;
  vao = 0;
  fps = 0;
  image = NULL;
  y_plane = NULL;
  u_plane = NULL;
  v_plane = NULL;
  frame_timeout = 0;
  frame_prev_timeout = 0;
  frame_delay = 0;
  frame_delay_adjusted = 0;
  frame_diff = 0;
  frame_diff_avg = 0;
}

bool YUV420PGrabber::setup(int winW, int winH, int videoW, int videoH, int framerate) {

  if(!winW || !winH) {
    printf("error: invalid win_w or win_h: %dx%d\n", winW, winH);
    return false;
  }

  if(!videoW || !videoH) {
    printf("error: invalid video_w or video_h: %dx%d\n", videoW, videoH);
    return false;
  }

  if(!framerate) {
    printf("error: invalid fps: %d\n", framerate);
    return false;
  }

  vid_w = videoW;
  vid_h = videoH;
  win_w = winW;
  win_h = winH;
  uv_w = vid_w * 0.5;
  uv_h = vid_h * 0.5;
  fps = framerate;

  image = new unsigned char[vid_w * vid_h + (uv_w * uv_h * 2)];
  if(!image) {
    printf("error: cannot allocate image buffer.\n");
    return false;
  }

  y_plane = image;
  u_plane = image + (vid_w * vid_h);
  v_plane = u_plane + (uv_w * uv_h);

  if(!setupTextures()) {
    printf("error: cannot setup textures.\n");
    return false;
  }

  if(!setupFBO()) {
    printf("error: cannot setup fbo.\n");
    return false;
  }

  if(!setupVAO()) {
    printf("error: cannot setup vao.\n");
    return false;
  }

  if(!setupShaders()) {
    printf("error: cannot setup shaders.\n");
    return false;
  }

  frame_delay = (1.0/fps) * 1000 * 1000 * 1000;

  return true;
}

void YUV420PGrabber::start() {
  frame_timeout = uv_hrtime() + frame_delay;
  time_started = uv_hrtime() / (1000 * 1000);
}

bool YUV420PGrabber::setupFBO() {
  assert(win_w && win_h && vid_w && vid_h && scene_fbo == 0);
  assert(y_tex && u_tex && v_tex);

  glGenFramebuffers(1, &scene_fbo);
  glBindFramebuffer(GL_FRAMEBUFFER, scene_fbo);
  
  glGenRenderbuffers(1, &scene_depth);
  glBindRenderbuffer(GL_RENDERBUFFER, scene_depth);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT32, win_w, win_h);
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, scene_depth);
  
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, scene_tex, 0);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, y_tex, 0);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, u_tex, 0);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_2D, v_tex, 0);

  GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
  if(status != GL_FRAMEBUFFER_COMPLETE) {
    printf("error: framebuffer is not yet complete.\n");
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    return false;
  }

  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  
  return true;
}

bool YUV420PGrabber::setupTextures() {
  assert(win_w && win_h && vid_w && vid_h && y_tex == 0);

  glGenTextures(1, &scene_tex);
  glBindTexture(GL_TEXTURE_2D, scene_tex);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, win_w, win_h, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL); 
  setTextureParameters();

  glGenTextures(1, &y_tex);
  glBindTexture(GL_TEXTURE_2D, y_tex);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, vid_w, vid_h, 0, GL_RED, GL_UNSIGNED_BYTE, NULL);
  setTextureParameters();

  glGenTextures(1, &u_tex);
  glBindTexture(GL_TEXTURE_2D, u_tex);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, uv_w, uv_h, 0, GL_RED, GL_UNSIGNED_BYTE, NULL);
  setTextureParameters();

  glGenTextures(1, &v_tex);
  glBindTexture(GL_TEXTURE_2D, v_tex);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, uv_w, uv_h, 0, GL_RED, GL_UNSIGNED_BYTE, NULL);
  setTextureParameters();

  return true;
}

bool YUV420PGrabber::setupVAO() {
  assert(vao == 0);

#if USE_APPLE_VAO
  glGenVertexArraysAPPLE(1, &vao);
#else
  glGenVertexArrays(1, &vao);  
#endif

  return true;
}

void YUV420PGrabber::bindVAO() {
#if USE_APPLE_VAO
  glBindVertexArrayAPPLE(vao);
#else
  glBindVertexArray(vao);
#endif
}

void YUV420PGrabber::unbindVAO() {
#if USE_APPLE_VAO
  glBindVertexArrayAPPLE(0);
#else
  glBindVertexArray(0);
#endif
}


bool YUV420PGrabber::setupShaders() {
  GLint u_tex = -1;

  // Y-shader
  vert_y = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vert_y, 1, &YUV420P_Y_VS, NULL);
  glCompileShader(vert_y);
  printShaderCompileInfo(vert_y);

  frag_y = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(frag_y, 1, &YUV420P_Y_FS, NULL);
  glCompileShader(frag_y);
  printShaderCompileInfo(frag_y);

  prog_y = glCreateProgram();
  glAttachShader(prog_y, vert_y);
  glAttachShader(prog_y, frag_y);

#if YUV420P_GRABBER_GLSL_VERSION == 150
  glBindFragDataLocation(prog_y, 0, "fragcol");
#endif

  glLinkProgram(prog_y);
  printShaderLinkInfo(prog_y);

  glUseProgram(prog_y);
  u_tex = glGetUniformLocation(prog_y, "u_tex");
  if(u_tex < 0) {
    printf("error: cannot find u_tex in the y-shader.\n");
    return false;
  }
  glUniform1i(u_tex, 0);

  // U & V - shader
  vert_uv = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vert_uv, 1, &YUV420P_Y_VS, NULL);
  glCompileShader(vert_uv);
  printShaderCompileInfo(vert_uv);

  frag_uv = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(frag_uv, 1, &YUV420P_UV_FS, NULL);
  glCompileShader(frag_uv);
  printShaderCompileInfo(frag_uv);

  prog_uv = glCreateProgram();
  glAttachShader(prog_uv, vert_uv);
  glAttachShader(prog_uv, frag_uv);


#if YUV420P_GRABBER_GLSL_VERSION == 150
  glBindFragDataLocation(prog_uv, 0, "u_channel");
  glBindFragDataLocation(prog_uv, 1, "v_channel");
#endif

  glLinkProgram(prog_uv);
  printShaderLinkInfo(prog_uv);
  
  glUseProgram(prog_uv);
  u_tex = glGetUniformLocation(prog_uv, "u_tex");
  if(u_tex < 0) {
    printf("error: cannot find u_tex in the y-shader.\n");
    return false;
  }
  glUniform1i(u_tex, 0);
  
  // Pass through shader
  frag_pt = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(frag_pt, 1, &YUV420P_PT_FS, NULL);
  glCompileShader(frag_pt);
  printShaderCompileInfo(frag_pt);

  prog_pt = glCreateProgram();
  glAttachShader(prog_pt, vert_y);
  glAttachShader(prog_pt, frag_pt);

#if YUV420P_GRABBER_GLSL_VERSION == 150
  glBindFragDataLocation(prog_pt, 0, "fragcol");
#endif

  glLinkProgram(prog_pt);
  printShaderLinkInfo(prog_pt);

  glUseProgram(prog_pt);
  u_tex = glGetUniformLocation(prog_pt, "u_tex");
  if(u_tex < 0) {
    printf("error: cannot find u_tex in the y-shader.\n");
    return false;
  }
  glUniform1i(u_tex, 0);

  glUseProgram(0);
  return true;
}

void YUV420PGrabber::beginGrab() {
  assert(scene_fbo && win_w && win_h);

  GLenum drawbufs[] = { GL_COLOR_ATTACHMENT0 } ; 
  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, scene_fbo);
  glDrawBuffers(1, drawbufs);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glViewport(0,0, win_w, win_h);
}

void YUV420PGrabber::endGrab() {
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, scene_tex);

  bindVAO();
  {
    // render Y plane
    GLenum drawbufs[] = { GL_COLOR_ATTACHMENT1 };
    glDrawBuffers(1, drawbufs);
    glViewport(0,0,vid_w, vid_h);

    glUseProgram(prog_y);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
  }

  glDisable(GL_DEPTH_TEST);
  {
    // render U-V planes
    GLenum drawbufs[] = { GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3 }; 
    glDrawBuffers(2, drawbufs);
    glViewport(0, 0, uv_w, uv_h);

    glUseProgram(prog_uv);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
  }
  glEnable(GL_DEPTH_TEST);  

  unbindVAO();

  glUseProgram(0);
  glBindTexture(GL_TEXTURE_2D, 0);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glDrawBuffer(GL_BACK);
  glViewport(0,0, win_w, win_h);
}

void YUV420PGrabber::downloadTextures() {
  glBindTexture(GL_TEXTURE_2D, 0);
  glBindFramebuffer(GL_READ_FRAMEBUFFER, scene_fbo);

  // IMPORTANT: on mac 10.8, the order of download is important, it needs 
  // to start downloading the GL_COLOR_ATTACHMENT3 first, then 2, then 1.
  // seems like a driver bug (!?)

  glReadBuffer(GL_COLOR_ATTACHMENT3);
  glReadPixels(0, 0, uv_w, uv_h, GL_RED, GL_UNSIGNED_BYTE, v_plane);

  glReadBuffer(GL_COLOR_ATTACHMENT2);
  glReadPixels(0, 0, uv_w, uv_h, GL_RED, GL_UNSIGNED_BYTE, u_plane);

  glReadBuffer(GL_COLOR_ATTACHMENT1);
  glReadPixels(0, 0, vid_w, vid_h, GL_RED, GL_UNSIGNED_BYTE, y_plane);

  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glDrawBuffer(GL_BACK);
}

// @todo - this makes the screen flicker!
void YUV420PGrabber::draw() {
  assert(win_w);
  assert(win_h);

  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glDrawBuffer(GL_BACK);
  glViewport(0, 0, win_w, win_h);

  bindVAO();

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, scene_tex);

  glUseProgram(prog_pt);
  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

  unbindVAO();
  glUseProgram(0);
  glBindTexture(GL_TEXTURE_2D, 0);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glDrawBuffer(GL_BACK);
}

void YUV420PGrabber::printShaderCompileInfo(GLuint shader) {
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

void YUV420PGrabber::printShaderLinkInfo(GLuint shader) {
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

void YUV420PGrabber::print() {
  printf("win_w: %d\n", win_w);
  printf("win_h: %d\n", win_h);
  printf("vid_w: %d\n", vid_w);
  printf("vid_h: %d\n", vid_h);
  printf("uv_w: %d\n", uv_w);
  printf("uv_h: %d\n", uv_h);
  printf("scene_tex: %d\n", scene_tex);
  printf("y_tex: %d\n", y_tex);
  printf("u_tex: %d\n", u_tex);
  printf("v_tex: %d\n", v_tex);
}
