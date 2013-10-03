#include <signal.h>
#include <iostream>
#include <streamer/core/RTMPWriter.h>

// ---------------------------------------------------

void rtmp_sigpipe_handler(int signum) {
  printf("got sigpipe!\n");
}

// ---------------------------------------------------

RTMPData::RTMPData()
  :timestamp(0)
{
}

// ---------------------------------------------------

RTMPWriter::RTMPWriter() 
  :rtmp(NULL)
  ,state(RW_STATE_NONE)
  ,cb_disconnect(NULL)
  ,cb_user(NULL)
{

  //#if !defined(NDEBUG)
  RTMP_LogSetLevel(RTMP_LOGDEBUG);
  RTMP_LogSetOutput(stderr);
  //#endif

  signal(SIGPIPE, rtmp_sigpipe_handler);
  // signal(SIGPIPE, SIG_IGN);
  
}

RTMPWriter::~RTMPWriter() {

  cb_disconnect = NULL;
  cb_user = NULL;

  if(rtmp && state == RW_STATE_INITIALIZED) {
    RTMP_Close(rtmp);
  }

  if(rtmp) {
    RTMP_Free(rtmp);
    rtmp = NULL;
  }

  state = RW_STATE_NONE;

}

bool RTMPWriter::initialize() {

  if(!settings.url.size()) {
    printf("error: cannot initialize the RTMP Writer, no url set, call setURL() first.\n");
    return false;
  }

  if(state == RW_STATE_INITIALIZED) {
    printf("error: already initialized.\n");
    return false;
  }

  if(rtmp) {
    printf("error: already initialized a rtmp context, not creating another one!\n");
    ::exit(EXIT_FAILURE);
  }

  rtmp = RTMP_Alloc();
  if(!rtmp) {
    printf("error: cannot allocate the rtmp context.\n");
    ::exit(EXIT_FAILURE);
  }

  RTMP_Init(rtmp);

  if(!RTMP_SetupURL(rtmp, (char*)settings.url.c_str())) {
    printf("error: cannot setup the url for the RTMP Writer.\n");
    RTMP_Free(rtmp);
    rtmp = NULL;
    return false;
  }

  if(settings.username.size()) {
    rtmp->Link.pubUser.av_val = (char*)settings.username.c_str();
    rtmp->Link.pubUser.av_len = settings.username.size();
  }

  if(settings.password.size()) {
    rtmp->Link.pubPasswd.av_val = (char*)settings.password.c_str();
    rtmp->Link.pubPasswd.av_len = settings.password.size();
  }

  rtmp->Link.flashVer.av_val = (char*)"FMLE/3.0 (compatible; FMSc/1.0)"; // when streaming to a FMS you need this!
  rtmp->Link.flashVer.av_len = (int)strlen(rtmp->Link.flashVer.av_val);

  RTMP_EnableWrite(rtmp);

  if(!RTMP_Connect(rtmp, NULL)) {
    printf("error: cannot connect to the rtmp server: %s\n", settings.url.c_str());
    RTMP_Free(rtmp);
    rtmp = NULL;
    /*
    if(state == RW_STATE_RECONNECTING) {
      state = RW_STATE_NONE;
      printf("@todo need to call the disconnect callback.\n");
      //reconnect(); 
     }
    */

    return false;
  }

  if(!RTMP_ConnectStream(rtmp, 0)) {
    printf("error: cannot connect to the rtmp stream on %s.\n", settings.url.c_str());
    RTMP_Free(rtmp);
    rtmp = NULL;

    if(state == RW_STATE_RECONNECTING) {
      state = RW_STATE_NONE;
      // reconnect(); 
    }

    return false;
  }

  state = RW_STATE_INITIALIZED;

  return true;
}

void RTMPWriter::write(uint8_t* data, size_t nbytes) {

  if(state == RW_STATE_NONE) {
    printf("error: cannot write to rtmp server because we haven't been initialized. did you call initialize()?\n");
    return;
  }
  else if(state == RW_STATE_RECONNECTING) {
    //printf("reconnecting... ignoring data...\n");
    return;
  }
  else if(state == RW_STATE_DISCONNECTED) {
    // the caller needs to call reconnect() 
    return;
  }
  
  //printf("rtmp: %ld\n", nbytes);
  int r = RTMP_Write(rtmp, (const char*)data, (int)nbytes);
  if(r < 0) {

    // @todo - we should close and cleanup here!!!!
    RTMP_Close(rtmp);
    RTMP_Free(rtmp);
    rtmp = NULL;

    printf("error: something went wrong while trying to write data to the rtmp server.\n");
    if(state == RW_STATE_DISCONNECTED) {
      return;
    }
    // when initialized and we arrive here, it means we're disconnected
    else if(state == RW_STATE_INITIALIZED) {

      state = RW_STATE_DISCONNECTED;
      if(cb_disconnect) {
        cb_disconnect(this, cb_user);
      }

    }
  }
}

void RTMPWriter::reconnect() {

  if(state == RW_STATE_RECONNECTING) {
    printf("warning: already reconnecting ....");
    return;
  }

  close();

  state = RW_STATE_RECONNECTING;

  initialize();
}

void RTMPWriter::close() {

  if(rtmp) {
    RTMP_Close(rtmp);
    RTMP_Free(rtmp);
    rtmp = NULL;
  }

}

// @todo - read() is blocking, we would need to handle handle the socket ourself
void RTMPWriter::read() {

  if(state != RW_STATE_INITIALIZED) {
    printf("error: cannot read because we're not initialized.\n");
    return;
  }

  char buf[512];
  int r = RTMP_Read(rtmp, buf, sizeof(buf));
  printf("read: >> %d << \n", r);

}
