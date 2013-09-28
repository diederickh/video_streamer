#include <streamer/core/Endian.h>
#include <streamer/core/BitStream.h>
#include <streamer/amf/AMF0.h>

AMF0::AMF0(BitStream& bs) 
  :bs(bs)
{
}

// returns an array with all found amf elements, the reader must free all!
bool AMF0::parse(std::vector<AMFType*>& result, size_t nbytes) {

  size_t nstart = bs.size();
  while(bs.size()) {
    switch(bs[0]) {
      case AMF0_TYPE_ECMA_ARRAY: {
        bs.flush(1);
        result.push_back(readEcmaArray());
        break;
      }
      case AMF0_TYPE_OBJECT: {
        bs.flush(1);
        result.push_back(readObject());
        break;
      }
      case AMF0_TYPE_STRING: {
        bs.flush(1);
        result.push_back(readString());
        break;
      }
      case AMF0_TYPE_BOOLEAN: {
        bs.flush(1);
        result.push_back(readBoolean());
        break;
      }
      case AMF0_TYPE_NUMBER: {
        bs.flush(1);
        result.push_back(readNumber());
        break;
      }
      default: {
        printf("warning: unhandled amf type: %02X\n", bs[0]);
        return true;
      }
    }

    // make sure we don't read more then nbytes
    size_t nend = bs.size();
    size_t ndelta = nstart - nend;
    if(ndelta >= nbytes) {
      return true;
    }
  }
  return result.size();
}

AMF0String* AMF0::readString() {
  AMF0String* v = new AMF0String(bs);
  v->read();
  return v;
}

AMF0Number* AMF0::readNumber() {
  AMF0Number* n = new AMF0Number(bs);
  n->read();
  return n;
}

AMF0Boolean* AMF0::readBoolean() {
  AMF0Boolean* v = new AMF0Boolean(bs);
  v->read();
  return v;
}

AMF0EcmaArray* AMF0::readEcmaArray() {
  AMF0EcmaArray* a = new AMF0EcmaArray(bs);
  a->read();
  return a;
}

AMF0Object* AMF0::readObject() {
  AMF0Object* o = new AMF0Object(bs);
  o->read();
  return o;
}
