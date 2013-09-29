/*

  # Endian swap functions

  When you need to write a value in a specific endianness use the _To*_ macros.
  To read data which is stored in a specific endianness use and you want to convert 
  it to your system endianness, use the _From*_ macros.

  _Convert from native endianness into a specific endianness_
  ````c++
  // to big endian
  ToBE16(..)
  ToBE32(..)
  ToBE64(..)
  ```

  _Convert from a specific endianness into native endianness_
  ````c++
  uint16_t val = FromBE16(...);
  uint32_t val = FromBE32(...);
  uint64_t val = FromBE64(...);
  ````

 */
#ifndef ROXLU_ENDIAN_H
#define ROXLU_ENDIAN_H

#include <stdint.h>

static inline uint16_t endian_swap16(uint16_t value) {
  return (uint16_t)((value >> 8) | (value << 8));
}

static inline uint32_t endian_swap24(uint32_t value) {

 return (((value & 0x00ff0000) >> 16) |
         ((value & 0x0000ff00))       |
         ((value & 0x000000ff) << 16)) & 0x00FFFFFF;
}

static inline uint32_t endian_swap32(uint32_t value) {
  return (((value & 0x000000ff) << 24) |
          ((value & 0x0000ff00) <<  8) |
          ((value & 0x00ff0000) >>  8) |
          ((value & 0xff000000) >> 24));
}

static inline uint64_t endian_swap64(uint64_t value) {
  return (((value & 0x00000000000000ffLL) << 56) |
          ((value & 0x000000000000ff00LL) << 40) |
          ((value & 0x0000000000ff0000LL) << 24) |
          ((value & 0x00000000ff000000LL) << 8)  |
          ((value & 0x000000ff00000000LL) >> 8)  |
          ((value & 0x0000ff0000000000LL) >> 24) |
          ((value & 0x00ff000000000000LL) >> 40) |
          ((value & 0xff00000000000000LL) >> 56));
}

#ifdef VIDEO_STREAMER_LITTLE_ENDIAN
#  define ToBE16(n) endian_swap16(n)
#  define ToBE24(n) endian_swap24(n)
#  define ToBE32(n) endian_swap32(n)
#  define ToBE64(n) endian_swap64(n)
#  define ToLE16(n) (n)
#  define ToLE24(n) (n)
#  define ToLE32(n) (n)
#  define ToLE64(n) (n)
#  define FromBE16(n) endian_swap16(n)
#  define FromBE24(n) endian_swap24(n)
#  define FromBE32(n) endian_swap32(n)
#  define FromBE64(n) endian_swap64(n)
#  define FromLE16(n) (n)
#  define FromLE24(n) (n)
#  define FromLE32(n) (n)
#  define FromLE64(n) (n)
#else  
#  error "We are pretty sure you're on a Little Endian machine .. @todo - we should use a runtime check or cmake check"
#  define ToBE16(n) (n)
#  define ToBE24(n) (n)
#  define ToBE32(n) (n)
#  define ToBE64(n) (n)
#  define ToLE16(n) endian_swap16(n)
#  define ToLE24(n) endian_swap24(n)
#  define ToLE32(n) endian_swap32(n)
#  define ToLE64(n) endian_swap64(n)
#  define FromBE16(n) (n)
#  define FromBE24(n) (n)
#  define FromBE32(n) (n)
#  define FromBE64(n) (n)
#  define FromLE16(n) endian_swap16(n)
#  define FromLE24(n) endian_swap24(n)
#  define FromLE32(n) endian_swap32(n)
#  define FromLE64(n) endian_swap64(n)
#endif

#endif
