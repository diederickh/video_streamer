#include <streamer/core/BitStream.h>
#include <streamer/amf/types/AMF0Boolean.h>

AMF0Boolean::AMF0Boolean(BitStream& bs)
  :AMFType(AMF0_TYPE_BOOLEAN, bs)
  ,value(false)
{
}

void AMF0Boolean::print() {
  printf("amf0_boolean: %c\n", (value) ? 'y' : 'n');
}

void AMF0Boolean::read() {
  value = bs.getU8();
}

void AMF0Boolean::write() {
  bs.putU8(value ? 1 : 0);
}
