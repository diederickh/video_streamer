#ifndef ROXLU_RTMP_WRITER_H
#define ROXLU_RTMP_WRITER_H

extern "C" {
#  include <librtmp/rtmp.h>
#  include <librtmp/log.h>
}

#include <string>
#include <stdint.h>
#include <vector>
#include <iterator>
#include <algorithm>

// ---------------------------------------------------

struct RTMPData {               /* represents the data of a FLVTag which contains either video, audio or script data */
  RTMPData();
  void putBytes(uint8_t* ptr, size_t nbytes);
  void setTimeStamp(uint32_t ts);

  std::vector<uint8_t> data;
  uint32_t timestamp;
};

// ---------------------------------------------------

class RTMPWriter {
 public:
  RTMPWriter();
  ~RTMPWriter();
  bool initialize();
  void setURL(std::string url);
  void write(uint8_t* data, size_t nbytes);
  void read();
 private:
  bool is_initialized;
  std::string url;
  RTMP* rtmp;
};

// ---------------------------------------------------

inline void RTMPWriter::setURL(std::string u) {
  url = u;
}

// ---------------------------------------------------

inline void RTMPData::putBytes(uint8_t* ptr, size_t nbytes) {
  std::copy(ptr, ptr+nbytes, std::back_inserter(data));
}

inline void RTMPData::setTimeStamp(uint32_t ts) {
  timestamp = ts;
}

#endif
