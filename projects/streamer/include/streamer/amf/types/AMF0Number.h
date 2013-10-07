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
#ifndef ROXLU_AMF0_NUMBER_H
#define ROXLU_AMF0_NUMBER_H

#include <streamer/amf/types/AMFType.h>

class BitStream;

struct AMF0Number : public AMFType {
  AMF0Number(BitStream& bs);
  void print();
  void read();
  void write();
  double value;
};

#endif
