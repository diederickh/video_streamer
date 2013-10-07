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

  # MultiVideoStreamer 

  The multi video streamer will create multiple audio and video encoder instances
  and streams these to the specified servers. Some media server providers have a 
  feature to stream multiple quality video to their end users; this is when you 
  would use the MultiVideoStreamer.
 
  If you only need to stream one quality stream, use the VideoStreamer class.

  ---
  
  After you've successfully loaded the settings, setup() and start()ed the MultiVideoStreamer
  you can start adding video and audio packets. It's important that you use a MemoryPool to 
  handle packets. The memory pool uses a manual reference counting system to allow multiple
  encoder to work with the same data. YOU DO NOT HAVE TO THINK ABOUT THE REFERENCE COUNTS WHEN
  USING THIS CLASS. WE WILL ADD THE APPROPRIATE REFERENCE COUNTS FOR YOU!

  --- 

  The feature of using multiple quality video/audio streams has had a big influence on the 
  design of the whole Video Streamer library. This is the reason why we have a memory pool,
  where raw (input) video and audio data are shared between multiple video and audio 
  encoders. 

  IMPORTANT NOTE ABOUT AVPACKET
  -----------------------------

  It's important to know that I'm not entirely happy with the design of AVPacket and the way
  multiple video streams are handled. Though the current solution seems to be the most elegant
  I could think off to mix multi and basic streams into one AVPacket.  The thing is, that 
  the YUV420PGrabber creates one big texture that will hold all the texture/video data for 
  multiple streams (see this image where we have all the video data for 3 different sized
  video streams: http://www.flickr.com/photos/diederick/10027076563/ ). All this video data
  is assigned to the AVPacket::data member. We use strides and plane pointers to tell 
  the video encoder where to fetch it's data from. 

  Therefore, when using a multi packet. you need to assing all the strides + planes all the
  different streams are using. And this is where the "stream_id" member of VideoEncoder 
  comes to work. 

*/

#ifndef ROXLU_MULTI_VIDEOSTREAMER_H
#define ROXLU_MULTI_VIDEOSTREAMER_H

#include <streamer/core/MemoryPool.h>
#include <streamer/videostreamer/VideoStreamer.h>
#include <streamer/videostreamer/VideoStreamerConfig.h>
#include <vector>

// -----------------------------------------------------------

struct MultiStreamerInfo { /* contains all the necessary information for one video streamer to "run", everythin is owned and managed by the MultiVideoStreamer instance  */
  MultiStreamerInfo();
  VideoStreamer* streamer;
  int32_t id;
};

// -----------------------------------------------------------

class MultiVideoStreamer {
 public:
  MultiVideoStreamer();
  ~MultiVideoStreamer();
  bool loadSettings(std::string filepath); /* call before setup() - load the settings where you define all of the stream properties */
  bool setup(); /* after you've called loadSettings(), call setup() to setup all the streams and allocate the instances we need*/
  bool start(); /* this will start all the streams */
  void update(); /* call this regurlarly - it will make sure that we reconnect to the streaming server when we get disconncted */
  bool stop(); /* stop all the streams! */
  void shutdown(); /* deallocates all allocated memory from start(). */
  void print(); /* print debug information about the different qualities we've got */
  void addVideo(AVPacket* pkt); /* add video - make sure to set the timestamp! - DO NOT ADD ANY EXTRA REFERENCE COUNTERS! WE ADD THE REFCOUNTS FOR YOU! - BY DEFAULT AN AVPACKET HAS A REFCOUNT OF 1 WHEN RETURNED FROM THE FREE PACKETS IN A MEMORY POOL, WE WILL INCREASE THIS NUMBER BY THE NECESSARY AMOUNT! */
  void addAudio(AVPacket* pkt); /* add video - make sure to set the timestamp! - DO NOT ADD ANY EXTRA REFERENCE COUNTERS! WE ADD THE REFCOUNTS FOR YOU! - BY DEFAULT AN AVPACKET HAS A REFCOUNT OF 1 WHEN RETURNED FROM THE FREE PACKETS IN A MEMORY POOL, WE WILL INCREASE THIS NUMBER BY THE NECESSARY AMOUNT! */
  MultiStreamerInfo* operator[](unsigned int dx); /* returns the stream info for the given index */
  size_t size(); /* returns the number of MultiStreamerInfo objects - which represent a stream */
 private:
  VideoStreamerConfig config;
  std::vector<MultiStreamerInfo*> streamers; /* the memory pools and streamers that we use for the multi streaming; MultiVideoStreamer owns and manages these objects */
};

inline size_t MultiVideoStreamer::size() {
  return streamers.size();
}

inline MultiStreamerInfo* MultiVideoStreamer::operator[](unsigned int dx) {
#if !defined(NDEBUG)
  return streamers.at(dx);
#else
  return streamers[dx];
#endif
}

inline void MultiVideoStreamer::update() {
  for(std::vector<MultiStreamerInfo*>::iterator it = streamers.begin(); it != streamers.end(); ++it) {
    MultiStreamerInfo* msi = *it;
    msi->streamer->update();
  }
}

#endif
