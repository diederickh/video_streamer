#include <decklink/DeckLinkCaptureCallback.h>
#include <iostream>

DeckLinkCaptureCallback::DeckLinkCaptureCallback() 
  :cb_user(NULL)
  ,cb_frame(NULL)
{
}

DeckLinkCaptureCallback::~DeckLinkCaptureCallback() {
  cb_user = NULL;
  cb_frame = NULL;
}

HRESULT DeckLinkCaptureCallback::VideoInputFormatChanged(BMDVideoInputFormatChangedEvents ev,
                                                         IDeckLinkDisplayMode* mode,
                                                         BMDDetectedVideoInputFormatFlags flags)
{
  printf("-- video input format changed.\n");
  return S_OK;
}

HRESULT DeckLinkCaptureCallback::VideoInputFrameArrived(IDeckLinkVideoInputFrame* vframe, 
                                                        IDeckLinkAudioInputPacket* aframe) 
{
  if(cb_frame) {
    cb_frame(vframe, aframe, cb_user);
  }

  return S_OK;
}


