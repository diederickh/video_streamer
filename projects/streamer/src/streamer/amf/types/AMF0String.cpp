#include <streamer/core/Endian.h>
#include <streamer/core/BitStream.h>
#include <streamer/amf/types/AMF0String.h>

AMF0String::AMF0String(BitStream& bs)
  :AMFType(AMF0_TYPE_STRING, bs)
{
}

AMF0String::AMF0String(BitStream& bs, std::string value) 
  :AMFType(AMF0_TYPE_STRING, bs)
  ,value(value)
{

}

void AMF0String::print() {
  printf("amf0_string: %s\n", value.c_str());
}

void AMF0String::read() {
  uint16_t len = FromBE16(bs.getU16());
  value = bs.getString(len);
}

void AMF0String::write() {
  uint16_t len = value.size();
  bs.putU16(ToBE16(len));
  bs.putString(value);
}
