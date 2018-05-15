#!/bin/bash

. abi_settings.sh $1 $2 $3

pushd ffmpeg

case $1 in
  armeabi-v7a | armeabi-v7a-neon)
    CPU='cortex-a8'
  ;;
  x86)
    CPU='i686'
  ;;
esac

make clean

./configure \
--cross-prefix="$CROSS_PREFIX" \
--sysroot="$NDK_SYSROOT" \
--enable-libfreetype \
--disable-avdevice \
--enable-pic \
--disable-ffserver \
--disable-ffmpeg \
--disable-ffplay \
--disable-ffprobe \
--disable-symver \
--disable-doc \
--disable-static \
--enable-shared \
--target-os=linux \
--arch="$NDK_ABI" \
--enable-cross-compile \
--pkg-config="${2}/ffmpeg-pkg-config" \
--prefix="${2}/build/${1}" \
--extra-cflags="-I${TOOLCHAIN_PREFIX}/include -I${TOOLCHAIN_PREFIX}/include/freetype2 " \
--extra-ldflags="-L${TOOLCHAIN_PREFIX}/lib  " \
--extra-libs="-lpng " \
--extra-cxxflags="$CXX_FLAGS" || exit 1

make  && make install && make clean || exit 1

popd
