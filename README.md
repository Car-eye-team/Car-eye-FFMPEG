## car-eye-FFMPEG
* FFmpeg  compiled with x264, libass, fontconfig, freetype and fribidi
* simple interface for application
* support android and windows

## Supported Architecture
* ARMV7
* X86

## Compile in android (Android studio 3.0+)
* Set environment variable
export ANDROID_NDK={Android NDK Base Path}
Run following commands to compile ffmpeg
sudo apt-get --quiet --yes install build-essential git autoconf libtool pkg-config gperf gettext yasm python-lxml
./android_build.sh
