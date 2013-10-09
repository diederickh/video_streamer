# VideoStreamer

_Documentation will be added when we have a working version of this lib_


## Prepare

- Download the DeckLink SDK and for mac, copy the include directory to _projects/decklink/sdk/include_

This library makes use of the following libraries:

- x264
- rapidxml
- glxw (only for opengl apps)
- glfw3 (only for opengl apps)
- mp3lame 
- libfaac
- libyuv
- openssl
- zlib
- nanomsg (only for daemon, which is under development)
- librtmp
- libuv 

_Build system command_
````sh
python rbs.py -t build -c clang -b debug -a 64 -s rapidxml,glxw,glfw,uv,lamemp3,rtmp,x264,openssl,zlib,nanomsg,libyuv
````

## Things to be aware of

_Windows .idl files_

The DeckLink SDK uses .idl file that are used to genereate code (header/c files).
When you want to use this code, make sure that you have not defined WIN32_LEAN_AND_MEAN
as this seems to give problems with the _interface_ keyword, which is a MSVC++ 
thing. Also, make sure to first include the DeckLink header (<decklink/DeckLink.h>)
and then the windows.h if you need it. Also, make sure to include the 
<DeckLinkAPI.h> (generate .h file)  as soon as possible