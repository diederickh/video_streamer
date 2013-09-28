#ifndef ROXLU_DECKLINK_H
#define ROXLU_DECKLINK_H

#include <iostream>
#include <DeckLinkAPI.h>
#include <decklink/DeckLinkCard.h>
#include <decklink/DeckLinkTypes.h>

#if defined(_WIN32)
#  include <windows.h>
#endif

class DeckLink {
 public:
  DeckLink();
  ~DeckLink();
  bool setup(int device); /* select the given device */
  bool setVideoMode(BMDDisplayMode mode, BMDPixelFormat format); /* set the display-mode and pixel-format for the selected device; make sure that you've called setup() first. */
  bool setCallback(decklink_capture_callback cbFrames, void* user); /* sets the callback on the selected device; this will receive video and audio frames. see DeckLinkTypes.h for more info on the callback */
  bool start(); /* start captureing on the selected device */
  bool stop(); /* stop captureing */
  bool printDevices(); /* shows info about the devices */
 private:
  bool getCapabilities(IDeckLink* dl);
  DeckLinkCard* card;
};

inline bool DeckLink::setVideoMode(BMDDisplayMode mode, BMDPixelFormat format) {
  if(!card) {
    printf("error: no active card found. did you setup() ?\n");
    return false;
  }
  return card->setVideoMode(mode, format);
}

inline bool DeckLink::start() {
  if(!card) {
    printf("error: no active card found. did you setup() ?\n");
    return false;
  }
  return card->start();
}

inline bool DeckLink::stop() {
  if(!card) {
    printf("error: no active card found. did you setup() ?\n");
    return false;
  }
  return card->stop();
}

inline bool DeckLink::setCallback(decklink_capture_callback cbFrames, void* user) {
  if(!card) {
    printf("error: no active card found. did you setup() ?\n");
    return false;
  }
  card->setCallback(cbFrames, user);
  return true;
}

#endif
