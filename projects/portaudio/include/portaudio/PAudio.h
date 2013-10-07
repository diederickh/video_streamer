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

  # PAuido

  Very thin and basic wrapper around PortAudio to get cross
  platform audio in. 

 */

#ifndef ROXLU_PORTAUDIO_H
#define ROXLU_PORTAUDIO_H

#include <portaudio.h>

// -----------------------------------------------------------
int paudio_in(const void* input, void* output, unsigned long nframes,
                      const PaStreamCallbackTimeInfo* time, 
                      PaStreamCallbackFlags status, void* userData);
// -----------------------------------------------------------

typedef void(*paudio_in_callback)(const void* input, unsigned long nframes, void* user);

// -----------------------------------------------------------

class PAudio {
 public:
  PAudio();
  ~PAudio();
  void setCallback(paudio_in_callback cb, void* user);
  bool start();
  bool stop();
  bool openInputStream(int device, int nchannels, PaSampleFormat format, double samplerate, unsigned long framesPerBuffer);
  int listDevices();
  int getDefaultInputDevice(); /* returns the ID of the default input device or < 0 on error */
  bool isInputFormatSupported(int device, int nchannels, PaSampleFormat format, double samplerate);

 public:
  paudio_in_callback cb_audio;
  void* cb_user;

 private:
  PaStream* input_stream;
};

inline void PAudio::setCallback(paudio_in_callback cb, void* user) {
  cb_audio = cb;
  cb_user = user;
}

#endif
