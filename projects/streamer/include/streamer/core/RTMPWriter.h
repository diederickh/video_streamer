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

#define RTMP_DATA_TYPE_NONE 0  /* unset */
#define RTMP_DATA_TYPE_AV 1    /* audio/video data */
#define RTMP_DATA_TYPE_STOP 2  /* stop packet */

struct RTMPData {               /* represents the data of a FLVTag which contains either video, audio or script data */
  RTMPData();
  void putBytes(uint8_t* ptr, size_t nbytes);
  void setTimeStamp(uint32_t ts);

  std::vector<uint8_t> data;
  uint32_t timestamp;
  uint8_t type;
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
  void setCallbacks(rtmp_callback disconnectCB, void* user);
 private:
  void reconnect();
  void close();
 private:
  ServerSettings settings;
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

inline void RTMPWriter::setCallbacks(rtmp_callback disconnectCB, 
                                     void* user) {
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
