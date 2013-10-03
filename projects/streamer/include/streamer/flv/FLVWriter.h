/*
  # FLVWriter

  The FLVWriter is a bare class which only knows about stuff found in the 
  FLV directory so it might be used in any other project. Do not couple this 
  class with other classes outside the `flv` directory.

 */
#ifndef ROXLU_FLV_WRITER_H
#define ROXLU_FLV_WRITER_H

#include <streamer/core/BitStream.h>
#include <streamer/flv/FLVTypes.h>
#include <streamer/flv/FLVListener.h>

class FLVWriter {
 public:
  FLVWriter(BitStream& bs);

  /* setup */
  void setVideoCodec(uint8_t codec);
  void setAudioCodec(uint8_t codec);
  void setAudioSampleRate(uint8_t rate);
  void setAudioDataRate(uint8_t rate); /* data rate in kbits; so for 20.000 bits you use 20 */
  void setAudioSize(uint8_t size);
  void setAudioType(uint8_t type);
  void setAudioDelay(double delay);
  void setCanSeekToEnd(bool s);
  void setFrameRate(double r); /* e.g. 60.0 */
  void setHeight(double h);
  void setWidth(double w);
  void setVideoDataRate(double rate);
  void setDecoderConfigurationRecord(AVCDecoderConfigurationRecord rec);

  bool open(); /* open the FLV stream - writer header and uses the current state; make sure to call the appropriate set*() functions */
  void writeVideoTag(FLVTag& tag); /* sets the correct codec and makes sure the given tag is a video tag */
  void writeAudioTag(FLVTag& tag); /* writes an audio tag; makes sure the tag is valid */
  bool close(); /* closes the stream, rewrites meta data info if necessary etc.. */

  void addListener(FLVListener* listener); /* add a listener which gets called when a new FLVTag has been generated and added to the bitstream */

 private:
  void writeFLVTag(FLVTag& tag, BitStream& bitstream); /* inspects the given tag and write it into the bitstream */
  void writeHeader(BitStream& bitstream); /* write the first part of the bitstream */
  void writeMetaData(BitStream& bitstream); /* writes the `onMetaData` AMF0 object */
  void writeDecoderConfigurationRecord(BitStream& bitstream); /* writes the previously set AVC Decoder Configuration Record */
  void rewriteMetaData(); 
  void rewriteFLVTag(FLVTag& tag, BitStream& bitstream, size_t offset, size_t previousSize); /* rewrites the given flvtag in `bitstream`, starting at offset and replacing all `previousSize` bytes. */
  void appendVideoHeader(FLVTag& tag, BitStream& bitstream); /* appends the necessary data to the bitstream (bs) for the given tag, code flow is: writeVideoTag() --> writeFLVTag() { --> appendVideoHeader() } -->  */
  void appendAudioHeader(FLVTag& tag, BitStream& bitstream); 
  bool validateAudioSettings(); /* when the video codec has been set we check if all necessary members are initialized correctly */
  bool validateVideoSettings(); /* when the audio codec has been set we check the other sound properties too */
  bool hasAudio();
  bool hasVideo();
  void createMetaData(BitStream& s);
 private:
  AVCDecoderConfigurationRecord avc_cfg;
  uint8_t audio_codec;
  uint8_t video_codec;
  uint8_t audio_samplerate;
  uint8_t audio_datarate;
  uint8_t audio_type;
  uint8_t audio_size;
  double audio_delay;
  bool can_seek_to_end;
  double duration;
  double fps;
  double height;
  double width;
  double video_data_rate;
  BitStream& bs;
  size_t metadata_offset;  /* after adding all tags we can rewrite the metadata so it can adjust some fields it can only know when all frames have been written */
  size_t metadata_size;  /* the size of the metadata flv tag */
  uint32_t last_timestamp; /* we keep track of the last timestamp to reset the duration meta data field */
  FLVListeners listeners; /* we will notify listeners whenever we generate a new FLVTag in the bitstream */
};

inline void FLVWriter::setDecoderConfigurationRecord(AVCDecoderConfigurationRecord rec) {
  avc_cfg = rec;
}

inline void FLVWriter::setAudioCodec(uint8_t codec) {
  audio_codec = codec;
}

inline void FLVWriter::setVideoCodec(uint8_t codec) {
  video_codec = codec;
}

inline void FLVWriter::setAudioSampleRate(uint8_t rate) {
  audio_samplerate = rate;
}

inline void FLVWriter::setAudioDataRate(uint8_t rate) {
  audio_datarate = rate;
}

inline void FLVWriter::setAudioSize(uint8_t size) {
  audio_size = size;
}

inline void FLVWriter::setAudioType(uint8_t type) {
  audio_type = type;
}

inline void FLVWriter::setAudioDelay(double d) {
  audio_delay = d;
}

inline void FLVWriter::setCanSeekToEnd(bool s) {
  can_seek_to_end = s;
}

inline void FLVWriter::setFrameRate(double r) {
  fps = r;
}

inline void FLVWriter::setHeight(double h) {
  height = h;
}

inline void FLVWriter::setWidth(double w) {
  width = w;
}

inline void FLVWriter::setVideoDataRate(double r) {
  video_data_rate = r;
}

inline bool FLVWriter::hasAudio() {
  return audio_codec != FLV_SOUNDFORMAT_UNKNOWN;
}

inline bool FLVWriter::hasVideo() {
  return video_codec != FLV_VIDEOCODEC_UNKNOWN;
}

inline void FLVWriter::writeVideoTag(FLVTag& tag) {
  tag.makeVideoTag();
  tag.setVideoCodec(video_codec);
  tag.setAVCPacketType(FLV_AVC_NALU);
  writeFLVTag(tag, bs);
}

inline void FLVWriter::writeAudioTag(FLVTag& tag) {
  tag.makeAudioTag();
  tag.setAudioCodec(audio_codec);
  writeFLVTag(tag, bs);
}

inline void FLVWriter::addListener(FLVListener* l) {

  if(bs.size()) {
    printf("warning: you're adding a listener while there is already some FLV data; the listener might need this!\n");
  }

  listeners.addListener(l);
}

#endif
