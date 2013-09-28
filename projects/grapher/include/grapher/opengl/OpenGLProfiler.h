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

 # OpenGLProfiler

 This class uses OpenGL GL_TIMESTAMP query objects to inspect 
 the performance of grouped OpenGL calls. The markers you add using
 addMarker() and measure using measure(), will keep track of the time
 spend between two marker points. 

 Therefore it's a good idea to work with `[marker]_start` and `[marker]_end`
 names which will give you the time between the marker start and end in 
 nanoseconds.

 You can use this class directly of by using the grapher tool and the 
 macros: GRAPHER_ADD_GPU_MARKER and GPU_MARKER.  

 See this gist for an example:  https://gist.github.com/roxlu/9fa9af699b7c2c49b1da#file-main-cpp

 Pseudo code:

 ````c++
 void setup() {
   GRAPHER_INIT("client.xml");
   GRAPHER_START();
   GRAPHER_ADD_GPU_MARKER("frame_start");
   GRAPHER_ADD_GPU_MARKER("frame_end");
 }
 
 void draw() {
   GPU_MARKER("frame_start");
     glDrawArays(GL_TRIANGLES, 0, ...);
   GPU_MARKER("frame_end")

   GRAPHER_UPDATE()
 }
 ````
 */


#ifndef ROXLU_OPENGL_PROFILER_H
#define ROXLU_OPENGL_PROFILER_H

#if defined(__APPLE__)
#  include <OpenGL/gl3.h>
#  include <OpenGL/gl3ext.h>
#else 
#   error "Only tested on mac"
#endif

#define NUM_QUERY_OBJECTS 3

#include <map>
#include <vector>
#include <string>

#define GRAPHER_ADD_GPU_MARKER(graphid, name) glprof_add_marker(graphid, name); /* add a new gpu marker */
#define GPU_MARKER(graphid, name) glprof_measure(name);   /* add a gpu marker, for the given graphid (which you defined in the client.xml file) for the given graph name */

// ------------------------------------------

struct  OpenGLProfilerMarker {
  OpenGLProfilerMarker();
  void toggle();
  GLuint ids[NUM_QUERY_OBJECTS];
  unsigned int read_dx;
  unsigned int write_dx;
  GLuint64 timestamp;
  int graph_id; /* the id of the graph on which this will be used */
};

// ------------------------------------------

class OpenGLProfiler {
 public:
  OpenGLProfiler();
  ~OpenGLProfiler();

  void addMarker(std::string name, int graphID); /* call this only ones to setup a marker */
  void measure(std::string name); /* `tick` the given marker name, we will measure time between markers */
  void update(); /* update will try to read all added markers, call this once every frame! */

  /* super basic usage of timer queries; getTimeStamps() will take as long as it needs to get the timestamp between begin() and end(). */
  void begin();
  void end();
  GLuint64 getTimeStamp();

  std::map<std::string, OpenGLProfilerMarker> markers;
  std::vector<std::string> measure_order;

  GLuint qid;
};

extern OpenGLProfiler* grapher_glprof;
extern void glprof_init(); /* creates the grapher_glprof instance; must be done after you created a GL context. this will be called from GRAPHER_INIT() when -DUSE_OPENGL_PROFILER is found */
extern void glprof_measure(std::string marker); /* this will update a marker ; we measure the time between two markers */
extern void glprof_add_marker(uint8_t id, std::string marker); /* adds a new marker to the profiler object */
extern void glprof_update(); /* updates and processes the gpu markers <> reads query counters */

// ------------------------------------------

inline void OpenGLProfiler::begin() {
  glBeginQuery(GL_TIME_ELAPSED, qid);  
}

inline void OpenGLProfiler::end() {
  glEndQuery(GL_TIME_ELAPSED);
}

inline GLuint64 OpenGLProfiler::getTimeStamp() {
  GLint done = 0;
  GLuint64 ts = 0;
  while(!done) {
    glGetQueryObjectiv(qid, GL_QUERY_RESULT_AVAILABLE, &done);
  }
  glGetQueryObjectui64v(qid, GL_QUERY_RESULT, &ts);
  return ts;
}


#endif
