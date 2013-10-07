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

*/

#ifndef ROXLU_AMF0_H
#define ROXLU_AMF0_H

#include <stdio.h>
#include <streamer/amf/types/AMFTypes.h>
#include <streamer/core/BitStream.h>

class AMF0 {
 public:
  AMF0(BitStream& bs);
  void flush(size_t nbytes);
  bool parse(std::vector<AMFType*>& result, size_t nbytes); /* parse nbytes of data and convert to amf0 elements */

  /* check the current element type */
  bool isString();
  bool isEcmaArray();  
  bool isStrictArray();
  bool isObject();

  /* create amf0 elements - your are the owner, so you need to free them */
  AMF0String* createString(std::string v);
  AMF0Number* createNumber(double v);
  AMF0Boolean* createBoolean(bool value);
  AMF0EcmaArray* createEcmaArray();

  /* read amf0 from the bitstream */
  AMF0String* readString();
  AMF0EcmaArray* readEcmaArray();
  AMF0Number* readNumber();
  AMF0Boolean* readBoolean();
  AMF0Object* readObject();

  /* writing the amf0 objects to the bitstream - these don't do much */
  void writeEcmaArray(AMF0EcmaArray* ar);
  void writeNumber(AMF0Number* n);
  void writeString(AMF0String* s);
  void writeBoolean(AMF0Boolean* b);

 private:
  BitStream& bs;
};

inline bool AMF0::isString() {
  return bs[0] == AMF0_TYPE_STRING;
}

inline bool AMF0::isEcmaArray() {
  return bs[0] == AMF0_TYPE_ECMA_ARRAY;
}

inline bool AMF0::isStrictArray() {
  return bs[0] == AMF0_TYPE_STRICT_ARRAY;
}

inline bool AMF0::isObject() {
  return bs[0] == AMF0_TYPE_OBJECT;
}

inline void AMF0::flush(size_t nbytes) {
  bs.flush(nbytes);
}

inline AMF0String* AMF0::createString(std::string v) {
  AMF0String* str = new AMF0String(bs);
  str->value = v;
  return str;
}

inline AMF0Number* AMF0::createNumber(double v) {
  AMF0Number* n = new AMF0Number(bs);
  n->value = v;
  return n;
}

inline AMF0EcmaArray* AMF0::createEcmaArray() {
  AMF0EcmaArray* ea = new AMF0EcmaArray(bs);
  return ea;
}

inline AMF0Boolean* AMF0::createBoolean(bool v) {
  AMF0Boolean* b = new AMF0Boolean(bs);
  b->value = v;
  return b;
}

inline void AMF0::writeEcmaArray(AMF0EcmaArray* ar) {
  ar->write();
}

inline void AMF0::writeNumber(AMF0Number* n) {
  n->write();
}

inline void AMF0::writeString(AMF0String* s) {
  s->write();
}

inline void AMF0::writeBoolean(AMF0Boolean* b) {
  b->write();
}
              
#endif
