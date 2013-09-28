/*

  # Daemon

  The daemon class is used to accept video/audio streams from a 
  client which are then encoded and streamed to an RTMP server. 
  This is like a "hidden" Flash Media Live Encoder.

 */
#ifndef ROXLU_VIDEOSTREAMER_DAEMON_H
#define ROXLU_VIDEOSTREAMER_DAEMIN_H


#include <map>
#include <string>
#include <stdint.h>
#include <streamer/daemon/DaemonConfig.h>
#include <streamer/daemon/Runner.h>

typedef std::map<uint32_t, Runner*>::iterator runner_iterator;

class Daemon {
public:
  Daemon();
  ~Daemon();
  bool setup(std::string configpath); /* sets the state of all `runners` and ourself */
  bool initialize(); /* kicks off the runners which can start to receive data */
 private:
  DaemonConfig config;
  std::map<uint32_t, Runner*> runners; /* the runners which ingest video/audio streams, encode it and send it to a server */
};

#endif
