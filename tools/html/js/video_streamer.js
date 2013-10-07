function setup_flv_playback() {

  $f("playback_container", "swf/flowplayer-3.2.16.swf", {
    clip: {
      url: "raw.flv"
      ,live: false
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
        ,netConnectionUrl: "rtmp://test.home.roxlu.com/flvplayback/"
      }
    }
  });

}

function setup_rtmpdump_playback() {
  $f("rtmp_container", "swf/flowplayer-3.2.16.swf", {
    clip: {
      url: "dump.flv"
      ,live: false
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


function setup_raw_playback() {
  $f("raw_container", "swf/flowplayer-3.2.16.swf", {
    clip: {
      url: "raw.flv"
      ,live: false
      ,provider:"rtmpd"
/*      ,bufferLength:5 */
    }
    ,plugins: {
      rtmpd: {
        url: "swf/flowplayer.rtmp-3.2.12.swf"
        ,netConnectionUrl: "rtmp://localhost/flvplayback/"
      }
    }
  });
}

function setup_wowza_playback() {

  $f("wowza_container", "swf/flowplayer-3.2.16.swf", {
    clip: {
      url: "livestream"
      ,live: true
      ,provider:"rtmpd"
    }
    ,plugins: {
      rtmpd: {
        url: "swf/flowplayer.rtmp-3.2.12.swf"
        ,netConnectionUrl: "rtmp://test.home.roxlu.com/flvplayback/"
      }
    }
  });

}


function setup_wowza_live_playback() {

  $f("wowza_live_container", "swf/flowplayer-3.2.16.swf", {
    clip: {
      url: "live"
      ,live: true
      ,provider:"rtmpd"
    }
    ,plugins: {
      rtmpd: {
        url: "swf/flowplayer.rtmp-3.2.12.swf"
//        ,netConnectionUrl: "rtmp://test.home.roxlu.com/flvplayback/"
        ,netConnectionUrl: "rtmp://localhost/flvplayback/"
      }
    }
  });

}

function setup_file_playback() {

  flowplayer("file_container", {
      src:"swf/flowplayer-3.2.16.swf"
     },
     {
       clip: "swf/raw.flv?" +new Date()
//       clip: "swf/valid.mp4?" +new Date()
     }
  );
}

function setup_akamai_playback() {
  $f("akamai_container", "swf/flowplayer-3.2.16.swf", {
    clip: {
      url: "Nike_3000@141936"
      ,live: true
      ,provider:"rtmpd"
    }
    ,plugins: {
      rtmpd: {
        url: "swf/flowplayer.rtmp-3.2.12.swf"
        ,netConnectionUrl: "rtmp://cp123194.live.edgefcs.net/live/Nike_3000@141936"
        //,netConnectionUrl: "rtmp://p.ep141936.i.akamaientrypoint.net/EntryPoint/"
      }
    }
  });
}

function setup_quality0_playback() {
  $f("quality0_live_container", "swf/flowplayer-3.2.16.swf", {
    clip: {
      url: "quality0"
      ,live: true
      ,provider:"rtmpd"
    }
    ,plugins: {
      rtmpd: {
        url: "swf/flowplayer.rtmp-3.2.12.swf"
        ,netConnectionUrl: "rtmp://test.home.roxlu.com/flvplayback/"
      }
    }
  });
}

function setup_quality1_playback() {
  $f("quality1_live_container", "swf/flowplayer-3.2.16.swf", {
    clip: {
      url: "quality1"
      ,live: true
      ,provider:"rtmpd"
    }
    ,plugins: {
      rtmpd: {
        url: "swf/flowplayer.rtmp-3.2.12.swf"
        ,netConnectionUrl: "rtmp://test.home.roxlu.com/flvplayback/"
      }
    }
  });
}




function video_streamer_init() {
  if(document.location.origin == "file://") {
    alert("Open this file from a webserver");
    //return;
  }

  //setup_flv_playback();
  //setup_akamai_playback();
  //setup_live_playback();
  setup_wowza_playback();
  setup_wowza_live_playback();
  setup_quality0_playback();
  setup_quality1_playback();
  //setup_raw_playback();
  //setup_rtmpdump_playback();
  //setup_file_playback();
}
