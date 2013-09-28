#include <grapher/Vertices.h>
#include <cmath>

Vertices::Vertices() 
  :vao(0)
  ,vbo(0)
  ,changed(false)
  ,bytes_allocated(0)
{
}

Vertices::~Vertices() {
}


bool Vertices::setup() {
#if USE_APPLE
  glGenVertexArraysAPPLE(1, &vao);
  glBindVertexArrayAPPLE(vao);
#else
  glGenVertexArrays(1, &vao);
  glBindVertexArray(vao);
#endif
  glGenBuffers(1, &vbo);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);

  glEnableVertexAttribArray(0); // pos
  glEnableVertexAttribArray(1); // col;

  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)0);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)8);
  
  return true;
}

void Vertices::update() {

  if(!changed) {
    return;
  }

  changed = false;

  size_t bytes_needed = numBytes();
  if(!bytes_needed) {
    return;
  }

  glBindBuffer(GL_ARRAY_BUFFER, vbo);

  if(bytes_needed) {
    while(bytes_allocated < bytes_needed) {
      bytes_allocated = std::max<size_t>(bytes_allocated * 2, 1024);
    }
    glBufferData(GL_ARRAY_BUFFER, bytes_allocated, NULL, GL_DYNAMIC_DRAW);
  }

  glBufferSubData(GL_ARRAY_BUFFER, 0, bytes_needed, getPtr());
}
