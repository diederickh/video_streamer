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

 # BitStream
 
 The bitstream class is used to encode/decode the FLV data and can be used for
 any other binary encoding. 

*/

#ifndef ROXLU_BITSTREAM_H
#define ROXLU_BITSTREAM_H

#include <stdint.h>
#include <assert.h>
#include <vector>
#include <string>
#include <iterator>
#include <algorithm>

class BitStream {
 public:
  BitStream();
  ~BitStream();

  bool loadFile(std::string filepath);
  bool saveFile(std::string filepath);

  typedef std::vector<uint8_t>::iterator iterator;
  std::vector<uint8_t>::iterator begin();
  std::vector<uint8_t>::iterator end();

  size_t size();
  void push_back(uint8_t v);
  void erase(BitStream::iterator first, BitStream::iterator last);
  void insert(BitStream::iterator position, BitStream::iterator first, BitStream::iterator last);
  void copy(std::vector<uint8_t>& vec);
  void clear();

  void flush(size_t nbytes); /* flush a couple of bytes; same as erase but a bit shorter */
  uint8_t* getPtr(); /* returns a pointer to the first byte in the buffer */

  void putU8(uint8_t v);
  void putU16(uint16_t v);
  void putU24BigEndian(uint32_t v); /* writes the first 3 bytes, lsb first, in big endian format */
  void putU32(uint32_t v);
  void putU64(uint64_t v);
  void putS8(int8_t v);
  void putS16(int16_t v);
  void putS32(int32_t v);
  void putS64(int64_t v);
  void putBit(uint8_t v);  /* write 0 or 1 */
  void putBits(uint8_t v, uint8_t nbits); /* write nbits from v */
  void putBytes(uint8_t* b, size_t nbytes); /* write multiple bytes */
  void putString(std::string str);

  void rewriteU8(size_t dx, uint8_t v); 
  void rewriteU16(size_t dx, uint16_t v);
  void rewriteU32(size_t dx, uint32_t v);
  void rewriteU64(size_t dx, uint64_t v);
  void rewriteU24BigEndian(size_t dx, uint32_t v); /* rewrite U24 at the given dx */
  void rewriteS8(size_t dx, int8_t v); 
  void rewriteS16(size_t dx, int16_t v);
  void rewriteS32(size_t dx, int32_t v);
  void rewriteS64(size_t dx, int64_t v);

  uint8_t getU8();
  uint16_t getU16();
  uint32_t getU24(); /* amf */
  uint32_t getU32();
  uint64_t getU64();
  int8_t getS8();
  int16_t getS16();
  int32_t getS32();
  int64_t getS64();

  void getBytes(size_t nbytes, std::vector<uint8_t>& result);
  uint8_t getBit();
  uint8_t getBits(uint8_t nbits);
  void flushBits(uint8_t nbits);

  std::string getString(size_t nbytes);
  void print(size_t nbytes);

  uint8_t& operator[](unsigned int dx);

  BitStream& operator<<(uint8_t& v);
  BitStream& operator<<(uint16_t& v);
  BitStream& operator<<(uint32_t& v);
  BitStream& operator<<(uint64_t& v);
  BitStream& operator<<(int8_t& v);
  BitStream& operator<<(int16_t& v);
  BitStream& operator<<(int32_t& v);
  BitStream& operator<<(int64_t& v);

  BitStream& operator>>(uint8_t& v);
  BitStream& operator>>(uint16_t& v);
  BitStream& operator>>(uint32_t& v);
  BitStream& operator>>(uint64_t& v);
  BitStream& operator>>(int8_t& v);
  BitStream& operator>>(int16_t& v);
  BitStream& operator>>(int32_t& v);
  BitStream& operator>>(int64_t& v);
  
 private:
  std::vector<uint8_t> buffer;
  uint8_t bit_offset;
  uint8_t byte_to_write; /* the byte we write when putBit()/putBits() has written a complete byte */
};

inline size_t BitStream::size() {
  return buffer.size();
}

inline void BitStream::push_back(uint8_t v) {
  buffer.push_back(v);
}

inline void BitStream::erase(BitStream::iterator first, BitStream::iterator last) {
  buffer.erase(first, last);
}

inline void BitStream::insert(BitStream::iterator position, BitStream::iterator first, BitStream::iterator last) {
  buffer.insert(position, first, last);
}

inline BitStream::iterator BitStream::begin() {
  return buffer.begin();
}

inline BitStream::iterator BitStream::end() {
  return buffer.end();
}

inline void BitStream::clear() {
  buffer.clear();
}

inline void BitStream::flush(size_t nbytes) {
  erase(begin(), begin() + nbytes);
}

inline uint8_t* BitStream::getPtr() {
  return &buffer.front();
}

// write one bit into a temporary byte; we flush when all bits have been written
// we start writing with msb
inline void BitStream::putBit(uint8_t v) {
  v = (v) ? 1 : 0;
  byte_to_write |= (v << (7 - bit_offset));
  ++bit_offset;

  if(bit_offset > 7) {
    putU8(byte_to_write);
    bit_offset = 0;
    byte_to_write = 0x00;
  }
}

// we write the nbits from msb to lsb but start
// at 8-nbits .. :) so if we have this byte:
// 0 0 0 1 0 0 1 0 
// we write
// 1 0 0 1 0 
inline void BitStream::putBits(uint8_t v, uint8_t nbits) {
  v = v << (8 - nbits);
  uint8_t mask = 0;
  for(uint8_t i = 0; i < nbits; ++i) {
    mask = 1 << (7 - i);
    putBit( ((v & mask) == mask) );
  }
}


inline void BitStream::copy(std::vector<uint8_t>& vec) {
  std::copy(vec.begin(), vec.end(), std::back_inserter(buffer));
}

inline void BitStream::putBytes(uint8_t* bytes, size_t nbytes) {
  std::copy(bytes, bytes+nbytes, std::back_inserter(buffer));
}

inline void BitStream::putString(std::string str) {
  putBytes((uint8_t*)str.c_str(), str.size());
}

inline void BitStream::putU8(uint8_t v) {
  push_back(v);
}

inline void BitStream::putU16(uint16_t v) {
  uint8_t* ptr = (uint8_t*)&v;
  push_back(ptr[0]);
  push_back(ptr[1]);
}

/* writes the first 3 bytes, lsb first, in big endian format */
inline void BitStream::putU24BigEndian(uint32_t v) {
  uint8_t* p = (uint8_t*)&v;
  putU8(p[2]);
  putU8(p[1]);
  putU8(p[0]);
}

inline void BitStream::putU32(uint32_t v) {
  uint8_t* ptr = (uint8_t*)&v;
  push_back(ptr[0]);
  push_back(ptr[1]);
  push_back(ptr[2]);
  push_back(ptr[3]);
}

inline void BitStream::putU64(uint64_t v) {
  uint8_t* ptr = (uint8_t*)&v;
  push_back(ptr[0]);
  push_back(ptr[1]);
  push_back(ptr[2]);
  push_back(ptr[3]);
  push_back(ptr[4]);
  push_back(ptr[5]);
  push_back(ptr[6]);
  push_back(ptr[7]);
}

inline void BitStream::putS8(int8_t v) {
  push_back((uint8_t)v);
}

inline void BitStream::putS16(int16_t v) {
  uint8_t* ptr = (uint8_t*)&v;
  push_back(ptr[0]);
  push_back(ptr[1]);
}

inline void BitStream::putS32(int32_t v) {
  uint8_t* ptr = (uint8_t*)&v;
  push_back(ptr[0]);
  push_back(ptr[1]);
  push_back(ptr[2]);
  push_back(ptr[3]);
}

inline void BitStream::putS64(int64_t v) {
  uint8_t* ptr = (uint8_t*)&v;
  push_back(ptr[0]);
  push_back(ptr[1]);
  push_back(ptr[2]);
  push_back(ptr[3]);
  push_back(ptr[4]);
  push_back(ptr[5]);
  push_back(ptr[6]);
  push_back(ptr[7]);
}

inline void BitStream::rewriteU24BigEndian(size_t dx, uint32_t v) {
  uint8_t* p = (uint8_t*)&v;
  buffer[dx + 0] = p[2];
  buffer[dx + 1] = p[1];
  buffer[dx + 2] = p[0];
}

inline uint8_t BitStream::getU8() {
  assert(buffer.size() > 0);
  uint8_t r = buffer[0];
  buffer.erase(buffer.begin(), buffer.begin() + 1);
  return r;
}

inline uint16_t BitStream::getU16() {
  assert(buffer.size() > 1);
  uint16_t r = 0;
  memcpy((char*)&r, (char*)&buffer.front(), 2);
  buffer.erase(buffer.begin(), buffer.begin() + 2);
  return r;
}

inline uint32_t BitStream::getU24() {
  assert(buffer.size() > 2);
  uint32_t r = 0;
  uint8_t* src = &buffer.front();
  uint8_t* dest = (uint8_t*)&r;
  dest[0] = src[0];
  dest[1] = src[1];
  dest[2] = src[2];
  dest[3] = 0;
  buffer.erase(buffer.begin(), buffer.begin() + 3);
  return r;
}

inline uint32_t BitStream::getU32() {
  assert(buffer.size() > 3);
  uint32_t r = 0;
  memcpy((char*)&r, (char*)&buffer.front(), 4);
  buffer.erase(buffer.begin(), buffer.begin() + 4);
  return r;
}

inline uint64_t BitStream::getU64() {
  assert(buffer.size() > 7);
  uint64_t r = 0;
  memcpy((char*)&r, (char*)&buffer.front(), 8);
  buffer.erase(buffer.begin(), buffer.begin() + 8);
  return r;
}

inline int8_t BitStream::getS8() {
  assert(buffer.size() > 0);
  int8_t r = buffer[0];
  buffer.erase(buffer.begin(), buffer.begin() + 1);
  return r;
}

inline int16_t BitStream::getS16() {
  assert(buffer.size() > 1);
  int16_t r = 0;
  memcpy((char*)&r, (char*)&buffer.front(), 2);
  buffer.erase(buffer.begin(), buffer.begin() + 2);
  return r;
}

inline int32_t BitStream::getS32() {
  assert(buffer.size() > 3);
  int32_t r = 0;
  memcpy((char*)&r, (char*)&buffer.front(), 4);
  buffer.erase(buffer.begin(), buffer.begin() + 4);
  return r;
}

inline int64_t BitStream::getS64() {
  assert(buffer.size() > 7);
  int64_t r = 0;
  memcpy((char*)&r, (char*)&buffer.front(), 8);
  buffer.erase(buffer.begin(), buffer.begin() + 8);
  return r;
}

// offset 0 == most significant bit
inline uint8_t BitStream::getBit() {
  assert(buffer.size() > 0);
  uint8_t mask = 1 << (7 - bit_offset);
  uint8_t curr_byte = buffer[0];
  ++bit_offset;

  if(bit_offset > 7) {
    bit_offset = 0;
    flush(1);
  }

  return (curr_byte & mask) == mask;
}

inline std::string BitStream::getString(size_t nbytes) {
  assert(size() >= nbytes);
  std::string result((char*)&buffer[0], nbytes);
  flush(nbytes);
  return result;
}

// read up to nbytes
inline void BitStream::getBytes(size_t nbytes, std::vector<uint8_t>& result) {
  assert(size() >= nbytes);
  std::copy(begin(), begin() + nbytes, std::back_inserter(result));
  flush(nbytes);
}

// reads nbits starting from the current bit offset and writing to the normal position (starting a lsb)
inline uint8_t BitStream::getBits(uint8_t nbits) {

  if(nbits > 8) {
    printf("error: we can only retrieve up to 8 bits\n");
    return 0;
  }
  
  uint8_t result = 0;
  for(uint8_t  i = 0; i < nbits; ++i) {
    uint8_t bitval = getBit();
    if(bitval) {
      result |= (1 << (nbits-i-1));
    }
  }

  return result;
}

inline void BitStream::flushBits(uint8_t nbits) {
  size_t nbytes = nbits / 8;
  size_t rest_bits = nbits % 8;
  if(nbytes) {
    flush(nbytes);
  }
  
  size_t end_bit = bit_offset + rest_bits;
  if(end_bit > 8) {
    flush(1);
    bit_offset = end_bit - 8;
  }
  else {
    bit_offset = end_bit;
  }
}

inline void BitStream::rewriteU8(size_t dx, uint8_t v) {
  assert(size() <= dx + 1);
  buffer[dx] = v;
}

inline void BitStream::rewriteU16(size_t dx, uint16_t v) {
  assert(size() <= dx + 2);
  uint8_t* ptr = (uint8_t*)&v;
  buffer[dx + 0] = ptr[0];
  buffer[dx + 1] = ptr[1];
}

inline void BitStream::rewriteU32(size_t dx, uint32_t v) {
  assert(size() <= dx + 4);
  uint8_t* ptr = (uint8_t*)&v;
  buffer[dx + 0] = ptr[0];
  buffer[dx + 1] = ptr[1];
  buffer[dx + 2] = ptr[2];
  buffer[dx + 3] = ptr[3];
}

inline void BitStream::rewriteU64(size_t dx, uint64_t v) {
  assert(size() <= dx + 8);
  uint8_t* ptr = (uint8_t*)&v;
  buffer[dx + 0] = ptr[0];
  buffer[dx + 1] = ptr[1];
  buffer[dx + 2] = ptr[2];
  buffer[dx + 3] = ptr[3];
  buffer[dx + 4] = ptr[4];
  buffer[dx + 5] = ptr[5];
  buffer[dx + 6] = ptr[6];
  buffer[dx + 7] = ptr[7];
}

inline void BitStream::rewriteS8(size_t dx, int8_t v) {
  assert(size() <= dx + 8);
  buffer[dx] = v;
}

inline void BitStream::rewriteS16(size_t dx, int16_t v) {
  assert(size() <= dx + 2);
  uint8_t* ptr = (uint8_t*)&v;
  buffer[dx + 0] = ptr[0];
  buffer[dx + 1] = ptr[1];
}

inline void BitStream::rewriteS32(size_t dx, int32_t v) {
  assert(size() <= dx + 4);
  uint8_t* ptr = (uint8_t*)&v;
  buffer[dx + 0] = ptr[0];
  buffer[dx + 1] = ptr[1];
  buffer[dx + 2] = ptr[2];
  buffer[dx + 3] = ptr[3];
}

inline void BitStream::rewriteS64(size_t dx, int64_t v) {
  assert(size() <= dx + 8);
  uint8_t* ptr = (uint8_t*)&v;
  buffer[dx + 0] = ptr[0];
  buffer[dx + 1] = ptr[1];
  buffer[dx + 2] = ptr[2];
  buffer[dx + 3] = ptr[3];
  buffer[dx + 4] = ptr[4];
  buffer[dx + 5] = ptr[5];
  buffer[dx + 6] = ptr[6];
  buffer[dx + 7] = ptr[7];
}

inline BitStream& BitStream::operator<<(uint8_t& v) {
  putU8(v);
  return *this;
}

inline BitStream& BitStream::operator<<(uint16_t& v) {
  putU16(v);
  return *this;
}

inline BitStream& BitStream::operator<<(uint32_t& v) {
  putU32(v);
  return *this;
}

inline BitStream& BitStream::operator<<(uint64_t& v) {
  putU64(v);
  return *this;
}

inline BitStream& BitStream::operator<<(int8_t& v) {
  putS8(v);
  return *this;
}

inline BitStream& BitStream::operator<<(int16_t& v) {
  putS16(v);
  return *this;
}

inline BitStream& BitStream::operator<<(int32_t& v) {
  putS32(v);
  return *this;
}

inline BitStream& BitStream::operator<<(int64_t& v) {
  putS64(v);
  return *this;
}

inline BitStream& BitStream::operator>>(uint8_t& v) {
  v = getU8();
  return *this;
}

inline BitStream& BitStream::operator>>(uint16_t& v) {
  v = getU16();
  return *this;
}

inline BitStream& BitStream::operator>>(uint32_t& v) {
  v = getU32();
  return *this;
}

inline BitStream& BitStream::operator>>(uint64_t& v) {
  v = getU64();
  return *this;
}

inline BitStream& BitStream::operator>>(int8_t& v) {
  v = getS8();
  return *this;
}

inline BitStream& BitStream::operator>>(int16_t& v) {
  v = getS16();
  return *this;
}

inline BitStream& BitStream::operator>>(int32_t& v) {
  v = getS32();
  return *this;
}

inline BitStream& BitStream::operator>>(int64_t& v) {
  v = getS64();
  return *this;
}

inline uint8_t& BitStream::operator[](unsigned int dx) {
  assert(buffer.size() > dx);
  return buffer[dx];
}

#endif
