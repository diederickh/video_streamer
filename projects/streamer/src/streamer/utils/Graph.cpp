#include <sstream>
#include <fstream>
#include <cmath>
#include <iostream>
#include <streamer/utils/Graph.h>

Graph network_graph;
Graph frames_graph;

// --------------------------------------------------------------

GraphData::GraphData()
  :total(0)
  ,num_added(0)
  ,mode(GRAPH_MODE_AVG)
  ,width(1)
  ,type(2)
  ,color("red")
  ,point_type(2)
{
}

// --------------------------------------------------------------

Graph::Graph()
  :title("")
  ,x_label("x")
  ,y_label("y")
  ,ytics("10")
  ,max_values(500)
{
}

Graph::~Graph() {
}

void Graph::start(float timescale) {
  timeout_delay = 1000 * 1000 * timescale;
  timeout = uv_hrtime() + timeout_delay;
}

void Graph::update() {
  uint64_t n = uv_hrtime();
  if(n >= timeout) {
    timeout = uv_hrtime() + timeout_delay;
    std::map<std::string, GraphData>::iterator it = graphs.begin();
    while(it != graphs.end()) {
      GraphData& gd = it->second;
      gd.push();
      if(gd.data.size() > max_values) {
        gd.data.erase(gd.data.begin());
      }
      ++it;
    }
  }
}

bool Graph::save(std::string filename) {
  if(!filename.size()) {
    printf("error: no filename given to export graph.\n");
    return false;
  }
  
  if(!graphs.size()) {
    printf("error: no graphs to save.\n");
    return false;
  }
  
  // create data file.
  std::map<std::string, GraphData>::iterator it = graphs.begin();
  size_t nels = it->second.size();
  std::stringstream ss;
  for(size_t i = 0; i < nels; ++i) {
    ss << i;
    for(it = graphs.begin(); it != graphs.end(); ++it) {
      GraphData& gd = it->second;
      ss << "\t" << gd[i] << "\t";
    }
    ss << "\n";
  }

  std::string data_file = filename +".dat";
  std::ofstream ofs(data_file.c_str(), std::ios::out);
  if(!ofs.is_open()) {
    printf("error: cannot open output file.\n");
    return false;
  }

  ofs << ss.rdbuf();
  ofs.close();

  // create plot file.
  std::string svg_file = filename +".svg";
  std::string gp_file = filename +".gp";
  std::stringstream pss;
  
  pss << "reset" << std::endl
      << "set title \"" << title << "\"" << std::endl
      << "set xlabel \"" << x_label << "\"" << std::endl 
      << "set ylabel \"" << y_label << "\"" << std::endl
      << "set ytics " << ytics << std::endl
      << "set grid" << std::endl
      << "set terminal svg size 1280,600 fname 'Helvetica' fsize 10" << std::endl
      << "set output '" << svg_file << "'" << std::endl
      << "set yrange [" << yrange << "]" << std::endl
      << "set style increment user " << std::endl
    ;
    //      << "unset colorbox" << std::endl;
  

  size_t n = graphs.size();
  size_t dx = 2;
  size_t c = 1;

  // line styles
  char style_cmd[1024];
  for(it = graphs.begin(); it != graphs.end(); ++it) {
    GraphData& gd = it->second;
    std::string style = gd.getStyle();
    sprintf(style_cmd, style.c_str(), c);
    pss << style_cmd << std::endl;
    ++c;
  }


  // plot
  pss << "plot ";

  dx = 2;
  c = 0;
  for(it = graphs.begin(); it != graphs.end(); ++it) {

    GraphData& gd = it->second;

    pss << "\t \"" << data_file << "\" using " << "1:" << dx << " "
        << "title \"" << it->first << "\" "
        << "w lines " 
        << "ls " << (c+1) <<  " ";

    dx++, c++;
    if(c < n) {
      pss << ", \\" << std::endl;
    }
  }
  
  std::ofstream pofs(gp_file.c_str(), std::ios::out);
  if(!pofs.is_open()) {
    printf("error: cannot open gnuplot output file.\n");
    return false;
  }
  pofs << pss.rdbuf();
  pofs.close();

  // and execute
  std::string cmd = "./gnuplot " +gp_file;
  system(cmd.c_str());
  return true;
}
