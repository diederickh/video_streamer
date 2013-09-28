/*

  # H264 Parser

  The _VideoStreamer_ lib makes use of h264 that gets muxed into
  FLV. This class is used to inspect some elementary fields from a 
  h264 string. Because the nal units are all nicely separated we 
  done need to do any parsing; we expect that each "data" ptr contains
  a valid nal unit. 

*/
#ifndef ROXLU_H264_PARSER_H
#define ROXLU_H264_PARSER_H

#include <iostream>
#include <stdint.h>

struct nal_sps {
  nal_sps();

  uint8_t profile_idc;
  uint8_t constraint_set0_flag;
  uint8_t constraint_set1_flag;
  uint8_t constraint_set2_flag;
  uint8_t constraint_set3_flag;
  uint8_t constraint_set4_flag;
  uint8_t constraint_set5_flag;
  uint8_t reserved_zero_2bits;
  uint8_t level_idc;
  
};


struct NalUnit {
  NalUnit();

  uint8_t forbidden_zero_bit;
  uint8_t nal_ref_idc;
  uint8_t nal_unit_type;

  nal_sps sps;
};




class H264Parser {
 public:
  H264Parser(uint8_t* nal);
  bool parse();
  bool parseSPS(NalUnit& n);
  bool parsePPS(NalUnit& n);

  uint8_t readBit();
  uint8_t readBits(uint8_t nbits);
  uint8_t u(uint8_t nbits); /* unsigned integer using n-bits */
  uint8_t u8(); /* read on byte */
  uint8_t f(uint8_t nbits); /* fixed pattern bit string */
  uint8_t b(); /* byte */
 private:
  uint8_t* nal;
  uint32_t bit_offset;
  uint32_t byte_offset;
};


inline uint8_t H264Parser::readBit() {
  uint8_t byte = nal[byte_offset];
  uint8_t mask = 1 << (7 - bit_offset);

  ++bit_offset;
  if(bit_offset > 7) {
    bit_offset = 0;
    ++byte_offset;
  } 

  return (byte & mask) ? 1 : 0;
}

inline uint8_t H264Parser::readBits(uint8_t nbits) {
  if(nbits > 7) {
    return 0;
  }
  uint8_t result = 0x00;
  uint8_t bitpos = 0;
  for(uint8_t i = 0; i < nbits; ++i) {
    bitpos = 7 - bit_offset;
    result |= (readBit() << bitpos);
  }

  return result;
}

inline uint8_t H264Parser::u8() {
  uint8_t result = nal[byte_offset];
  byte_offset++;
  return result;
}

inline uint8_t H264Parser::u(uint8_t nbits) {
  return readBits(nbits);
}

inline uint8_t H264Parser::f(uint8_t nbits) {
  return u(nbits) ? 1: 0;
}

#endif
