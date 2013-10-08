/* 

   # Config 
   
   Simple XML config file parser.

   IMPORTANT: rapidxml doest not take ownership of you xml buffer, 
   so when using it somewhere else make sure your xml buffer/string 
   exists as long as you're parsing it.!

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
       uint16_t width = cfg.getU16(n, "width");
       uint16_t height = cfg.getU16(n, "height");
       uint16_t fps = cfg.getU16(n, "fps");
   }  
   catch(ConfigException ex) {
      printf("error: %s\n", ex->what());
      ::exit(EXIT_FAILURE);
   }

   ````
   
   # Todo
   - handle cdata elements
   - implement attributes

 */

// @todo - move Config to a separate project
#ifndef ROXLU_XML_CONFIG_H
#define ROXLU_XML_CONFIG_H

#include <sstream>
#include <string>
#include <vector>
#include <stdint.h>
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

  int8_t readS8(xml_node<>* parent, std::string path, int8_t defaultValue);
  int16_t readS16(xml_node<>* parent, std::string path, int16_t defaultValue);
  int32_t readS32(xml_node<>* parent, std::string path, int32_t defaultValue);
  int64_t readS64(xml_node<>* parent, std::string path, int64_t defaultValue);
  uint8_t readU8(xml_node<>* parent, std::string path, uint8_t defaultValue);
  uint16_t readU16(xml_node<>* parent, std::string path, uint16_t defaultValue);
  uint32_t readU32(xml_node<>* parent, std::string path, uint32_t defaultValue);
  uint64_t readU64(xml_node<>* parent, std::string path, uint64_t defaultValue);
  std::string readString(xml_node<>* parent, std::string path, std::string defaultValue);

  template<class T>
    T read(xml_node<>* parent, std::string path, T defaultValue) {

    if(!doesNodeExists(parent, path)) {
      return defaultValue;
    }

    return read<T>(parent, path);
  }

  bool doesNodeExists(xml_node<>* parent, std::string path);
  bool parsePath(std::string& path, std::vector<std::string>& result);
  xml_node<>* getNode(std::string path); /* get node from the "root" element */
  xml_node<>* getNode(xml_node<>* parent, std::string path); /* get node from the given parent */
  bool getNodeValue(xml_node<>* parent, std::string path, std::stringstream& ss);
 private:
  xml_document<> doc;
  std::string xml_str;
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

inline int8_t Config::readS8(xml_node<>* parent, std::string path, int8_t defaultValue) {
  return read<int8_t>(parent, path, defaultValue);
}

inline int16_t Config::readS16(xml_node<>* parent, std::string path, int16_t defaultValue) {
  return read<int16_t>(parent, path, defaultValue);
}

inline int32_t Config::readS32(xml_node<>* parent, std::string path, int32_t defaultValue) {
  return read<int32_t>(parent, path, defaultValue);
}

inline int64_t Config::readS64(xml_node<>* parent, std::string path, int64_t defaultValue) {
  return read<int64_t>(parent, path, defaultValue);
}

inline uint8_t Config::readU8(xml_node<>* parent, std::string path, uint8_t defaultValue) {
  return read<uint8_t>(parent, path, defaultValue);
}

inline uint16_t Config::readU16(xml_node<>* parent, std::string path, uint16_t defaultValue) {
  return read<uint16_t>(parent, path, defaultValue);
}

inline uint32_t Config::readU32(xml_node<>* parent, std::string path, uint32_t defaultValue) {
  return read<uint32_t>(parent, path, defaultValue);
}

inline uint64_t Config::readU64(xml_node<>* parent, std::string path, uint64_t defaultValue) {
  return read<uint64_t>(parent, path, defaultValue);
}

inline std::string Config::readString(xml_node<>* parent, std::string path, std::string defaultValue) {
  return read<std::string>(parent, path, defaultValue);
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
    child = prev_child->first_node(el.c_str());
    if(!child) {
      throw ConfigException("error: cannot getNode() because we cannot find it: " +el);
    }
    ++it;
  }
  return child;
}

// @todo - we should merge this function with get node
inline bool Config::doesNodeExists(xml_node<>* parent, std::string path) {

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
    child = prev_child->first_node(el.c_str());
    if(!child) {
      return false;
    }
    ++it;
  }
  return true;
}


inline xml_node<>* Config::getNode(std::string path) {
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

#endif
