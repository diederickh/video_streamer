/*

---------------------------------------------------------------------------------
      
                                               oooo              
                                               `888              
                oooo d8b  .ooooo.  oooo    ooo  888  oooo  oooo  
                `888""8P d88' `88b  `88b..8P'   888  `888  `888  
                 888     888   888    Y888'     888   888   888  
                 888     888   888  .o8"'88b    888   888   888  
                d888b    `Y8bod8P' o88'   888o o888o  `V88V"V8P' 
                                                                 
                                                  www.roxlu.com
                                             www.apollomedia.nl  
                                          www.twitter.com/roxlu
              
---------------------------------------------------------------------------------
 
  # FLV Types

  This files contains basic structs that are used for reading and writing
  FLV data. The most important struct is the FLVTag which represents either a
  buffer of encoded audio, encoded video or script data which is used to mux
  a FLV stream.

  _Creating a FLVTag for video data_
  ````c++
  FLVTag tag;
  tag.makeVideoTag();
  ````

  _Creating a FLVTag for audio data_
  ````c++
  FLVTag tag;
  tag.makeVideoTag();
  ````
  
  _Creating a FLVTag for script data_
  ````c++
  FLVTag tag;
  tag.makeScriptTag();
  ````

  By default we set all values to unknown or zero:
  ````c++
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
      ,data(NULL)
      ,size(0)
 {
 }
  ````
 */
#ifndef ROXLU_FLV_TYPES_H
#define ROXLU_FLV_TYPES_H

#include <streamer/core/BitStream.h>
#include <stdint.h>
#include <vector>

#define FLV_TAG_TYPE_AUDIO 0x08 
#define FLV_TAG_TYPE_VIDEO 0x09 
#define FLV_TAG_TYPE_META 0x12  

#define FLV_SOUNDFORMAT_LINEAR_PCM_NE    0 
#define FLV_SOUNDFORMAT_ADPCM            1
#define FLV_SOUNDFORMAT_MP3              2
#define FLV_SOUNDFORMAT_LINEAR_PCM_LE    3 
#define FLV_SOUNDFORMAT_NELLYMOSER_16KHZ 4
#define FLV_SOUNDFORMAT_NELLYMOSER_8KHZ  5
#define FLV_SOUNDFORMAT_G711_A_LAW       7
#define FLV_SOUNDFORMAT_G711_MU_LAW      8
#define FLV_SOUNDFORMAT_RESERVED         9
#define FLV_SOUNDFORMAT_AAC              10
#define FLV_SOUNDFORMAT_SPEEX            11
#define FLV_SOUNDFORMAT_MP3_8KHZ         14
#define FLV_SOUNDFORMAT_DEVICE_SPECIFIC  15
#define FLV_SOUNDFORMAT_UNKNOWN          99

#define FLV_SOUNDRATE_5_5KHZ   0 
#define FLV_SOUNDRATE_11KHZ    1
#define FLV_SOUNDRATE_22KHZ    2
#define FLV_SOUNDRATE_44KHZ    3
#define FLV_SOUNDRATE_UNKNOWN  99

#define FLV_SOUNDSIZE_8BIT      0
#define FLV_SOUNDSIZE_16BIT     1
#define FLV_SOUNDSIZE_UNKNOWN   99

#define FLV_SOUNDTYPE_MONO      0
#define FLV_SOUNDTYPE_STEREO    1
#define FLV_SOUNDTYPE_UNKNOWN   99

#define FLV_VIDEOFRAME_KEY_FRAME               1
#define FLV_VIDEOFRAME_INTER_FRAME             2
#define FLV_VIDEOFRAME_DISPOSABLE_INTER_FRAME  3
#define FLV_VIDEOFRAME_GENERATED_KEY_FRAME     4
#define FLV_VIDEOFRAME_COMMAND_FRAME           5
#define FLV_VIDEOFRAME_UNKNOWN                 99

#define FLV_VIDEOCODEC_JPEG            1
#define FLV_VIDEOCODEC_SORENSON        2
#define FLV_VIDEOCODEC_SCREEN_VIDEO    3
#define FLV_VIDEOCODEC_ON2_VP6         4
#define FLV_VIDEOCODEC_ON2_VP6_ALPHA   5
#define FLV_VIDEOCODEC_SCREEN_VIDEO2   6
#define FLV_VIDEOCODEC_AVC             7
#define FLV_VIDEOCODEC_UNKNOWN         99

#define FLV_AAC_CONFIG 0
#define FLV_AAC_RAW_DATA 1
#define FLV_AAC_UNKNOWN 99

#define FLV_AVC_SEQUENCE_HEADER 0
#define FLV_AVC_NALU 1
#define FLV_AVC_EOF 2
#define FLV_AVC_UNKNOWN 99

double flv_samplerate_to_double(uint8_t r);

struct AVCDecoderConfigurationRecord {
  AVCDecoderConfigurationRecord();

  uint8_t configuration_version;
  uint8_t avc_profile_indication;
  uint8_t profile_compatibility;
  uint8_t avc_level_indication;

  std::vector<uint8_t> sps;  /* we only pass one sps and pps */
  std::vector<uint8_t> pps;  /* we only pass one sps and pps */
  std::vector<uint8_t> sei; /* tmp - also adding sei */

  /* only used by reader */
  uint8_t nal_size_length_minus_one;
  uint8_t number_of_sps;
  uint8_t number_of_pps;
};

struct FLVHeader {
  FLVHeader();
  void print();

  char tag[3];
  uint8_t version;
  uint8_t type_flags_audio;
  uint8_t type_flags_video;
  uint32_t data_offset;
};

struct FLVTag {
  FLVTag();

  void setTimeStamp(uint32_t ts, uint8_t tse = 0); /* set the timestamp + timestamp_extended members */
  void setData(uint8_t* ptr, uint32_t size);   /* set the data + size members */
  void setVideoCodec(uint8_t codec);
  void setFrameType(uint8_t ft); /* set video frame type */
  void setAACPacketType(uint8_t pt); /* set the aac packet type, 0 = audio specific config, 1 = raw data */
  void setAVCPacketType(uint8_t pt); /* set the avc packet type, 0 = sequence header, aka AVC Decoder Configuration */
  void setCompositionTime(int32_t t);
  void setAudioCodec(uint8_t codec); /* sets the kind of audio codec used */

  void makeVideoTag(); /* set the type to a video tag */
  void makeAudioTag(); /* set the type to a audio tag */
  void makeScriptTag(); /* set the type to a script tag */

  /* shared header */
  uint8_t reserved;
  uint8_t filter;
  uint8_t tag_type;
  uint32_t data_size;
  uint32_t timestamp;
  uint8_t timestamp_extended;
  uint32_t stream_id;

  /* audio specific header - used by reader*/
  uint8_t sound_format;
  uint8_t sound_rate;
  uint8_t sound_type;
  uint8_t sound_size;
  uint8_t aac_packet_type; 
  std::vector<uint8_t> sound_data; 

  /* video specific data - used by reader */
  uint8_t frame_type;
  uint8_t codec_id;
  uint8_t avc_packet_type;
  int32_t composition_time; 
  std::vector<uint8_t> video_data;  

  /* used when encoding; contains a ptr to "live" data with will be overwritten during a next encode() call */
  uint8_t* data;
  size_t size;
  BitStream bs; /* in some cases this is used to construct FLVTag specific data; make sure to set the `FLVTag::data` member like: FLVTag tag; tag.setData(tag.bs.getPtr(), tag.bs.size()); */
};

inline void FLVTag::setVideoCodec(uint8_t codec) {
  codec_id = codec;
}

inline void FLVTag::setFrameType(uint8_t ft) {
  frame_type = ft;
}

inline void FLVTag::setAVCPacketType(uint8_t pt) {
  avc_packet_type = pt;
}

inline void FLVTag::setAACPacketType(uint8_t pt) {
  aac_packet_type = pt;
}

inline void FLVTag::setCompositionTime(int32_t t) {
  composition_time = t;
}

inline void FLVTag::setAudioCodec(uint8_t codec) {
  sound_format = codec;
}

inline void FLVTag::makeVideoTag() {
  tag_type = FLV_TAG_TYPE_VIDEO;
}

inline void FLVTag::makeAudioTag() {
  tag_type = FLV_TAG_TYPE_AUDIO;
}

inline void FLVTag::makeScriptTag() {
  tag_type = FLV_TAG_TYPE_META;
}

inline void FLVTag::setTimeStamp(uint32_t ts, uint8_t tse) {
  timestamp = ts;
  timestamp_extended = tse;
}

inline void FLVTag::setData(uint8_t* ptr, uint32_t s) {
  data = ptr;
  size = s;
}
#endif
