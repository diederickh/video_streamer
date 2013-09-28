#include <grapher/Graph.h>
#include <grapher/Types.h>
#include <algorithm>
#include <iterator>
#include <iostream>

// ------------------------------------------------

// send data to the grapher app 
void graph_thread(void* user) {
  Graph* g = static_cast<Graph*>(user);

  int sock = nn_socket(AF_SP, NN_PUSH);
  if(sock == -1) {
    printf("error: cannot create client socket.\n");
    return;
  }

  int rc = nn_connect(sock, g->address.c_str());
  if(rc < 0) {
    printf("error: cannot connect, %s\n", nn_strerror(errno));
    return;
  }
  std::vector<GraphPacket*> todo;

  while(true) {
    
    // get the work!
    uv_mutex_lock(&g->mutex);
    {
      while(g->work.size() == 0) {
        uv_cond_wait(&g->cv, &g->mutex);
      }
      std::copy(g->work.begin(), g->work.end(), std::back_inserter(todo));
      g->work.clear();
    }
    uv_mutex_unlock(&g->mutex);

    // perform work
    bool must_stop = false;
    for(std::vector<GraphPacket*>::iterator it = todo.begin(); it != todo.end(); ++it) {
      GraphPacket* p = *it;

      if(p->data[0] == PKT_TYPE_STOP) {
        must_stop = true;
        break;
      }
      else {
        int rc = nn_send(sock, (char*)&p->data.front(), p->data.size(), 0);
        if(rc < 0) {
          printf("error: cannot send to grapher: %s\n", nn_strerror(rc));
        }
      }

      delete p;
      p = NULL;
    } // for

    todo.clear();

    if(must_stop) {
      break;
    }

  }
  printf("@todo -> cleanup socket.\n");
}

// ------------------------------------------------

Graph::Graph() 
  :thread(NULL)
  ,is_running(false)
{
  uv_mutex_init(&mutex);
  uv_cond_init(&cv);
}

Graph::~Graph() {
  printf("error: need to correctly shutdown graph; stop thread!\n");
  if(is_running) {
    stop();
    uv_thread_join(&thread);
  }
  is_running = false;
}

bool Graph::load(std::string config) {

  GraphConfigReader cfg;
  ClientSettings client_settings;
  if(!cfg.loadClientConfig(config, client_settings)) {
    return false;
  }
  
  if(!setup(client_settings.address)) {
    return false;
  }

  for(std::vector<GraphConfig>::iterator it = client_settings.graphs.begin(); it != client_settings.graphs.end(); ++it) {
    GraphConfig& gc = *it;
    addGraph(gc);
  }

  return true;
}

bool Graph::setup(std::string addr) {

  if(!addr.size()) {
    return false;
  }

  address = addr;
  
  return true;
}

bool Graph::start() {
  
  if(!address.size()) {
    printf("error: no address set; cannot connect to grapher.\n");
    return false;
  }

  uv_thread_create(&thread, graph_thread, this);

  is_running = true;

  return true;
}

bool Graph::stop() {
  
  if(!is_running) {
    printf("error: cannot stop as we're not running.\n");
    return false;
  }

  GraphPacket* gp = new GraphPacket();
  gp->putU8(PKT_TYPE_STOP);
  addPacket(gp);

  return true;
}


void Graph::addGraph(const GraphConfig& gc) {
  GraphPacket* gp = new GraphPacket();
  gp->putU8(PKT_TYPE_CONFIG);
  gp->putU16(gc.width);
  gp->putU16(gc.height);
  gp->putU16(gc.x);
  gp->putU16(gc.y);
  gp->putU8(gc.xtics);
  gp->putU8(gc.ytics);
  gp->putS32(gc.yrange[0]);
  gp->putS32(gc.yrange[1]);
  gp->putU8(gc.id);
  addPacket(gp);
}

void Graph::addPacket(GraphPacket* gp) {
  uv_mutex_lock(&mutex);
    work.push_back(gp);
    uv_cond_signal(&cv);
  uv_mutex_unlock(&mutex);
}


// ------------------------------------------------


// Code for the simple public API ~ Macros
Graph grapher;

void grapher_load_client_xml(std::string filepath) {
  if(!grapher.load(filepath)) {
    printf("error: cannot load client xml file: `%s`\n" ,filepath.c_str());
    ::exit(EXIT_FAILURE);
  }
#if defined(USE_OPENGL_PROFILER)
  glprof_init();
#endif
}

void grapher_start() {
  if(!grapher.start()) {
    printf("error: cannot start the graph.\n");
    ::exit(EXIT_FAILURE);
  }
}

void grapher_add_value(uint8_t id, std::string name, float value) {
  grapher.add(id, name, value);
}

void grapher_update() {
#if defined(USE_OPENGL_PROFILER)
  glprof_update();
#endif
}
