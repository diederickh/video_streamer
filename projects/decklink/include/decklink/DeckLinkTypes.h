#ifndef ROXLU_DECKLINK_TYPES_H
#define ROXLU_DECKLINK_TYPES_H

#include <DeckLinkAPI.h>
//#include <decklink/Common.h>

/*
  The capture callback you set if you want to handle the frames. 
  This function will be called whenever the DeckLink SDK triggers our
  own internally used callback handler. 

  By default a frame is only valid for the duration of the callback
  unless you call `vframe->AddRef()` in which case you can hold on to 
  the frame untill you call `vframe->Release()`. 
  
 */
//extern "C" {

typedef void(*decklink_capture_callback)(IDeckLinkVideoInputFrame* vframe, 
                                         IDeckLinkAudioInputPacket* aframe,
                                         void* user);
//}

#endif

