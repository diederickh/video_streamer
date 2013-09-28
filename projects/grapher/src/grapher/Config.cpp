// @todo - Use the config project class
#include <iostream>
#include <fstream>
#include <rapidxml.hpp>
#include <grapher/Config.h>

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

  std::string str( (std::istreambuf_iterator<char>(ifs)) , std::istreambuf_iterator<char>());
  if(!str.size()) {
    printf("error: empty xml.\n");
    return false;
  }

  try {
    doc.parse<parse_full>((char*)str.c_str());
  }
  catch(...) {
    printf("error: error while parsin xml.\n");
    return false;
  }
  return true;
}
