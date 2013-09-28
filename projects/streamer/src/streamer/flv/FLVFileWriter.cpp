#include <streamer/flv/FLVFileWriter.h>

FLVFileWriter::FLVFileWriter(std::string filepath) {

  ofs.open(filepath.c_str(), std::ios::out | std::ios::binary);
  if(!ofs.is_open()) {
    printf("error: cannot open the file for the FLFFileWriter: %s\n", filepath.c_str());
    ::exit(EXIT_FAILURE);
  }

}

FLVFileWriter::~FLVFileWriter() {

  if(ofs.is_open()) {
    ofs.close();
  }

}

void FLVFileWriter::onSignature(BitStream& bs) {
  ofs.write((char*)bs.getPtr(), bs.size());
}

void FLVFileWriter::onTag(BitStream& bs, FLVTag& tag) {
  ofs.write((char*)bs.getPtr(), bs.size());
  ofs.flush();
}
