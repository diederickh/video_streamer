#ifndef ROXLU_GRAPHER_VERTICES_H
#define ROXLU_GRAPHER_VERTICES_H

#include <GLFW/glfw3.h>
#include <glext.h>
#include <vector>

// ----------------------------------------------------------

struct Vertex {
  Vertex(float x, float y, float r, float g, float b);
  float pos[2];
  float color[3];
};

// ----------------------------------------------------------

struct Vertices {
public:
  Vertices();
  ~Vertices();
  bool setup();
  void push_back(const Vertex& v);
  size_t size();
  void clear();
  void update();
  size_t numBytes();
  const float* getPtr();
  void bind();
public:
  GLuint vbo;
  GLuint vao;
  std::vector<Vertex> vertices;
  bool changed;
  size_t bytes_allocated;
};

inline Vertex::Vertex(float x, float y, float r, float g, float b)  {
  pos[0] = x;
  pos[1] = y;
  float rcp = 1.0f/255.0;
  color[0] = r * rcp;
  color[1] = g * rcp;
  color[2] = b * rcp;
}

inline void Vertices::push_back(const Vertex& v) {
  vertices.push_back(v);
  changed = true;
}

inline size_t Vertices::size() {
  return vertices.size();
}

inline void Vertices::clear() {
  vertices.clear();
  changed = true;
}

inline size_t Vertices::numBytes() {
  return size() * sizeof(Vertex);
}

inline const float* Vertices::getPtr() {
  if(!vertices.size()) {
    return NULL;
  }
  return &vertices[0].pos[0];
}

inline void Vertices::bind() {
#if USE_APPLE
  glBindVertexArrayAPPLE(vao);
#else 
  glBindVertexArray(vao);
#endif
}

#endif

