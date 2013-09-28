#ifndef ROXLU_AMF0_STRING_H
#define ROXLU_AMF0_STRING_H

#include <streamer/amf/types/AMFType.h>

class BitStream;

struct AMF0String : public AMFType {
  AMF0String(BitStream& bs);
  AMF0String(BitStream& bs, std::string value);
  void print();
  void read();
  void write();
  std::string value;
};

#endif
