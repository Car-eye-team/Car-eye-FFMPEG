/*
 * Car eye 车辆管理平台: www.car-eye.cn
 * Car eye 开源网址: https://github.com/Car-eye-team
 * CarEyeTypes.h
 *
 * Author: Wgj
 * Date: 2018-05-16 21:36
 * Copyright 2018
 *
 * 本库使用到的数据类型定义声明
 */
#ifndef __CarEyeTypes_H_
#define __CarEyeTypes_H_


#ifdef _WIN32
#define CE_API  __declspec(dllexport)
#define CE_APICALL  __stdcall
#else
#define CE_API
#define CE_APICALL 
#endif




#ifdef __cplusplus
  #define __STDC_CONSTANT_MACROS
  #ifdef _STDINT_H
   #undef _STDINT_H
  #endif
  # include <stdint.h>
 #endif


#ifdef __ANDROID__       //android的编译器会自动识别到这个为真。

#include <android/log.h>

#define __STDC_CONSTANT_MACROS

#define CarEyeLog(...) __android_log_print(ANDROID_LOG_DEBUG, "Car-eye-ffmpeg", __VA_ARGS__)


 


#endif


 // 编码器对象句柄定义
#define CarEye_Encoder_Handle void*
 // 解码器对象句柄定义
#define CarEye_Decoder_Handle void*
 // 水印编码器对象句柄定义
#define CarEye_OSD_Handle void*

 // 最大音频帧大小 1 second of 48khz 32bit audio
#define MAX_AUDIO_FRAME_SIZE 192000

 // 媒体编码类型定义 与FFMPEG中一一对应，H265定义与其他库定义需要转换
typedef enum
{
	// 不进行编码
	CAREYE_CODEC_NONE = 0,
	// H264编码
	CAREYE_CODEC_H264 = 0x1C,
	// H265编码
	CAREYE_CODEC_H265 = 0xAE,
	// MJPEG编码
	CAREYE_CODEC_MJPEG = 0x08,
	// MPEG4编码
	CAREYE_CODEC_MPEG4 = 0x0D,
	// AAC编码
	CAREYE_CODEC_AAC = 0x15002,
	// G711 Ulaw编码 对应FFMPEG中的AV_CODEC_ID_PCM_MULAW定义
	CAREYE_CODEC_G711U = 0x10006,
	// G711 Alaw编码 对应FFMPEG中的AV_CODEC_ID_PCM_ALAW定义
	CAREYE_CODEC_G711A = 0x10007,
	// G726编码 对应FFMPEG中的AV_CODEC_ID_ADPCM_G726定义
	CAREYE_CODEC_G726 = 0x1100B,
}CarEye_CodecType;

// YUV视频流格式定义，与FFMPEG中一一对应
typedef enum
{
	CAREYE_FMT_YUV420P = 0,
	CAREYE_FMT_YUV422P = 4,
	CAREYE_FMT_YUV444P = 5,
	CAREYE_FMT_YUV410P = 6,
	CAREYE_FMT_YUV411P = 7,
}CarEye_AVType;

// YUV媒体流结构定义
typedef struct
{
	// Y分量数据存储区
	unsigned char *Y;
	// Y分量数据字节数
	int YSize;
	// U分量数据存储区
	unsigned char *U;
	// U分量数据字节数
	int USize;
	// V分量数据存储区
	unsigned char *V;
	// V分量数据字节数
	int VSize;
}CarEye_YUVFrame;

#define MAX_STRING_LENGTH 1024
#define MAX_FILE_NAME 64
// 水印结果定义
typedef struct
{
	// 视频宽度
	int Width;
	// 视频高度
	int Height;
	// 视频帧率（FPS）
	int FramesPerSecond;
	// 添加水印的视频格式
	CarEye_AVType YUVType;
	// 水印起始X轴坐标
	int X;
	// 水印起始Y轴坐标
	int Y;
	// 水印字体大小
	int FontSize;
	// 16进制的RGB颜色值，如绿色：0x00FF00
	unsigned int FontColor;
	// 水印透明度 0~1
	float Transparency;
	// 水印内容
	char SubTitle[MAX_STRING_LENGTH];
	// 字体名称，字体文件放到库的同目录下，如“arial.ttf”
	// Windows下系统目录使用格式："C\\\\:/Windows/Fonts/msyh.ttc"
	char FontName[MAX_FILE_NAME];
}CarEye_OSDParam;

/*
* Comments: 使用本库之前必须调用一次本方法
* Param : None
* @Return void
*/
#ifdef __cplusplus
extern "C"
{
#endif
CE_API void CE_APICALL CarEye_MPEG_Init(void);

#ifdef __cplusplus
}
#endif

#endif // __CarEyeTypes_H_