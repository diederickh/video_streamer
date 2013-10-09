#include <streamer/core/Endian.h>
#include <streamer/core/Debug.h>
#include <streamer/flv/FLVWriter.h>
#include <streamer/amf/AMF0.h>

FLVWriter::FLVWriter(BitStream& bs) 
  :bs(bs)
  ,audio_codec(FLV_SOUNDFORMAT_UNKNOWN)
  ,video_codec(FLV_VIDEOCODEC_UNKNOWN)
  ,audio_samplerate(FLV_SOUNDRATE_UNKNOWN)
  ,audio_datarate(0)
  ,audio_type(FLV_SOUNDTYPE_UNKNOWN)
  ,audio_size(FLV_SOUNDSIZE_UNKNOWN)
  ,audio_delay(0.0)
  ,can_seek_to_end(false)
  ,duration(0.0)
  ,fps(0.0)
  ,height(0.0)
  ,width(0.0)
  ,video_data_rate(0.0)
  ,metadata_offset(0)
  ,metadata_size(0)
  ,last_timestamp(0)
{

}

bool FLVWriter::open() {

  if(!hasVideo() && !hasAudio()) {
    printf("error: it seems you're not using video AND audio for the flv writer which is not allowed.\n");
    return false;
  }

  if(hasVideo()) {
    if(!validateVideoSettings()) {
      return false;
    }
  }

  if(hasAudio()) {
    if(!validateAudioSettings()) {
      return false;
    }
  }

  writeHeader(bs);
  writeMetaData(bs);
  writeDecoderConfigurationRecord(bs);

  if(audio_codec == FLV_SOUNDFORMAT_AAC) {

    if(audio_specific_config.size() == 0) {
      printf("error: you're specified to use AAC audio for the FLVWriter, but we did not yet receive the audio specific config.\n");
      return false;
    }

    writeAudioSpecificConfig(bs);
  }

  return true;
}

bool FLVWriter::close() {

  if(!bs.size()) {
#if !defined(NDEBUG)
    printf("warning: nothing written to flv bitstream.\n");
#endif
  }
  else {
    rewriteMetaData();
  }

  printf("FLVWriter::close() -- %ld\n", bs.size());

  return true;
}

void FLVWriter::writeHeader(BitStream& s) {
  s.putU8('F');            // signature 'F'
  s.putU8('L');            // signature 'L'
  s.putU8('V');            // signature 'V'
  s.putU8(1);              // version, always 1 for FLV version 1
  s.putBits(0x00, 5);      // reserved, 0
  s.putBit(hasAudio());    // 1 = audio tags are present
  s.putBit(0);             // reserved, 0
  s.putBit(hasVideo());    // 1 = video tags are present
  s.putU32(ToBE32(9));     // length of this header, 9
  s.putU32(0);             // first previous tag size

  listeners.onSignature(s);
}

void FLVWriter::writeMetaData(BitStream& s) {
  BitStream bs_md;
  createMetaData(bs_md);

  FLVTag tag;
  tag.makeScriptTag();
  tag.setTimeStamp(0,0);
  tag.setData(bs_md.getPtr(), bs_md.size());
  
  metadata_offset = s.size();
  {
    writeFLVTag(tag, s);
  }
  metadata_size = s.size() - metadata_offset;
}

void FLVWriter::createMetaData(BitStream& s) {
  AMF0 amf(s);

  AMF0String str(s, "onMetaData");
  s.putU8(AMF0_TYPE_STRING);
  amf.writeString(&str);

  AMF0EcmaArray ar(s);
  if(hasVideo()) {
    ar.add("width", amf.createNumber(width));
    ar.add("height", amf.createNumber(height));
    ar.add("framerate", amf.createNumber(fps));
    ar.add("videocodecid", amf.createNumber(video_codec));
    ar.add("videodatarate", amf.createNumber(video_data_rate));
  }

  if(hasAudio()) {
    ar.add("audiodatarate", amf.createNumber(audio_datarate));
    ar.add("audiosamplerate", amf.createNumber(flv_samplerate_to_double(audio_samplerate)));
    ar.add("audiodelay", amf.createNumber(audio_delay));
    ar.add("audiosamplesize", amf.createNumber( (audio_size == FLV_SOUNDSIZE_8BIT) ? 8.0 : 16.0));
    ar.add("audiocodecid", amf.createNumber(audio_codec));
    ar.add("stereo", amf.createBoolean(audio_type == FLV_SOUNDTYPE_STEREO));
  }

  ar.add("filesize", amf.createNumber(0.0));
  ar.add("duration", amf.createNumber(last_timestamp/1000.0));

  amf.writeEcmaArray(&ar);
}

void FLVWriter::writeDecoderConfigurationRecord(BitStream& s) {
  FLVTag tag;

  tag.setVideoCodec(video_codec);
  tag.setFrameType(FLV_VIDEOFRAME_KEY_FRAME);
  tag.setAVCPacketType(FLV_AVC_SEQUENCE_HEADER);
  tag.setCompositionTime(0);
  tag.makeVideoTag();
  tag.setTimeStamp(0,0);

  tag.bs.putU8(avc_cfg.configuration_version);               // configurationVersion (1)
  tag.bs.putU8(avc_cfg.avc_profile_indication);              // AVCProfileIndication
  tag.bs.putU8(avc_cfg.profile_compatibility);               // profile_compatibility
  tag.bs.putU8(avc_cfg.avc_level_indication);                // AVCLevelIndication
  tag.bs.putU8(0xff);                                        // 6 bits reserved + nal size length -1
  tag.bs.putU8(0xe1);                                        // 3 bits reserved + number of sps
  tag.bs.putU16(ToBE16(avc_cfg.sps.size()));                 // sequenceParameterSetLength
  tag.bs.putBytes(&avc_cfg.sps.front(), avc_cfg.sps.size()); // sequenceParameterSetNALUnit
  tag.bs.putU8(1);                                           // numberOfPictureParameterSets
  tag.bs.putU16(ToBE16(avc_cfg.pps.size()));                 // pictureParameterSetLength
  tag.bs.putBytes(&avc_cfg.pps.front(), avc_cfg.pps.size()); // pictureParameterSetNLAUnit
  tag.bs.print(100);
  tag.setData(tag.bs.getPtr(), tag.bs.size());
  
  print_decoder_configuration_record(&avc_cfg);

  writeFLVTag(tag, s);
}

// writes AAC audio codec config
void FLVWriter::writeAudioSpecificConfig(BitStream& s) {
  FLVTag tag;

  tag.setAudioCodec(audio_codec);
  tag.setAACPacketType(FLV_AAC_CONFIG);
  tag.setCompositionTime(0);
  tag.makeAudioTag();
  tag.setTimeStamp(0,0);

  tag.bs.putBytes(&audio_specific_config.front(), audio_specific_config.size());
  tag.setData(tag.bs.getPtr(), tag.bs.size());
 
  writeFLVTag(tag, s);
}

void FLVWriter::rewriteMetaData() {
  BitStream bs_md;
  createMetaData(bs_md);

  FLVTag tag;
  tag.makeScriptTag();
  tag.setData(bs_md.getPtr(), bs_md.size());
  rewriteFLVTag(tag, bs, metadata_offset, metadata_size);
}

void FLVWriter::rewriteFLVTag(FLVTag& tag, BitStream& s, size_t offset, size_t prevSize) {
  BitStream tmp_bs;
  s.erase(s.begin() + offset, s.begin() + offset + prevSize);
    writeFLVTag(tag, tmp_bs);
  s.insert(s.begin() + offset, tmp_bs.begin(), tmp_bs.end());
}

void FLVWriter::writeFLVTag(FLVTag& tag, BitStream& s) {

  size_t size_position = s.size() + 1;

  s.putBits(0x00, 2);               // reserved, should be 0
  s.putBit(tag.filter);             // filter 
  s.putBits(tag.tag_type, 5);       // type of contents (audio, video, script)
  s.putU24BigEndian(0x00);          // length of the message after stream-id, gets rewriten
  s.putU24BigEndian(tag.timestamp); // timestamp in millis
  s.putU8(tag.timestamp >> 24);     // timestamp extended
  s.putU24BigEndian(0);             // stream id - always zero

  size_t start_size = s.size();
  {
    if(tag.tag_type == FLV_TAG_TYPE_VIDEO) {
      appendVideoHeader(tag, s);
    }
    else if(tag.tag_type == FLV_TAG_TYPE_AUDIO) {
      appendAudioHeader(tag, s);
    }
  }

#if 0
  // testing .. the SEI should be added after the first video keyframe  -- according to x264
  // ----------------------------------------
  static bool seq = false;
  static int fn = 0;
  if(avc_cfg.sei.size() && seq) {
    s.putBytes(&avc_cfg.sei.front(), avc_cfg.sei.size());
    avc_cfg.sei.clear();
  }
  if(tag.avc_packet_type == FLV_AVC_SEQUENCE_HEADER) {
    seq = true;
  }
  fn++;
  // ----------------------------------------
#endif

  size_t length = s.size() - start_size;

  s.rewriteU24BigEndian(size_position, tag.size + length);    // rewrite length of message after stream id
  s.putBytes(tag.data, tag.size);                             // tag specific data
  s.putU32(ToBE32(tag.size + 11 + length));                   // previous tag size

  listeners.onTag(s, tag);

  if(tag.timestamp > last_timestamp) {
    last_timestamp = tag.timestamp;
  }

}

void FLVWriter::appendVideoHeader(FLVTag& tag, BitStream& s) {
  /*
  printf("tag.frame_type: %d\n", tag.frame_type);
  printf("tag.codec_id: %d\n", tag.codec_id);
  printf("tag.avc_packet_type: %d\n", tag.avc_packet_type);
  printf("tag.composition_time: %d\n", tag.composition_time);
  */
  s.putBits(tag.frame_type, 4);   // type of video frame
  s.putBits(tag.codec_id, 4);     // codec identitfier
  
  if(tag.codec_id != FLV_VIDEOCODEC_AVC) {
    printf("error: cannot write the video header, we only implement the AVC video codec, you gave %d, FLV_VIDEOCODEC_AVC = %d\n", tag.codec_id, FLV_VIDEOCODEC_AVC);
    ::exit(EXIT_FAILURE);
  }

  s.putU8(tag.avc_packet_type);
  s.putU24BigEndian(tag.composition_time);
}

void FLVWriter::appendAudioHeader(FLVTag& tag, BitStream& s) {
  // @todo - when the audio codec is AAC and and sound_type != stereo or samplerate != 44khz we need to stop directly! with AAC we to set stereo+44hz ( but the data may contain a different format ^.^ )
  s.putBits(tag.sound_format, 4);
  s.putBits(audio_samplerate, 2);
  s.putBit(audio_size == FLV_SOUNDSIZE_16BIT);
  s.putBit(audio_type == FLV_SOUNDTYPE_STEREO);

  if(audio_codec == FLV_SOUNDFORMAT_AAC) {

#if !defined(NDEBUG)
    if(tag.aac_packet_type != FLV_AAC_CONFIG && tag.aac_packet_type != FLV_AAC_RAW_DATA) {
      printf("error: you're trying to add a AAC audio packet but the `aac_packet_type` is not set to a valid valid: %d\n", tag.aac_packet_type);
      ::exit(EXIT_FAILURE);
    }
#endif

    s.putU8(tag.aac_packet_type); 

  }
}

bool FLVWriter::validateAudioSettings() {

  if(audio_codec == FLV_SOUNDFORMAT_UNKNOWN) {
    return true;
  }

  if(audio_samplerate == FLV_SOUNDRATE_UNKNOWN) {
    printf("error: you want to use audio but no sound rate has been set. call setAudioRate().\n");
    return false;
  }

  if(audio_type == FLV_SOUNDTYPE_UNKNOWN) {
    printf("error: you want to use audio but no sound type has been set. call setAudioType().\n");
    return false;
  }

  if(audio_size == FLV_SOUNDSIZE_UNKNOWN) {
    printf("error: you want to use audio but no sound size has been set. call setAudioSize().\n");
    return false;
  }

  return true;
}

bool FLVWriter::validateVideoSettings() {

  if(video_codec == FLV_VIDEOCODEC_UNKNOWN) {

    if(audio_codec == FLV_SOUNDFORMAT_UNKNOWN) {
      printf("error: no video and no audio codec set.\n");
      return false;
    }
    return true;
  }
  
  if(width <= 0.0) {
    printf("error: invalid width set.\n");
    return false;
  }

  if(height <= 0.0) {
    printf("error: invalid height set.\n");
  }

  if(fps <= 0.0) {
    printf("error: invalid frame rate set.\n");
    return false;
  }

  if(!avc_cfg.pps.size()) {
    printf("error: invalid avc decoder configuration record, no pps set, size = %ld .\n", avc_cfg.pps.size());
    return false;
  }

  if(!avc_cfg.sps.size()) {
    printf("error: invalid avc decoder configuration record, no sps set, size = %ld.\n", avc_cfg.sps.size());
    return false;
  }

  return true;
}
