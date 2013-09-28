#include <streamer/core/BitStream.h>
#include <fstream>

BitStream::BitStream() 
  :bit_offset(0)
  ,byte_to_write(0)
{
}

BitStream::~BitStream() {
}

bool BitStream::loadFile(std::string filepath) {

  if(!filepath.size()) {
    printf("error: not valid file given.\n");
    return false;
  }

  std::ifstream ifs(filepath.c_str(), std::ios::binary | std::ios::in);
  if(!ifs.is_open()) {
    printf("error: cannot open: %s\n", filepath.c_str());
    return false;
  }

  buffer.assign(std::istreambuf_iterator<char>(ifs), std::istreambuf_iterator<char>());

  ifs.close();

  return true;
}

bool BitStream::saveFile(std::string filepath) {

  std::ofstream ofs(filepath.c_str(), std::ios::binary | std::ios::out);
  if(!ofs.is_open()) {
    printf("error: cannot open: %s\n", filepath.c_str());
    return false;
  }

  ofs.write((char*)&buffer.front(), buffer.size());
  
  ofs.close();

  return true;
}

void BitStream::print(size_t nbytes) {
  size_t col = 0;
  for(size_t i = 0; i < nbytes && i < size(); ++i) {
    printf("%02X ", buffer[i]);
    ++col;
    if(col == 40) {
      printf("\n");
      col = 0;
    }
  }
}
