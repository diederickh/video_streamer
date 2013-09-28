#ifndef ROXLU_AMF0_NUMBER_H
#define ROXLU_AMF0_NUMBER_H

#include <streamer/amf/types/AMFType.h>

class BitStream;

struct AMF0Number : public AMFType {
  AMF0Number(BitStream& bs);
  void print();
  void read();
  void write();
  double value;
};

#endif
