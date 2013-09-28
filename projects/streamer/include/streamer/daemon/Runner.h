/*

  # Runner

  A runner is a sperate process which ingests video/audio streams
  for the given StreamConfig. It will create a new VideoStreamer instance
  based on these configurations. It has it's own 'socket', which can be
  any of the nanomsg defined addresses (inproc, ipc, tcp) and this is 
  with what the client communicates.

  This class is used/instatiated by the main Daemon class.


 */
#ifndef ROXLU_VIDEOSTREAMER_RUNNER_H
#define ROXLU_VIDEOSTREAMER_RUNNER_H

#include <streamer/daemon/DaemonConfig.h>
#include <streamer/videostreamer/VideoStreamer.h>

extern "C" {
#  include <uv.h>
#  include <nanomsg/nn.h>
#  include <nanomsg/pubsub.h>
#  include <nanomsg/pipeline.h>
}

void runner_thread(void* user);

class Runner {
 public:
  Runner(StreamConfig* sc);
  ~Runner();
  bool setup();
  bool initialize();

  void handleCommand(uint8_t type, uint32_t nbytes, std::vector<uint8_t>& buffer);

 public:
  VideoStreamer streamer; /* the instance which does the encoding and streaming */
  StreamConfig* sc; /* stream configuration (we are not the owner) */
  int sock;
  uv_thread_t thread;
  volatile bool must_run;
};
#endif
