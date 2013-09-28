#include <streamer/core/H264Parser.h>
#include <streamer/core/Debug.h>

// ------------------------------------------------
nal_sps::nal_sps() 
  :profile_idc(0)
  ,constraint_set0_flag(0)
  ,constraint_set1_flag(0)
  ,constraint_set2_flag(0)
  ,constraint_set3_flag(0)
  ,constraint_set4_flag(0)
  ,constraint_set5_flag(0)
  ,reserved_zero_2bits(0)
  ,level_idc(0)
{

}

// ------------------------------------------------

NalUnit::NalUnit()
  :forbidden_zero_bit(0)
  ,nal_ref_idc(0)
  ,nal_unit_type(0)
{
}

// ------------------------------------------------

H264Parser::H264Parser(uint8_t* nal) 
  :nal(nal)
  ,bit_offset(0)
  ,byte_offset(0)
{
}

bool H264Parser::parse() {
  if(!nal) {
    printf("error: cannot parse h264, no nal unit set. %p\n", nal);
    return false;
  }

  NalUnit nu;
  nu.forbidden_zero_bit = f(1);
  nu.nal_ref_idc = u(2);
  nu.nal_unit_type = u(5);

  int num_header_bytes = 1;
  if(nu.nal_unit_type == 14 || nu.nal_unit_type == 20 || nu.nal_unit_type == 21) {
    printf("warning: not handling unit type 14, 20 or 21 yet.\n");
    // num_header_bytes = ...
    return false;
  }
  
  for(int i = 0; i < num_header_bytes; ++i) {
    printf("h264 parser, warning: need to read the header bytes! %d\n", num_header_bytes);
  }
  
  switch(nu.nal_unit_type) {
    case NAL_SPS: {
      parseSPS(nu);
      break;
    }
    default: { 
      printf("error: unhandled nal type.\n");
      break;
    }
  }
  print_nal_unit(&nu);

  return true;
}

bool H264Parser::parseSPS(NalUnit& n) {
  n.sps.profile_idc = u8();
  n.sps.constraint_set0_flag = f(1);
  n.sps.constraint_set1_flag = f(1);
  n.sps.constraint_set2_flag = f(1);
  n.sps.constraint_set3_flag = f(1);
  n.sps.constraint_set4_flag = f(1);
  n.sps.constraint_set5_flag = f(1);
  n.sps.reserved_zero_2bits = f(2);
  n.sps.level_idc = u8();
  return true;
}

bool H264Parser::parsePPS(NalUnit& n) {
  
  return true;
}

