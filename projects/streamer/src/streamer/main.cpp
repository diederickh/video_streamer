extern "C" {
#  include <uv.h>
}

#include <sstream>
#include <signal.h>
#include <stdio.h>
#include <fstream>
#include <streamer/amf/AMF0.h>
#include <streamer/core/BitStream.h>
#include <streamer/core/AudioEncoder.h>
#include <streamer/core/VideoEncoder.h>
#include <streamer/core/TestPattern.h>
#include <streamer/core/H264Parser.h>
#include <streamer/core/RTMPWriter.h>
#include <streamer/core/EncoderThread.h>
#include <streamer/core/RTMPThread.h>
#include <streamer/flv/FLVReader.h>
#include <streamer/flv/FLVWriter.h>
#include <streamer/videostreamer/VideoStreamer.h>
#include <streamer/videostreamer/VideoStreamerConfig.h>
#include <streamer/daemon/Daemon.h>
#include <streamer/daemon/Channel.h>
#include <tinylib/tinylib.h>
#include <rapidxml.hpp>
using namespace rapidxml;

#if defined(USE_GRAPH)
# include <streamer/utils/Graph.h>
#endif

#define TEST_STREAMER_XMLCONFIG 1 /* tests loading a xml file for The VideoStreamer instead of providing the objects manually */
#define TEST_DAEMON 0
#define TEST_VIDEOSTREAMER 0
#define TEST_RTMP_WRTIER 0
#define TEST_FLV_READER 0
#define TEST_FLV_WRITER 0
#define TEST_X264_WRITER 0
#define TEST_AMF0_METADATA 0
#define TEST_H264_PARSER 0
#define TEST_AUDIO_GENERATOR 0
#define TEST_MP3_ENCODER 0
#define USE_AUDIO 1

bool must_run = false;
void signal_handler(int p);

int main() {

  // generic signal handler
  signal(SIGINT, signal_handler);
  must_run = true;

#if TEST_H264_PARSER
  std::vector<uint8_t> data;
  data.push_back(0x01);
  H264Parser hp(&data.front());
  hp.parse();
#endif

#if TEST_FLV_WRITER
  VideoSettings vs;
  vs.width = 320;
  vs.height = 240;
  vs.fps = 25;

  AudioSettings as;
  as.samplerate = AV_AUDIO_SAMPLERATE_44100;
  as.mode = AV_AUDIO_MODE_STEREO;
  as.bitsize = AV_AUDIO_BITSIZE_S16;
  as.bitrate = 320;

  BitStream flv_bs;
  FLVWriter fw(flv_bs);
  fw.setVideoCodec(FLV_VIDEOCODEC_AVC);
  fw.setWidth(vs.width);
  fw.setHeight(vs.height);
  fw.setFrameRate(vs.fps);
  fw.setCanSeekToEnd(false);

#if USE_AUDIO
  fw.setAudioCodec(FLV_SOUNDFORMAT_MP3);
  fw.setAudioSampleRate(FLV_SOUNDRATE_44KHZ);
  fw.setAudioDataRate(as.bitrate);
  fw.setAudioSize(FLV_SOUNDSIZE_16BIT);
  fw.setAudioType(FLV_SOUNDTYPE_STEREO);
#endif

  VideoEncoder ve;
  if(!ve.setup(vs)) {
    printf("error: cannot setup the VideoEncoder");
    return EXIT_FAILURE;
  }

  if(!ve.initialize()) {
    printf("error: cannot intialize the VideoEncoder");
    return EXIT_FAILURE;
  }
    
#if USE_AUDIO
  AudioEncoder ae;
  if(!ae.setup(as)) {
    printf("error: cannot setup the audio encoder.\n");
    return 0;
  }
    
  if(!ae.initialize()) {
    printf("error: cannot initialize the audio encoder.\n");
    return 0;
  }
#endif

  AVCDecoderConfigurationRecord avc;
  ve.createDecoderConfigurationRecord(avc);
  fw.setDecoderConfigurationRecord(avc);

  if(!fw.open()) {
    printf("error: cannot open the FLV. We probably forgot to setup some specs.\n");
  }

  std::vector<int16_t> audio_samples;
  FLVTag flv_tag;
  AVPacket pkt;
  TestPattern tp;
  tp.setup(vs.width, vs.height, vs.fps);
  pkt.allocate(vs.width * vs.height + (vs.width * vs.height / 4) * 2);

  for(int i = 0; i < 250; ++i) {
    tp.update();
    tp.generateVideoFrame(pkt.data);

    ve.encodePacket(&pkt, flv_tag);
    fw.writeVideoTag(flv_tag);

    pkt.data.clear();
#if USE_AUDIO
    int num_samples = tp.generateAudioFrame(pkt.data);
    ae.encodePacket(&pkt, flv_tag);
    flv_tag.setTimeStamp(tp.duration * 1000); 
    fw.writeAudioTag(flv_tag);
#endif
  }

  fw.close();

  if(!flv_bs.saveFile("raw.flv")) {
    printf("error: cannot save the test flv.\n");
  }

  // read again..
  BitStream bs_reader;
  bs_reader.loadFile("raw.flv");

  FLVReader flv_reader(bs_reader);
  flv_reader.parse();
#endif

#if TEST_AMF0_METADATA

  // test using AMF0
  BitStream amf0_bs;
  AMF0 amf(amf0_bs);

  AMF0Number* n = amf.createNumber(10);
  amf.writeNumber(n);
  delete n;

  n = amf.readNumber(); 
  n->print();
  delete n;

  AMF0String* str = amf.createString("just some string");
  amf.writeString(str);
  delete str;

  str = amf.readString();
  str->print();
  delete str;

  AMF0Boolean* b = amf.createBoolean(false);
  amf.writeBoolean(b);
  delete b;

  b = amf.readBoolean();
  b->print();
  delete b;

  AMF0EcmaArray* array = amf.createEcmaArray();
  array->add("duration", amf.createNumber(10.0));
  array->add("width", amf.createNumber(320));
  array->add("height", amf.createNumber(240));
  array->add("codec", amf.createNumber(1));
  
  amf.writeEcmaArray(array);
  delete array;

  array = amf.readEcmaArray();
  array->print();
  delete array;

  printf("size of bs.: %ld\n", amf0_bs.size());

#endif

#if TEST_FLV_READER

  class ReaderListener : public FLVListener {
  public:
    std::stringstream ss_video_fps;
    uint32_t last_timestamp;
    uint32_t timediff;
    std::ofstream ofs_video_fps;
    uint32_t total;
    uint32_t num;

    ReaderListener() {
      total = 0;
      num = 0;
      ofs_video_fps.open("reader_video_fps.data", std::ios::out);
      if(!ofs_video_fps.is_open()) {
        printf("cannot open video_fps.data.\n");
        ::exit(EXIT_FAILURE);
      }
    }

    ~ReaderListener() {
      ofs_video_fps << ss_video_fps.rdbuf();
      ofs_video_fps.close();
    }

    void onSignature(BitStream& bs) {
      last_timestamp = 0;
      timediff = 0;
    }

    void onTag(BitStream& bs, FLVTag& tag) {
      if(tag.tag_type != FLV_TAG_TYPE_VIDEO) {
        return;
      }
      if(last_timestamp) {
        timediff = tag.timestamp - last_timestamp;
        total += timediff;
      }
      if(num >= 30) {
        uint32_t avg = total / num;
        total = 0; 
        num = 0;
        ss_video_fps << avg << std::endl;        
      }


      num++;
      printf("diff: %d\n", timediff);
      last_timestamp = tag.timestamp;
      if(tag.data_size) {
        std::copy(tag.video_data.begin(), tag.video_data.end(), std::back_inserter(data));
      }
    }
    std::vector<uint8_t> data;
  };
  
  ReaderListener reader_listener;

  BitStream bs;
  //bs.loadFile("./barsandtone.flv");
  //bs.loadFile("./dump.flv");
  bs.loadFile("./fmle_dump4.flv");
  //bs.loadFile("./raw.flv");
  //bs.loadFile("./berlin.flv");
  FLVReader flv(bs);
  flv.addListener(&reader_listener);
  flv.parse();

  printf("reader_listener.data.size() = %ld\n", reader_listener.data.size());
  if(reader_listener.data.size()) {
    std::ofstream ofs("extracted_h264_from_dump.h264", std::ios::binary | std::ios::out);
    if(!ofs.is_open()) {
      printf("error: cannot open the output file for the h264 dump\n");
      return 0;
    }
    ofs.write((char*)&reader_listener.data.front(), reader_listener.data.size());
    ofs.close();
  }
#endif


#if TEST_X264_WRITER
  VideoSettings vs;
  vs.width = 320;
  vs.height = 240;
  vs.fps = 20;

  VideoEncoder ve;
  if(!ve.setup(vs)) {
    printf("error: cannot setup the VideoEncoder");
    return EXIT_FAILURE;
  }

  if(!ve.initialize()) {
    printf("error: cannot intialize the VideoEncoder");
    return EXIT_FAILURE;
  }

  std::vector<uint8_t> test_frame;
  TestPattern tp;

  tp.openFile("test_frames/raw.yuv");
  tp.setup(vs.width, vs.height, 24);

  
  FLVTag flv_tag;
  AVPacket pkt;
  pkt.allocate(vs.width * vs.height + (vs.width * vs.height / 4) * 2);

  ve.openFile("raw.h264");
  for(int i = 0; i < 540; ++i) {
    tp.update();
    tp.generateVideoFrame(pkt.data);
    tp.writeFrameToFile(pkt.data);
    ve.encodePacket(&pkt, flv_tag);
    ve.writeTagToFile(flv_tag);
  }
  ve.closeFile();
#endif

#if TEST_AUDIO_GENERATOR
  {
    std::vector<int16_t> audio_samples;
    TestPattern tp;
    tp.setup(320,240,25);
    for(int i = 0; i < 50; ++i) {
      tp.update();
      uint32_t num_samples = tp.generateAudioFrame(audio_samples);  
    }

    printf("%ld samples.\n", audio_samples.size());
    std::ofstream ofs("audio_s16.pcm", std::ios::out | std::ios::binary);
    if(!ofs.is_open()) {
      printf("error: cannot open the file for the audio.\n");
      return 0;
    }
    ofs.write((char*)&audio_samples.front(), audio_samples.size() * sizeof(int16_t));
    ofs.close();
    
  }
#endif

#if TEST_MP3_ENCODER
  {
    AudioSettings as;
    as.samplerate = AV_AUDIO_SAMPLERATE_44100;
    as.mode = AV_AUDIO_MODE_STEREO;
    as.bitsize = AV_AUDIO_BITSIZE_S16;
    as.bitrate = 320;
    
    AudioEncoder ae;
    if(!ae.setup(as)) {
      printf("error: cannot setup the audio encoder.\n");
      return 0;
    }
    
    if(!ae.initialize()) {
      printf("error: cannot initialize the audio encoder.\n");
      return 0;
    }

    std::vector<uint8_t> mp3_data;
    std::vector<int16_t> audio_samples;
    AVPacket pkt;
    TestPattern tp;
    FLVTag tag;

    tp.setup(320,240,25);
    for(int i = 0; i < 50; ++i) {

      // generate a raw audio packet.
      tp.update();
      uint32_t num_samples = tp.generateAudioFrame(audio_samples);  
      uint8_t* ptr = (uint8_t*)&audio_samples.front();

      // encode the raw audio using mp3
      ae.encodePacket(&pkt, tag);
      std::copy(tag.data, tag.data + tag.size, std::back_inserter(mp3_data));
      
      pkt.data.clear();
      audio_samples.clear();
    }

    // write the mp3 data
    std::ofstream ofs("raw.mp3", std::ios::out | std::ios::binary);
    if(!ofs.is_open()) {
      printf("error: cannot open the raw.mp3 file to which we want to write encoded audio..\n");
      return 0;
    }
    ofs.write((char*)&mp3_data.front(), mp3_data.size());
    ofs.close();
  
  }
#endif

#if TEST_RTMP_WRTIER
  { // rtmp_writer 

    // setup rtmp
    RTMPWriter rtmp;
    rtmp.setURL("rtmp://localhost/flvplayback/livestream");

    if(!rtmp.initialize()) {
      printf("error: cannot init rtmpwriter.\n");
      return 0;
    }

    // setup audio encoder
#if USE_AUDIO
    AudioSettings as;
    as.samplerate = AV_AUDIO_SAMPLERATE_44100;
    as.mode = AV_AUDIO_MODE_STEREO;
    as.bitsize = AV_AUDIO_BITSIZE_S16;
    as.bitrate = 320;

    AudioEncoder ae;
    if(!ae.setup(as)) {
      printf("error: cannot setup the audio encoder.\n");
      return 0;
    }
    
    if(!ae.initialize()) {
      printf("error: cannot initialize the audio encoder.\n");
      return 0;
    }
#endif    

    // setup encoder 
    VideoSettings vs;
    vs.width = 320;
    vs.height = 240;
    vs.fps = 25;

    VideoEncoder ve;
    if(!ve.setup(vs)) {
      printf("error: cannot setup the VideoEncoder");
      return EXIT_FAILURE;
    }

    if(!ve.initialize()) {
      printf("error: cannot intialize the VideoEncoder");
      return EXIT_FAILURE;
    }

    AVCDecoderConfigurationRecord avc;
    ve.createDecoderConfigurationRecord(avc);

    // setup flv muxer
    BitStream flv_bs;
    FLVWriter flv(flv_bs);
    flv.setVideoCodec(FLV_VIDEOCODEC_AVC);
    flv.setWidth(vs.width);
    flv.setHeight(vs.height);
    flv.setFrameRate(vs.fps);
    flv.setCanSeekToEnd(false);
    flv.setDecoderConfigurationRecord(avc);

#if USE_AUDIO
    flv.setAudioCodec(FLV_SOUNDFORMAT_MP3);
    flv.setAudioSampleRate(FLV_SOUNDRATE_44KHZ);
    flv.setAudioDataRate(as.bitrate);
    flv.setAudioSize(FLV_SOUNDSIZE_16BIT);
    flv.setAudioType(FLV_SOUNDTYPE_STEREO);
#endif

    // setup test pattern generator
    TestPattern tp;
    tp.setup(vs.width, vs.height, vs.fps);
    tp.update();

    // the rtmp writer thread
    RTMPThread rtmp_thread(flv, rtmp);
    rtmp_thread.start();

    // the encoder thread
    EncoderThread enc(flv, ve, ae);
    enc.start();

    if(!flv.open()) {
      printf("error: cannot open the flv muxer.\n");
    }

    if(!flv.open()) {
      printf("error: cannot open the flv muxer.\n");
    }

    uint64_t start_time = uv_hrtime() / 1000000;
    uint32_t curr_time = 0;
    tp.start();
    while(must_run) {
      tp.update();

      // generate a test video packet.
      AVPacket* vid_pkt = new AVPacket();
      vid_pkt->allocate(vs.width * vs.height + (vs.width * vs.height / 4) * 2);      
      tp.generateVideoFrame(vid_pkt->data);
      curr_time = uint32_t((uv_hrtime() / 1000000) - start_time);
      vid_pkt->makeVideoPacket();
      vid_pkt->setTimeStamp(curr_time);
      enc.addPacket(vid_pkt);

      // generate a test audio packet
      AVPacket* audio_pkt = new AVPacket();
      tp.generateAudioFrame(audio_pkt->data);
      audio_pkt->makeAudioPacket();
      audio_pkt->setTimeStamp(curr_time);
      enc.addPacket(audio_pkt);

      usleep(1000 * ((1.0/vs.fps) * 1000));
    }

    enc.stop();
    rtmp_thread.stop();
    flv.close();
    
    if(!flv_bs.saveFile("raw.flv")) {
      printf("error: cannot save the threaded flv\n");
    }

  } // rtmp_writer
#endif

#if TEST_VIDEOSTREAMER
  {
    VideoSettings vs;
    vs.width = 320;
    vs.height = 240;
    vs.fps = 25;

    AudioSettings as;
    ///as.samplerate = AV_AUDIO_SAMPLERATE_22050;
    as.samplerate = AV_AUDIO_SAMPLERATE_44100;
    as.mode = AV_AUDIO_MODE_STEREO;
    as.bitsize = AV_AUDIO_BITSIZE_S16;
    as.bitrate = 96;
    as.quality = 6;

    ServerSettings ss;
    ss.url = "rtmp://localhost/flvplayback/livestream";
    //  ss.url = "rtmp://localhost/livestream/livestream";

    VideoStreamer streamer;
#if USE_AUDIO
    streamer.setAudioSettings(as);
#endif
    streamer.setVideoSettings(vs);
    streamer.setServerSettings(ss);

    if(!streamer.setup()) {
      return 0;
    }

    if(!streamer.start()) {
      printf("error: something went wrong when starting the streamer.\n");
      return 0;
    }

    uint64_t start_time = uv_hrtime() / 1000000;
    uint32_t curr_time = 0;

    TestPattern tp;
    tp.setup(vs.width, vs.height, vs.fps, as.samplerate);
    tp.start();

#if defined(USE_GRAPH)
    network_graph.yrange = "0:20000";
    network_graph.ytics = "1000";
    network_graph.max_values = 300;
    frames_graph.yrange = "-5:75";
    frames_graph.ytics = "5";
    frames_graph.max_values = 300;
    

    network_graph["h264"].setLineStyle(2, 1, 1, "red");
    network_graph["rtmp"].setLineStyle(2, 1, 2, "orange");
    network_graph["mp3"].setLineStyle(2, 1,3, "green");
    frames_graph["video_frames"].setLineStyle(2,1,1,"blue");
    frames_graph["audio_frames"].setLineStyle(2,1,2,"green");
    frames_graph["enc_audio"].setLineStyle(3,1,3,"orange");
    frames_graph["enc_video"].setLineStyle(3,1,4,"red");
    frames_graph["enc_audio_video"].setLineStyle(3,1,5,"cyan");

    network_graph["h264"].setMode(GRAPH_MODE_ABS);
    network_graph["rtmp"].setMode(GRAPH_MODE_ABS);
    network_graph["mp3"].setMode(GRAPH_MODE_ABS);
    frames_graph["video_frames"].setMode(GRAPH_MODE_ABS);
    frames_graph["audio_frames"].setMode(GRAPH_MODE_ABS);
    frames_graph["enc_audio"].setMode(GRAPH_MODE_ABS);
    frames_graph["enc_video"].setMode(GRAPH_MODE_ABS);
    frames_graph["enc_audio_video"].setMode(GRAPH_MODE_ABS);
    frames_graph.start((1.0/vs.fps) * 1000);
    network_graph.start(1000);
#endif

      uint64_t last_time = 0;
      uint64_t time_diff = 0;
      uint64_t total_diffs = 0;
      uint64_t num_frames = 0;
      
    
    while(must_run) {
#if defined(USE_GRAPH)
      frames_graph.update();
      network_graph.update();
#endif

      tp.update();
      if(tp.hasVideoFrame()) {

        curr_time = uint32_t((uv_hrtime() / 1000000) - start_time);
        if(last_time) {
#if defined(USE_GRAPH)
          frames_graph["video_frames"] += (curr_time - last_time) ;
#endif        
          // printf("> %lld\n", curr_time - last_time);
          total_diffs += curr_time - last_time;
          num_frames++;
        }
        printf("verbose: test use of streamer.getTimeStamp()!\n");
        last_time = curr_time;
        AVPacket* vid_pkt = new AVPacket();
        vid_pkt->allocate(vs.width * vs.height + (vs.width * vs.height / 4) * 2);      
        tp.generateVideoFrame(vid_pkt->data);
        vid_pkt->makeVideoPacket();
        vid_pkt->setTimeStamp(curr_time);
        //vid_pkt->setTimeStamp(streamer.getTimeStamp());
        streamer.addVideo(vid_pkt);
      }  

#if USE_AUDIO
      if(tp.hasAudioFrame()) {

#if defined(USE_GRAPH)
        frames_graph["audio_frames"] += 1;
#endif        
        printf("verbose: test use of streamer.getTimeStamp()!\n");
        curr_time = uint32_t((uv_hrtime() / 1000000) - start_time);
        AVPacket* audio_pkt = new AVPacket();
        tp.generateAudioFrame(audio_pkt->data);
        audio_pkt->makeAudioPacket();
        audio_pkt->setTimeStamp(curr_time);
        //audio_pkt->setTimeStamp(streamer.getTimeStamp());
        streamer.addAudio(audio_pkt);
      }
#endif

    } // while(must_run)


    if(!streamer.stop()) {
      printf("error: something went wrong when stopping the streamer.\n");
    }

#if defined(USE_GRAPH)    
    network_graph.save("network");
    frames_graph.save("frames");
#endif
    printf("Total frames: %lld, Total diffs: %lld, Avarage diffs: %lld\n", num_frames, total_diffs, (total_diffs/num_frames));
  }


#endif

#if TEST_DAEMON
  printf("testing daemon.\n");

  // get exe path
#if defined(_WIN32)
  // ok.. a bit too much ...
  char exe_path[1024];
  char config_path[1024];
  size_t len = 1024; 
  char drive[512];
  char dir[1024];
  char fname[1024];
  char exe[512];
  uv_exepath(exe_path, (size_t*)&len);
  _splitpath(exe_path, drive, dir, fname, exe);
  sprintf(config_path, "%s%s\config.xml", drive, dir);
/*
  std::ifstream ifs(config_path);
  std::stringstream xbuf;
  xbuf << ifs.rdbuf();
  ifs.close();

  std::string xml = xbuf.str();
  xml_document<> doc;
  doc.parse<0>(&xml.front());
xml_node<>* root = doc.first_node();
printf("%s\n", root->name());

  printf("config: %s\n", config_path);
*/
#else
  std::string config_path = "/Users/roxlu/Documents/programming/cpp/video_streamer/install/bin/config.xml";
#endif

  Daemon d;
  if(!d.setup(config_path)) {
    printf("error: cannot setup daemon\n");
    ::exit(EXIT_FAILURE);
  }

  if(!d.initialize()) {
    printf("error: cannot initialize the daemon.\n");
    ::exit(EXIT_FAILURE);
  }

  Channel channel;
  if(!channel.setup(0, "tcp://127.0.0.1:1234")) {
    printf("error: cannot setup the channel.\n");
    ::exit(EXIT_FAILURE);
  }
  if(!channel.initialize()) {
    printf("error: cannot initialize the channel.\n");
    ::exit(EXIT_FAILURE);
  }

  VideoSettings vs;
  vs.width = 320;
  vs.height = 240;
  vs.fps = 15;

  AudioSettings as;
  as.samplerate = AV_AUDIO_SAMPLERATE_44100;
  as.mode = AV_AUDIO_MODE_STEREO;
  as.bitsize = AV_AUDIO_BITSIZE_S16;
  as.bitrate = 320;

  TestPattern tp;
  tp.setup(vs.width, vs.height, vs.fps, 44100);

  std::vector<uint8_t> frame_data;

#if DAEMON_SAVE_FILE  
  std::ofstream ofs("send.dat", std::ios::out | std::ios::binary);
  if(!ofs.is_open()) {
    printf("Cannot open send.dat.\n");
    ::exit(EXIT_FAILURE);
  }
#endif

  tp.start();
  while(must_run) {
    tp.update();
    if(tp.hasVideoFrame()) {
      int64_t n = uv_hrtime();
      {
        tp.generateVideoFrame(frame_data);
        printf(">> timestamp: %d, size: %ld\n", tp.timestamp, frame_data.size());
        channel.addVideoPacket(frame_data, tp.timestamp);
#if DAEMON_SAVE_FILE
        ofs.write((char*)&frame_data.front(), frame_data.size());
#endif
      }
      int64_t d = uv_hrtime() - n;
      
      //printf("ts: %d, size: %ld, diff: %lld - %f\n", tp.timestamp, p->data.size(), d, d/1000000.0f);
    }
  }
#if DAEMON_SAVE_FILE
  ofs.close();
#endif
  
#endif

#if TEST_STREAMER_XMLCONFIG
  std::string exe_dir = rx_get_exe_path();

  VideoStreamerConfig vconf;
  if(!vconf.load(exe_dir +"videostreamer_config_example.xml")) {
    printf("error: cannot load the example streamer configuration.\n");
    ::exit(EXIT_FAILURE);
  }

  StreamerConfiguration* sc = vconf.getByID(0);
  if(!sc) {
    printf("error: cannot find streamer config for id = 0.\n");
    ::exit(EXIT_FAILURE);
  }

  if(!sc->validate()) {
    printf("error: cannot validate the configs.\n");
  }

  sc->print();
#endif // TEST_STREAMER_XMLCONFIG

  return 1;
}


void signal_handler(int p) {
  printf("verbose: receive signal, stop rtmp writer.\n");
  must_run = false;  
}
