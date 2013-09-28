#include <streamer/daemon/Runner.h>
#include <streamer/daemon/Types.h>
#include <streamer/core/Endian.h>
#include <iterator>
#include <algorithm>
#include <fstream>


// @todo cleanup - remove these states as they are not used anymore
#define RU_PARSE_STATE_NONE 0 /* start parsing, wait for the first 4 bytes which is the stream id */
#define RU_PARSE_STATE_ID 1 /* we got the stream id,  now get the ChannelPacket type, which is 1 byte */
#define RU_PARSE_STATE_TYPE 2 /* we got the id+type, get the size of data for this packet */
#define RU_PARSE_STATE_SIZE 3 /* we got the id+type+size, handle the data */
#define RU_PARSE_STATE_DATA 4

// --------------------------------------------------------

void runner_thread(void* user) {
#if RUNNER_SAVE_FILE
  std::ofstream ofs("read.dat", std::ios::out | std::ios::binary);
  if(!ofs.is_open()) {
    printf("error: cannot open read.dat.\n");
    ::exit(EXIT_FAILURE);
  }
#endif

  Runner* r = static_cast<Runner*>(user);
  int rc = 0;

  char* buf = NULL;
  char* data = NULL;
  bool parsing = false;
  int state = RU_PARSE_STATE_NONE;
  uint32_t pkt_stream_id = 0; /* the parsed stream id */
  uint8_t pkt_type = 0; /* channel packet type */
  
  while(r->must_run) {

    rc = nn_recv(r->sock, &buf, NN_MSG, 0);
    if(rc < 0) {
      printf("error: error while reading data. %s\n", nn_strerror(errno));
      continue;
    }

    // get stream id + packet type
    data = buf;
    pkt_stream_id = FromBE32(*(uint32_t*)data);
    pkt_type = data[4];
    data = data + 5;  // (stream id + type)

    switch(pkt_type) {
      case CP_TYPE_AVPACKET: {

        uint32_t ts = FromBE32(*(uint32_t *)data);
        AVPacket* av_pkt = new AVPacket();
        av_pkt->makeVideoPacket();
        av_pkt->setTimeStamp(ts);
        av_pkt->copy((uint8_t*)data+4, rc-9); // 9 = stream_id(4)+type(1)+timestamp(4)
        r->streamer.addVideo(av_pkt);
#if RUNNER_SAVE_FILE
        ofs.write((data+4), rc-9);
#endif
        printf("<< timestamp: %d, size: %d\n", ts, (rc-9));
        break;
      }
      default: {
        printf("error: unhandled packet type: %d\n", pkt_type);
        break;
      }
    }

    nn_freemsg(buf);
  }
#if RUNNER_SAVE_FILE
  ofs.close();
#endif
}

// --------------------------------------------------------

Runner::Runner(StreamConfig* sc)
  :sc(sc)
  ,sock(0)
  ,thread(NULL)
  ,must_run(false)
{
}

Runner::~Runner() {
  printf("error: ~Runner() need to join the thread.\n");
  printf("error: ~Runner() need to join the streamer thread + correctly shut it down.\n");
  sc = NULL;
}

bool Runner::setup() {
  if(!sc->address.size()) {
    printf("error: invalid addess: %s\n", sc->address.c_str());
    return false;
  }

  // validate the settings.
  if(!sc->validate()) {
    printf("error: cannot validate the StreamConfig in Runner.\n");
    return false;
  }

  sock = nn_socket(AF_SP, NN_PULL);
  if(sock == -1) {
    printf("error: cannot create the runner pub sock.\n");
    return false;
  }

  int rc = 0;
  printf("address: %s\n", sc->address.c_str());
  rc = nn_bind(sock, sc->address.c_str());
  if(rc < 0) {
    printf("error: cannot bind the pub sock (%d): %s\n", rc, nn_strerror(errno));
    return false;
  }

  // copy public settings
  VideoSettings video_settings;
  video_settings.width = sc->width;
  video_settings.height = sc->height;
  video_settings.fps = sc->fps;
  streamer.setVideoSettings(video_settings);

  if(sc->usesAudio()) {
    AudioSettings audio_settings;
    audio_settings.samplerate = sc->samplerate;
    audio_settings.mode = sc->mode;
    audio_settings.bitsize = sc->bitsize;
    audio_settings.quality = sc->quality;
    audio_settings.bitrate = sc->bitrate;
    audio_settings.in_bitsize = sc->in_bitsize;
    audio_settings.in_interleaved = sc->in_interleaved;
    streamer.setAudioSettings(audio_settings);
  }

  ServerSettings server_settings;
  server_settings.url = sc->rtmp_url;
  streamer.setServerSettings(server_settings);

  //streamer.setOutputFile("runner.flv");

  if(!streamer.setup()) {
    printf("error: cannot setup the streamer.\n");
    return false;
  }

  return true;
}

bool Runner::initialize() {

  if(must_run) {
    printf("error: cannot initialize the runner as it's already initialized.\n");
    return false;
  }

  if(!streamer.start()) {
    printf("error: cannot start the streamer.\n");
    return false;
  }

  must_run = true;
  uv_thread_create(&thread, runner_thread, this);
  return true;
}

void Runner::handleCommand(uint8_t type, uint32_t nbytes, std::vector<uint8_t>& buffer) {
  switch(type) {
    default: { 
      printf("error: unknown command.\n");
      break;
    }
  };
}
