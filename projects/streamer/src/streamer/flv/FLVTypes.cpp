#include <streamer/flv/FLVTypes.h>
#include <stdio.h>

// ---------------------------------------------------------

double flv_samplerate_to_double(uint8_t r) {
  switch(r) {
    case FLV_SOUNDRATE_5_5KHZ: return 5500.0; 
    case FLV_SOUNDRATE_11KHZ: return 11025.0;
    case FLV_SOUNDRATE_22KHZ: return 22050.0; 
    case FLV_SOUNDRATE_44KHZ: return 44100.0;
    default: return 0.0;
  }
}

// ---------------------------------------------------------

AVCDecoderConfigurationRecord::AVCDecoderConfigurationRecord()
  :configuration_version(1)
  ,avc_profile_indication(0)
  ,profile_compatibility(0)
  ,avc_level_indication(0)
  ,nal_size_length_minus_one(0)
  ,number_of_sps(0)
  ,number_of_pps(0)
{
}

// ---------------------------------------------------------

FLVHeader::FLVHeader()
  :version(0)
  ,type_flags_audio(0)
  ,type_flags_video(0)
  ,data_offset(0)
{
  tag[0] = tag[1] = tag[2] = 0x00;
}

void FLVHeader::print() {
  printf("flv_header.tag: %c%c%c\n", tag[0], tag[1], tag[2]);
  printf("flv_header.version: %d\n", version);
  printf("flv_header.type_flags_audio: %d\n", type_flags_audio);
  printf("flv_header.type_flags_video: %d\n", type_flags_video);
  printf("flv_header.data_offset: %d\n", data_offset);
}

// ---------------------------------------------------------

FLVTag::FLVTag()
  :reserved(0)
  ,filter(0)
  ,tag_type(0)
  ,data_size(0)
  ,timestamp(0)
  ,timestamp_extended(0)
  ,stream_id(0)
  ,sound_format(FLV_SOUNDFORMAT_UNKNOWN)
  ,sound_rate(FLV_SOUNDRATE_UNKNOWN)
  ,sound_type(FLV_SOUNDTYPE_UNKNOWN)
  ,sound_size(FLV_SOUNDSIZE_UNKNOWN)
  ,frame_type(FLV_VIDEOFRAME_UNKNOWN)
  ,codec_id(FLV_VIDEOCODEC_UNKNOWN)
  ,avc_packet_type(FLV_AVC_UNKNOWN)
  ,composition_time(0)
  ,data(NULL)
  ,size(0)
{
}

