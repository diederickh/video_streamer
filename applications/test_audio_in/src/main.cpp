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

std::ofstream ofs;

void on_audio(const void* input, unsigned long nframes, void* user) {
  
  size_t nbytes = nframes * sizeof(short int) * 2;
  printf("nbytes: %ld\n", nbytes);
  if(ofs.is_open()) {
    ofs.write((char*)input, nbytes);
  }

}

bool must_run = true;

void sighandler(int sig);

int main() {
  
  std::string of = rx_get_exe_path()+"raw.pcm";
  ofs.open(of.c_str(), std::ios::binary | std::ios::out);
  if(!ofs.is_open()) {
    printf("error: cannot open output file.\n");
    ::exit(EXIT_FAILURE);
  }

  PAudio pa;

  pa.listDevices();

  pa.setCallback(on_audio, NULL);

  if(!pa.openInputStream(0, 2, paInt16, 44100, 512)) {
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

void sighandler(int sig) {
  must_run = false;
}
