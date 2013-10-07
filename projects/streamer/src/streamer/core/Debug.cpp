#include <streamer/core/Debug.h>
#include <streamer/core/Log.h>

std::string flv_soundformat_to_string(uint8_t t) {
  switch(t) {
    case FLV_SOUNDFORMAT_LINEAR_PCM_NE: return "FLV_SOUNDFORMAT_LINEAR_PCM_NE"; 
    case FLV_SOUNDFORMAT_ADPCM: return "FLV_SOUNDFORMAT_ADPCM"; 
    case FLV_SOUNDFORMAT_MP3: return "FLV_SOUNDFORMAT_MP3"; 
    case FLV_SOUNDFORMAT_LINEAR_PCM_LE : return "FLV_SOUNDFORMAT_LINEAR_PCM_LE "; 
    case FLV_SOUNDFORMAT_NELLYMOSER_16KHZ: return "FLV_SOUNDFORMAT_NELLYMOSER_16KHZ"; 
    case FLV_SOUNDFORMAT_NELLYMOSER_8KHZ: return "FLV_SOUNDFORMAT_NELLYMOSER_8KHZ"; 
    case FLV_SOUNDFORMAT_G711_A_LAW: return "FLV_SOUNDFORMAT_G711_A_LAW"; 
    case FLV_SOUNDFORMAT_G711_MU_LAW: return "FLV_SOUNDFORMAT_G711_MU_LAW"; 
    case FLV_SOUNDFORMAT_RESERVED: return "FLV_SOUNDFORMAT_RESERVED"; 
    case FLV_SOUNDFORMAT_AAC : return "FLV_SOUNDFORMAT_AAC "; 
    case FLV_SOUNDFORMAT_SPEEX : return "FLV_SOUNDFORMAT_SPEEX "; 
    case FLV_SOUNDFORMAT_MP3_8KHZ : return "FLV_SOUNDFORMAT_MP3_8KHZ "; 
    case FLV_SOUNDFORMAT_DEVICE_SPECIFIC : return "FLV_SOUNDFORMAT_DEVICE_SPECIFIC "; 
    case FLV_SOUNDFORMAT_UNKNOWN: return "FLV_SOUNDFORMAT_UNKNOWN"; 
    default: return "UNKNOWN";
  }
}

std::string flv_videocodec_to_string(uint8_t t) {
  switch(t) {
    case FLV_VIDEOCODEC_JPEG: return "FLV_VIDEOCODEC_JPEG"; 
    case FLV_VIDEOCODEC_SORENSON: return "FLV_VIDEOCODEC_SORENSON"; 
    case FLV_VIDEOCODEC_SCREEN_VIDEO: return "FLV_VIDEOCODEC_SCREEN_VIDEO"; 
    case FLV_VIDEOCODEC_ON2_VP6 : return "FLV_VIDEOCODEC_ON2_VP6 "; 
    case FLV_VIDEOCODEC_ON2_VP6_ALPHA: return "FLV_VIDEOCODEC_ON2_VP6_ALPHA"; 
    case FLV_VIDEOCODEC_SCREEN_VIDEO2: return "FLV_VIDEOCODEC_SCREEN_VIDEO2"; 
    case FLV_VIDEOCODEC_AVC: return "FLV_VIDEOCODEC_AVC"; 
    case FLV_VIDEOCODEC_UNKNOWN: return "FLV_VIDEOCODEC_UNKNOWN"; 
    default: return "UNKNOWN";
  }
}

std::string flv_frametype_to_string(uint8_t t) {
  switch(t) {
    case FLV_VIDEOFRAME_KEY_FRAME: return "FLV_VIDEOFRAME_KEY_FRAME"; 
    case FLV_VIDEOFRAME_INTER_FRAME: return "FLV_VIDEOFRAME_INTER_FRAME"; 
    case FLV_VIDEOFRAME_DISPOSABLE_INTER_FRAME: return "FLV_VIDEOFRAME_DISPOSABLE_INTER_FRAME"; 
    case FLV_VIDEOFRAME_GENERATED_KEY_FRAME: return "FLV_VIDEOFRAME_GENERATED_KEY_FRAME"; 
    case FLV_VIDEOFRAME_COMMAND_FRAME: return "FLV_VIDEOFRAME_COMMAND_FRAME"; 
    case FLV_VIDEOFRAME_UNKNOWN: return "FLV_VIDEOFRAME_UNKNOWN"; 
    default: return "UNKNOWN";
  }
}

std::string flv_soundrate_to_string(uint8_t t) {
  switch(t) {
    case FLV_SOUNDRATE_5_5KHZ : return "FLV_SOUNDRATE_5_5KHZ "; 
    case FLV_SOUNDRATE_11KHZ: return "FLV_SOUNDRATE_11KHZ"; 
    case FLV_SOUNDRATE_22KHZ: return "FLV_SOUNDRATE_22KHZ"; 
    case FLV_SOUNDRATE_44KHZ: return "FLV_SOUNDRATE_44KHZ"; 
    case FLV_SOUNDRATE_UNKNOWN: return "FLV_SOUNDRATE_UNKNOWN";
    default: return "UNKNOWN";
  }
}

std::string flv_soundsize_to_string(uint8_t t) {
  switch(t) {
    case FLV_SOUNDSIZE_8BIT: return "FLV_SOUNDSIZE_8BIT"; 
    case FLV_SOUNDSIZE_16BIT: return "FLV_SOUNDSIZE_16BIT"; 
    case FLV_SOUNDSIZE_UNKNOWN: return "FLV_SOUNDSIZE_UNKNOWN"; 
    default: return "UNKNOWN";
  }
}

std::string flv_soundtype_to_string(uint8_t t) {
  switch(t) {
    case FLV_SOUNDTYPE_MONO: return "FLV_SOUNDTYPE_MONO"; 
    case FLV_SOUNDTYPE_STEREO: return "FLV_SOUNDTYPE_STEREO"; 
    case FLV_SOUNDTYPE_UNKNOWN: return "FLV_SOUNDTYPE_UNKNOWN"; 
    default: return "UNKNOWN";
  }
}

std::string flv_tagtype_to_string(uint8_t t) {
  switch(t) {
    case FLV_TAG_TYPE_META: return "FLV_TAG_TYPE_META";
    case FLV_TAG_TYPE_AUDIO: return "FLV_TAG_TYPE_AUDIO";
    case FLV_TAG_TYPE_VIDEO: return "FLV_TAG_TYPE_VIDEO";
    default: return "UNKNOWN";
  }
}

void flv_print_tag(FLVTag& tag) {
  return;
  printf("flv_tag.reserved: %d\n", tag.reserved);
  printf("flv_tag.filter: %d\n", tag.filter);
  printf("flv_tag.tag_type: %d\n", tag.tag_type);
  printf("flv_tag.data_size: %d\n", tag.data_size);
  printf("flv_tag.timestamp: %d\n", tag.timestamp);
  printf("flv_tag.timestamp_extended: %d\n", tag.timestamp_extended);
  printf("flv_tag.stream_id: %d\n", tag.stream_id);
  printf("flv_tag.tag_type: %s\n", flv_tagtype_to_string(tag.tag_type).c_str());

  if(tag.tag_type == FLV_TAG_TYPE_AUDIO) {
    printf("flv_tag.sound_format: %s\n", flv_soundformat_to_string(tag.sound_format).c_str());
    printf("flv_tag.sound_rate: %s\n", flv_soundrate_to_string(tag.sound_rate).c_str());
    printf("flv_tag.sound_size: %s\n", flv_soundsize_to_string(tag.sound_size).c_str());
    printf("flv_tag.sound_type: %s\n", flv_soundtype_to_string(tag.sound_type).c_str());
    printf("flv_tag.sound_data.size: %ld\n", tag.sound_data.size());
  }
  else if(tag.tag_type == FLV_TAG_TYPE_VIDEO) {
    printf("flv_tag.frame_type: %s\n", flv_frametype_to_string(tag.frame_type).c_str());
    printf("flv_tag.codec_id: %s\n", flv_videocodec_to_string(tag.codec_id).c_str());
    printf("flv_tag.avc_packet_type: %d\n", tag.avc_packet_type);
    printf("flv_tag.composition_time: %d\n", tag.composition_time);
    printf("flv_tag.video_data.size: %ld\n", tag.video_data.size());
  }

  printf("-------------------------------------------------------------\n");
}


void print_x264_params(x264_param_t* p) {
 
  STREAMER_VERBOSE("--\n");
  STREAMER_VERBOSE("x264_param_t.i_threads: %d\n", p->i_threads);
  STREAMER_VERBOSE("x264_param_t.i_lookahead_threads: %d\n", p->i_lookahead_threads);
  STREAMER_VERBOSE("x264_param_t.b_sliced_threads: %d\n", p->b_sliced_threads);
  STREAMER_VERBOSE("x264_param_t.b_deterministic: %d \n", p->b_deterministic);
  STREAMER_VERBOSE("x264_param_t.b_cpu_independent: %d \n", p->b_cpu_independent);
  STREAMER_VERBOSE("x264_param_t.i_sync_lookahead: %d\n", p->i_sync_lookahead);
  STREAMER_VERBOSE("x264_param_t.i_width: %d\n", p->i_width);
  STREAMER_VERBOSE("x264_param_t.i_height: %d\n", p->i_height);
  STREAMER_VERBOSE("x264_param_t.i_csp: %d \n", p->i_csp);
  STREAMER_VERBOSE("x264_param_t.i_level_idc: %d\n", p->i_level_idc);
  STREAMER_VERBOSE("x264_param_t.i_frame_total: %d \n", p->i_frame_total);
  STREAMER_VERBOSE("x264_param_t.i_nal_hrd: %d \n", p->i_nal_hrd);

  // skipping vui
  STREAMER_VERBOSE("--\n");
  STREAMER_VERBOSE("x264_param_t.i_frame_reference: %d\n", p->i_frame_reference);
  STREAMER_VERBOSE("x264_param_t.i_dpb_size: %d \n", p->i_dpb_size);
  STREAMER_VERBOSE("x264_param_t.i_keyint_max: %d \n", p->i_keyint_max);
  STREAMER_VERBOSE("x264_param_t.i_keyint_min: %d \n", p->i_keyint_min);
  STREAMER_VERBOSE("x264_param_t.i_scenecut_threshold: %d: \n", p->i_scenecut_threshold);
  STREAMER_VERBOSE("x264_param_t.b_intra_refresh: %d\n", p->b_intra_refresh);
  STREAMER_VERBOSE("x264_param_t.i_bframe: %d\n", p->i_bframe);
  STREAMER_VERBOSE("x264_param_t.i_bframe_adaptive: %d\n", p->i_bframe_adaptive);

  // skipping a lot..
  STREAMER_VERBOSE("--\n");
  STREAMER_VERBOSE("x264_param_t.i_rc_method: %d\n", p->rc.i_rc_method);
  STREAMER_VERBOSE("x264_param_t.i_qp_constant: %d\n", p->rc.i_qp_constant);
  STREAMER_VERBOSE("x264_param_t.i_qp_min: %d\n", p->rc.i_qp_min);
  STREAMER_VERBOSE("x264_param_t.i_qp_max: %d \n", p->rc.i_qp_max);
  STREAMER_VERBOSE("x264_param_t.i_qp_step: %d \n", p->rc.i_qp_step);
  STREAMER_VERBOSE("x264_param_t.i_bitrate: %d \n", p->rc.i_bitrate);

  // skipping .. 
  STREAMER_VERBOSE("--\n");
  STREAMER_VERBOSE("x264_param_t.b_aud: %d\n", p->b_aud);
  STREAMER_VERBOSE("x264_param_t.b_repeat_headers: %d \n", p->b_repeat_headers);
  STREAMER_VERBOSE("x264_param_t.b_annexb: %d (flv does not support annexb)\n", p->b_annexb);
  STREAMER_VERBOSE("x264_param_t.i_sps_id: %d \n", p->i_sps_id);
  STREAMER_VERBOSE("x264_param_t.b_vfr_input: %d\n", p->b_vfr_input);
  STREAMER_VERBOSE("x264_param_t.b_pulldown: %d\n", p->b_pulldown);
  STREAMER_VERBOSE("x264_param_t.i_fps_num: %d\n", p->i_fps_num);
  STREAMER_VERBOSE("x264_param_t.i_fps_den: %d\n", p->i_fps_den);
  STREAMER_VERBOSE("x264_param_t.i_timebase_num: %d \n", p->i_timebase_num);
  STREAMER_VERBOSE("x264_param_t.i_timebase_den: %d\n", p->i_timebase_den);
  STREAMER_VERBOSE("-------------------------------------------------------------\n");

}

void print_nal_unit(NalUnit* nu) {
  STREAMER_VERBOSE("nal.forbidden_zero_bit: %d\n", nu->forbidden_zero_bit);
  STREAMER_VERBOSE("nal.nal_ref_idc: %d\n", nu->nal_ref_idc);
  STREAMER_VERBOSE("nal.nal_unit_type: %d\n", nu->nal_unit_type);
  print_nal_sps(nu->sps);
  STREAMER_VERBOSE("-------------------------------------------------------------\n");
}

void print_nal_sps(nal_sps& n) {
  STREAMER_VERBOSE("sps.profile_idc: %d\n", n.profile_idc);
  STREAMER_VERBOSE("sps.constraint_set0_flag: %d\n", n.constraint_set0_flag);
  STREAMER_VERBOSE("sps.constraint_set1_flag: %d\n", n.constraint_set1_flag);
  STREAMER_VERBOSE("sps.constraint_set2_flag: %d\n", n.constraint_set2_flag);
  STREAMER_VERBOSE("sps.constraint_set3_flag: %d\n", n.constraint_set3_flag);
  STREAMER_VERBOSE("sps.constraint_set4_flag: %d\n", n.constraint_set4_flag);
  STREAMER_VERBOSE("sps.constraint_set5_flag: %d\n", n.constraint_set5_flag);
  STREAMER_VERBOSE("sps.reserved_zero_2bits: %d\n", n.reserved_zero_2bits);
  STREAMER_VERBOSE("sps.level_idc: %d\n", n.level_idc);
}

void print_decoder_configuration_record(AVCDecoderConfigurationRecord* r) {
  STREAMER_VERBOSE("cfg.configuration_version: %d\n", r->configuration_version);
  STREAMER_VERBOSE("cfg.avc_profile_indication: %d\n", r->avc_profile_indication);
  STREAMER_VERBOSE("cfg.profile_compatiblity: %d\n", r->profile_compatibility);
  STREAMER_VERBOSE("cfg.avc_level_indication: %d\n", r->avc_level_indication);
  STREAMER_VERBOSE("cfg.nal_size_length_minus_one: %02X\n", r->nal_size_length_minus_one);
  STREAMER_VERBOSE("cfg.number_of_sps: %d\n", r->number_of_sps);
  STREAMER_VERBOSE("cfg.number_of_pps: %d\n", r->number_of_pps);
  STREAMER_VERBOSE("-------------------------------------------------------------\n");
}

std::string av_audio_samplerate_to_string(uint32_t t) {
  switch(t) {
    case AV_AUDIO_SAMPLERATE_11025: return "11025";
    case AV_AUDIO_SAMPLERATE_22050: return "22050";
    case AV_AUDIO_SAMPLERATE_44100: return "44100";
    default: return "AV_AUDIO_SAMPLERATE_UNKNOWN"; 
  }
}

std::string av_audio_mode_to_string(uint32_t t) {
  switch(t) {
    case AV_AUDIO_MODE_MONO: return "AV_AUDIO_MODE_MONO"; 
    case AV_AUDIO_MODE_STEREO: return "AV_AUDIO_MODE_STEREO";
    default: return "AV_AUDIO_MODE_UNKNOWN";
  }
}

std::string av_audio_bitsize_to_string(uint32_t t) {
  switch(t) {
    case AV_AUDIO_BITSIZE_S8: return "AV_AUDIO_BITSIZE_S8";
    case AV_AUDIO_BITSIZE_S16: return "AV_AUDIO_BITSIZE_S16";
    case AV_AUDIO_BITSIZE_F32: return "AV_AUDIO_BITSIZE_F32";
    default: return "AV_AUDIO_BITSIZE_UNKNOWN";
  }
}
