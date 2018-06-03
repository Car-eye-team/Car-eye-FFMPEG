/*
 * Car eye 车辆管理平台: www.car-eye.cn
 * Car eye 开源网址: https://github.com/Car-eye-team
 * CarEyeEncoderAPI.h
 *
 * Author: Wgj
 * Date: 2018-05-16 22:40
 * Copyright 2018
 *
 * Car eye基于FFMPEG的音视频编码接口声明
 */
#ifndef __CarEyeEncoderAPI_H_
#define __CarEyeEncoderAPI_H_

#include "CarEyeTypes.h"

// 原始流结构定义
typedef struct
{
	// 视频输入流格式
	CarEye_AVType InVideoType;
	// 期望输出的视频流格式，不期望输出可设置为CAREYE_CODEC_NONE
	CarEye_CodecType OutVideoType;
	// 期望输出的音频流格式，不期望输出可设置为CAREYE_CODEC_NONE
	CarEye_CodecType OutAudioType;
	// 视频帧率(FPS)，推荐值：25
	unsigned char	FramesPerSecond;
	// 视频宽度像素
	unsigned short	Width;
	// 视频的高度像素
	unsigned short  Height;
	// 一组图片中的图片数量，推荐值：10
	int				GopSize;
	// 非B帧之间的B帧的最大数量，推荐值：1
	int				MaxBFrames;
	// 视频码率，越高视频越清楚，相应体积也越大 如：4000000
	unsigned int	VideoBitrate;

	// 音频采样率 如：44100
	unsigned int	SampleRate;
	// 音频比特率 如：64000，越高声音越清楚，相应体积也越大
	unsigned int	AudioBitrate;
}CarEye_OriginalStream;

#ifdef __cplusplus
extern "C"
{
#endif

	/*
	* Comments: 创建一个编码器对象
	* Param aInfo: 要编码的媒体信息
	* @Return CarEye_Encoder_Handle 成功返回编码器对象，否则返回NULL
	*/
	CE_API CarEye_Encoder_Handle CE_APICALL CarEye_EncoderCreate(CarEye_OriginalStream aInfo);

	/*
	* Comments: 释放编码器资源
	* Param aEncoder: 要释放的编码器
	* @Return None
	*/
	CE_API void CE_APICALL CarEye_EncoderRelease(CarEye_Encoder_Handle aEncoder);

	/*
	* Comments: 将输入YUV视频编码为设置好的格式数据输出
	* Param aEncoder: 申请到的有效编码器
	* Param aFilter: 如需添加水印，则传入已创建的水印编码器对象
	* Param aYuv: 要编码的YUV数据
	* Param aBytes: [输出]编码后的视频流
	* @Return int < 0编码失败，> 0为编码后数据字节个数 ==0表示参数无效
	*/
	CE_API int CE_APICALL CarEye_EncoderYUV(CarEye_Encoder_Handle aEncoder,
										CarEye_YUVFrame *aYuv, int pts, unsigned char *aBytes);

	/*
	* Comments: 获取PCM编码时接受的最大字节数
	* Param aEncoder: 申请到的有效编码器
	* @Return PCM编码缓冲区最大字节数
	*/
	CE_API int CE_APICALL CarEye_GetPcmMaxSize(CarEye_Encoder_Handle aEncoder);

	/*
	* Comments: 将输入的PCM音频编码为指定数据格式输出
	* Param aEncoder: 申请到的有效编码器
	* Param aPcm: 要编码的PCM数据
	* Param aSize: 要编码音频流字节数
	* Param aBytes: [输出] 编码后的音频流
	* Param aPts: 当前编码帧的序号
	* @Return int < 0编码失败，> 0为编码后PCM的字节个数 ==0表示参数无效
	*/
	CE_API int CE_APICALL CarEye_EncoderPCM(CarEye_Encoder_Handle aEncoder,
		unsigned char *aPcm, int aSize, int aPts,
		unsigned char *aBytes);

#ifdef __cplusplus
}
#endif


#endif // __CarEyeEncoderAPI_H_