#!/bin/bash
d=${PWD}

if [ ! -d ${d}/rtmpd ] ; then 
    mkdir ${d}/rtmpd 
  
    cd ${d}/rtmpd
    svn co --username anonymous --password "" https://svn.rtmpd.com/crtmpserver/trunk/ .
    
    if [ "${OSTYPE}" = "darwin10.0" ] ; then 
        svn up -r 760
    elif [ "${OSTYPE}" = "darwin12" ] ; then 
        # svn up -r 799 // worked with darwin 12.?? not with  x86_64-apple-darwin12.5.0, 
        # latest test with r 811 worked on mac 10.8.5
    fi

    cd ${d}/rtmpd/builders/cmake
    ./run
fi
