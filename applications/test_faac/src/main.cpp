/*

  # Test Audio input

  When using the settings from below you can use avconv to convert the test capture
  to an mp3 file.
  
  for: 2 channels, sampleformat: paInt16, samplerate: 44100
  ./avconv -v debug -f s16le -ac 2 -ar 44100 -i raw.pcm out.mp3
  
 */
#include <iostream>
#include <fstream>
#include <tinylib/tinylib.h>
#include <portaudio/PAudio.h>
#include <streamer/flv/FLVTypes.h>
#include <streamer/core/EncoderTypes.h>
#include <streamer/core/Log.h>
#include "AudioEncoderFAAC.h"

std::ofstream ofs;

void on_audio(const void* input, unsigned long nframes, void* user);

AudioEncoderFAAC faac;
bool must_run = true;

void sighandler(int sig);

int main() {
  
  AudioSettings audio_settings;
  audio_settings.samplerate = AV_AUDIO_SAMPLERATE_44100;
  audio_settings.mode = AV_AUDIO_MODE_STEREO; // meaning 2 channels
  audio_settings.bitrate = 64;
  audio_settings.bitsize = AV_AUDIO_BITSIZE_S16; 
  audio_settings.in_bitsize = audio_settings.bitsize;
  audio_settings.in_interleaved = true;
  
  faac.setOutputFile(rx_get_exe_path() +"raw.aac");

  if(!faac.setup(audio_settings)) {
    STREAMER_ERROR("Cannot setup faac. Stopping now.\n");
    ::exit(EXIT_FAILURE);
  }
  
  if(!faac.initialize()) {
    STREAMER_ERROR("Cannot initialize faac. Stopping now.\n");
    ::exit(EXIT_FAILURE);
  }

  faac.print();

  std::string of = rx_get_exe_path()+"raw.pcm";
  ofs.open(of.c_str(), std::ios::binary | std::ios::out);
  if(!ofs.is_open()) {
    printf("error: cannot open output file.\n");
    ::exit(EXIT_FAILURE);
  }

  PAudio pa;

  pa.listDevices();

  pa.setCallback(on_audio, NULL);

  if(!pa.openInputStream(pa.getDefaultInputDevice(), 2, paInt16, 44100, 1024)) { // faac.getSamplesNeededForEncoding())) {
    printf("error: cannot open input stream.\n");
    ::exit(EXIT_FAILURE);
  }
  
  if(!pa.start()) {
    printf("error: cannot start the input stream.\n");
    ::exit(EXIT_FAILURE);
  }

  printf("default input device: %d\n", pa.getDefaultInputDevice());

  signal(SIGINT, sighandler);

  printf("audio in test");
  while(must_run) {
    printf("..\n");
    sleep(1);
  }

  if(ofs.is_open()) {
    ofs.close();
  }

  return 0;
}

void on_audio(const void* input, unsigned long nframes, void* user) {

  size_t nbytes = nframes * sizeof(short int) * 2;

#if 0 

  printf("nbytes: %ld\n", nbytes);
  if(ofs.is_open()) {
    ofs.write((char*)input, nbytes);
  }  

#else 

  uint8_t* ptr = (uint8_t*)input;
  AVPacket* pkt = new AVPacket(NULL);
  pkt->makeAudioPacket();
  pkt->allocate(nbytes);
  pkt->data.assign(ptr, ptr+nbytes);

  FLVTag tag; 
  faac.encodePacket(pkt, tag);

  delete pkt;
  pkt = NULL;

#endif
}


void sighandler(int sig) {
  must_run = false;

}
