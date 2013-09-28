#include <streamer/core/BitStream.h>
#include <streamer/amf/types/AMFType.h>

AMFType::AMFType(uint8_t type, BitStream& bs)
  :type(type)
  ,bs(bs)
{
}

AMFType::~AMFType() {
}

