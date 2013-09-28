#include <iostream>
#include <streamer/videostreamer/VideoStreamer.h>
#include <decklink/DeckLink.h>

#if defined(_WIN32)
#  include <windows.h>
#endif

int main() {
  printf("DeckLink Stream.\n");

  DeckLink dl;
  dl.printDevices();

  return 0;
}
