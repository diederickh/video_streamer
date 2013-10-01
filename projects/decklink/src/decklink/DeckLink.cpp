#include <decklink/DeckLink.h>
#include <iostream>

#if defined(_WIN32)
#  include <comutil.h>
#endif

DeckLink::DeckLink() 
  :card(NULL)
{
#if defined(_WIN32)
  CoInitialize(NULL);
#endif
}

DeckLink::~DeckLink() {
  if(card) {
    delete card;
  }
  card = NULL;
}

bool DeckLink::setup(int device) {

  if(card) {
    printf("error: already setup a decklink device; only call setup once.\n");
    return false;
  }

#if defined(_WIN32)
  IDeckLinkIterator* dl_iterator = NULL;
  HRESULT r = CoCreateInstance(CLSID_CDeckLinkIterator, NULL, CLSCTX_ALL, IID_IDeckLinkIterator, (void**)&dl_iterator);
  if(r != S_OK) {
    printf("error: cannot get the decklink iterator.\n");
    return false;
  }
#else
  IDeckLinkIterator* dl_iterator = CreateDeckLinkIteratorInstance();
  if(!dl_iterator) {
    printf("error: cannot get the decklink iterator.\n");
    return false;
  }
#endif

  int i = 0;
  IDeckLink* dl_decklink;
  while(dl_iterator->Next(&dl_decklink) == S_OK) {

    if(i == device) {
      card = new DeckLinkCard(dl_decklink);
      dl_decklink->Release();
      dl_decklink = NULL;
      break;
    }

    dl_decklink->Release();
    dl_decklink = NULL;
    ++i;
  }

  dl_iterator->Release();
  dl_iterator = NULL;

  if(!card) {
    printf("error: cannot find the given device: %d\n", device);
    return false;
  }

  return true;
}

bool DeckLink::printDevices() {

#if defined(_WIN32)
  IDeckLinkIterator* dl_iterator = NULL;
  HRESULT r = CoCreateInstance(CLSID_CDeckLinkIterator, NULL, CLSCTX_ALL, IID_IDeckLinkIterator, (void**)&dl_iterator);
  if(r != S_OK) {
    printf("error: cannot get the decklink iterator.\n");
    return false;
  }
#else 
  IDeckLinkIterator* dl_iterator = CreateDeckLinkIteratorInstance();
  if(!dl_iterator) {
    printf("error: cannot get the decklink iterator.\n");
    return false;
  }
#endif

  int i = 0;
  IDeckLink* dl_decklink;
  while(dl_iterator->Next(&dl_decklink) == S_OK) {
    printf("\nDevice %d\n", i);
    printf("---------------------------------------------------\n");
    getCapabilities(dl_decklink);
    printf("---------------------------------------------------\n");

    dl_decklink->Release();
    dl_decklink = NULL;

    ++i;
  }

  dl_iterator->Release();
  dl_iterator = NULL;

  return true;
}

bool DeckLink::getCapabilities(IDeckLink* dl) {

  // PRINT SOME ATTRIBUTES
  // ---------------------------------------------------------------------
#if defined(_WIN32)
  BSTR model_name = NULL;
  HRESULT r = dl->GetModelName(&model_name);
  if(r != S_OK) {
    printf("error: cannot get the model name.\n");
    return false;
  }
  _bstr_t model_name_str(model_name, false);
  printf("> Model name: %s\n", (char*)model_name_str);

#else 

  CFStringRef model_name;
  HRESULT r = dl->GetModelName(&model_name);
  if(r != S_OK) {
    printf("error: cannot get the model name.\n");
    CFRelease(model_name);
    return false;
  }

  char name_buf[64];
  CFStringGetCString(model_name, name_buf, 64, kCFStringEncodingMacRoman);
  CFRelease(model_name);

  printf("> Model name: %s\n", name_buf);
#endif

  IDeckLinkAttributes* attr = NULL;
  r = dl->QueryInterface(IID_IDeckLinkAttributes, (void**)&attr);
  if(r != S_OK) {
    printf("error: cannot query the IDeckLinkAttributes.\n");
    return false;
  }
#if defined(_WIN32)
  BOOL supported = false;
  LONGLONG count = 0;
#else
  bool supported = false;
  int64_t count = 0;
#endif
  
  // serial port
  r = attr->GetFlag(BMDDeckLinkHasSerialPort, &supported);
  if(r != S_OK) {
    printf("error: cannot get serial port flag.\n");
    return false;
  }
  printf("> Has serial port: %c\n", (supported) ? 'y' : 'n');
  
  // subdevices
  r = attr->GetInt(BMDDeckLinkNumberOfSubDevices, &count);
  if(r != S_OK) {
    printf("erorr: cannot get number of sub devices.\n");
    return false;
  }
  printf("> Number of subdevices: %lld\n", count);

  // audio channels
  r = attr->GetInt(BMDDeckLinkMaximumAudioChannels, &count);
  if(r != S_OK) {
    printf("error: cannot get the max. num. audio channeld.\n");
    return false;
  }
  printf("> Maximum number of audio channels: %lld\n", count);

  // internal keying
  r = attr->GetFlag(BMDDeckLinkSupportsInternalKeying, &supported);
  if(r != S_OK) {
    printf("error: cannot get the internalkeying flag.\n");
    return false;
  }
  printf("> Has internal keying: %c\n", (supported) ? 'y' : 'n');

  // PRINT INPUT INFORMATION
  // ---------------------------------------------------------------------
  IDeckLinkInput* input = NULL;
  r = dl->QueryInterface(IID_IDeckLinkInput, (void**)&input);
  if(r != S_OK) {
    printf("error: cannot query the IID_IDeckLinkInput interface.\n");
    return false;
  }

  IDeckLinkDisplayModeIterator* mode_iter = NULL;
  r = input->GetDisplayModeIterator(&mode_iter);
  if(r != S_OK) {
    printf("error: cannot get the IDeckLinkDisplayModeIterator.\n");
    return false;
  }
  
  printf("> Modes:\n");
  int i = 0;
  IDeckLinkDisplayMode* mode = NULL;
  while(mode_iter->Next(&mode) == S_OK) {

#if defined(_WIN32)
    BSTR mode_bstr = NULL;
    r = mode->GetName(&mode_bstr);
    if(r != S_OK) {
      printf("error: cannot get the display mode name.\n");
      i++;
      return false;
    }
    
    _bstr_t mode_name(mode_bstr, false);
    printf("  [%d] %s\n", i, (char*) mode_name);
#else

  CFStringRef mode_name_tmp;
  HRESULT r = dl->GetModelName(&mode_name_tmp);
  if(r != S_OK) {
    printf("error: cannot get the display mode name.\n");
    CFRelease(mode_name_tmp);
    return false;
  }

  char mode_name_buf[512];
  CFStringGetCString(mode_name_tmp, mode_name_buf, 512, kCFStringEncodingMacRoman);
  CFRelease(mode_name_tmp);

  printf("  [%d] %s\n", i, mode_name_buf);
#endif
    mode->Release();

    i++;
  }
  
  mode_iter->Release();
  mode_iter = NULL;
  
  attr->Release();
  attr = NULL;

  input->Release();
  input = NULL;
  
  return true;
}
