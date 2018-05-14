### Compilation instructions
* Run the msys.bat script£¬and enter into ffmpeg project directory£¬after that configure the project£¬now record my configure£º  
	 ./configure --target-os=win32 --disable-debug --disable-static --enable-shared --prefix=../Win32/Release --toolchain=msvc --disable-doc --disable-programs --disable-ffmpeg --disable-ffprobe --disable-ffserver --disable-ffplay --disable-avdevice
* Edit the config.h encode to utf8
* make & make install
