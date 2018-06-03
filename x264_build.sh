#!/bin/bash

. abi_settings.sh $1 $2 $3

pushd x264

make clean

case $1 in
  armeabi-v7a | armeabi-v7a-neon)
    HOST=arm-linux
  ./configure \
  --cross-prefix="$CROSS_PREFIX" \
  --sysroot="$NDK_SYSROOT" \
  --host="$HOST" \
  --enable-pic \
  --enable-strip \
  --enable-static \
  --disable-shared \
  --prefix="${TOOLCHAIN_PREFIX}" \
  --disable-cli || exit 1
  ;;
arm64-v8a)
   HOST=aarch64-linux
  ./configure \
  --cross-prefix="$CROSS_PREFIX" \
  --sysroot="$NDK_SYSROOT" \
  --host="$HOST" \
  --enable-pic \
  --enable-strip \
  --enable-static \
  --disable-shared \
  --prefix="${TOOLCHAIN_PREFIX}" \
  --disable-cli || exit 1
  ;;
  x86)
    HOST=i686-linux 
  ./configure \
  --cross-prefix="$CROSS_PREFIX" \
  --sysroot="$NDK_SYSROOT" \
  --host="$HOST" \
  --enable-pic \
  --disable-asm \
  --enable-strip \
  --enable-static \
  --disable-shared \
  --prefix="${TOOLCHAIN_PREFIX}" \
  --disable-cli || exit 1
  ;;
esac

echo $CFLAGS


make -j${NUMBER_OF_CORES} install && make clean || exit 1


popd
