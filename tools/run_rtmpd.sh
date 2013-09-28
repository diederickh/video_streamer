#!/bin/bash

d=${PWD}
if [ ! -d ${d}/rtmpd ] ; then 
    ./setup.sh
fi

cd ${d}/rtmpd/builders/cmake
./crtmpserver/crtmpserver crtmpserver/crtmpserver.lua
