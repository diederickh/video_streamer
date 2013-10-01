#!/bin/sh

d=${PWD}
if [ ! -d ${d}/packaged ] ; then 
    mkdir ${d}/packaged
fi

cd ${d}/packaged
cmake ../
cmake --build . --target install
