#ifndef ROXLU_AMF0_ECMA_ARRAY_H
#define ROXLU_AMF0_ECMA_ARRAY_H

#include <streamer/amf/types/AMFType.h>
#include <streamer/amf/types/AMF0Property.h>
#include <streamer/amf/types/AMF0Object.h>
#include <string>
#include <vector>

struct AMF0String;
class BitStream;

// -------------------------------------------------

struct AMF0EcmaArray : public AMFType {
  AMF0EcmaArray(BitStream& bs);
  ~AMF0EcmaArray();

  void print();
  void read();
  void write();
  void add(std::string name, AMFType* v);
  void add(AMF0String* name, AMFType* v);

  AMF0Object obj;
};


#endif
