/*
  # Graph

  This classes uses libcairo,libpixman,libplplot to generate a graph
  which is used to inspect the video encoder, audio encoder, network io etc..

 */
#ifndef ROXLU_GRAPH_H
#define ROXLU_GRAPH_H

extern "C" {
#  include <uv.h>
}

#include <map>
#include <string>
#include <vector>
#include <sstream>

#define GRAPH_MODE_NONE 0    /* do nothing with the values */
#define GRAPH_MODE_AVG 1     /* avarage the values */
#define GRAPH_MODE_ABS 2     /* use the absolute given values */
// --------------------------------------------------------------

struct GraphData {
  GraphData();

  std::string getStyle(); /* get the style command, use %d for the line number, e.g: `set style line %d lt 2 lw 3` */
  void setLineStyle(int lineType, int w, int pointType, std::string color); /* set all styles */
  void setLineType(int type); /* set the line type */
  void setLineWidth(int w); /* set the line width */
  void setLinePointType(int t); /* set the point type */
  void setLineColor(std::string col); /* set the line color */

  void setMode(uint8_t m); /* GRAPH_MODE_AVG, etc.. */
  void push(); /* pushes the avarage on top of the data vector */
  size_t size(); /* total number of elements */
  void operator+=(const uint64_t&); /* add something to the total */
  uint64_t& operator[](const unsigned int&); /* get an entry from the data */

  uint8_t mode; /* defaults to GRAPH_MODE_AVG */
  uint64_t total;
  uint64_t num_added;
  std::vector<uint64_t> data; /* keeps track of avarages, see "push()" */

  /* style */
  std::string color; /* line color, a named color, if not set we pick one */
  int type; /* line type */
  int width; /* line width */
  int point_type; /* th epoint type */
};

// --------------------------------------------------------------

class Graph {
 public:
  Graph();
  ~Graph();
  void start(float timescale); /* timescale: every X-millis we avarage the data which has been added. */
  void update();
  bool save(std::string filename); /* filename = name of file w/o extension */

  GraphData& operator[](std::string);
  std::map<std::string, GraphData> graphs;
  std::string title;
  std::string x_label;
  std::string y_label;
  std::string yrange; 
  std::string ytics; /* every `ytics` we create a label on the y axis */
  size_t max_values; /* maximum number of values to keep track of.. if more values are added we pop from the front */

  uint64_t timeout; /* when we 'timeout' and should add a new entry to the avarages, see update() */
  uint64_t timeout_delay; /* delay between time outs */
};

// --------------------------------------------------------------

inline GraphData& Graph::operator[](std::string name) {
  return graphs[name];
}

inline void GraphData::operator+=(const uint64_t& v) {
  total += v;
  num_added++;
}

inline void GraphData::push() {
  if(num_added == 0) {
    data.push_back(0);
  }
  else {
    if(mode == GRAPH_MODE_AVG) {
      data.push_back(total/num_added);
    }
    else if(mode == GRAPH_MODE_ABS) {
      data.push_back(total);
    }
  }
  total = 0;
  num_added = 0;
}

inline size_t GraphData::size() {
  return data.size();
}

inline uint64_t& GraphData::operator[](const unsigned int& dx) {
  return data[dx];
}

inline void GraphData::setMode(uint8_t m) {
  mode = m;
}

inline void GraphData::setLineType(int t) {
  type = t;
}

inline void GraphData::setLineWidth(int w) {
  width = w;
}

inline void GraphData::setLineColor(std::string c) {
  color = c;
}

inline void GraphData::setLinePointType(int pt) {
  point_type = pt;
}

inline std::string GraphData::getStyle() {
  std::stringstream ss;
  ss << "set style line %d "
     << "lc rgb \"" << color << "\" "
     << "pt " << point_type << " " 
     << "ps 1 " << " "
     << "lt " << type << " "  
     << "lw " << width
    ;
  std::string s = ss.str();
  return s;
}

inline void GraphData::setLineStyle(int lt, int w, int pt, std::string c) {
  setLineType(lt);
  setLineWidth(w);
  setLineColor(c);
  setLinePointType(pt);
}

// --------------------------------------------------------------

// --------------------------------------------------------------

extern Graph network_graph;
extern Graph frames_graph;

#endif
