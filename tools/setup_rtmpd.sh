#!/bin/bash
d=${PWD}

if [ ! -d ${d}/rtmpd ] ; then 
    mkdir ${d}/rtmpd 
  
    cd ${d}/rtmpd
    svn co --username anonymous --password "" https://svn.rtmpd.com/crtmpserver/trunk/ .
    
    if [ "${OSTYPE}" = "darwin10.0" ] ; then 
        svn up -r 760
    elif [ "${OSTYPE}" = "darwin12" ] ; then 
        svn up -r 799
    fi

    cd ${d}/rtmpd/builders/cmake
    ./run
fi
