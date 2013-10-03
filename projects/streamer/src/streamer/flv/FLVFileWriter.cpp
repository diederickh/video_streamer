#include <streamer/flv/FLVFileWriter.h>

FLVFileWriter::FLVFileWriter() {
}

FLVFileWriter::~FLVFileWriter() {

  if(ofs.is_open()) {
    ofs.close();
  }

}

bool FLVFileWriter::open(std::string filepath) {

  if(ofs.is_open()) {
    printf("error: flvfilewriter already opened the output.\n");
    return false;
  }

  ofs.open(filepath.c_str(), std::ios::out | std::ios::binary);
  if(!ofs.is_open()) {
    printf("error: cannot open the file for the FLFFileWriter: %s\n", filepath.c_str());
    ::exit(EXIT_FAILURE);
  }

  return true;
}

bool FLVFileWriter::close() {
  
  if(!ofs.is_open()) {
    printf("error: cannot close the flvfilewriter because we're not open.\n");
    return false;
  }

  ofs.close();

  return true;
}

void FLVFileWriter::onSignature(BitStream& bs) {

#if !defined(NDEBUG)
  if(!ofs.is_open()) {
    printf("error: cannot write the signature to the flvfilewriter because you didn't opened the file.\n");
    return;
  }
#endif

  ofs.write((char*)bs.getPtr(), bs.size());
}

void FLVFileWriter::onTag(BitStream& bs, FLVTag& tag) {

#if !defined(NDEBUG)
  if(!ofs.is_open()) {
    printf("error: cannot write the flvtag to the flvfilewriter because you didn't opened the file.\n");
    return;
  }
#endif

  ofs.write((char*)bs.getPtr(), bs.size());
  ofs.flush();
}
