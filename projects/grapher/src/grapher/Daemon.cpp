#include <grapher/Daemon.h>
#include <grapher/Types.h>
#include <grapher/Endian.h>
#include <iostream>

// -----------------------------------------------------------

void daemon_thread(void* user) {
  Daemon* d = static_cast<Daemon*>(user);
  bool must_run = true;
  int rc = 0;
  int sock = 0;

  sock = nn_socket(AF_SP, NN_PULL);
  if(sock == -1) {
    printf("error: cannot create grapher socket.\n");
    ::exit(EXIT_FAILURE);
  }

  rc = nn_bind(sock, d->address.c_str());
  if(rc < 0) {
    printf("error: cannot bind grapher socket. %s\n", nn_strerror(errno));
    ::exit(EXIT_FAILURE);
  }

  char* data = NULL;
  char* buf = NULL;

  while(true) {

    d->lock();
      must_run = d->must_run;
    d->unlock();

    if(!must_run) {
      break;
    }

    rc = nn_recv(sock, &buf, NN_MSG, 0);
    if(rc < 0) {
      printf("error: cannot recv(), %s\n", nn_strerror(errno));
      continue;
    }

    data = buf;
    switch(data[0]) {

      // new data for a graph
      case PKT_TYPE_DATA: {

        uint16_t len;
        GraphDataPacket* gdp = new GraphDataPacket();
        uint32_t v = FromBE32(*(uint32_t*)&data[1]);
        memcpy((char*)&gdp->value, (char*)&v, 4);
        len = FromBE16(*(uint16_t*)&data[5]);
        std::copy((char*)data+7, (char*)data+7+len, std::back_inserter(gdp->name));

        d->lock();
          d->data_packets.push_back(gdp);
        d->unlock();

        break;
      }

      // new graph config! create a new graph!
      case PKT_TYPE_CONFIG: {
        if(rc != 20) {
          printf("error: not correct number of bytes for config.\n");
          ::exit(EXIT_FAILURE);
        }

        GraphConfig gc;
        gc.width = FromBE16(*(uint16_t*)&data[1]);
        gc.height = FromBE16(*(uint16_t*)&data[3]);
        gc.x = FromBE16(*(uint16_t*)&data[5]);
        gc.y = FromBE16(*(uint16_t*)&data[7]);
        gc.xtics = data[9];
        gc.ytics = data[10];
        gc.yrange[0] = FromBE32(*(int32_t*)&data[11]);
        gc.yrange[1] = FromBE32(*(int32_t*)&data[15]);
        gc.id = data[19];

        d->lock();
          d->configs.push_back(gc);
        d->unlock();

#if 0
        gc.print();
#endif
        break;
      }

      default: {
        printf("error: unhandled packet type.\n");
        break;
      }
    }; // switch

    nn_freemsg(buf);
  }

  printf("error -> cleanup socket in daemon.\n");
}


// -----------------------------------------------------------

Daemon::Daemon() 
  :thread(NULL)
  ,must_run(false)
  ,active_graph(-1)
  ,win_w(0)
  ,win_h(0)

{
  uv_mutex_init(&mutex);
}

Daemon::~Daemon() {
}

bool Daemon::setup(std::string addr, int winW, int winH) {
  assert(winW && winH);

  if(!addr.size()) {
    printf("error: address invalid: '%s'\n", addr.c_str());
    return false;
  }

  address = addr;
  win_w = winW;
  win_h = winH;
  
  return true;
}


bool Daemon::start() {

  if(!address.size()) {
    printf("error: address not set; call setup first.\n");
    return false;
  }

  must_run = true;

  uv_thread_create(&thread, daemon_thread, this);

  return true;
}

void Daemon::stop() {

  lock();
  must_run = false;
  unlock();

}

void Daemon::update() {
  std::vector<GraphDataPacket*> work_packets; /* copy of the data values we need to plot, received in the thread */

  // create new graphs.
  lock();
  {
    if(configs.size()) {
      for(std::vector<GraphConfig>::iterator it = configs.begin(); it != configs.end(); ++it) {

        GraphConfig& gc = *it;
        GraphDrawer* drawer = new GraphDrawer();
        drawer->setPosition(gc.x, gc.y);
        drawer->setSize(gc.width, gc.height);
        drawer->setRange(gc.yrange[0], gc.yrange[1]);
        drawer->xtics = gc.xtics;
        drawer->ytics = gc.ytics;
        graphs[gc.id] = drawer;

        // make the graph active if there is none set yet.
        if(active_graph < 0) {
          active_graph = gc.id;
        }

      }
      configs.clear();
    }

    // copy the data we received over to our own buffer.
    if(data_packets.size()) {
      std::copy(data_packets.begin(), data_packets.end(), std::back_inserter(work_packets));
      data_packets.clear();
    }
  }
  unlock();

  // update data packets
  for(std::vector<GraphDataPacket*>::iterator it = work_packets.begin(); it != work_packets.end(); ++it) {
    GraphDataPacket* gdp = *it;
    graphs[gdp->id]->add(gdp->name, gdp->value);
    delete gdp;
  }
  work_packets.clear(); // not necessary, just safe

  // update the graphs (backgrounds, lines etc..);
  for(std::map<uint8_t, GraphDrawer*>::iterator it = graphs.begin(); it != graphs.end(); ++it) {
    GraphDrawer* gd = it->second;
    if(!gd->isSetup()) {
      gd->setup(win_w, win_h);
    }
    gd->update();
  }
}

void Daemon::onResize(int winW, int winH) {
  for(std::map<uint8_t, GraphDrawer*>::iterator it = graphs.begin(); it != graphs.end(); ++it) {
    GraphDrawer* gd = it->second;
    gd->onResize(winW, winH);
  }
  win_w = winW;
  win_h = winH;
}

void Daemon::draw() {

  if(active_graph < 0) {
    return;
  }

  graphs[active_graph]->draw();
}
