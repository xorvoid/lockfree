#!/bin/bash
set -e
THISDIR=$(dirname $(realpath $0))
cd $THISDIR

if [ ! -d build ]; then
    meson build
fi

(cd build && ninja $@)
