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
#include <streamer/core/EncoderTypes.h>

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

/* debug for EncoderTypes.h defines */
std::string av_audio_samplerate_to_string(uint32_t t);
std::string av_audio_mode_to_string(uint32_t t);
std::string av_audio_bitsize_to_string(uint32_t t);

#endif
