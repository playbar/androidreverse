#!/bin/sh

#$NDK/build/tools/make-standalone-toolchain.sh --use-llvm --platform=android-21 --install-dir=/program/toolchain/android-armv7 --arch=arm --stl=libc++
#$NDK/build/tools/make-standalone-toolchain.sh --use-llvm --platform=android-21 --install-dir=/program/toolchain/android-arrch64 --arch=arm64 --stl=libc++

set -e
echo $@
echo $*

if [[ $1 != 64 ]]
then
	export NDKROOT=/program/toolchain/android-armv7/bin
	export SYSROOT=/program/toolchain/android-armv7/sysroot
	export HWKIND=generic
	export AR=$NDKROOT/arm-linux-androideabi-ar
	export LD=$NDKROOT/arm-linux-androideabi-ld
	export CC=$NDKROOT/arm-linux-androideabi-gcc
	export CXX=$NDKROOT/arm-linux-androideabi-g++
	export RANLIB=$NDKROOT/arm-linux-androideabi-ranlib
else
	export NDKROOT=/program/toolchain/android-arrch64/bin
	export SYSROOT=/program/toolchain/android-arrch64/sysroot
	export HWKIND=generic
	export AR=$NDKROOT/aarch64-linux-android-ar
	export LD=$NDKROOT/aarch64-linux-android-ld
	export CC=$NDKROOT/aarch64-linux-android-gcc
	export CXX=$NDKROOT/aarch64-linux-android-g++
    export RANLIB=$NDKROOT/aarch64-linux-android-ranlib
fi
ls -l $AR $LD $CC $CXX $RANLIB

./autogen.sh

if [[ $1 != 64 ]]; then
CPPFLAGS="--sysroot=$SYSROOT -D__ANDROID_API__=21" \
CFLAGS="--sysroot=$SYSROOT -D__ANDROID_API__=21" \
./configure --prefix=/vendor/valgrind \
--host=armv7-unknown-linux --target=armv7-unknown-linux \
--with-tmpdir=/sdcard
else
CPPFLAGS="--sysroot=$SYSROOT -D__ANDROID_API__=21" \
CFLAGS="--sysroot=$SYSROOT -D__ANDROID_API__=21" \
./configure --prefix=/vendor/valgrind \
--host=aarch64-unknown-linux --target=aarch64-unknown-linux \
--with-tmpdir=/sdcard
fi

make clean
make -j4
#make -j4 install DESTDIR=`pwd`/install_valgrind_arm$1

