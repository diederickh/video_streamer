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

  # Resizer FBO

  This classes uses a FBO to resize Y, U and V planes that are stored
  in the textures that are given to this class,  different scales. Each 
  FBO can resize to a maximum of 2 different scales. 

*/
#ifndef ROXLU_HWSCALE_RESIZER_FBO_H
#define ROXLU_HWSCALE_RESIZER_FBO_H

#if defined(__APPLE__)
#  include <OpenGL/gl3.h>
#  include <OpenGL/glext.h>
#else 
#   include <GLXW/glxw.h> 
#endif

#include <hwscale/opengl/YUV420PGrabber.h>

class ResizerFBO {
 public:
  ResizerFBO(GLuint ytex, GLuint utex, GLuint vtex);
  ~ResizerFBO();
  bool setup(int inWidth, int inHeight, int w0, int h0, int w1 = 0, int h1 = 0); /* inWidth/inHeight is the size of the y_tex. set the width and heights of the two resized outputs */
  void resize(YUV420PGrabber& grabber);  /* does the actual resizing */
  GLuint createTexture(int w, int h);

 public:
  int in_width;
  int in_height;
  GLuint y_tex;
  GLuint u_tex;
  GLuint v_tex;
  int width[2];
  int height[2];

  GLuint fbo;
  GLuint depth;
  GLuint resized_y_tex[2];
  GLuint resized_u_tex[2];
  GLuint resized_v_tex[2];
};

#endif
