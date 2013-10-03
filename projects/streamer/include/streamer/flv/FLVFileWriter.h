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
