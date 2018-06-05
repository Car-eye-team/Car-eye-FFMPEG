/*
 * Car eye 车辆管理平台: www.car-eye.cn
 * Car eye 开源网址: https://github.com/Car-eye-team
 * CarEyeDecoderAPI.h
 *
 * Author: Wgj
 * Date: 2018-05-16 22:54
 * Copyright 2018
 *
 * Car eye基于FFMPEG的音视频解码接口声明
 */
#ifndef __CarEyeDecoderAPI_H_
#define __CarEyeDecoderAPI_H_

#include "CarEyeTypes.h"

// 媒体帧信息定义
typedef struct
{
	// 视频编码格式
	CarEye_CodecType VCodec;
	// 音频解码格式，无音频则置为CAREYE_CODEC_NONE
	CarEye_CodecType ACodec;
	// 视频帧率(FPS)
	unsigned char	FramesPerSecond;
	// 视频宽度像素
	unsigned short	Width;
	// 视频的高度像素
	unsigned short  Height;
	// 视频码率，越高视频越清楚，相应体积也越大 如：4000000
	unsigned int	VideoBitrate;

	// 音频采样率
	unsigned int	SampleRate;
	// 音频声道数
	unsigned int	Channels;
	// 音频采样精度 16位 8位等，库内部固定为16位
	unsigned int	BitsPerSample;
	// 音频比特率 如：64000，越高声音越清楚，相应体积也越大
	unsigned int	AudioBitrate;
}CarEye_FrameInfo;

#ifdef __cplusplus
extern "C"
{
#endif

	/*
	* Comments: 创建一个解码器对象
	* Param aInfo: 要解码的媒体信息
	* @Return CarEye_Decoder_Handle 成功返回解码器对象，否则返回NULL
	*/
	CE_API CarEye_Decoder_Handle CE_APICALL CarEye_DecoderCreate(CarEye_FrameInfo aInfo);

	/*
	* Comments: 释放解码器资源
	* Param aDecoder: 要释放的解码器
	* @Return None
	*/
	CE_API void CE_APICALL CarEye_DecoderRelease(CarEye_Decoder_Handle aDecoder);

	/*
	* Comments: 获取YUV输出缓冲区的字节大小
	* Param aDecoder: 解码器
	* @Return int 输出缓冲区大小 < 0失败
	*/
	CE_API int CE_APICALL CarEye_GetYUVSize(CarEye_Decoder_Handle aDecoder);

	/*
	* Comments: 将输入视频解码为YUV420格式数据输出
	* Param aDecoder: 申请到的有效解码器
	* Param aFilter: 如需添加水印，则传入已创建的水印编码器对象
	* Param aBytes: 要进行解码的视频流
	* Param aSize: 要解码视频流字节数
	* Param aYuv: [输出] 解码成功后输出的YUV420数据
	* @Return int < 0解码失败，> 0为解码后YUV420的字节个数 ==0表示参数无效
	*/
	CE_API int CE_APICALL CarEye_DecoderYUV420(CarEye_Decoder_Handle aDecoder, 
		unsigned char *aBytes, int aSize,
		unsigned char *aYuv);

	/*
	* Comments: 将输入音频解码为PCM格式数据输出
	* Param aDecoder: 申请到的有效解码器
	* Param aBytes: 要进行解码的音频流
	* Param aSize: 要解码音频流字节数
	* Param aYuv: [输出] 解码成功后输出的PCM数据
	* @Return int < 0解码失败，> 0为解码后PCM的字节个数 ==0表示参数无效
	*/
	CE_API int CE_APICALL CarEye_DecoderPCM(CarEye_Decoder_Handle aDecoder,
		unsigned char *aBytes, int aSize,
		unsigned char *aPcm);

#ifdef __cplusplus
}
#endif


#endif // __CarEyeDecoderAPI_H_