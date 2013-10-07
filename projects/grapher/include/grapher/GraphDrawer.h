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

*/
#ifndef ROXLU_GRAPHER_GRAPHDRAWER_H
#define ROXLU_GRAPHER_GRAPHDRAWER_H

#include <vector>
#include <deque>
#include <map>
#include <string>
#include <grapher/Vertices.h>
#include <bmfont/BMFFont.h>

// ----------------------------------------------------------

/* visual element */
struct Element {
  int offset;
  int count;
  GLenum type;
};

// ----------------------------------------------------------
class GraphDrawer;

struct GraphData {
  GraphData();
  void add(float v);
  void createLine(GraphDrawer& graph);
  void setColor(float r, float g, float b);

  std::deque<float> values;
  float color[3];
};

// ----------------------------------------------------------

class GraphDrawer {
 public:
  GraphDrawer();
  ~GraphDrawer();
  void setSize(int w, int h);
  void setPosition(int x, int y);
  void setRange(int rmin, int rmax);
  bool setup(int winW, int winH);
  void onResize(int winW, int winH);
  void update();
  void draw();
  void add(const std::string name, float v);
  bool isSetup();
  GraphData& operator[](const std::string graph);

 private:
  void createBackground();
  void createLines();
  void createRect(int xx, int yy, int w, int h, float bgcol[3], float linecol[3]);
 public:
  bool is_setup;
  BMFFont<> label_font;
  std::map<std::string, GraphData> graphs;
  std::vector<Element> elements;
  Vertices vertices;
  int width;  /* width of graph, user defined */
  int height; /* height of graph, user defined */
  int x; /* x-position to start drawnig, user defined */
  int y; /* y-position to start drawing, user defined */
  int yrange[2]; /* rango of the values, user defined */
  int ytics; /* number of x-labels, user defined */
  int xtics; /* number of y-labels, user defined */
  float ytotal; 
  float yscale;
  int xborder;
};

// ----------------------------------------------------------

inline void GraphDrawer::setSize(int w, int h) {
  width = w;
  height = h;
}

inline void GraphDrawer::setPosition(int xx, int yy) {
  x = xx;
  y = yy;
}

inline void GraphDrawer::setRange(int rmin, int rmax) {
  if(!height) {
    printf("error: first setSize(), then setRange()!.\n");
    ::exit(EXIT_FAILURE);
  }
  yrange[0] = rmin;
  yrange[1] = rmax;
  ytotal = yrange[1] - yrange[0];
  yscale = height/ytotal;
}

inline GraphData& GraphDrawer::operator[](const std::string graph) {
  return graphs[graph];
}

inline void GraphDrawer::add(const std::string name, float v) {
  if(v > yrange[1]) {
    v = yrange[1];
  }
  else if(v < yrange[0]) {
    v = yrange[0];
  }
  v = ((v - yrange[0])/ytotal) * height;
  graphs[name].add(v);
}

inline bool GraphDrawer::isSetup() {
  return is_setup;
}

// ----------------------------------------------------------

inline void GraphData::add(float v) {
  values.push_back(v);
}

inline void GraphData::setColor(float r, float g, float b) {
  color[0] = r;
  color[1] = g;
  color[2] = b;
}


#endif
