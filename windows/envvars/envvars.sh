#!/usr/bin/env bash
# SPDX-License-Identifier: BSD-3-Clause
# mrv2 
# Copyright Contributors to the mrv2 Project. All rights reserved.



#
# Environment variables used to locate the libs and auxiliary executables
# on Windows.
#


#
# Set the location of libintl on windows 32 and 64 bits.
# On Linux and macOS this is picked from /usr/local
#
export LIBINTL_ROOT=$PWD/windows/win${bits}/

#
# Set the location of FFmpeg on Windows.
# We copy the contents of this directory to the build directory.
export FFMPEG_ROOT=${BUILD_DIR}/FFmpeg/install

#
# Alternatively, if you used media-autobuild_suite to compile FFmpeg
# you should set MABS_ROOT to the root of the media-autobuild_suite.
# Here, I set it to my own local installed locations on two window machines.
#
if [[ -d "/D/media-autobuild_suite/" ]]; then
    export MABS_ROOT="/D/media-autobuild_suite/"
else
    export MABS_ROOT="/E/media-autobuild_suite/"
fi

#
# The installation of the ffmpeg files should match those of
# windows/media-autobuild_suite.
#
# Alternatively, set FFMPEG_ROOT to use a local installation of FFMPEG
# that does not match the install locations of media-autobuild_suite.
#

if [[ ! -d "${FFMPEG_ROOT}" ]]; then
    if [[ ! -d "${MABS_ROOT}" ]]; then
	# media-autobuild_suite not installed, pick local pre-built library
	echo "media-autobuild_suite not installed in:"
	echo "   MABS_ROOT=${MABS_ROOT}"
	export MABS_ROOT=${PWD}/windows/
	echo "Will pick local pre-built libs from:"
	echo "   MABS_ROOT=${MABS_ROOT}"
	sleep 3
    fi
else
    echo "FFMPEG_ROOT=$FFMPEG_ROOT"
fi

