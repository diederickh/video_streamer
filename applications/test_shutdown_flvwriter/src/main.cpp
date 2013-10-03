/*

  # Test FLV Shutdown

  This example shows how to initialize/open and shutdown/close the most 
  important parts of the FLV writing process. See the readme for a more thorough
  description.

  
 */
#include <iostream>
#include <signal.h>
#include <streamer/core/TestPattern.h>
#include <streamer/core/EncoderTypes.h>
#include <streamer/core/VideoEncoder.h>
#include <streamer/flv/FLVWriter.h>
#include <streamer/flv/FLVFileWriter.h>
#include <streamer/core/MemoryPool.h>
#include <tinylib/tinylib.h>

bool must_run = true;

void sighandler(int sig);

int main() {
  printf("flv writer shutdown test - ddd.\n");

  VideoSettings video_cfg;
  video_cfg.width = 640;
  video_cfg.height = 360;
  video_cfg.fps = 15;

  MemoryPool mempool;
  uint32_t nbytes = video_cfg.width * video_cfg.height * 2; // yuv420p
  if(!mempool.allocateVideoFrames(10, nbytes)) {
    printf("error: cannot allocate memory pool.\n");
    ::exit(EXIT_FAILURE);
  }

  VideoEncoder enc;
  if(!enc.setup(video_cfg)) {
    printf("error: cannot setup video encoder.\n");
    ::exit(EXIT_FAILURE);
  }
  
  TestPattern tp;
  if(!tp.setup(video_cfg.width, video_cfg.height, video_cfg.fps, 44100)) {
    printf("error: cannot setup testpattern.\n");
    ::exit(EXIT_FAILURE);
  }

  BitStream flv_bitstream;
  FLVWriter flv_writer(flv_bitstream);
  
  flv_writer.setVideoCodec(FLV_VIDEOCODEC_AVC);
  flv_writer.setWidth(video_cfg.width);
  flv_writer.setHeight(video_cfg.height);
  flv_writer.setFrameRate(video_cfg.fps);

  FLVFileWriter flv_file; 
  flv_writer.addListener(&flv_file);

  signal(SIGINT, sighandler);

  int runs = 20;
  int num_frames = 150;
  int frame = 0;

  for(int run = 0; run < runs; ++run) {
    frame = 0;


    // Initialize 
    if(!enc.initialize()) {
      printf("error: cannot initialize encoder.\n");
      ::exit(EXIT_FAILURE);
    }

    AVCDecoderConfigurationRecord avc_rec;
    if(!enc.createDecoderConfigurationRecord(avc_rec)) {
      printf("error: cannot create avc record.\n");
      ::exit(EXIT_FAILURE);
    }
    flv_writer.setDecoderConfigurationRecord(avc_rec);

    char outname[512];
    sprintf(outname, "test_%04d.flv", run);

    if(!flv_file.open(rx_get_exe_path() +outname)) {
      ::exit(EXIT_FAILURE);
    }

    if(!flv_writer.open()) {
      printf("error: cannot open flv writer.\n");
      ::exit(EXIT_FAILURE);
    }

    tp.start();

    // Write the frames
    while(frame < num_frames && must_run) {

      tp.update();

      if(tp.hasVideoFrame()) {
        AVPacket* pkt = mempool.getFreeVideoPacket();
        if(pkt) {
          tp.generateVideoFrame(pkt->data, pkt->planes, pkt->strides);

          FLVTag flv_tag;
          if(!enc.encodePacket(pkt, flv_tag)) {
            printf("error: cannot encode packet.\n");
            ::exit(EXIT_FAILURE);
          }

          flv_tag.setTimeStamp(tp.getTimeStamp());
          flv_writer.writeVideoTag(flv_tag);

          // we need to release the packet here, so we can reuse it!
          pkt->release();

          printf("Write frame: %d\n", flv_tag.timestamp);          
          frame++;
        }
      }
    }
    
    // Shutdown 
    if(!flv_writer.close()) {
      printf("error: cannot close the flvwriter.\n");
      ::exit(EXIT_FAILURE);
    }

    if(!flv_file.close()) {
      printf("erorr: cannot close the flv file writer.\n");
      ::exit(EXIT_FAILURE);
    }

    if(!enc.shutdown()) {
      printf("error: cannot shutdown the encoder.\n");
      ::exit(EXIT_FAILURE);
    }

    if(!must_run) {
      break;
    }

  }

  return 0;
}

void sighandler(int sig) {
  must_run = false;
}
