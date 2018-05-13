#!/bin/bash

SUPPORTED_ARCHITECTURES=(armeabi-v7a x86) 
ANDROID_NDK_ROOT_PATH=${ANDROID_NDK}
if [[ -z "$ANDROID_NDK_ROOT_PATH" ]]; then
  echo "You need to set ANDROID_NDK environment variable, please check instructions"
  exit
fi
ANDROID_API_VERSION=21
NDK_TOOLCHAIN_ABI_VERSION=4.9

NUMBER_OF_CORES=$(nproc)
HOST_UNAME=$(uname -m)
TARGET_OS=linux

CFLAGS='-Os -fpic -marm'
LDFLAGS=''

FFMPEG_PKG_CONFIG="$(pwd)/ffmpeg-pkg-config"
