#include <iostream>
#include <streamer/core/RTMPWriter.h>

// ---------------------------------------------------

RTMPData::RTMPData()
  :timestamp(0)
{
}

// ---------------------------------------------------

RTMPWriter::RTMPWriter() 
  :rtmp(NULL)
  ,is_initialized(false)
{
  printf("RTMPWriter().\n");

  //#if !defined(NDEBUG)
  RTMP_LogSetLevel(RTMP_LOGDEBUG);
  RTMP_LogSetOutput(stderr);
  //#endif

  rtmp = RTMP_Alloc();
  if(!rtmp) {
    printf("error: cannot allocate the rtmp context.\n");
    ::exit(EXIT_FAILURE);
  }
}

RTMPWriter::~RTMPWriter() {

  if(rtmp && is_initialized) {
    RTMP_Close(rtmp);
  }

  if(rtmp) {
    RTMP_Free(rtmp);
    rtmp = NULL;
  }
}

bool RTMPWriter::initialize() {

  if(!settings.url.size()) {
    printf("error: cannot initialize the RTMP Writer, no url set, call setURL() first.\n");
    return false;
  }

  if(is_initialized) {
    printf("error: already initialized.\n");
    return false;
  }

  RTMP_Init(rtmp);

  printf("url: %s\n", settings.url.c_str());
  if(!RTMP_SetupURL(rtmp, (char*)settings.url.c_str())) {
    printf("error: cannot setup the url for the RTMP Writer.\n");
    return false;
  }

  #if 1
  std::string username = "123194";
  std::string password = "ROXLU";
  rtmp->Link.pubUser.av_val = (char*)username.c_str();
  rtmp->Link.pubUser.av_len = username.size();
  rtmp->Link.pubPasswd.av_val = (char*)password.c_str();
  rtmp->Link.pubPasswd.av_len = password.size();
  rtmp->Link.flashVer.av_val = "FMLE/3.0 (compatible; FMSc/1.0)";
  rtmp->Link.flashVer.av_len = (int)strlen(rtmp->Link.flashVer.av_val);
  #endif

  RTMP_EnableWrite(rtmp);

  if(!RTMP_Connect(rtmp, NULL)) {
    printf("error: cannot connect to the rtmp server: %s\n", settings.url.c_str());
    RTMP_Free(rtmp);
    rtmp = NULL;
    return false;
  }

  if(!RTMP_ConnectStream(rtmp, 0)) {
    printf("error: cannot connect to the rtmp stream on %s.\n", settings.url.c_str());
    RTMP_Free(rtmp);
    rtmp = NULL;
    return false;
  }

  is_initialized = true;

  return true;
}

void RTMPWriter::write(uint8_t* data, size_t nbytes) {

  if(!is_initialized) {
    printf("error: cannot write to rtmp server because we haven't been initialized. did you call initialize()?\n");
    return;
  }

  int r = RTMP_Write(rtmp, (const char*)data, (int)nbytes);
  if(!r) {
    printf("error: something went wrong while trying to write data to the rtmp server.\n");
  }
}

void RTMPWriter::read() {

  if(!is_initialized) {
    printf("error: cannot read because we're not initialized.\n");
    return;
  }

  //RTMPPacket pkt = {0};
  char buf[512];
  int r = RTMP_Read(rtmp, buf, sizeof(buf));
  printf("read: >> %d << \n", r);

}
