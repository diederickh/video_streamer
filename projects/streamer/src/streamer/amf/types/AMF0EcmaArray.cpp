#include <streamer/core/Endian.h>
#include <streamer/core/BitStream.h>
#include <streamer/amf/types/AMFTypes.h>
#include <streamer/amf/types/AMF0EcmaArray.h>
#include <sstream>

AMF0EcmaArray::AMF0EcmaArray(BitStream& bs) 
  :AMFType(AMF0_TYPE_ECMA_ARRAY, bs)
  ,obj(bs)
{
}

AMF0EcmaArray::~AMF0EcmaArray() {
  obj.removeElements();
}

void AMF0EcmaArray::add(std::string name, AMFType* v) {
  obj.add(name, v);
}

void AMF0EcmaArray::add(AMF0String* name, AMFType* v) {
  obj.add(name, v);
}

void AMF0EcmaArray::print() {
  obj.print();
}

void AMF0EcmaArray::read() {
  uint32_t num = FromBE32(bs.getU32());
  obj.readElements();
}

void AMF0EcmaArray::write() {
  bs.putU8(AMF0_TYPE_ECMA_ARRAY);

  uint32_t len = (uint32_t)obj.values.size();
  bs.putU32(ToBE32(len));
  
  obj.writeElements();
}
