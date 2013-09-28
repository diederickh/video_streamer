#ifndef ROXLU_AMF0_BOOLEAN_H
#define ROXLU_AMF0_BOOLEAN_H

#include <streamer/amf/types/AMFType.h>

class BitStream;

struct AMF0Boolean : public AMFType {
  AMF0Boolean(BitStream& bs);
  void print();
  void read();
  void write();
  bool value;
};

#endif
