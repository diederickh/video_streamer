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

   # AudioEncoder

   Base class for the AudioEncoderFAAC, AudioEncoderMP3

 */
#ifndef ROXLU_AUDIO_ENCODER_H
#define ROXLU_AUDIO_ENCODER_H

#include <streamer/flv/FLVTypes.h>
#include <streamer/core/EncoderTypes.h>

class AudioEncoder {
 public:
  AudioEncoder();
  virtual ~AudioEncoder();
  virtual bool setup(AudioSettings settings) = 0;
  virtual bool initialize() = 0;
  virtual bool encodePacket(AVPacket* p, FLVTag& result) = 0;
  virtual bool shutdown() = 0;
};

#endif
