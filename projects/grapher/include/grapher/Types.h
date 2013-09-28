#ifndef ROXLU_GRAPHER_TYPES_H
#define ROXLU_GRAPHER_TYPES_H

#define PKT_TYPE_NONE 0
#define PKT_TYPE_STOP 1 /* stop the thread */
#define PKT_TYPE_CONFIG 2 /* graph config */
#define PKT_TYPE_DATA 3 /* graph data */

#include <iostream>
#include <vector>

struct GraphConfig {

  GraphConfig()
    :width(0)
    ,height(0)
    ,xtics(0)
    ,ytics(0)
    ,id(0)
    ,x(0)
    ,y(0)
  {
     yrange[0] = yrange[1] = 0;
  }

  void print();

  uint16_t width;
  uint16_t height;
  uint16_t x;
  uint16_t y; 
  uint8_t xtics;
  uint8_t ytics;
  int32_t yrange[2];
  uint8_t id;          /* id of the graph; you can create multiple graphs, this identifies one such graph */
};


/* settings for the graph application which embeds the server */
struct ServerSettings {
  std::string address; /* tcp, inproc or ipc address, see nanomsg for valid addresses, but e.g.: tcp://127.0.0.1:7878  (only numerical) */
  int width; /* width of the window */
  int height; /* height of the window */
};


/* client config */
struct ClientSettings {
 std::string address;
 std::vector<GraphConfig> graphs;
};


// -----------------------------------------------
inline void GraphConfig::print() {
  printf("width: %d.\n", width);
  printf("height: %d.\n", height);
  printf("x: %d.\n", x);
  printf("y: %d.\n", y);
  printf("xtics: %d\n", xtics);
  printf("ytics: %d\n", ytics);
  printf("yrange: %d\n", yrange[0]);
  printf("yrange: %d\n", yrange[1]);
  printf("id: %d\n", id);
}
#endif
