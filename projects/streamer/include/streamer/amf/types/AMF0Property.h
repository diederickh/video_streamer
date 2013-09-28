#ifndef ROXLU_AMF0_PROPERTY_H
#define ROXLU_AMF0_PROPERTY_H

#include <streamer/amf/types/AMFType.h>
#include <string>

struct AMF0String;

struct AMF0Property {
  AMF0Property();
  ~AMF0Property();
  AMFType* value;
  AMF0String* name;
};

#endif
