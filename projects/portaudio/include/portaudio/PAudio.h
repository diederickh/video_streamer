/*

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
