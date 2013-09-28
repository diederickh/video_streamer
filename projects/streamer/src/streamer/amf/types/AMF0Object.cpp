#include <streamer/core/Endian.h>
#include <streamer/core/BitStream.h>
#include <streamer/amf/types/AMFTypes.h>
#include <streamer/amf/types/AMF0Object.h>
#include <sstream>

AMF0Object::AMF0Object(BitStream& bs)
  :AMFType(AMF0_TYPE_OBJECT, bs)
{
}

AMF0Object::~AMF0Object() {
  removeElements();
}

void AMF0Object::removeElements() {
  for(std::vector<AMF0Property*>::iterator it = values.begin(); it != values.end(); ++it) {
    AMF0Property* p = *it;
    delete p;
    p = NULL;
  }
  values.clear();
}

void AMF0Object::print() {
  std::stringstream ss;

  for(std::vector<AMF0Property*>::iterator it = values.begin(); it != values.end(); ++it) {
    AMF0Property* p = *it;
    AMFType* el = p->value;
    ss << p->name->value << " = ";
    switch(el->type) {
      case AMF0_TYPE_STRING: {
        AMF0String* str = static_cast<AMF0String*>(el);
        ss << str->value << "\n";
        break;
      }
      case AMF0_TYPE_NUMBER: {
        AMF0Number* num = static_cast<AMF0Number*>(el);
        ss << num->value << "\n";
        break;
      }
      case AMF0_TYPE_BOOLEAN: {
        AMF0Boolean* b = static_cast<AMF0Boolean*>(el);
        ss << ((b->value) ? 'Y' : 'N') << "\n";
        break;
      }
      default: break;
    }
  }

  std::string str = ss.str();
  printf("%s\n", str.c_str());
}

void AMF0Object::add(std::string name, AMFType* v) {
  AMF0String* s = new AMF0String(bs);
  s->value = name;
  add(s, v);
}

void AMF0Object::add(AMF0String* name, AMFType* v) {
  AMF0Property* p = new AMF0Property();
  p->name = name;
  p->value = v;
  values.push_back(p);
}

void AMF0Object::read() {
  /*
  uint8_t marker = bs.getU8();
  if(marker != AMF0_TYPE_OBJECT) {
    printf("error: the current marker is not a valid object, marker = %02X\n", marker);
    return;
  }
  */
  readElements();
}

void AMF0Object::readElements() {
  while(true) {

    AMF0String* name = new AMF0String(bs); 
    name->read();

    uint8_t type = bs.getU8();
    switch(type) {

      case AMF0_TYPE_NUMBER: {
        AMF0Number* el = new AMF0Number(bs);
        el->read();
        add(name, el);
        break;
      }
      case AMF0_TYPE_BOOLEAN: {
        AMF0Boolean* el =  new AMF0Boolean(bs);
        el->read();
        add(name, el);
        break;
      }
      case AMF0_TYPE_STRING: {
        AMF0String* el = new AMF0String(bs);
        el->read();
        add(name, el);
        break;
      }
      case AMF0_TYPE_OBJECT_END: {
        return;
      }
      default: {
        printf("unhandled ecma array element: %02X\n", type);
        return;
      }
    }
  }
}

void AMF0Object::write() {
  bs.putU8(AMF0_TYPE_OBJECT);
  writeElements();
}

void AMF0Object::writeElements() {

  for(std::vector<AMF0Property*>::iterator it = values.begin(); it != values.end(); ++it) {
    AMF0Property* p = *it;
    p->name->write();
    bs.putU8(p->value->type);
    p->value->write();
  }

  AMF0String str(bs);
  str.write();

  bs.putU8(AMF0_TYPE_OBJECT_END);
}
