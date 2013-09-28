#ifndef ROXLU_GRAPHER_DAEMON_H
#define ROXLU_GRAPHER_DAEMON_H

extern "C" {
#  include <uv.h>
#  include <nanomsg/nn.h>
#  include <nanomsg/pipeline.h>
}

#include <string>
#include <map>
#include <grapher/GraphDrawer.h>
#include <grapher/Types.h>

// ------------------------------------------------

void daemon_thread(void* user);

// ------------------------------------------------

// data for a graph (id) and a specific line (name)
struct GraphDataPacket {
  std::string name; /* name of the graph line */
  uint8_t id; /* the GraphDrawer instance */
  float value; /* the value for the graph */
};

// ------------------------------------------------

class Daemon {
public:
  Daemon();
  ~Daemon();
  bool setup(std::string address, int winW, int winH);
  bool start();
  void stop();
  void lock();
  void unlock();
  void update();
  void draw();
  void onResize(int winW, int winH);

  void prev();  /* switch to previous graph */
  void next();  /* switch to next graph */
public:
  std::vector<GraphConfig> configs; /* when we receive a new graph config it's added to this vector so the GraphDrawer* is created in the main thread */
  std::map<uint8_t, GraphDrawer*> graphs;
  std::vector<GraphDataPacket*> data_packets; /* the data that the client send to us */
  bool must_run;
  std::string address;
  uv_mutex_t mutex;
  uv_thread_t thread;
  int32_t active_graph;
  int win_w;
  int win_h;
};

inline void Daemon::lock() {
  uv_mutex_lock(&mutex);
}

inline void Daemon::unlock() {
 uv_mutex_unlock(&mutex);
}

inline void Daemon::next() {
  if(!graphs.size()) {
    return;
  }

  ++active_graph %= graphs.size();
}

inline void Daemon::prev() {
  if(!graphs.size()) {
    return;
  }
  if(active_graph == 0) {
    active_graph = graphs.size() - 1;
  }
  else {
    active_graph--;
  }
}

#endif
