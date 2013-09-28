#include <streamer/core/Endian.h>
#include <streamer/core/BitStream.h>
#include <streamer/amf/types/AMF0Number.h>

AMF0Number::AMF0Number(BitStream& bs)
  :AMFType(AMF0_TYPE_NUMBER, bs)
  ,value(0.0)
{
}

void AMF0Number::print() {
  printf("amf0_number: %f\n", value);
}

void AMF0Number::read() {
  uint64_t num = FromBE64(bs.getU64());
  memcpy((char*)&value, (char*)&num, 8);
}

void AMF0Number::write() {
  uint64_t v = 0;
  memcpy((char*)&v, (char*)&value, 8);
  bs.putU64(ToBE64(v));
}

