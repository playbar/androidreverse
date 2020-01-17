#!/bin/sh

set -e
echo $@
echo $*

if [[ $1 != 64 ]]
then
	export NDKROOT=/program/ndk/toolchains/arm-linux-androideabi-4.9/prebuilt/darwin-x86_64/bin
	export SYSROOT=/program/ndk/sysroot
	export HWKIND=generic
	export AR=$NDKROOT/arm-linux-androideabi-ar
	export LD=$NDKROOT/arm-linux-androideabi-ld
	export CC=$NDKROOT/arm-linux-androideabi-gcc
	#export RANLIB=$NDKROOT/arm-linux-androideabi-ranlib
else 
	export NDKROOT=/program/ndk/toolchains/aarch64-linux-android-4.9/prebuilt/darwin-x86_64/bin
	export SYSROOT=/program/ndk//sysroot
	export HWKIND=generic
	export AR=$NDKROOT/aarch64-linux-android-ar
	export LD=$NDKROOT/aarch64-linux-android-ld
	export CC=$NDKROOT/aarch64-linux-android-gcc
fi
ls -l $AR $LD $CC $RANLIB

./autogen.sh

if [[ $1 != 64 ]]; then
CPPFLAGS="--sysroot=$SYSROOT" \
CFLAGS="--sysroot=$SYSROOT" \
./configure --prefix=/vendor/valgrind \
--host=armv7-unknown-linux --target=armv7-unknown-linux \
--with-tmpdir=/sdcard
else
CPPFLAGS="--sysroot=$SYSROOT" \
CFLAGS="--sysroot=$SYSROOT" \
./configure --prefix=/vendor/valgrind \
--host=aarch64-unknown-linux --target=aarch64-unknown-linux \
--with-tmpdir=/sdcard
fi

make clean
make -j4
make -j4 install DESTDIR=`pwd`/install_valgrind_arm$1
