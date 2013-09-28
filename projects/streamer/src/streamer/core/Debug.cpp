#include <streamer/core/Debug.h>

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
 
  printf("--\n");
  printf("x264_param_t.i_threads: %d\n", p->i_threads);
  printf("x264_param_t.i_lookahead_threads: %d\n", p->i_lookahead_threads);
  printf("x264_param_t.b_sliced_threads: %d\n", p->b_sliced_threads);
  printf("x264_param_t.b_deterministic: %d \n", p->b_deterministic);
  printf("x264_param_t.b_cpu_independent: %d \n", p->b_cpu_independent);
  printf("x264_param_t.i_sync_lookahead: %d\n", p->i_sync_lookahead);
  printf("x264_param_t.i_width: %d\n", p->i_width);
  printf("x264_param_t.i_height: %d\n", p->i_height);
  printf("x264_param_t.i_csp: %d \n", p->i_csp);
  printf("x264_param_t.i_level_idc: %d\n", p->i_level_idc);
  printf("x264_param_t.i_frame_total: %d \n", p->i_frame_total);
  printf("x264_param_t.i_nal_hrd: %d \n", p->i_nal_hrd);

  // skipping vui
  printf("--\n");
  printf("x264_param_t.i_frame_reference: %d\n", p->i_frame_reference);
  printf("x264_param_t.i_dpb_size: %d \n", p->i_dpb_size);
  printf("x264_param_t.i_keyint_max: %d \n", p->i_keyint_max);
  printf("x264_param_t.i_keyint_min: %d \n", p->i_keyint_min);
  printf("x264_param_t.i_scenecut_threshold: %d: \n", p->i_scenecut_threshold);
  printf("x264_param_t.b_intra_refresh: %d\n", p->b_intra_refresh);
  printf("x264_param_t.i_bframe: %d\n", p->i_bframe);
  printf("x264_param_t.i_bframe_adaptive: %d\n", p->i_bframe_adaptive);

  // skipping a lot..
  printf("--\n");
  printf("x264_param_t.i_rc_method: %d\n", p->rc.i_rc_method);
  printf("x264_param_t.i_qp_constant: %d\n", p->rc.i_qp_constant);
  printf("x264_param_t.i_qp_min: %d\n", p->rc.i_qp_min);
  printf("x264_param_t.i_qp_max: %d \n", p->rc.i_qp_max);
  printf("x264_param_t.i_qp_step: %d \n", p->rc.i_qp_step);
  printf("x264_param_t.i_bitrate: %d \n", p->rc.i_bitrate);

  // skipping .. 
  printf("--\n");
  printf("x264_param_t.b_aud: %d\n", p->b_aud);
  printf("x264_param_t.b_repeat_headers: %d \n", p->b_repeat_headers);
  printf("x264_param_t.b_annexb: %d (flv does not support annexb)\n", p->b_annexb);
  printf("x264_param_t.i_sps_id: %d \n", p->i_sps_id);
  printf("x264_param_t.b_vfr_input: %d\n", p->b_vfr_input);
  printf("x264_param_t.b_pulldown: %d\n", p->b_pulldown);
  printf("x264_param_t.i_fps_num: %d\n", p->i_fps_num);
  printf("x264_param_t.i_fps_den: %d\n", p->i_fps_den);
  printf("x264_param_t.i_timebase_num: %d \n", p->i_timebase_num);
  printf("x264_param_t.i_timebase_den: %d\n", p->i_timebase_den);
  printf("-------------------------------------------------------------\n");

}

void print_nal_unit(NalUnit* nu) {
  printf("nal.forbidden_zero_bit: %d\n", nu->forbidden_zero_bit);
  printf("nal.nal_ref_idc: %d\n", nu->nal_ref_idc);
  printf("nal.nal_unit_type: %d\n", nu->nal_unit_type);
  print_nal_sps(nu->sps);
  printf("-------------------------------------------------------------\n");
}

void print_nal_sps(nal_sps& n) {
  printf("sps.profile_idc: %d\n", n.profile_idc);
  printf("sps.constraint_set0_flag: %d\n", n.constraint_set0_flag);
  printf("sps.constraint_set1_flag: %d\n", n.constraint_set1_flag);
  printf("sps.constraint_set2_flag: %d\n", n.constraint_set2_flag);
  printf("sps.constraint_set3_flag: %d\n", n.constraint_set3_flag);
  printf("sps.constraint_set4_flag: %d\n", n.constraint_set4_flag);
  printf("sps.constraint_set5_flag: %d\n", n.constraint_set5_flag);
  printf("sps.reserved_zero_2bits: %d\n", n.reserved_zero_2bits);
  printf("sps.level_idc: %d\n", n.level_idc);
}

void print_decoder_configuration_record(AVCDecoderConfigurationRecord* r) {
  printf("cfg.configuration_version: %d\n", r->configuration_version);
  printf("cfg.avc_profile_indication: %d\n", r->avc_profile_indication);
  printf("cfg.profile_compatiblity: %d\n", r->profile_compatibility);
  printf("cfg.avc_level_indication: %d\n", r->avc_level_indication);
  printf("cfg.nal_size_length_minus_one: %02X\n", r->nal_size_length_minus_one);
  printf("cfg.number_of_sps: %d\n", r->number_of_sps);
  printf("cfg.number_of_pps: %d\n", r->number_of_pps);
  printf("-------------------------------------------------------------\n");
}
