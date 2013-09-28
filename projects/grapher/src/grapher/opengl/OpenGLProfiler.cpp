#include <grapher/opengl/OpenGLProfiler.h>
#include <grapher/Graph.h>
#include <iostream>

// ------------------------------------------

OpenGLProfilerMarker::OpenGLProfilerMarker() 
  :read_dx(0)
  ,write_dx(0)
  ,timestamp(0)
  ,graph_id(0)
{
}

void OpenGLProfilerMarker::toggle() {
}

// ------------------------------------------

OpenGLProfiler::OpenGLProfiler() {
  glGenQueries(1, &qid);
}

OpenGLProfiler::~OpenGLProfiler() {
}

void OpenGLProfiler::addMarker(std::string name, int graphID) {
  OpenGLProfilerMarker marker;
  glGenQueries(NUM_QUERY_OBJECTS, marker.ids);
  for(int i = 0; i < (NUM_QUERY_OBJECTS - 1); ++i) {
    glQueryCounter(marker.ids[i], GL_TIMESTAMP);
  }
  
  marker.graph_id = graphID;
  marker.write_dx = NUM_QUERY_OBJECTS - 1;
  markers[name] = marker;
}

void OpenGLProfiler::update() {

  GLint is_ready = 0;
  
  std::map<std::string, OpenGLProfilerMarker>::iterator it = markers.begin(); 
  while(it != markers.end()) {
    OpenGLProfilerMarker& m = it->second;
    glGetQueryObjectiv(m.ids[m.read_dx], GL_QUERY_RESULT_AVAILABLE, &is_ready);
    if(is_ready) {
      glGetQueryObjectui64v(m.ids[m.read_dx], GL_QUERY_RESULT, &m.timestamp);
      ++m.read_dx %= NUM_QUERY_OBJECTS;
    }
    else {
      printf("++ failed reading profiler marker, tried: %d. try to increase NUM_QUERY_OBJECTS\n", m.read_dx);
    }
    is_ready = 0;
    ++it;
  }

  GLuint64 prev_time = 0;
  GLuint64 diff_time = 0;

  for(std::vector<std::string>::iterator it = measure_order.begin(); it != measure_order.end(); ++it) {
    OpenGLProfilerMarker& m = markers[(*it)];
    if(prev_time) {
      diff_time = m.timestamp - prev_time;
      GRAPHER_ADD(m.graph_id, *it, diff_time);
      //printf("diff: %lld, diff ms: %f\n", diff_time, double(diff_time/1000000.0));
    }
    prev_time = m.timestamp;
  }
  measure_order.clear();
}

void OpenGLProfiler::measure(std::string name) {
  OpenGLProfilerMarker& marker = markers[name];
  glQueryCounter(marker.ids[marker.write_dx], GL_TIMESTAMP);
  ++marker.write_dx %= NUM_QUERY_OBJECTS;

  measure_order.push_back(name);
}


// Public API
// ------------------------------------------------

OpenGLProfiler* grapher_glprof = NULL;

void glprof_init() {
  grapher_glprof = new OpenGLProfiler();
};

void glprof_measure(std::string marker) {
  if(!grapher_glprof) {
    printf("error: no OpenGLGrapher instance found; did you call GRAPHER_INIT().");
    return;
  }
  grapher_glprof->measure(marker);
}

void glprof_add_marker(uint8_t id, std::string marker) {
  if(!grapher_glprof) {
    printf("error: no OpenGLGrapher instance found; did you call GRAPHER_INIT().");
    return;
  }
  grapher_glprof->addMarker(marker, id);
}

void glprof_update() {
 if(!grapher_glprof) {
    printf("error: no OpenGLGrapher instance found; did you call GRAPHER_INIT().");
    return;
  }
 grapher_glprof->update();
}
