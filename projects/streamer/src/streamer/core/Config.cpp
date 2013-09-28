#include <iostream>
#include <fstream>
#include <rapidxml.hpp>
#include <rapidxml_utils.hpp>
#include <streamer/core/Config.h>

Config::Config() {
}

Config::~Config() {
}

bool Config::load(std::string filepath) {

  std::ifstream ifs(filepath.c_str(), std::ios::in);
  if(!ifs.is_open()) {
    printf("error: cannot open the xml from: `%s`\n", filepath.c_str());
    return false;
  }

  xml_str.assign ( (std::istreambuf_iterator<char>(ifs)) , std::istreambuf_iterator<char>());
  if(!xml_str.size()) {
    printf("error: empty xml.\n");
    return false;
  }

  try {
    doc.parse<0>((char*)xml_str.c_str());
  }
  catch(...) {
    printf("error: error while parsing xml.\n");
    return false;
  }
  return true;
}
