#include <streamer/videostreamer/MultiVideoStreamer.h>
#include <streamer/core/Log.h>
#include <streamer/core/Debug.h>

// -----------------------------------------------------------

MultiStreamerInfo::MultiStreamerInfo()
  :audio_encoder(NULL)
  ,streamer(NULL)
  ,id(-1)
{
}

// -----------------------------------------------------------

MultiVideoStreamer::MultiVideoStreamer() {
}

MultiVideoStreamer::~MultiVideoStreamer() {
  if(streamers.size()) {
    shutdown();
  }
}

bool MultiVideoStreamer::loadSettings(std::string filepath) {

  if(!filepath.size()) {
    STREAMER_ERROR("Error: invalid filepath for MultiVideoStreamer.\n");
    return false;
  }

  if(streamers.size()) {
    STREAMER_ERROR("It seems that you've already loaded and setup the MultiVideoStreamer - we're not initializing again.\n");
    return false;
  }  

  if(!config.load(filepath)) {
    STREAMER_ERROR("Something went wrong while loading the configuration for the multi video streamer.\n");
    return false;
  }

  STREAMER_VERBOSE("Loading MultiVideoStreamer config file: %s\n", filepath.c_str());
  return true;
}

bool MultiVideoStreamer::setup() {
  
  if(!config.size()) {
    STREAMER_ERROR("Cannot setup the MultiVideoStreamer because we haven't loaded the configuration. Did you call loadSettings()? \n");
    return false;
  }

  for(size_t i = 0; i < config.size(); ++i) {

    StreamerConfiguration* cfg = config[i];
    if(!cfg) {
      STREAMER_ERROR("The VideoStreamerConfig returned an invalid configuration. Stopping now!\n");
      ::exit(EXIT_FAILURE);
    }

    MultiStreamerInfo* msi = new MultiStreamerInfo();

    if(cfg->audio.codec_id == AV_AUDIO_CODEC_MP3) {
      msi->audio_encoder = new AudioEncoderMP3();
    }
    else if(cfg->audio.codec_id == AV_AUDIO_CODEC_AAC) {
      msi->audio_encoder = new AudioEncoderFAAC();
    }
    else {
      STREAMER_ERROR("Error, we cannot find a suitable encoder for the given audio codec id: %d, use one of the AV_AUDIO_CODEC_{MP3, AAC, ..} values. \n");
      ::exit(EXIT_FAILURE);
    }

    msi->streamer = new VideoStreamer(*msi->audio_encoder);
    msi->streamer->setServerSettings(cfg->server);
    msi->streamer->setAudioSettings(cfg->audio);
    msi->streamer->setVideoSettings(cfg->video);
    msi->streamer->setStreamID(cfg->id);
    msi->id = cfg->id;
    
    if(!msi->streamer->setup()) {
      STREAMER_ERROR("Cannot setup a streamer instance for stream id: %d, stopping now.\n", msi->id);
      ::exit(EXIT_FAILURE);
    }

    streamers.push_back(msi);
  }

  return true;
}

// creates the VideoStreamer instances
bool MultiVideoStreamer::start() {

  if(!config.size()) {
    STREAMER_ERROR("Cannot start the MultiVideoStreamer because you haven't successfully set it up!\n");
    return false;
  }

  if(!streamers.size()) {
    STREAMER_ERROR("Cannot start the MultiVideoStreamer because we couldn't find any streamer instances. Did you call setup()?\n");
    return false;
  }

  for(size_t i = 0; i < streamers.size(); ++i) {

    MultiStreamerInfo* msi = streamers[i];
    if(!msi->streamer->start()) {
      STREAMER_ERROR("Cannot start a streamer instance for stream id: %d, stopping now.\n", msi->id);
      ::exit(EXIT_FAILURE);
    }
  }
  return true;

}

void MultiVideoStreamer::addVideo(AVPacket* pkt) {

#if !defined(NDEBUG)
  if(!pkt) {
    STREAMER_ERROR("Invald packet given to the MultiVideoStreamer::addVideo(). Stopping now.\n");
    ::exit(EXIT_FAILURE);
  }
#endif

  pkt->addRef(streamers.size()-1); // - 1 because the memory pool adds a refcount of 1 when you asked for a free packet 
  
  // add the packet to all streamers
  for(std::vector<MultiStreamerInfo*>::iterator it = streamers.begin(); it != streamers.end(); ++it) {
    MultiStreamerInfo* msi = *it;
    msi->streamer->addVideo(pkt);
  }

}

void MultiVideoStreamer::addAudio(AVPacket* pkt) {

#if !defined(NDEBUG)
  if(!pkt) {
    STREAMER_ERROR("Invald packet given to the MultiVideoStreamer::addAudio(). Stopping now.\n");
    ::exit(EXIT_FAILURE);
  }
#endif

  pkt->addRef(streamers.size()-1); // - 1 because the memory pool adds a refcount of 1 when you asked for a free packet 

  // add the packet to all streamers
  for(std::vector<MultiStreamerInfo*>::iterator it = streamers.begin(); it != streamers.end(); ++it) {
    MultiStreamerInfo* msi = *it;
    msi->streamer->addAudio(pkt);
  }

}

bool MultiVideoStreamer::stop() {

  bool result = true;

  for(std::vector<MultiStreamerInfo*>::iterator it = streamers.begin(); it != streamers.end(); ++it) {
    MultiStreamerInfo* msi = *it;
    if(!msi->streamer->stop()) {
      result = false;
    }
  }

  return result;
}

void MultiVideoStreamer::shutdown() {

  if(!streamers.size()) {
    STREAMER_ERROR("Trying ot shutdown the MultiVideoStreamer, but it seems like no streamer instances were created. Did setup() returned success?\n");
    return;
  }

  for(size_t i = 0; i < streamers.size(); ++i) {
    MultiStreamerInfo* msi = streamers[i];

    delete msi->streamer;
    msi->streamer = NULL;
    msi->id = -1;
    
    if(msi->audio_encoder) {
      delete msi->audio_encoder;
      msi->audio_encoder = NULL;
    }

    delete msi;
    msi = NULL;
  }

  streamers.clear();
}

void MultiVideoStreamer::print() {

  if(!config.size()) {
    STREAMER_WARNING("Cannot print MultiVideoStreamer information because we haven't loaded any configuration yet.\n");
    return;
  }

  for(size_t i = 0; i < config.size(); ++i) {

    StreamerConfiguration* cfg = config[i];
    if(!cfg) {
      STREAMER_ERROR("VideoStreamerConfig returned an invalid StreamerConfiguration in MultiVideoStreamer. Stopping now.\n");
      ::exit(EXIT_FAILURE);
    }

    STREAMER_VERBOSE("[%d] video: %d x %d @ %d, %d kbps, audio: %s Hz, %s, %s, %d kbps. \n", cfg->id, 
                     cfg->video.width, cfg->video.height, cfg->video.fps, cfg->video.bitrate,
                     av_audio_samplerate_to_string(cfg->audio.samplerate).c_str(), 
                     av_audio_mode_to_string(cfg->audio.mode).c_str(),
                     av_audio_bitsize_to_string(cfg->audio.bitsize).c_str(),
                     cfg->audio.bitrate
                     );
                                        
  }
}
