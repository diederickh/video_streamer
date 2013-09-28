#include <streamer/flv/FLVReader.h>
#include <streamer/amf/AMF0.h>
#include <streamer/core/BitStream.h>
#include <streamer/core/Endian.h>
#include <streamer/core/H264Parser.h>
#include <streamer/core/Debug.h>

FLVReader::FLVReader(BitStream& bs) 
  :bs(bs)
  ,state(FLV_READER_STATE_NONE)
  ,tag_size(0)
{

}

void FLVReader::parse() {

  while(bs.size()) {
    switch(state) {

      case FLV_READER_STATE_NONE: {
        if(parseHeader()) {
          state = FLV_READER_STATE_PREV_TAG_SIZE;
          continue;
        }
        else {
          return;
        }
      }

      case FLV_READER_STATE_PREV_TAG_SIZE: {
        if(bs.size() < 3) {
          return;
        }
        tag_size = FromBE32(bs.getU32());
        state = FLV_READER_STATE_TAG;
        continue;
      }

      case FLV_READER_STATE_TAG: {
        FLVTag tag;
        if(!parseFLVTag(tag)) {
          return;
        }
        state = FLV_READER_STATE_PREV_TAG_SIZE;

#if !defined(NDEBUG)
        flv_print_tag(tag);
#endif

        continue;
      }

      default: {
        printf("error: unhandled state.\n");
        return;
      }
    } // switch(state)
  } // while(bs.size())

}


bool FLVReader::parseHeader() {

  if(bs.size() < 9) { 
    return false;
  }

  if(bs[0] != 'F' || bs[1] != 'L' || bs[2] != 'V') {
    return false;
  }

  sprintf(header.tag, "%s", "FLV");

  bs.flush((size_t)3);
  
  header.version = bs.getU8();
  bs.flushBits(5);

  header.type_flags_audio = bs.getBit();
  bs.flushBits(1);
  header.type_flags_video = bs.getBit();

  if(!header.type_flags_audio && !header.type_flags_video) {
    printf("error: no audio or video flag set in the flv header. invalid file.\n");
    return false;
  }

  header.data_offset = FromBE32(bs.getU32());

#if !defined(NDEBUG)
  header.print();
#endif

  return true;
}

bool FLVReader::parseFLVTag(FLVTag& tag) {

  if(bs.size() < 11) {
    return false;
  }

  tag.reserved = bs.getBits(2);
  tag.filter = bs.getBit();
  tag.tag_type = bs.getBits(5);
  tag.data_size = FromBE24(bs.getU24());
  tag.timestamp = FromBE24(bs.getU24());
  tag.timestamp_extended = bs.getU8();
  tag.stream_id = FromBE24(bs.getU24());

  if(tag.tag_type == FLV_TAG_TYPE_AUDIO) {
    return parseFLVTagAudio(tag);
  }
  else if(tag.tag_type == FLV_TAG_TYPE_VIDEO) {
    return parseFLVTagVideo(tag);
  }
  else if(tag.tag_type == FLV_TAG_TYPE_META) {
    return parseFLVTagMeta(tag);
  }
  else {
    printf("error: unhandled tag type: %d\n", tag.tag_type);
  }

  return false;
}


bool FLVReader::parseFLVTagMeta(FLVTag& tag) {

  if(tag.filter == 1) {
    printf("error: the body data is encrypted. not supported.\n");
    return false;
  }

  AMF0 amf(bs);
  std::vector<AMFType*> amf_els;
  if(!amf.parse(amf_els, tag.data_size)) {
    printf("error: invalid script data\n");
    return false;
  }

  for(std::vector<AMFType*>::iterator it = amf_els.begin(); it != amf_els.end(); ++it) {
    AMFType* el = *it;
#if !defined(NDEBUG)
    el->print();
#endif
    delete el;
    el = NULL;
  }
  amf_els.clear();

  return true;
}

bool FLVReader::parseFLVTagAudio(FLVTag& tag) {
  tag.sound_format = bs.getBits(4);
  tag.sound_rate = bs.getBits(2);
  tag.sound_size = bs.getBit();
  tag.sound_type = bs.getBit();

  if(tag.sound_format == FLV_SOUNDFORMAT_AAC) {
    printf("warning: we do not yet handle the sound_format: FLV_SOUNDFORMAT_AAC\n");
  }

  if(tag.filter == 1) {
    printf("error: we do not yet handle encrypted body data.\n");
    return false;
  }

  bs.getBytes(tag.data_size - 1, tag.sound_data); 

  return true;
}

bool FLVReader::parseFLVTagVideo(FLVTag& tag) {
  size_t start = bs.size();

  tag.frame_type = bs.getBits(4);
 
  tag.codec_id = bs.getBits(4);
  
  if(tag.codec_id == FLV_VIDEOCODEC_AVC) {
    tag.avc_packet_type = bs.getU8();
    tag.composition_time = bs.getU24(); // @todo <-- should be getS24()!
  }

  if(tag.filter == 1) {
    printf("error: cannot read video tag because the data is encrypted.\n");
    return false;
  }

  size_t d = start - bs.size();
  bs.getBytes(tag.data_size - d, tag.video_data);

  if(tag.codec_id == FLV_VIDEOCODEC_AVC) {
    if(tag.avc_packet_type == FLV_AVC_SEQUENCE_HEADER) {
      printf("Sequence header.\n");
      AVCDecoderConfigurationRecord rec;
      BitStream config_bs;
      config_bs.putBytes(&tag.video_data.front(), tag.video_data.size());
      // parseDecoderConfigurationRecord(config_bs, rec);
    }
    else if(tag.avc_packet_type == FLV_AVC_NALU) {
      // printf("Nalu.\n");
      // AVC sequence header - AVCDecoderConfiguration Recors - SPS & PPS
      // see T-REC-H.264-201304-I!!PDF-E.pdf, page 63, section 7.3 - 7.3.2.1 "Sequence parameter set RBSP syntax"
      //H264Parser hp(&tag.video_data.front());
      //hp.parse();
    }
  }
  
  listeners.onTag(bs, tag);

  return true;
}

bool FLVReader::parseDecoderConfigurationRecord(BitStream& s, AVCDecoderConfigurationRecord& rec) {

  rec.configuration_version = s.getU8();
  rec.avc_profile_indication = s.getU8();
  rec.profile_compatibility= s.getU8();
  rec.avc_level_indication = s.getU8();
  rec.nal_size_length_minus_one = s.getU8();

  s.flushBits(3);
  rec.number_of_sps = s.getBits(5);
  for(int i = 0; i < rec.number_of_sps; ++i) { // should use a uint16
    s.print(10);
    uint16_t len = FromBE16(s.getU16());
    s.print(len);
    H264Parser hp(s.getPtr());
    hp.parse();
    s.flush(len);
  }

  rec.number_of_pps = s.getU8();
  for(int i = 0; i < rec.number_of_pps; ++i) {
    uint16_t len = FromBE16(s.getU16());
    s.flush(len);
  }
  
  printf("---  left: %ld\n", s.size());
  s.print(10);
  print_decoder_configuration_record(&rec);
  return true;
}
