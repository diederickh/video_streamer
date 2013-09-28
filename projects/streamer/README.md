# VideoStreamer

## Compile info
Add _VIDEO_STREAMER_LITTLE_ENDIAN_ on little endian systems.

## Parsing a FLV file
Use a _BitStream_ class with the _FLVReader_ to inspec the _FLVTags_ in a FLV file. 
The _FLVReader_ was creatd for debugging purposes and implements all the necessary 
parsing for elements that we use to stream. 

````c++
#include <core/BitStream.h>
#include <flv/FLVReader.h>

BitStream bs;
bs.loadFile("./barsandtone.flv");

FLVReader flv(bs);
flv.parse();
````

## Setting up C++ RTMP Server
The [C++ RTMP Server](http://www.rtmpd.com/) is a kind of unknown but very
good open source streaming server. It uses cmake to compile and it's just 
a couple of steps to set it up. 

_Download the sources (done in tools/setup_rtmpd.sh)_
````sh
cd rmtpd
svn co --username anonymous --password "" https://svn.rtmpd.com/crtmpserver/trunk/ .        
````

### FLV playback

First thing to do is testing a simple FLV playback. Download a flv file 
e.g. [this one](http://www.mediacollege.com/video-gallery/testclips/barsandtone.flv) and put 
it in some directory `/Users/roxlu/media`. The only thing we need to do now is to configure
rtmpd so it knows where to find the flv files. rtmpd uses a lua script for all the 
configuration which can be foudn in `builders/cmake/crtmpserver/crtmpserver.lua`. 
Search for `mediaFolder' as that is the config property that tells the server where
to find files. Some examples are names like: `namedStorage1` and `namedStorage2` ..etc,
ignore those as we don't need a named storage. At the bottom of the `mediaStorage` 
property there is an unnamed sections. This is the one you need (i've removed some 
parts, this is just an example)

````lua
mediaStorage = {
  namedStorage1={       
  }
  -- this is the property that you want to use:
  ,{
     mediaFolder="/Users/roxlu/media/"
  }
}
```` 

_Create a HTML page with a flash player_
To playback the flv file we need a flash player. I've been using the [flowplayer](http://flash.flowplayer.org/download/)
with the [rtmp plugin](http://flash.flowplayer.org/plugins/streaming/rtmp.html). Flowplayer has
great information on how to setup a basic player. The steps you need to take are

 - download the flow player + rtmp plugin 
 - create a index.html
 - create a javascript file with the code that initializes the player.

_Start C++ RTMP server_
````sh
cd rtmpd\builders\cmake\
./run
````

### Live stream playback
To create a live stream playback you need to first setup rtmpd server as described above. Because
I'm using flowplayer I followed the documentation that describes how to setup a live video player
using their RTMP plugin. See [this page for more information](http://flash.flowplayer.org/plugins/streaming/rtmp.html#live). 
It all comes down to creating some javascript which sets the live properties; e.g.

_Flow player live playback setup_
````javascript
function setup_live_playback() {
  $f("live_container", "swf/flowplayer-3.2.16.swf", {
    clip: {
      url: "livestream"
      ,live: true
      ,provider:"rtmpd"
    }
    ,plugins: {
      rtmpd: {
        url: "swf/flowplayer.rtmp-3.2.12.swf"
        ,netConnectionUrl: "rtmp://localhost/flvplayback/"
      }
    }
  });
}
````

_Flash Media Encoder setup_
The default configuration file for the C++ RTMP Server contains everything you need
to create a live stream. The RTMP server has a application called "flvplayback" that 
can be used as an entry point for incoming live video. Each stream needs a name so 
you can reference it in your video player. To get a live stream into the 
player you need some software that streams the video to the C++ RTMP server.  I used 
the Flash Media Encoder (FMLE) as a test. You need to tell the FMLE to what server it 
needs to publish the video stream; you can add the necessary settings in the right 
part of the application. You can use the following settings. The `FMS URL` is the 
IP/domain on which the C++ RTMP server is running, the `stream` is the name of the 
stream you want to use; you can use any name you wish.

````sh
FMS URL: rtmp://localhost/flvplayback
Stream: livestream
````


## How to debug your FLV streams

While working on a streaming video library it's a wise choice to look at already
existing applications and inspect the data they are producing so you have something to 
compare your own streams with. I used Flash Media Live Encoder to produce a h264 stream
from a webcam with mp3 audio and used rtmpdump to capture the stream and write it 
to a file. In FMLE you have to define an URL + Stream name. This is all the data you 
need to know. Use the following command to capture the stream:

````sh
./rtmpdump -r "rtmp://192.168.0.210/flvplayback/livestream" -f"LNX 11,6,602,180" -v "rtmp://192.168.0.210/flvplayback/livestream" -o out.flv
````

_Or in short_
````sh
./rtmpdump -r "[FMS_URL]/[STREAM]" -f"LNX 11,6,602,180" -v "[FMS_URL]/[STREAM]" -o out.flv
````

FMS_URL is the same as the value you used in FMLE, and STREAM is the "Stream" value you 
used in FMLE.


### X264
- call x264_param_apply_profile() after setting your custom settings; it restricts the options to the profile
- settins i_bframe = 2, makes sure that you're using main profile, else x264 will go back to baseline
- `./x264 --crf 24 -o out.flv --input-res 240x320 -o out.flv raw.yuv`
- `./x264 --crf 24 -o out.flv --input-res 240x320 --bframes 2 --preset ultrafast --tune zerolatency --profile main -o out.flv raw.yuv`
- make sure to minimize the number of I-frames, and maximize the number of B/P-frames
- good info on parameters: http://obsproject.com/forum/viewtopic.php?f=18&t=642
- you can use inputs like I420, I422 etc.. x264 will encode to the same format as the input

_Settings_

- _vbv_: Video Buffer Verifier, in x264 is used to control output bitrate for streaming.
- _weightp_: some players do not support weighted p-frames, set weightp = 0, for flash.

### Dependencies

VideoStreamer depends on the following libraries:

- libuv
- x264
- librtmp
- openssl 
- zlib
- lamemp3

References
[0] = X264 for live streaming: http://veetle.com/index.php/article/view/x264
[1] = Some good info on settings: http://doom10.org/index.php?topic=2084.0 
[3] = Adobe H264 info: http://www.adobe.com/devnet/adobe-media-server/articles/h264_encoding.html 

### Notes:
- make sure that all packets you send are 100% ascending order of timestamps
- when you have unsync audio/video you could try:
  - start by using the default parameters, this worked for me:
  ````c++
  p->i_threads = 1;
  p->i_width = settings.width;
  p->i_height = settings.height;
  p->i_fps_num = settings.fps;
  p->i_fps_den = 1;
  p->b_annexb = 0; 
  ````
  - using a i_keyint_max = 5 value, and using 'framenums' as pic_in.i_pts (for x264_encoder_encode) makes the video in sync with audio.
  - use no compression with h264 at all, but using:
  ````c++
  x264_param_t p;
  p.rc.i_rc_method = X264_RC_CQP;
  p.rc.i_qp_constant = 0
  ````
- when you need to connect to a flash media server you need to specify a correct
  flash version string in the rtmp instance, see RTMPWriter::initialize().
- when you need to authenticate you need to set the `rtmp->Link.pubUser` and
  `rtmp->Link.pubPasswd`


