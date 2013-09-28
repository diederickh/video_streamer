extern "C" {
#  include <uv.h>
}

#include <iostream>
#include <windows.h>
#include <signal.h>
#include <decklink/DeckLink.h>
#include <decklink/DeckLinkTypes.h>
//#include <streamer/videostreamer/VideoStreamer.h>
#include <xmmintrin.h>
#include <stdint.h>
#include <libyuv/libyuv.h>
#include <fstream>

#define WRITE_TO_YUV_FILE 1

void sighandler(int signum);

void cb_frame(IDeckLinkVideoInputFrame* vframe,
              IDeckLinkAudioInputPacket* aframe,
              void* user);

bool must_run;

#if WRITE_TO_YUV_FILE
std::ofstream ofs;
#endif

uint8_t yuv420p[1382400];

int main() {
  printf("decklink.\n");

  DeckLink dl;
  if(!dl.setup(0)) {
    printf("error: cannot setup the decklink.\n");
    ::exit(EXIT_FAILURE);
  }
  
  if(!dl.setCallback(cb_frame, NULL)) {
    printf("error: cannot set callback.\n");
    ::exit(EXIT_FAILURE);
  }

  if(!dl.setVideoMode(bmdModeHD720p60, bmdFormat8BitYUV)) {
    printf("error: cannot set the video mode.\n");
    ::exit(EXIT_FAILURE);
  }

#if WRITE_TO_YUV_FILE
  ofs.open("C:\\Users\\Diederick\\Downloads\\avconv\\raw.yuv", std::ios::out | std::ios::binary);
  if(!ofs.is_open()) {
    printf("error: cannot open output file raw.yuv.\n");
    ::exit(EXIT_FAILURE);
  }
#endif  

  if(!dl.start()) {
    ::exit(EXIT_FAILURE);
  }

  signal(SIGINT, sighandler);

  must_run = true;
  while(must_run) {
    Sleep(1000);
  }
  printf("Yay!\n");

  if(!dl.stop()) {
    printf("error: failed to stop!\n");
  }

#if WRITE_TO_YUV_FILE
  if(ofs.is_open()) {
  ofs.close();
}
#endif
  return 0;
}

  void sighandler(int signum) {
    printf("\nsignal!\n");
    must_run = false;
}


void cb_frame(IDeckLinkVideoInputFrame* vframe,
    IDeckLinkAudioInputPacket* aframe,
    void* user)
  {

  if(vframe) {
    long unsigned int w = vframe->GetWidth();
    long unsigned int h = vframe->GetHeight();
    long unsigned int stride = vframe->GetRowBytes();
    uint32_t nbytes = stride * h;
    uint8_t* yuv422 = NULL;

    HRESULT r = vframe->GetBytes((void**)&yuv422);
    if(r != S_OK) {
      printf("error: cannot get the video frame bytes.\n");
      ::exit(EXIT_FAILURE);
    }

    uint64_t now = uv_hrtime();
    {
      uint8_t* dest_y = yuv420p;
      uint8_t* dest_u = &yuv420p[(w * h)];
      uint32_t dest_v_offset = (w * h) + ((w * 0.5) * (h * 0.5));
      uint8_t* dest_v = &yuv420p[dest_v_offset];
      int dest_stride_y = w;
      int dest_stride_u = w >> 1;
      int dest_stride_v = w >> 1;

      libyuv::UYVYToI420(yuv422, stride,
                         dest_y, dest_stride_y,
                         dest_u, dest_stride_u,
                         dest_v, dest_stride_v,
                         w, h);

    }
    uint64_t d = uv_hrtime() - now;

#if WRITE_TO_YUV_FILE
    ofs.write((char*)yuv420p, sizeof(yuv420p));
#endif
    printf("width: %ld, height: %ld, stride: %ld, conv: %lld ns. conv: %lld ms.\n", w, h, stride, d, d/1000000);
  }

}

