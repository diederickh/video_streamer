#include <iostream>
#include <portaudio/PAudio.h>


// ------------------------------------------------------------------------

int paudio_in(const void* input, void* output, unsigned long nframes,
                      const PaStreamCallbackTimeInfo* time, 
                      PaStreamCallbackFlags status, void* userData)
{
  
  PAudio* a = static_cast<PAudio*>(userData);
  a->cb_audio(input, nframes, a->cb_user);
  return 0; // 0 = continue, 1 = stop
}

// ------------------------------------------------------------------------

PAudio::PAudio() 
  :input_stream(NULL)
  ,cb_audio(NULL)
  ,cb_user(NULL)
{

  PaError err = Pa_Initialize();
  if(err != paNoError) {
    printf("error: cannot initialize port audio.\n");
    ::exit(EXIT_FAILURE);
  }

}

PAudio::~PAudio() {

  if(input_stream) {
    stop();
  }

}

int PAudio::listDevices() {

  int num = Pa_GetDeviceCount();
  if(num <= 0) {
    printf("error: cannot find any port audio devices.\n");
    return 0;
  }

  const PaDeviceInfo* dev_info;
  for(int i = 0; i < num; ++i) {
    dev_info = Pa_GetDeviceInfo(i);
    printf("[%d] = %s, input channels: %d, output channels: %d, default samplerate: %f\n",
           i, dev_info->name, dev_info->maxInputChannels, 
           dev_info->maxOutputChannels, dev_info->defaultSampleRate);
           
  }

  return num;
}

int PAudio::getDefaultInputDevice() {
  return Pa_GetDefaultInputDevice();
}

bool PAudio::isInputFormatSupported(int device, int nchannels, PaSampleFormat format, double samplerate) {
  PaStreamParameters input;
  PaError err;

  input.channelCount = nchannels;
  input.device = device;
  input.hostApiSpecificStreamInfo = NULL;
  input.sampleFormat = format;
  input.suggestedLatency = Pa_GetDeviceInfo(device)->defaultLowInputLatency;

  err = Pa_IsFormatSupported(&input, NULL, samplerate);
  if(err == paFormatIsSupported) {
    return true;
  }
  else {
    return false;
  }
}


bool PAudio::openInputStream(int device, int nchannels, 
                             PaSampleFormat format, double samplerate, 
                             unsigned long framesPerBuffer) 
{

  PaDeviceIndex total_devices = Pa_GetDeviceCount();
  if(!total_devices || device >= total_devices) {
    printf("error: unknown device.\n");
    return false;
  }

  if(!isInputFormatSupported(device, nchannels, format, samplerate)) {
    printf("error: cannot open input stream, format not supported.\n");
    return false;
  }

  PaStreamParameters params;
  memset(&params, 0, sizeof(params));

  params.channelCount = nchannels;
  params.device = device;
  params.hostApiSpecificStreamInfo = NULL;
  params.sampleFormat = format;
  params.suggestedLatency = Pa_GetDeviceInfo(device)->defaultLowInputLatency;

  PaError err = Pa_OpenStream(&input_stream, &params, NULL
                              ,samplerate, framesPerBuffer, paClipOff
                              ,paudio_in, (void*) this);

  if(err != paNoError) {
    printf("error: cannot open input stream: %s\n", Pa_GetErrorText(err));
    return false;
  }
   
  return true;
}


bool PAudio::start() {

  if(!cb_audio) {
    printf("error: cannot start the audio input because you have no yet set a callback. make sure that you called setCallback before calling start().\n");
    return false;
  }

  if(input_stream == NULL) {
    printf("error: cannot start  the audio input stream, because input_stream is null. did you call openInputStream?\n");
    return false;
  }

  PaError err = Pa_StartStream(input_stream);
  if(err != paNoError) {
    printf("error: cannot start input stream. Pa_StartStream returned an error.\n");
    return false;
  }

  return true;
}

bool PAudio::stop() {

  if(!input_stream) {
    return true;
  }

  PaError err = Pa_StopStream(input_stream);
  if(err != paNoError) {
    printf("error: cannot stop the input stream.\n");
    return false;
  }
  
  return true;
}

