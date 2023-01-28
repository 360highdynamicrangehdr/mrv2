#!/usr/bin/env bash

#
# Turn on exit on error
#
set -o pipefail -e

#
# Get the auxiliary functions
#
. etc/functions.sh

#
# Determine OS Kernel, OS CPU architecture
#
export KERNEL=`uname`
if [[ $KERNEL == *MSYS* || $KERNEL == *MINGW* ]]; then
    export KERNEL=Msys
    export ARCH=`which cl.exe`
fi

if [[ $ARCH == "" ]]; then
    export ARCH=`uname -a`
fi


if [[ $ARCH == *64* ]]; then
    ARCH=64
else
    ARCH=32
fi

export DIST=0
export CLEAN_DIR=0
export CMAKE_OSX_ARCHIECTURES=""
export SHOW_INCLUDES=0
export CMAKE_BUILD_TYPE="Release"
for i in $@; do
    case $i in
	debug)
	    export CMAKE_BUILD_TYPE="Debug"
	    shift
	    ;;
	dist)
	    export DIST=1
	    shift
	    ;;
	arm*)
	    export CMAKE_OSX_ARCHIECTURES="-DCMAKE_OSX_ARCHIECTURES=arm64"
	    shift
	    ;;
	clean)
	    export CLEAN_DIR=1
	    shift
	    ;;
	-h*)
	    echo "$0 [debug] [clean] [dist] [arm64] [includes] [-help]"
	    echo ""
	    echo "* debug builds a debug build."
	    echo "* clean clears the directory before building -- use only with runme.sh"
	    echo "* dist builds a compatible distribution (macOS - compatible with Mojave) -- use only with runme.sh"
	    echo "* arm64 builds for Mac Silicon CMAKE_OSX_ARCHITECTURES=arm64"
	    exit 1
	    ;;
    esac
done



# Build a build directory with that information
export BUILD_DIR=BUILD-$KERNEL-$ARCH/$CMAKE_BUILD_TYPE

if [[ $TLRENDER_QT6 == "ON" ]]; then
    export BUILD_DIR=Qt6/$BUILD_DIR
elif [[ $TLRENDER_QT5 == "ON" ]]; then
    export BUILD_DIR=Qt5/$BUILD_DIR
fi

export CMAKE_FLAGS="${CMAKE_OSX_ARCHIECTURES} "
if [[ $DIST == 1 ]]; then
    if [[ $KERNEL == *Darwin* ]]; then
	export CMAKE_FLAGS="-DCMAKE_OSX_DEPLOYMENT_TARGET=10.14 ${CMAKE_FLAGS}"
	export PATH="/usr/local/opt/gnu-sed/libexec/gnubin:$PATH"
    fi
fi


export FLAGS=$@
if [[ $FLAGS == "" ]]; then
    export FLAGS="-j 4 -v"
fi

echo "Build directory is ${BUILD_DIR}"

if [[ $CLEAN_DIR == 1 ]]; then
    if [[ -d ${BUILD_DIR} ]]; then
	echo "Cleaning ${BUILD_DIR}"
	rm -rf $BUILD_DIR
    fi
fi

echo "Architecture is ${ARCH}"
echo "Compiler flags are ${FLAGS}"

if [[ $KERNEL == *Msys* ]]; then
    . $PWD/etc/windows_envvars.sh
    echo "FFMPEG_DIR=${FFMPEG_DIR}"
    export CMAKE_FLAGS="-DLIBINTL_ROOT=${LIBINTL_ROOT} ${CMAKE_FLAGS}"
fi


if [[ ! -d $BUILD_DIR/install/include ]]; then
    mkdir -p $BUILD_DIR/install/bin $BUILD_DIR/install/lib
    mkdir -p $BUILD_DIR/install/include

    if [[ $KERNEL == *Msys* ]]; then
	. $PWD/etc/copy_dlls.sh
    fi
fi
