#!/bin/bash
# this builds for the Syntronic red form-factor board for Prime
orig=`pwd`
build=build/
if [ ! -d "$build/" ]; then
	mkdir -p $build
	cd $build
	source /home/local/NEATO/running.gao/Documents/EE/TOF/environment-setup-aarch64-poky-linux
	cmake -G "Ninja" -DCMAKE_TOOLCHAIN_FILE=$OECORE_NATIVE_SYSROOT/usr/share/cmake/OEToolchainConfig.cmake $orig
else
	cd $build
fi
ninja
cd $orig
