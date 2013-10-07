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
#ifndef ROXLU_DECKLINK_CARD_H
#define ROXLU_DECKLINK_CARD_H

#include <DeckLinkAPI.h>
//#include <decklink/Common.h>
#include <decklink/DeckLinkCaptureCallback.h>
#include <decklink/DeckLinkTypes.h>

#if defined(_WIN32)
#  include <windows.h>
#endif

class DeckLinkCard {
 public:
  DeckLinkCard(IDeckLink* dl);
  ~DeckLinkCard();
  bool setVideoMode(BMDDisplayMode mode, BMDPixelFormat format); /* sets the given display mode, if supported */
  bool isVideoModeSupported(BMDDisplayMode mode, BMDPixelFormat format); /* checks whether the given display mode and pixel format are supported */
  void setCallback(decklink_capture_callback cbFrames, void* user); /* set the callback function which will receive the video and audio frames */
  bool initialize(); /* initializes the card for grabbing - call this after setting up this object */
  bool start(); /* start captureing, make sure that you've set the video mode */
  bool stop(); /* stop captureing */
 private:
  bool is_started;
  IDeckLink* dl; /* represents a decklink card (input, output) */
  IDeckLinkInput* input; /* input interface, allows capturing */
  BMDDisplayMode display_mode;
  BMDPixelFormat pixel_format;
  DeckLinkCaptureCallback* capture_callback;
 public:
  void* cb_user; /* the user that is passed into the frame callback */
  decklink_capture_callback cb_frame; /* the frame callback */
};

inline void DeckLinkCard::setCallback(decklink_capture_callback cbFrames, void* user) {
  cb_user = user;
  cb_frame = cbFrames;
}

#endif
