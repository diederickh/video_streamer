# Supported compilers
ROXLU_CLANG=1
ROXLU_GCC=2

# Create the build.debug, build.release, etc.. directories when they don't exist
roxlu_check_build_dirs() {
    d=${PWD}
    bd=${d}/../../bin/

    if [ ! -d ${d}/build.debug ] ; then 
        mkdir ${d}/build.debug
    fi
      
    if [ ! -d ${d}/build.release ] ; then 
        mkdir ${d}/build.release
    fi

    if [ ! -d ${d}/build.xcodeproject ] ; then
        mkdir ${d}/build.xcodeproject
    fi
}

roxlu_remove_build_dirs() {
    d=${PWD}

    if [ -d ${d}/build.release ] ; then 
        rm -rf ${d}/build.release
    fi

    if [ -d ${d}/build.debug ] ; then 
        rm -rf ${d}/build.debug
    fi

    if [ -d ${d}/build.xcodeproject ] ; then 
        rm -rf ${d}/build.xcodeproject
    fi
}


# Set the export CC and CXX variables to the default compiler; we default to clang on mac, gcc on linux
# but only when CC and CXX haven't been set yet
# you can pass a parameter that defines the compiler:
# e.g. roxlu_check_compiler ROXLU_CLANG
# e.g. roxlu_check_compiler ROXLU_GCC
roxlu_check_compiler() {

    if [ "${1}" = "" ] ; then
        return 
    fi

    if [ ${1} = ROXLU_CLANG ] ; then 
        roxlu_use_clang
    elif [ ${1} = ROXLU_GCC ] ; then 
        roxlu_use_gcc
    else
        roxlu_use_clang
    fi
}

roxlu_use_clang() {

    if [ -f /usr/bin/clang ] ; then
        export CC="/usr/bin/clang"
    else
        echo "Cannot find clang"
    fi

    if [ -f /usr/bin/clang++ ] ; then 
        export CXX="/usr/bin/clang++"
    else
        echo "Cannot find clang++"
    fi
}

roxlu_use_gcc() {

    if [ -f /usr/bin/gcc ] ; then 
        export CC="/usr/bin/gcc"
    else
        echo "Cannot find gcc"
    fi


    if [ -f /usr/bin/g++ ] ; then 
        export CXX="/usr/bin/g++"
    else
        echo "Cannot find g++"
    fi

}

roxlu_build_debug() {
    d=${PWD}
    cd ${d}/build.debug
    cmake -DCMAKE_BUILD_TYPE=Debug ../
    make -j4 # VERBOSE=1
    make install
}

roxlu_build_release() {
    cd build.release
    cmake -DCMAKE_BUILD_TYPE=Release ../
    make -j4
    make install
}

roxlu_create_xcodeproject() {
    d=${PWD}
    if [ -d ${d}/build.xcodeproject ] ; then 
        rm -rf ${d}/build.xcodeproject
    fi

    if [ ! -d ${d}/build.xcodeproject ] ; then 
        mkdir ${d}/build.xcodeproject
    fi


    cd ${d}/build.xcodeproject
    cmake -G Xcode ../
}

roxlu_run_release() {
    d=${PWD}
    bd=${d}/../../bin
    appdir=${bd}/../

    # get app name
    cd ${appdir}
    app=${PWD##*/}

    cd ${d}
    ./build_release.sh

    cd ${d}/../../install/bin

    ./${app}
}

roxlu_run_debug() {
    d=${PWD}
    bd=${d}/../../bin
    appdir=${bd}/../

    # get app name
    cd ${appdir}
    app=${PWD##*/}_debug

    # make sure we have the build + data dirs
    cd ${d}

    ./build_debug.sh

    cd ${d}/../../install/bin

    ./${app}

}
