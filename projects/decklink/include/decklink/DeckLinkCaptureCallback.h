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
#ifndef ROXLU_DECKLINK_CAPTURE_CALLBACK_H
#define ROXLU_DECKLINK_CAPTURE_CALLBACK_H

#include <DeckLinkAPI.h>
//#include <decklink/Common.h>
#include <decklink/DeckLinkTypes.h>

class DeckLinkCaptureCallback : public IDeckLinkInputCallback {
 public:
  DeckLinkCaptureCallback();
  ~DeckLinkCaptureCallback();
  virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID iid, LPVOID *ppv) { return E_NOINTERFACE; } 
  virtual ULONG STDMETHODCALLTYPE AddRef(void) { return 1; }
  virtual ULONG STDMETHODCALLTYPE Release(void) { return 1; } 
  virtual HRESULT STDMETHODCALLTYPE VideoInputFormatChanged(BMDVideoInputFormatChangedEvents, IDeckLinkDisplayMode*, BMDDetectedVideoInputFormatFlags);
  virtual HRESULT STDMETHODCALLTYPE VideoInputFrameArrived(IDeckLinkVideoInputFrame*, IDeckLinkAudioInputPacket*);
  void setCallback(decklink_capture_callback cbFrames, void* user); /* set the callback function which will receive the video and audio frames */
 private:
  void* cb_user; /* the user that is passed into the frame callback */
  decklink_capture_callback cb_frame; /* the frame callback */
};

inline void DeckLinkCaptureCallback::setCallback(decklink_capture_callback cbFrames, void* user) {
  cb_frame = cbFrames;
  cb_user = user;
}

#endif
