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

#ifndef ROXLU_VIDEOSTREAMER_CHANNEL_H
#define ROXLU_VIDEOSTREAMER_CHANNEL_H

extern "C" {
#  include <uv.h>
#  include <nanomsg/nn.h>
#  include <nanomsg/pubsub.h>
#  include <nanomsg/pipeline.h>
}

#include <string>
#include <vector>
#include <streamer/core/BitStream.h>
#include <streamer/core/EncoderTypes.h>
#include <streamer/core/Endian.h>
#include <streamer/daemon/Types.h>

// ----------------------------------------------

void channel_thread(void* user); /* the thread in which the channel work is done */

// ----------------------------------------------

struct ChannelPacket {
  ChannelPacket();
  BitStream bs;
};

// ----------------------------------------------

class Channel {
 public:
  Channel();
  ~Channel();
  bool setup(uint32_t id, std::string address); /* pass the stream id to which we're sending data (video/audio) and the address of the stream (ipc://, tcp://, inproc:// ) */
  bool initialize(); /* start the channel */
  bool shutdown(); /* stop the channel */
  int send(uint8_t* data, size_t nbytes);
  void addVideoPacket(std::vector<uint8_t>& pixels, uint32_t timestamp);
  // @todo - cleanup Channel.h
  //void addVideoPacket(AVPacket* p); /* add a video packet - we take ownership */
  //void addAudioPacket(AVPacket* p); /* add an audiopacket - we take ownership */
 private:
  //void addAVPacket(AVPacket* p); /* adds a generic av packet */
  void addPacket(ChannelPacket* p);

 public:
  std::string stream_address; 
  uint32_t stream_id; /* the video & audio, are sent to the server using unique streams */
  uv_thread_t thread;
  uv_cond_t cv;
  uv_mutex_t mutex;
  std::vector<ChannelPacket*> work; /* packets that need to be delivered to the server */
  int sock; 
};

// ----------------------------------------------
inline void Channel::addVideoPacket(std::vector<uint8_t>& pixels, uint32_t timestamp) {
  ChannelPacket* cp = new ChannelPacket();
  cp->bs.putU32(ToBE32(stream_id));
  cp->bs.putU8(CP_TYPE_AVPACKET);
  cp->bs.putU32(ToBE32(timestamp));
  cp->bs.putBytes(&pixels.front(), pixels.size());
  addPacket(cp);
}


#endif
