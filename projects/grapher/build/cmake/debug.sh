#!/bin/sh
set -x
d=${PWD}
bd=${d}/../../../install/bin
appdir=${bd}/../../

# get app name
cd ${appdir}
app=${PWD##*/}

if [ ! -d ${bd} ] ; then 
   mkdir ${bd}
   mkdir ${bd}/data
fi

cd ${d}
./build_debug.sh

cd ${bd}

lldb ./grapher_debug


