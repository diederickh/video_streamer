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
#include <iostream>
#include <streamer/core/EncoderTypes.h>

// ---------------------------------------------------

void rtmp_sigpipe_handler(int signum);

class RTMPWriter;
typedef void(*rtmp_callback)(RTMPWriter* writer, void* user);  // e.g. used for the rtmp callback

// ---------------------------------------------------

struct RTMPData {               /* represents the data of a FLVTag which contains either video, audio or script data */
  RTMPData();
  void putBytes(uint8_t* ptr, size_t nbytes);
  void setTimeStamp(uint32_t ts);

  std::vector<uint8_t> data;
  uint32_t timestamp;
};

// ---------------------------------------------------
#define RW_STATE_NONE 0
#define RW_STATE_INITIALIZED 1
#define RW_STATE_RECONNECTING 2
#define RW_STATE_DISCONNECTED 3

class RTMPWriter {
 public:
  RTMPWriter();
  ~RTMPWriter();
  bool setup(ServerSettings ss); /* call setup() with valid ServerSetting before calling initialize */
  bool initialize();
  void write(uint8_t* data, size_t nbytes);
  void read();
  void setDisconnectHandler(rtmp_callback disconnectCB, void* user);
 private:
  void reconnect();
 private:
  ServerSettings settings;
  //  bool is_initialized;
  RTMP* rtmp;
  int state; 
  rtmp_callback cb_disconnect; /* when set, it gets called when we get disconnected */
  void* cb_user; /* gets passed into cb_disconnect */
};

// ---------------------------------------------------

inline bool RTMPWriter::setup(ServerSettings ss) {

  if(!ss.url.size()) {
    printf("error: invalid server settings, no url set.\n");
    return false;
  }

  settings = ss;

  return true;
}

inline void RTMPWriter::setDisconnectHandler(rtmp_callback disconnectCB, void* user) {
  cb_disconnect = disconnectCB;
  cb_user = user;
}

// ---------------------------------------------------

inline void RTMPData::putBytes(uint8_t* ptr, size_t nbytes) {
  std::copy(ptr, ptr+nbytes, std::back_inserter(data));
}

inline void RTMPData::setTimeStamp(uint32_t ts) {
  timestamp = ts;
}

#endif
