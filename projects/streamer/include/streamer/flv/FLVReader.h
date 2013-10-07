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
#ifndef ROXLU_FLV_READER_H
#define ROXLU_FLV_READER_H

#include <string>
#include <streamer/flv/FLVTypes.h>
#include <streamer/flv/FLVListener.h>

#define FLV_READER_STATE_NONE 0
#define FLV_READER_STATE_TAG 1
#define FLV_READER_STATE_TAG_BODY 2
#define FLV_READER_STATE_PREV_TAG_SIZE 3

class BitStream;

class FLVReader {
 public:
  FLVReader(BitStream& bs);
  void parse();
  void addListener(FLVListener* l);
 private:
  bool parseHeader(); /* parses the first bytes; validates the FLV */
  bool parseFLVTag(FLVTag& result);
  bool parseFLVTagMeta(FLVTag& result);
  bool parseFLVTagAudio(FLVTag& result);
  bool parseFLVTagVideo(FLVTag& result);
  bool parseDecoderConfigurationRecord(BitStream& bitstream, AVCDecoderConfigurationRecord& rec);
 private:
  BitStream& bs;
  int state;
  uint32_t tag_size; /* the number of bytes in the current tag */
  FLVHeader header;
  FLVListeners listeners;
};

inline void FLVReader::addListener(FLVListener* l) {
  listeners.addListener(l);
}

#endif
