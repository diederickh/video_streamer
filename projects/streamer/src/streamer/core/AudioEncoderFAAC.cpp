#include <assert.h>
#include <streamer/core/AudioEncoderFAAC.h>
#include <iterator>
#include <algorithm>

#if FAAC_ENCODER_MEASURE_BITRATE
extern "C" {
#  include <uv.h>
}
#endif

AudioEncoderFAAC::AudioEncoderFAAC() 
  :encoder(NULL)
  ,nsamples_needed(0)
  ,nbytes_out(0)
  ,faac_buffer(NULL)
#if FAAC_ENCODER_MEASURE_BITRATE
  ,kbps_timeout(0)
  ,kbps_delay(1000 * 1000 * 1000)
  ,kbps_nbytes(0)
  ,kbps_time_started(0)
  ,kbps(0.0)
#endif
{
}

AudioEncoderFAAC::~AudioEncoderFAAC() {

  if(encoder) {
    shutdown();
  }

}


bool AudioEncoderFAAC::setup(AudioSettings s) {

  if(!validateSettings(s)) {
    return false;
  }

  settings = s;
  
  return true;
}

bool AudioEncoderFAAC::initialize() {

  if(encoder) {
    STREAMER_ERROR("It seems that you already initialized the AudioEncoderFAAC. I'm not initializing again. First shutdown().");
    return false;
  }

  if(!validateSettings(settings)) {
    return false;
  }

  encoder = faacEncOpen(settings.samplerate, 
                        getNumChannels(), 
                        &nsamples_needed, 
                        &nbytes_out);

  if(!encoder) {
    STREAMER_ERROR("Error while trying to open the FAAC encoder.\n");
    return false;
  }

  faac_buffer = new unsigned char[nbytes_out];
  if(!faac_buffer) {
    STREAMER_ERROR("Erorr while allocating the buffer for the encoded faac data.\n");
    shutdown();
    return false;
  }

  STREAMER_STATUS("FAAC, nsamples_needed: %ld, nbytes_out: %ld\n", nsamples_needed, nbytes_out);

  // configure faac
  faacEncConfigurationPtr config = faacEncGetCurrentConfiguration(encoder);
  config->mpegVersion = MPEG4;
  config->aacObjectType = LOW;
  config->useLfe = 0;
  config->quantqual = 100; 
  config->bitRate = settings.bitrate * 1000 / getNumChannels();
  config->outputFormat = 0; // 0 = Raw, 1 = ADTS

  if(settings.in_bitsize == AV_AUDIO_BITSIZE_S16) {
    config->inputFormat = FAAC_INPUT_16BIT; 
  }
  else if(settings.in_bitsize == AV_AUDIO_BITSIZE_F32) {
    config->inputFormat = FAAC_INPUT_FLOAT;
  }
  else {
    STREAMER_ERROR("Cannot initialize the AudioEncoderFAAC because the in_bitsize is invalid. we only support S16 of F32.\n");
    ::exit(EXIT_FAILURE);
  }

  int r = faacEncSetConfiguration(encoder, config);
  if(!r) {
    STREAMER_ERROR("Error while setting the configuration for faac.\n");
    shutdown();
    return false;
  }


  // get the decoder specific config
  unsigned char* header;
  unsigned long header_size = 0;
  r = faacEncGetDecoderSpecificInfo(encoder, &header, &header_size);
  if(r < 0) {
    STREAMER_ERROR("Error while trying to get the decoder specific info from libfaac.\n");
    shutdown();
    return false;
  }
  std::copy(header, header + header_size, std::back_inserter(audio_specific_config));

  free(header);
  header = NULL;

  // write data to file if output file has been specified
  if(output_file.size()) {
    ofs.open(output_file.c_str(), std::ios::out | std::ios::binary);
    if(!ofs.is_open()) {
      STREAMER_ERROR("Cannot open the output file for the AudioEncoderFAAC: %s\n", output_file.c_str());
      ::exit(EXIT_FAILURE);
    }
    ofs.write((char*)header, header_size);
  }
  
  STREAMER_STATUS("Bitrate in bits per channel: %d, decoder size: %ld\n", config->bitRate, header_size);

  return true;
}

bool AudioEncoderFAAC::encodePacket(AVPacket* p, FLVTag& tag) {
  assert(encoder);
  // @todo in AudioEncoderFAAC we might want to check if the given packet contains enough bytes using the nsamples_needed member
  // assert(p->data.size() == nsamples_needed);
  
  int written = faacEncEncode(encoder, (int32_t*)&p->data.front(), nsamples_needed, faac_buffer, nbytes_out);
  
  if(written) {
    if(output_file.size() && ofs.is_open()) {
      ofs.write((char*)faac_buffer, written);
    }
  }

  if(written < 0) {
    STREAMER_ERROR("The faac encoder returned an error while encoding the input data: %d\n", written);
    ::exit(EXIT_FAILURE);
  }

  tag.bs.clear();
  tag.bs.putBytes((uint8_t*)faac_buffer, written);
  tag.setData(tag.bs.getPtr(), tag.bs.size());
  tag.makeAudioTag();
  tag.setAACPacketType(FLV_AAC_RAW_DATA);
  tag.setTimeStamp(p->timestamp);

#if FAAC_ENCODER_MEASURE_BITRATE
  uint64_t now = uv_hrtime();
  if(!kbps_time_started) {
    kbps_time_started = now;
  }

  kbps_nbytes += written;

  if(now >= kbps_timeout) {
    double timediff = double(now - kbps_time_started) / (1000 * 1000 * 1000); // in sec
    kbps = ((kbps_nbytes * 8) / timediff) / 1000.0;
    STREAMER_STATUS("-- faac kbps: %02.2f\n", kbps);
    kbps_timeout = now + kbps_delay;
  }
#endif

  return true;
}

bool AudioEncoderFAAC::shutdown() {

  if(encoder) {
    faacEncClose(encoder);
    encoder = NULL;
  }

  if(faac_buffer) {
    delete[] faac_buffer;
    faac_buffer = NULL;
  }

  if(output_file.size()) {
    if(ofs.is_open()) {
      ofs.close();
    }
    output_file.clear();
  }

  nbytes_out = 0;
  nsamples_needed = 0;

  return true;
}

bool AudioEncoderFAAC::validateSettings(AudioSettings& s) {
  if(s.in_bitsize != AV_AUDIO_BITSIZE_S16 && s.in_bitsize != AV_AUDIO_BITSIZE_F32) {
    STREAMER_ERROR("Cannot setup the AudioEncoderFAAC use either AV_AUDIO_BITSIZE_S16 or AV_AUDIO_BITSIZE_F32 for the in_bitsize.\n");
    return false;
  }

  if(!s.bitrate) {
    STREAMER_ERROR("Cannot setup the AudioEncoderFAAC becuse the bitrate is invalid: %d\n", s.bitrate);
    return false;
  }

  /*
  if(s.bitsize != s.in_bitsize) {
    STREAMER_ERROR("Cannot setup AudioEncoderFAAC because the given in_bitsize is invalid.\n");
    return false;
  }
  */

  if(s.samplerate == AV_AUDIO_SAMPLERATE_UNKNOWN) {
    STREAMER_ERROR("Cannot setup AudioEncoderFAAC because the given samplerate is invalid.\n");
    return false;
  }

  return true;
}

void AudioEncoderFAAC::print() {

  if(!encoder) {
    return;
  }

  faacEncConfigurationPtr config = faacEncGetCurrentConfiguration(encoder);
  STREAMER_VERBOSE("faac.version: %d\n", config->version);
  STREAMER_VERBOSE("faac.name: %s\n", config->name);
  STREAMER_VERBOSE("faac.copyright: %s\n", config->copyright);
  STREAMER_VERBOSE("faac.mpegVersion: %s\n", faac_debug_mpeg_version_to_string(config->mpegVersion).c_str());
  STREAMER_VERBOSE("faac.aacObjectType: %s\n", faac_debug_object_type_to_string(config->aacObjectType).c_str());
  STREAMER_VERBOSE("faac.allowMidside: %d\n", config->allowMidside);
  STREAMER_VERBOSE("faac.useLfe: %d\n", config->useLfe);
  STREAMER_VERBOSE("faac.useTns: %d\n", config->useTns);
  STREAMER_VERBOSE("faac.bitRate: %ld (in bits per channel, not kbps)\n", config->bitRate);
  STREAMER_VERBOSE("faac.bandWidth: %d\n", config->bandWidth);
  STREAMER_VERBOSE("faac.quantqual: %ld\n", config->quantqual);
  STREAMER_VERBOSE("faac.outputFormat: %s\n", faac_debug_output_format_to_string(config->outputFormat).c_str());
}

// ------------------------------------------------------------------------------

std::string faac_debug_mpeg_version_to_string(int v) {
  if(v == MPEG2) {
    return "MPEG2";
  }
  else if(v == MPEG4) {
    return "MPEG4";
  }
  else {
    return "UNKNOWN";
  }
}

std::string faac_debug_object_type_to_string(int v) {
  switch(v) {
    case MAIN: return "MAIN";
    case LOW:  return "LOW";
    case SSR:  return "SSR";
    case LTP:  return "LTP";
    default:   return "UNKNOWN";
  }
}

std::string faac_debug_input_format_to_string(int v) {
  switch(v) {
    case FAAC_INPUT_NULL:  return "FAAC_INPUT_NULL"; 
    case FAAC_INPUT_16BIT: return "FAAC_INPUT_16BIT";
    case FAAC_INPUT_24BIT: return "FAAC_INPUT_24BIT";
    case FAAC_INPUT_32BIT: return "FAAC_INPUT_32BIT";
    default: return "UNKNOWN";
  }
}

std::string faac_debug_output_format_to_string(int v) {
  if(v == 0) {
    return "RAW";
  }
  else if(v == 1) {
    return "ADTS";
  }
  else {
    return "UNKNOWN";
  }
}
