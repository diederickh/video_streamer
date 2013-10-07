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

   # Config 
   
   Simple XML config file parser.

   _Pseudo code example_
   ````c++

   #if 0
   <?xml version="1.0" encoding="UTF-8"?>
   <config>
     <width>400</width>
     <height>500</height>
     <fps>60</fps>
   </config>
   #endif

   Config cfg;
   if(!cfg.load("myconfig.xml")) {
     ::exit(EXIT_FAILURE);
   }

   try { 
       xml_node<>* n = cfg.getNode("config");
       uint16_t width = cfg.readU16(n, "width");
       uint16_t height = cfg.readU16(n, "height");
       uint16_t fps = cfg.getU16(n, "fps");
   }  
   catch(ConfigException ex) {
      printf("error: %s\n", ex.what());
      ::exit(EXIT_FAILURE);
   }

   ````
   
   # Todo
   - handle cdata elements
   - implement attributes

 */
#ifndef ROXLU_XML_CONFIG_H
#define ROXLU_XML_CONFIG_H

#include <sstream>
#include <string>
#include <vector>
#include <inttypes.h>
#include <rapidxml.hpp>

using namespace rapidxml;

struct ConfigException : public std::exception {
 ConfigException(std::string m):m(m){}
  ~ConfigException() throw() {}
  const char* what() const throw() {  return m.c_str(); } 
  std::string m;
};

// ----------------------------------------------

class Config {
 public:
  Config();
  ~Config();

  bool load(std::string filepath);
  
  int8_t readS8(xml_node<>* parent, std::string path);
  int16_t readS16(xml_node<>* parent, std::string path);
  int32_t readS32(xml_node<>* parent, std::string path);
  int64_t readS64(xml_node<>* parent, std::string path);
  uint8_t readU8(xml_node<>* parent, std::string path);
  uint16_t readU16(xml_node<>* parent, std::string path);
  uint32_t readU32(xml_node<>* parent, std::string path);
  uint64_t readU64(xml_node<>* parent, std::string path);
  float readFloat(xml_node<>* parent, std::string path);
  double readDouble(xml_node<>* parent, std::string path);
  std::string readString(xml_node<>* parent, std::string path);

  template<class T>
    T read(xml_node<>* parent, std::string path) {
      std::stringstream ss;
      if(!getNodeValue(parent, path, ss)) {
        throw ConfigException("error: node not found: " +path);
      }
      T result;
      ss >> result;
      return result;
  }

  xml_node<>* firstNode(std::string n);
  bool parsePath(std::string& path, std::vector<std::string>& result);
  xml_node<>* getNode(std::string path); /* get node from the "root" element */
  xml_node<>* getNode(xml_node<>* parent, std::string path); /* get node from the given parent */
  bool getNodeValue(xml_node<>* parent, std::string path, std::stringstream& ss);
 public:
  xml_document<> doc;
};


inline uint8_t Config::readU8(xml_node<>* parent, std::string path) {
  return read<uint16_t>(parent, path); // when read as u8 it's converted to a char
}

inline uint16_t Config::readU16(xml_node<>* parent, std::string path) {
  return read<uint16_t>(parent, path);
}

inline uint32_t Config::readU32(xml_node<>* parent, std::string path) {
  return read<uint32_t>(parent, path);
}

inline uint64_t Config::readU64(xml_node<>* parent, std::string path) {
  return read<uint64_t>(parent, path);
}

inline int8_t Config::readS8(xml_node<>* parent, std::string path) {
  return read<int16_t>(parent, path); // when read as u8 it's converted to a char
}

inline int16_t Config::readS16(xml_node<>* parent, std::string path) {
  return read<int16_t>(parent, path);
}

inline int32_t Config::readS32(xml_node<>* parent, std::string path) {
  return read<int32_t>(parent, path);
}

inline int64_t Config::readS64(xml_node<>* parent, std::string path) {
  return read<int64_t>(parent, path);
}

inline float Config::readFloat(xml_node<>* parent, std::string path) {
  return read<float>(parent, path);
}

inline double Config::readDouble(xml_node<>* parent, std::string path) {
  return read<double>(parent, path);
}

inline std::string Config::readString(xml_node<>* parent, std::string path) {
  return read<std::string>(parent, path);
}

inline bool Config::getNodeValue(xml_node<>* parent, std::string path, std::stringstream& ss) {
  xml_node<>* child = getNode(parent, path);
  if(!child) {
    return false;
  }
  ss << child->value();
  return true;
}

// use a path string to get the child node of the parent
// e.g. video/width
inline xml_node<>* Config::getNode(xml_node<>* parent, std::string path) {

  if(!parent) {
    throw ConfigException("error: cannot getNode() because given parent is null.");
  }

  std::vector<std::string> els;
  if(!parsePath(path, els)) {
    throw ConfigException("error: cannot getNode() because we cannot find it.");
  }

  std::vector<std::string>::iterator it = els.begin();
  xml_node<>* child = parent;
  xml_node<>* prev_child = child;
  while(it != els.end()) {
    std::string el = *it;
    prev_child = child;
    child = child->first_node(el.c_str());
    if(!child) {
      throw ConfigException("error: cannot getNode() because we cannot find it: " +el);
    }
    ++it;
  }
  return child;
}

inline xml_node<>* Config::getNode(std::string path) {
  printf("--> name: %s\n", doc.name());
  return getNode(&doc, path);
}

// splits on '/'
inline bool Config::parsePath(std::string& path, std::vector<std::string>& result) {
  std::stringstream ss(path);
  std::string item;
  while(std::getline(ss, item, '/')) {
    result.push_back(item);
  }
  return result.size();
}

inline xml_node<>* Config::firstNode(std::string name) {
  return doc.first_node(name.c_str());
}

#endif
