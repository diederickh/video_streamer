/*

---------------------------------------------------------------------------------
      
                                               oooo              
                                               `888              
                oooo d8b  .ooooo.  oooo    ooo  888  oooo  oooo  
                `888""8P d88' `88b  `88b..8P'   888  `888  `888  
                 888     888   888    Y888'     888   888   888  
                 888     888   888  .o8"'88b    888   888   888  
                d888b    `Y8bod8P' o88'   888o o888o  `V88V"V8P' 
                                                                 
                                                  www.roxlu.com
                                             www.apollomedia.nl  
                                          www.twitter.com/roxlu
              
---------------------------------------------------------------------------------
  
  # Graph

  This is used to generate the data for a graph. It connect to the 
  graph daemon and sends all data you add to it to this daemon which 
  in turn draws it.

 */

#ifndef ROXLU_GRAPHER_GRAPH_H
#define ROXLU_GRAPHER_GRAPH_H

extern "C" {
#  include <uv.h>
#  include <nanomsg/nn.h>
#  include <nanomsg/pipeline.h>
}

#include <string>
#include <vector>
#include <grapher/Types.h>
#include <grapher/GraphConfigReader.h>

#if defined(USE_OPENGL_PROFILER)
#  include <grapher/opengl/OpenGLProfiler.h>
#endif

#define GRAPHER_INIT(client_xml) grapher_load_client_xml(client_xml);
#define GRAPHER_START() grapher_start();
#define GRAPHER_ADD(graph_id, graph_name, graph_value) grapher_add_value(graph_id, graph_name, graph_value);
#define GRAPHER_UPDATE() grapher_update(); /* call this when you want to process all markers, normally once per frame */

// ------------------------------------------------

void graph_thread(void* user);

// ------------------------------------------------

// Used for thread communication
struct GraphPacket {
  void putU8(uint8_t v);
  void putU16(uint16_t v);
  void putU32(uint32_t v);
  void putU64(uint64_t v);
  void putS8(int8_t v);
  void putS16(int16_t v);
  void putS32(int32_t v);
  void putS64(int64_t v);
  void putF32(float v);

  size_t size();
  std::vector<uint8_t> data;
};

// ------------------------------------------------

class Graph {
 public:
  Graph();
  ~Graph();
  bool load(std::string config); /* load the config, see GraphConfigReader for an example for a client config file. */
  bool setup(std::string address); /* connect to this address (not necessary if you have stored the address in the config) */
  bool start(); /* start the thread so you can start sending data */
  bool stop();  /* stop the thread */
  void addGraph(const GraphConfig& gc);
  void addPacket(GraphPacket* pkt); /* we take ownership and free the packets in the thread */
  void add(uint8_t id, std::string graphname, float value); /* add some value to the graph */
 public:
  bool is_running;
  std::string address;
  uv_thread_t thread;
  uv_cond_t cv;
  uv_mutex_t mutex;
  std::vector<GraphPacket*> work;
};

extern Graph grapher; /* global instance that is used by macros and other profilers */
extern void grapher_load_client_xml(std::string filepath); /* load the clent xml on the global object */
extern void grapher_start(); /* makes a connection to the grapher */
extern void grapher_add_value(uint8_t id, std::string name, float value); /* add a new value to the given graph id and line name */
extern void grapher_update();

// ------------------------------------------------

inline void Graph::add(uint8_t id, std::string graphname, float value) {
  GraphPacket* gp = new GraphPacket();
  gp->putU8(PKT_TYPE_DATA);
  gp->putF32(value);
  gp->putU16(graphname.size());
  std::copy(graphname.begin(), graphname.end(), std::back_inserter(gp->data));
  addPacket(gp);
}

inline size_t GraphPacket::size() {
  return data.size();
}

inline void GraphPacket::putU8(uint8_t v) {
  data.push_back(v);
}

inline void GraphPacket::putU16(uint16_t v) {
  uint8_t* p = (uint8_t*)&v;
  putU8(p[1]);
  putU8(p[0]);
}

inline void GraphPacket::putU32(uint32_t v) {
  uint8_t* p = (uint8_t*)&v;
  putU8(p[3]);
  putU8(p[2]);
  putU8(p[1]);
  putU8(p[0]);
}

inline void GraphPacket::putU64(uint64_t v) {
  uint8_t* p = (uint8_t*)&v;
  putU8(p[7]);
  putU8(p[6]);
  putU8(p[5]);
  putU8(p[4]);
  putU8(p[3]);
  putU8(p[2]);
  putU8(p[1]);
  putU8(p[0]);
}

inline void GraphPacket::putS8(int8_t v) {
  uint8_t* p = (uint8_t*)&v;
  putU8(p[0]);
}

inline void GraphPacket::putS16(int16_t v) {
  uint8_t* p = (uint8_t*)&v;
  putU8(p[1]);
  putU8(p[0]);
}

inline void GraphPacket::putS32(int32_t v) {
  uint8_t* p = (uint8_t*)&v;
  putU8(p[3]);
  putU8(p[2]);
  putU8(p[1]);
  putU8(p[0]);
}

inline void GraphPacket::putS64(int64_t v) {
  uint8_t* p = (uint8_t*)&v;
  putU8(p[7]);
  putU8(p[6]);
  putU8(p[5]);
  putU8(p[4]);
  putU8(p[3]);
  putU8(p[2]);
  putU8(p[1]);
  putU8(p[0]);
}

inline void GraphPacket::putF32(float v) {
  uint8_t* p = (uint8_t*)&v;
  putU8(p[3]);
  putU8(p[2]);
  putU8(p[1]);
  putU8(p[0]);
}

#endif
