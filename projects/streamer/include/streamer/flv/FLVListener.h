/*
  # FLVListener

  Interface for FLV events. e.g. the FLVReader fires an event 
  when it reads a FLVTag, the listener can handle this in any way it
  prefers.

  The FLVListeners (with 's') is just a wrapper to handle multiple listeners

 */

#ifndef ROXLU_FLV_LISTENER_H
#define ROXLU_FLV_LISTENER_H

#include <streamer/flv/FLVTypes.h>
#include <vector>

// ------------------------------------

class FLVListener {
 public:
  virtual void onSignature(BitStream& bs) = 0;  /* gets called when the flv signature has been created and added to the bitstream,  the bitstream is cleared after all listeners have been called so if  you need to hold on to the data, copy it */
  virtual void onTag(BitStream& bs, FLVTag& tag) = 0; /* gets called when the given tag has been created and added to the bitstream, the bitstream is cleared after all listeners have been called so if  you need to hold on to the data, copy it */
};

// ------------------------------------

class FLVListeners { /* Container for multiple listeners, the caller is responsible for freeing the listeners */
 public:
  void addListener(FLVListener* listener);
  void onSignature(BitStream& bs);  /* gets called when the FLV signature is created; this is basically the file header */
  void onTag(BitStream& bs, FLVTag& tag); /* gets called when a new FLV tag is created by the FLVWriter. The bitstream is cleared after all listeners have been called, so if you need to hold on to the data, copy it */
  
  std::vector<FLVListener*> listeners;
};

inline void FLVListeners::addListener(FLVListener* l) {
  listeners.push_back(l);
}


inline void FLVListeners::onSignature(BitStream& bs) {

  for(std::vector<FLVListener*>::iterator it = listeners.begin(); it != listeners.end(); ++it) {
    (*it)->onSignature(bs);
  }

  if(listeners.size()) {
    bs.clear();
  }
}

inline void FLVListeners::onTag(BitStream& bs, FLVTag& tag) {

  for(std::vector<FLVListener*>::iterator it = listeners.begin(); it != listeners.end(); ++it) {
    (*it)->onTag(bs, tag);
  }

  if(listeners.size()) {
    bs.clear();
  }
}

#endif
