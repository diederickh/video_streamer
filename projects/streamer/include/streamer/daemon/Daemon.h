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
