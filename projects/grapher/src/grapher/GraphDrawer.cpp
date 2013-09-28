#include <grapher/GraphDrawer.h>
#include <sstream>

// ----------------------------------------------------------

GraphData::GraphData() {
  color[0] = 255.0;
  color[1] = 0.0;
  color[2] = 0.0;
}

void GraphData::createLine(GraphDrawer& graph) {

  if(!values.size()) {
    return;
  }

  if(values.size() > graph.width) {
    while(values.size() > graph.width) {
      values.erase(values.begin());
    }
  }

  Element el;
  el.offset = graph.vertices.size();
  el.type = GL_LINE_STRIP;

  int bottom = graph.y + graph.height + graph.yrange[0];
  for(size_t i = 0; i < values.size(); ++i) {
    graph.vertices.push_back(Vertex(graph.xborder + i, bottom - values[i], color[0], color[1], color[2]));
  }
  el.count = graph.vertices.size() - el.offset;
  graph.elements.push_back(el);
}

// ----------------------------------------------------------

GraphDrawer::GraphDrawer()
  :width(0)
  ,height(0)
  ,x(0)
  ,y(0)
  ,yscale(0)
  ,ytotal(0)
  ,ytics(10)
  ,xtics(5)
  ,xborder(0)
  ,is_setup(false)
{
}

GraphDrawer::~GraphDrawer() {
  width = 0;
  height = 0;
  x = 0;
  y = 0;
  is_setup = false;
  xtics = 0;
  ytics = 0;
  xborder = 0;
}

bool GraphDrawer::setup(int winW, int winH) {

  vertices.setup();

  if(!label_font.setup("arial13.fnt", winW, winH, 1024, true)) {
    return false;
  }

  is_setup = true;

  return true;
}

void GraphDrawer::onResize(int winW, int winH) {
  label_font.onResize(winW, winH);
}

void GraphDrawer::update() {
  label_font.reset();
  elements.clear();
  vertices.clear();

  createBackground();
  createLines();

  vertices.update();
  label_font.update();
}

void GraphDrawer::draw() {
  glPointSize(1.0);

  vertices.bind();

  for(size_t i = 0; i < elements.size(); ++i) {
    Element& el = elements[i];
    glDrawArrays(el.type, el.offset, el.count);
  }

  label_font.draw();
}

void GraphDrawer::createBackground() {

  // create rectangle
  xborder = x + 50;
  float linecol[3] = { 0.0f, 0.0f, 0.0f};
  float bgcol[3] = {255.0f, 255.0f, 255.0f};
  createRect(xborder, y, width, height, bgcol, linecol);

  // create ytics
  int bottom = y + height;
  int space = height / ytics;
  int yy = bottom;
  int ndot_space = 6;
  int xx = xborder + ndot_space * 2;
  int nhor_points = (width-ndot_space*3) / ndot_space;

  {
    // horizontal dots
    Element el;
    el.offset = vertices.size();
    el.type = GL_POINTS;
    for(int j = 1; j < ytics; ++j) {
      yy = bottom - (j * space);
      for(int i = 0; i < nhor_points; ++i) {
        vertices.push_back(Vertex(xx + i * ndot_space, yy, 0, 0, 0));
      }
    }
    el.count = vertices.size() - el.offset;
    elements.push_back(el);

    // tiny tick lines horizontal (left and right)
    Element el_lines;
    el_lines.offset = vertices.size();
    el_lines.type = GL_LINES;
    for(int j = 1; j < ytics; ++j) {
      yy = bottom - (j * space);
      vertices.push_back(Vertex(xx - ndot_space * 2, yy, 0, 0, 0));
      vertices.push_back(Vertex(xx - ndot_space, yy, 0, 0, 0));
      vertices.push_back(Vertex(xx + width - (ndot_space * 3), yy, 0, 0, 0));
      vertices.push_back(Vertex(xx + width - (ndot_space * 2), yy, 0, 0, 0));
    }
    el_lines.count = vertices.size() - el_lines.offset;
    elements.push_back(el_lines);

    // y-labels
    int textw, texth = 0;
    int step = (ytotal / ytics);
    label_font.setColor(0,0,0);
    for(int j = 0; j <= ytics; ++j) {
      yy = bottom - (j * space);
      std::stringstream ss;
      ss << yrange[0] + j * step;
      std::string label = ss.str();
      label_font.getStringSize(label, textw, texth);
      label_font.addText(label, xborder-textw-10, yy + texth * 0.5);
    }
  }
  
  // create xtics
  space = width / xtics;
  xx = xborder;
  int nvert_points = height / ndot_space;

  {
    // xdots
    Element el;
    el.type = GL_POINTS;
    el.offset = vertices.size();
    for(int i = 1; i < xtics; ++i) {
      xx = xborder + i * space;
      for(int j = 2; j < nvert_points-1; ++j) {
        vertices.push_back(Vertex(xx, bottom - j * ndot_space, 0, 0, 0));
      }
    }
    el.count = vertices.size() - el.offset;
    elements.push_back(el);

    // lines
    Element el_lines;
    el_lines.type = GL_LINES;
    el_lines.offset = vertices.size();
    for(int i = 1; i < xtics; ++i) {
      xx = xborder + i * space;
      vertices.push_back(Vertex(xx, bottom, 0, 0, 0));
      vertices.push_back(Vertex(xx, bottom - ndot_space, 0, 0, 0));

      vertices.push_back(Vertex(xx, bottom - height, 0, 0, 0));
      vertices.push_back(Vertex(xx, bottom - height +  ndot_space, 0, 0, 0));

    }
    el_lines.count = vertices.size() - el_lines.offset;
    elements.push_back(el_lines);

    // x-labels
    int textw, texth = 0;
    int step = (width / xtics);
    for(int i = 0; i <= xtics; ++i) {
      std::stringstream ss;
      ss << i * step;
      std::string label = ss.str();
      label_font.getStringSize(label, textw, texth);
      label_font.addText(label, xborder + i * space - (textw * 0.5), bottom + texth + 5);
    }
  }

  // graph labels
  {
    Element el;
    el.type = GL_LINES;
    el.offset = vertices.size();

    int xright = xborder + width;
    int textw, texth = 0;
    int graph_label_y = y ;
    int c = 0;
    int max_textw = 0;
    float label_bg[3] = {255, 255, 255};

    // get max textw
    for(std::map<std::string, GraphData>::iterator it = graphs.begin(); it != graphs.end(); ++it) {
      GraphData& gd = it->second;
      label_font.getStringSize(it->first, textw, texth);
      if(textw > max_textw) {
        max_textw = textw;
      }
    }
    // white background behind graph titles
    createRect(xright-5, graph_label_y + 2, -(max_textw+60), graphs.size() * 20, label_bg, label_bg);

    // draw the legenda
    for(std::map<std::string, GraphData>::iterator it = graphs.begin(); it != graphs.end(); ++it) {
      GraphData& gd = it->second;
      label_font.getStringSize(it->first, textw, texth);

      int textx = xright - (textw + 60);
      int texty = graph_label_y + 20 + ( (c+1) * 5);;
      label_font.addText(it->first, textx, texty);

      vertices.push_back(Vertex(xright - 10, texty - texth * 0.5, gd.color[0], gd.color[1], gd.color[2]));
      vertices.push_back(Vertex(xright - 55, texty - texth * 0.5, gd.color[0], gd.color[1], gd.color[2]));

      graph_label_y += (texth);
      c++;
    }

    el.count = vertices.size() - el.offset;
    elements.push_back(el);
  }

}


void GraphDrawer::createLines() {
  for(std::map<std::string, GraphData>::iterator it = graphs.begin(); it != graphs.end(); ++it) {
    GraphData& gd = it->second;
    gd.createLine(*this);
  }
}

void GraphDrawer::createRect(int xx, int yy, int w, int h, float bgcol[3], float linecol[3]) {
  // fill
  {
    Element el;
    el.type = GL_TRIANGLES;
    el.offset = vertices.size();

    vertices.push_back(Vertex(xx, yy+h, bgcol[0], bgcol[1], bgcol[2]));
    vertices.push_back(Vertex(xx+w, yy+h, bgcol[0], bgcol[1], bgcol[2]));
    vertices.push_back(Vertex(xx+w, yy, bgcol[0], bgcol[1], bgcol[2]));

    vertices.push_back(Vertex(xx, yy+h, bgcol[0], bgcol[1], bgcol[2]));
    vertices.push_back(Vertex(xx+w, yy, bgcol[0], bgcol[1], bgcol[2]));
    vertices.push_back(Vertex(xx, yy, bgcol[0], bgcol[1], bgcol[2]));

    el.count = vertices.size() - el.offset;
    elements.push_back(el);
  }

{
    Element el;
    el.type = GL_LINE_LOOP;
    el.offset = vertices.size();

    vertices.push_back(Vertex(xx, yy+h, linecol[0], linecol[1], linecol[2]));
    vertices.push_back(Vertex(xx+w, yy+h, linecol[0], linecol[1], linecol[2]));
    vertices.push_back(Vertex(xx+w, yy, linecol[0], linecol[1], linecol[2]));
    vertices.push_back(Vertex(xx, yy, linecol[0], linecol[1], linecol[2]));

    el.count = vertices.size() - el.offset;
    elements.push_back(el);
  }
  
}
