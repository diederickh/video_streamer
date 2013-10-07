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

#ifndef ROXLU_AMF0_PROPERTY_H
#define ROXLU_AMF0_PROPERTY_H

#include <streamer/amf/types/AMFType.h>
#include <string>

struct AMF0String;

struct AMF0Property {
  AMF0Property();
  ~AMF0Property();
  AMFType* value;
  AMF0String* name;
};

#endif
