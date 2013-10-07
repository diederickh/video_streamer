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
#ifndef ROXLU_AMF_TYPE_H
#define ROXLU_AMF_TYPE_H

#include <stdint.h>

#define AMF0_TYPE_NUMBER 0x00
#define AMF0_TYPE_BOOLEAN 0x01
#define AMF0_TYPE_STRING 0x02
#define AMF0_TYPE_OBJECT 0x03
#define AMF0_TYPE_MOVIECLIP 0x04 
#define AMF0_TYPE_NULL 0x05
#define AMF0_TYPE_UNDEFINED 0x06
#define AMF0_TYPE_REFERENCE 0x07
#define AMF0_TYPE_ECMA_ARRAY 0x08
#define AMF0_TYPE_OBJECT_END 0x09
#define AMF0_TYPE_STRICT_ARRAY 0x0A
#define AMF0_TYPE_DATE 0x0B
#define AMF0_TYPE_LONG_STRING 0x0C
#define AMF0_TYPE_UNSUPPORTED 0x0D
#define AMF0_TYPE_RECORDSET 0x0E
#define AMF0_TYPE_XML 0x0F
#define AMF0_TYPE_TYPED_OBJECT 0x10 

class BitStream;

struct AMFType {
  AMFType(uint8_t type, BitStream& bs);

  virtual ~AMFType();
  virtual void read() = 0;
  virtual void write() = 0;
  virtual void print() = 0;

  uint8_t type;
  BitStream& bs;
};

#endif
