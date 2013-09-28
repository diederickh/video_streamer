#include <streamer/amf/types/AMF0Property.h>
#include <streamer/amf/types/AMF0String.h>

AMF0Property::AMF0Property()
  :value(NULL)
  ,name(NULL)
{
}

AMF0Property::~AMF0Property() {
  if(value) {
    delete value;
    value = NULL;
  }
  if(name) {
    delete name;
    name = NULL;
  }
}

