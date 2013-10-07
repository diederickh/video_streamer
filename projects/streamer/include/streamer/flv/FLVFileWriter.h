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
#ifndef ROXLU_FLV_FILE_WRITER_H
#define ROXLU_FILE_FILE_WRITER_H

#include <streamer/flv/FLVListener.h>
#include <fstream>

class FLVFileWriter : public FLVListener {
 public:
  FLVFileWriter();
  ~FLVFileWriter();
  bool open(std::string filepath);
  bool close();
  void onSignature(BitStream& bs);
  void onTag(BitStream& bs, FLVTag& tag);
 private:
  std::ofstream ofs;
};

#endif
