#ifndef ROXLU_AMF0_OBJECT_H
#define ROXLU_AMF0_OBJECT_H

#include <streamer/amf/types/AMFType.h>
#include <streamer/amf/types/AMF0Property.h>
#include <string>
#include <vector>

struct AMF0String;
class BitStream;

struct AMF0Object : public AMFType {
  AMF0Object(BitStream& bs);
  ~AMF0Object();

  void print();
  void read();
  void readElements(); 
  void write();
  void writeElements();
  void removeElements(); /* deletes all the values */
  
  void add(std::string name, AMFType* v);
  void add(AMF0String* name, AMFType* v);
  std::vector<AMF0Property*> values;
};

#endif
