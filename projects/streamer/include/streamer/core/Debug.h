/*
  # Debug

  Contains kinds of debug relates functions for AMF, FLV, X264 etc.. 

*/

#ifndef ROXLU_DEBUG_H
#define ROXLU_DEBUG_H

#include <stdint.h>

extern "C" {
#  include <x264.h>
}

#include <stdio.h>
#include <string>
#include <streamer/flv/FLVTypes.h>
#include <streamer/core/H264Parser.h>

std::string flv_tagtype_to_string(uint8_t t);
std::string flv_soundformat_to_string(uint8_t t);
std::string flv_frametype_to_string(uint8_t t);
std::string flv_videocodec_to_string(uint8_t t);
std::string flv_soundrate_to_string(uint8_t t);
std::string flv_soundsize_to_string(uint8_t t);
std::string flv_soundtype_to_string(uint8_t t);
void flv_print_tag(FLVTag& tag);

void print_x264_params(x264_param_t* p);
void print_nal_unit(NalUnit* nu);
void print_nal_sps(nal_sps& n);
void print_decoder_configuration_record(AVCDecoderConfigurationRecord* r);


#endif
