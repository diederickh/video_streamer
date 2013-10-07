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
#ifndef ROXLU_AMF0_ECMA_ARRAY_H
#define ROXLU_AMF0_ECMA_ARRAY_H

#include <streamer/amf/types/AMFType.h>
#include <streamer/amf/types/AMF0Property.h>
#include <streamer/amf/types/AMF0Object.h>
#include <string>
#include <vector>

struct AMF0String;
class BitStream;

// -------------------------------------------------

struct AMF0EcmaArray : public AMFType {
  AMF0EcmaArray(BitStream& bs);
  ~AMF0EcmaArray();

  void print();
  void read();
  void write();
  void add(std::string name, AMFType* v);
  void add(AMF0String* name, AMFType* v);

  AMF0Object obj;
};


#endif
