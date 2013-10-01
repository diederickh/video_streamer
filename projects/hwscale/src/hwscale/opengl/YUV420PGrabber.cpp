#include <assert.h>
#include <iostream>
#include <hwscale/opengl/YUV420PGrabber.h>

#define YUV420P_USE_APPLE_VAO 0

YUV420PSize::YUV420PSize() 
  :y_offset(0)
  ,u_offset(0)
  ,v_offset(0)
  ,y_stride(0)
  ,u_stride(0)
  ,v_stride(0)
  ,yw(0)
  ,yh(0)
  ,uvw(0)
  ,uvh(0)
  ,y_viewport_x(0)
  ,y_viewport_y(0)
  ,u_viewport_x(0)
  ,u_viewport_y(0)
  ,v_viewport_x(0)
  ,v_viewport_y(0)
  ,id(0)
{
  planes[0] = NULL;
  planes[1] = NULL;
  planes[2] = NULL;
  strides[0] = 0;
  strides[1] = 0;
  strides[2] = 0;
}

void YUV420PSize::print() {
  printf("--\n");
  printf("id: %d\n", id);
  printf("y-plane width: %d\n", yw);
  printf("y-plane height: %d\n", yh);
  printf("uv-planes width: %d\n" , uvw);
  printf("uv-planes height: %d\n", uvh);
  printf("y viewport (x,y): %d, %d\n", y_viewport_x, y_viewport_y);
  printf("u viewport (x,y): %d, %d\n", u_viewport_x, u_viewport_y);
  printf("v viewport (x,y): %d, %d\n", v_viewport_x, v_viewport_y);

  printf("\n");
}

// ------------------------------------------------

YUV420PGrabber::YUV420PGrabber()
  :win_w(0)
  ,win_h(0)
  ,vid_w(0)
  ,vid_h(0)
  ,uv_h(0)
  ,uv_w(0)
  ,fps(0)
  ,tex_w(0)
  ,tex_h(0)
  ,yuv_tex(0)
  ,vao(0)
  ,scene_fbo(0)
  ,scene_depth(0)
  ,scene_tex(0)
  ,prog_y(0)
  ,frag_y(0)
  ,vert_yuv(0)
  ,prog_pt(0)
  ,frag_pt(0)
  ,image(NULL)
   /*
  ,y_plane(NULL)
  ,u_plane(NULL)
  ,v_plane(NULL)
   */
  ,frame_timeout(0)
  ,frame_prev_timeout(0)
  ,frame_delay(0)
  ,frame_delay_adjusted(0)
  ,frame_diff(0)
  ,frame_diff_avg(0)
  ,prog_u(0)
  ,prog_v(0)
  ,frag_u(0)
  ,frag_v(0)
  ,pbo_read_dx(0) // not used for now
  ,pbo_write_dx(0) // now used for now 
  ,outfile_set(false)
  ,outfile_size_id(-1)
{
  memset((char*)pbo, 0x00, sizeof(pbo));
}

YUV420PGrabber::~YUV420PGrabber() {
  deleteShader(frag_y);
  deleteShader(vert_yuv);
  deleteShader(frag_u);
  deleteShader(frag_v);
  deleteShader(frag_pt);
  deleteProgram(prog_y);
  deleteProgram(prog_u);
  deleteProgram(prog_v);
  deleteProgram(prog_pt);
  deleteTexture(yuv_tex);
  deleteTexture(scene_tex);

  if(scene_fbo) {
    glDeleteFramebuffers(1, &scene_fbo);
  }

  if(scene_depth) {
    glDeleteRenderbuffers(1, &scene_depth);
  }

  if(vao) {
#if YUV420P_USE_APPLE_VAO
    glDeleteVertexArraysAPPLE(1, &vao);
#else
    glDeleteVertexArrays(1, &vao);
#endif  
  }

  if(outfile_set && ofs.is_open()) {
    ofs.close();
  }

  if(image) {
    delete[] image;
  }

  outfile_set = false;
  win_w = 0;
  win_h = 0;
  vid_w = 0;
  vid_h = 0;
  uv_w = 0;
  uv_h = 0;
  yuv_tex = 0;
  scene_fbo = 0;
  scene_depth = 0;
  scene_tex = 0;
  vao = 0;
  fps = 0;
  tex_w = 0;
  tex_h = 0;
  image = NULL;
  /*
  y_plane = NULL;
  u_plane = NULL;
  v_plane = NULL;
  */
  frame_timeout = 0;
  frame_prev_timeout = 0;
  frame_delay = 0;
  frame_delay_adjusted = 0;
  frame_diff = 0;
  frame_diff_avg = 0;
  
}

bool YUV420PGrabber::setup(int winW, int winH, int framerate) {

  if(!winW || !winH) {
    printf("error: invalid win_w or win_h: %dx%d\n", winW, winH);
    return false;
  }

  if(!framerate) {
    printf("error: invalid fps: %d\n", framerate);
    return false;
  }

  win_w = winW;
  win_h = winH;
  uv_w = vid_w * 0.5;
  uv_h = vid_h * 0.5;
  fps = framerate;

  if(!setupSizes()) {
    printf("error: cannot setup the sizes. did you add any? make sure to call addSize()\n");
    return false;
  }

  if(!setupTextures()) {
    printf("error: cannot setup textures.\n");
    return false;
  }

  if(!setupPBO()) {
    printf("error: cannot setup pbox.\n");
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


  image = new unsigned char[tex_w * tex_h];
  if(!image) {
    printf("error: cannot allocate image buffer.\n");
    return false;
  }

  // set the pointers to the image
  for(std::vector<YUV420PSize>::iterator it = sizes.begin(); it != sizes.end(); ++it) {
    YUV420PSize& s = *it;
    s.planes[0] = &image[s.y_offset];
    s.planes[1] = &image[s.u_offset];
    s.planes[2] = &image[s.v_offset];
  }

  /*
  y_plane = image;
  u_plane = image + (vid_w * vid_h);
  v_plane = u_plane + (uv_w * uv_h);
  */

  frame_delay = (1.0/fps) * 1000 * 1000 * 1000;

  return true;
}

void YUV420PGrabber::start() {
  frame_timeout = uv_hrtime() + frame_delay;
  time_started = uv_hrtime() / (1000 * 1000);
}

bool YUV420PGrabber::setupSizes() {
  
  if(!sizes.size()) {
    printf("error: no sizes added!\n");
    return false;
  }

  // get min and max y-plane sizes
  uint32_t max_w = 0;
  uint32_t min_w = 999999;
  uint32_t max_h = 0;
  for(std::vector<YUV420PSize>::iterator it = sizes.begin(); it != sizes.end(); ++it) {
    YUV420PSize& s = *it;
    if( (s.yw + s.uvw) > max_w) {
      max_w = s.yw + s.uvw;
    }
    if(s.yw < min_w) {
      min_w = s.yw;
    }
    max_h += s.yh;
  }

  if(!max_w || min_w == 999999) {
    printf("error: cannot find the min/max sizes of the y-planes.\n");
    return false;
  }

  tex_w = max_w;
  tex_h = max_h;

  // determine the viewports into which this part must be drawn.
  printf("tex_h: %d, tex_w: %d\n", tex_h, tex_w);

  std::sort(sizes.begin(), sizes.end(), YUV420PSize());
  int offset_y = 0;
  for(std::vector<YUV420PSize>::iterator it = sizes.begin(); it != sizes.end(); ++it) {
     YUV420PSize& s = *it;
     s.y_viewport_y = offset_y;
     s.y_viewport_x = 0;
     s.u_viewport_x = s.yw;
     s.u_viewport_y = offset_y;
     s.v_viewport_x = s.yw;
     s.v_viewport_y = offset_y + s.uvh;

#if 0
     // read upside down
     int line_y = (tex_h - (offset_y+s.yh)) ;
     s.y_offset = line_y * tex_w;
     s.u_offset = (line_y * tex_w) + s.u_viewport_x ; 
     s.v_offset = (tex_h - s.v_viewport_y) * tex_w + s.v_viewport_x ;
#else
     // read correctly 
     int line_y = offset_y;
     s.y_offset = line_y * tex_w;
     s.u_offset = ( line_y * tex_w ) + s.u_viewport_x ; 
     s.v_offset = ( s.v_viewport_y  * tex_w ) + s.v_viewport_x ;
#endif
     s.y_stride = s.strides[0] = tex_w;
     s.u_stride = s.strides[1] = tex_w;
     s.v_stride = s.strides[2] = tex_w;

     offset_y += s.yh;
  }

  return true;
}

bool YUV420PGrabber::setupPBO() {
  assert(tex_w > 0 && tex_h > 0); /* setupSizes() must be called first */
  
  size_t buffer_size = tex_w * tex_h;

  glGenBuffers(YUV420P_NUM_PBO, pbo);

  for(int i = 0; i < YUV420P_NUM_PBO; ++i) {
    glBindBuffer(GL_PIXEL_PACK_BUFFER, pbo[i]);
    glBufferData(GL_PIXEL_PACK_BUFFER, buffer_size, NULL, GL_STREAM_DRAW);
  }
  
  glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
  return true;
}

bool YUV420PGrabber::setupFBO() {
  assert(win_w && win_h && scene_fbo == 0);
  assert(yuv_tex);
  assert(tex_w && tex_h);

  glGenFramebuffers(1, &scene_fbo);
  glBindFramebuffer(GL_FRAMEBUFFER, scene_fbo);
  
  glGenRenderbuffers(1, &scene_depth);
  glBindRenderbuffer(GL_RENDERBUFFER, scene_depth);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT32, tex_w, tex_h);
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, scene_depth);
  
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, scene_tex, 0);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, yuv_tex, 0);

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
  //assert(win_w && win_h && vid_w && vid_h && yuv_tex == 0);
  assert(win_w && win_h && yuv_tex == 0);
  assert(tex_w && tex_h); // setupTextures() must be called -after- setupSizes() 

  glGenTextures(1, &scene_tex);
  glBindTexture(GL_TEXTURE_2D, scene_tex);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, tex_w, tex_h, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL); 
  setTextureParameters();

  glGenTextures(1, &yuv_tex);
  glBindTexture(GL_TEXTURE_2D, yuv_tex);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, tex_w, tex_h, 0, GL_RED, GL_UNSIGNED_BYTE, NULL);
  setTextureParameters();

  return true;
}

bool YUV420PGrabber::setupVAO() {
  assert(vao == 0);

#if YUV420P_USE_APPLE_VAO
  glGenVertexArraysAPPLE(1, &vao);
#else
  glGenVertexArrays(1, &vao);  
#endif

  return true;
}

void YUV420PGrabber::bindVAO() {
#if YUV420P_USE_APPLE_VAO
  glBindVertexArrayAPPLE(vao);
#else
  glBindVertexArray(vao);
#endif
}

void YUV420PGrabber::unbindVAO() {
#if YUV420P_USE_APPLE_VAO
  glBindVertexArrayAPPLE(0);
#else
  glBindVertexArray(0);
#endif
}


bool YUV420PGrabber::setupShaders() {
  GLint u_tex = -1;

  // Shared vertex shader 
  vert_yuv = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vert_yuv, 1, &YUV420P_YUV_VS, NULL);
  glCompileShader(vert_yuv);
  printShaderCompileInfo(vert_yuv);

  // Y-shader
  /*
  vert_y = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vert_y, 1, &YUV420P_Y_VS, NULL);
  glCompileShader(vert_y);
  printShaderCompileInfo(vert_y);
  */

  frag_y = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(frag_y, 1, &YUV420P_Y_FS, NULL);
  glCompileShader(frag_y);
  printShaderCompileInfo(frag_y);

  prog_y = glCreateProgram();
  glAttachShader(prog_y, vert_yuv);
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

  // U - shader
  frag_u = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(frag_u, 1, &YUV420P_U_FS, NULL);
  glCompileShader(frag_u);
  printShaderCompileInfo(frag_u);

  prog_u = glCreateProgram();
  glAttachShader(prog_u, vert_yuv);
  glAttachShader(prog_u, frag_u);
  glLinkProgram(prog_u);
  printShaderLinkInfo(prog_u);

  u_tex = glGetUniformLocation(prog_u, "u_tex");
  if(u_tex < 0) {
     printf("error: cannot find u_tex in frag_u shader.\n");
     return false;
  }
  glUseProgram(prog_u);
  glUniform1i(u_tex, 0);

  // V - Shader
  frag_v = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(frag_v, 1, &YUV420P_V_FS, NULL);
  glCompileShader(frag_v);
  printShaderCompileInfo(frag_v);

  prog_v = glCreateProgram();
  glAttachShader(prog_v, vert_yuv);
  glAttachShader(prog_v, frag_v);
  glLinkProgram(prog_v);
  printShaderLinkInfo(prog_v);

  u_tex = glGetUniformLocation(prog_v, "u_tex");
  if(u_tex < 0) {
     printf("error: cannot find u_tex in frag_u shader.\n");
     return false;
  }
  glUseProgram(prog_v);
  glUniform1i(u_tex, 0);

  // +++++++++++++++++++++++++++++++++++++++++++++++++++++
  
  // Pass through shader
  frag_pt = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(frag_pt, 1, &YUV420P_PT_FS, NULL);
  glCompileShader(frag_pt);
  printShaderCompileInfo(frag_pt);

  prog_pt = glCreateProgram();
  glAttachShader(prog_pt, vert_yuv);
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
  glBindFramebuffer(GL_FRAMEBUFFER, scene_fbo);
  glDrawBuffers(1, drawbufs);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glViewport(0,0, tex_w,tex_h);
}

void YUV420PGrabber::endGrab() {

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, scene_tex);

  glDisable(GL_DEPTH_TEST);

  GLenum drawbufs[] = { GL_COLOR_ATTACHMENT1 };
  glDrawBuffers(1, drawbufs);
  glClear(GL_COLOR_BUFFER_BIT); // we don't need this but it's just nice to have a clean slate ^.^

  bindVAO();
  {
    // render Y planes
    glUseProgram(prog_y);
    for(std::vector<YUV420PSize>::iterator it = sizes.begin(); it != sizes.end(); ++it) {
      YUV420PSize& s = *it;
      glViewport(s.y_viewport_x, s.y_viewport_y, s.yw, s.yh);
      glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);      
    }

    // render U planes
    glUseProgram(prog_u);
    for(std::vector<YUV420PSize>::iterator it = sizes.begin(); it != sizes.end(); ++it) {
      YUV420PSize& s = *it;
      glViewport(s.u_viewport_x, s.u_viewport_y, s.uvw, s.uvh);
      glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);      
    }

    // render V planes
    glUseProgram(prog_v);
    for(std::vector<YUV420PSize>::iterator it = sizes.begin(); it != sizes.end(); ++it) {
      YUV420PSize& s = *it;
      glViewport(s.v_viewport_x, s.v_viewport_y, s.uvw, s.uvh);
      glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);      
    }
  }

  unbindVAO();

  glUseProgram(0);
  glBindTexture(GL_TEXTURE_2D, 0);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glDrawBuffer(GL_BACK);
  glViewport(0,0, win_w, win_h);
}

void YUV420PGrabber::downloadTextures() {

  // simple synchronous download, we could use PBOs for async transfers here 
  glBindTexture(GL_TEXTURE_2D, 0);
  glBindFramebuffer(GL_READ_FRAMEBUFFER, scene_fbo);
  glReadBuffer(GL_COLOR_ATTACHMENT1);
  glReadPixels(0, 0, tex_w, tex_h, GL_RED, GL_UNSIGNED_BYTE, image);
  glBindFramebuffer(GL_FRAMEBUFFER,0);
  //  printf("FIRST: %02X\n", image[0]);

 if(outfile_set) {

   YUV420PSize s = getSize(outfile_size_id);
   //printf("writing first plane: %02X\n", *s.planes[0]);

   // y-plane
   for(size_t j = 0; j < s.yh; ++j) {
     size_t dx = j * s.strides[0];
     ofs.write((char*)s.planes[0] + dx, s.yw); // one line
   }

   // u-plane
   for(size_t j = 0; j < s.uvh; ++j) {
     size_t dx = j * s.strides[1];
     ofs.write((char*)s.planes[1] + dx, s.uvw); // one line
   }

   // v-plane
   for(size_t j = 0; j < s.uvh; ++j) {
     size_t dx = j * s.strides[2];
     ofs.write((char*)s.planes[2] + dx, s.uvw); // one line
   }
   
 }
  
/*
  glBindTexture(GL_TEXTURE_2D, 0);
  glBindFramebuffer(GL_READ_FRAMEBUFFER, scene_fbo);

  glReadBuffer(GL_COLOR_ATTACHMENT3);
  glReadPixels(0, 0, uv_w, uv_h, GL_RED, GL_UNSIGNED_BYTE, v_plane);

  glReadBuffer(GL_COLOR_ATTACHMENT2);
  glReadPixels(0, 0, uv_w, uv_h, GL_RED, GL_UNSIGNED_BYTE, u_plane);

  glReadBuffer(GL_COLOR_ATTACHMENT1);
  glReadPixels(0, 0, vid_w, vid_h, GL_RED, GL_UNSIGNED_BYTE, y_plane);
  
  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  glReadBuffer(GL_BACK);
  glDrawBuffer(GL_BACK);
*/
}

  // IMPORTANT: on mac 10.8, the order of download is important, it needs 
  // to start downloading the GL_COLOR_ATTACHMENT3 first, then 2, then 1.
  // seems like a driver bug (!?)
  


// @todo - this makes the screen flicker!
void YUV420PGrabber::draw() {
  assert(win_w);
  assert(win_h);

  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glDrawBuffer(GL_BACK);
  glDisable(GL_DEPTH_TEST);  
  glViewport(0, 0, win_w, win_h);
  // printf("win: %d x %d\n", win_w, win_h);

  bindVAO();

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, scene_tex);

  glUseProgram(prog_pt);

  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

  unbindVAO();
}

bool YUV420PGrabber::setOutput(int id, std::string filepath) {
  
  if(outfile_set) {
    printf("warning: you already opened a output file..closing it now.\n");
    ofs.close();
  }

  if(!filepath.size()) {
    printf("error: invalid filepath.\n");
    return false;
  }

  ofs.open(filepath.c_str(), std::ios::out | std::ios::binary);
  if(!ofs.is_open()) {
    printf("error: cannot open grabber output file: %s\n", filepath.c_str());
    return false;
  }

  outfile_set = true;
  outfile_size_id = id;

  return true;
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
  printf("--");

  for(std::vector<YUV420PSize>::iterator it = sizes.begin(); it != sizes.end(); ++it) {
    YUV420PSize& s = *it;
    s.print();
  }
}


