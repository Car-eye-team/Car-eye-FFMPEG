#ifndef __CAREYE_PUBLIC_H__
#define __CAREYE_PUBLIC_H__

#ifdef __ANDROID__  

#include <android/log.h>
#undef printf  
#define printf(...) __android_log_print(ANDROID_LOG_DEBUG, "Car-eye-ffmpeg", __VA_ARGS__)
#endif



//error number
#define NO_ERROR					0
#define PARAMTER_ERROR		1
#define NULL_MEMORY				2

#define MAX_FILTER_DESCR 512




#endif