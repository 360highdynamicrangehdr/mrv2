#!/bin/bash

if [[ $BASH_SOURCE != $0 ]]; then
    dir=$BASH_SOURCE
else
    dir=$0
fi


symlink=`readlink ${dir}`

if [ "$symlink" != "" ]; then
    dir="$symlink"
    dir=${dir%/*}
else
# Find directory where mrViewer bash script resides
    dir=${dir%/*}
fi

# If running from current directory, get full path
if [[ "$dir" == "." ]]; then
    dir=$PWD
fi


#
# Platform specific directory for mrViewer
#
dir=${dir%/*}

#
# Add mrv2's lib directory first to LD_LIBRARY_PATH (Linux)
# or DYLD_FALLBACK_LIBRARY_PATH (macOS)
#
export LD_LIBRARY_PATH="${dir}/lib:${LD_LIBRARY_PATH}"
export DYLD_FALLBACK_LIBRARY_PATH="${LD_LIBRARY_PATH}"

#
# For Linux, when running on Wayland we switch it to run on X11 emulation,
# as Wayland is still too buggy.
#
if [[ "$XDG_SESSION_TYPE" == "wayland" && "$FLTK_BACKEND" == "" ]]; then
    gfx_card=`lspci | grep 'VGA.*NVIDIA'`
    if [[ $gfx_card != "" ]]; then
	echo "Wayland support with NVidia cards is currently buggy."
	echo "If you still want to run FLTK applications with Wayland,"
	echo "set the environment variable FLTK_BACKEND to wayland, like:"
	echo ""
	echo "   export FLTK_BACKEND=wayland"
	echo ""
	if [[ "$FLTK_BACKEND" == "" ]]; then
	    echo "Setting the environment variable FLTK_BACKEND=x11."
	    echo ""
	    export FLTK_BACKEND=x11
	fi
    fi
fi
