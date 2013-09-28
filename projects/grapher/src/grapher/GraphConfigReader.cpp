#include <grapher/GraphConfigReader.h>
#include <iostream>

// loads server settings 
bool GraphConfigReader::loadServerConfig(std::string filepath, ServerSettings& result) {
  
  if(!filepath.size()) {
    printf("error: invalid filepath for server config: %s\n", filepath.c_str());
    return false;
  }

  Config c;
  if(!c.load(filepath)) {
    return false;
  }

  try {
    xml_node<>* n = c.firstNode("settings");
    if(!n) {
      printf("error: no settings found for server.\n");
      ::exit(EXIT_FAILURE);
    }

    xml_node<>* d = c.getNode(n, "server");
    result.width = c.readU16(d, "width");
    result.height = c.readU16(d, "height");
    result.address =  c.readString(d, "address");
  }
  catch(ConfigException ex) {
    printf("error: %s\n", ex.what());
    ::exit(EXIT_FAILURE);
  }
  
  return true;
}

// loads the graphs + server address.
bool GraphConfigReader::loadClientConfig(std::string filepath, ClientSettings& result) {
  
  if(!filepath.size()) {
    printf("error: invalid filepath for client config: %s\n", filepath.c_str());
    return false;
  }

  Config c;
  if(!c.load(filepath)) {
    return false;
  }

  try {
    xml_node<>* n = c.firstNode("settings");
    xml_node<>* graphs = c.getNode(n, "graphs");
    for(xml_node<>* graph = graphs->first_node(); graph; graph = graph->next_sibling()) {
      GraphConfig gc;
      gc.width = c.readU16(graph, "width");
      gc.height = c.readU16(graph, "height");
      gc.x = c.readU16(graph, "x");
      gc.y = c.readU16(graph, "y");
      gc.xtics = c.readU8(graph, "xtics");
      gc.ytics = c.readU8(graph, "ytics");
      gc.yrange[0] = c.readS32(graph, "min");
      gc.yrange[1] = c.readS32(graph, "max");
      gc.id  = c.readU8(graph, "id");
      result.graphs.push_back(gc);
    }

    xml_node<>* s = c.getNode(n, "server");
    result.address = c.readString(s, "address");

  }
 catch(ConfigException ex) {
    printf("error: %s\n", ex.what());
    ::exit(EXIT_FAILURE);
  }
  return true;
}
