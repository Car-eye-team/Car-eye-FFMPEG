/*
 * Car eye 车辆管理平台: www.car-eye.cn
 * Car eye 开源网址: https://github.com/Car-eye-team
 * CarEyeDecoderAPI.cpp
 *
 * Author: Wgj
 * Date: 2018-05-16 22:52
 * Copyright 2018
 *
 * Car eye基于FFMPEG的音视频解码接口实现
 */

#include <stdio.h>
#include "CarEyeDecoderAPI.h"
#include "CarEyeOSDAPI.h"

extern "C"
{
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libswresample/swresample.h"
};
#include "public.h"


/*
* Comments: 对视频帧添加水印，非对外接口函数
* Param : aFrame: [输入/输出] 要添加水印的视频帧
* @Return void
*/
int CarEye_OSD_Add(CarEye_OSD_Handle aFilter, AVFrame *aFrame);


// 解码器结构体定义
typedef struct
{
	// 视频解码器
	AVCodecContext *VDecoder;
	// 音频解码器
	AVCodecContext *ADecoder;
	// 解码后的视频帧 音视频帧对象分别定义，防止多线程分别解码音视频造成读写冲突
	AVFrame *VFrame;
	// 解码后的音频帧
	AVFrame *AFrame;
	// 视频的像素大小
	int PixelSize;
}CarEyeDecoder;

/*
* Comments: 利用解码器对媒体包进行解码并输出解码后的数据
* Param aDecoder: 有效的解码器
* Param aPacket: 要解码的媒体数据包
* Param aFrame: [输出] 解码后的数据
* @Return int 小于0失败，等于0成功
*/
static int Decode(AVCodecContext *aDecoder, AVPacket *aPacket, AVFrame *aFrame)
{
	int ret;

	ret = avcodec_send_packet(aDecoder, aPacket);
	if (ret < 0)
	{
		printf("Error sending a packet for decoding\n");
		return ret;
	}

	return avcodec_receive_frame(aDecoder, aFrame);
}

/*
* Comments: 创建一个解码器对象
* Param aInfo: 要解码的媒体信息
* @Return CarEye_Decoder_Handle 成功返回解码器对象，否则返回NULL
*/
CE_API CarEye_Decoder_Handle CE_APICALL CarEye_DecoderCreate(CarEye_FrameInfo aInfo)
{
	if (aInfo.ACodec == CAREYE_CODEC_NONE
		&& aInfo.VCodec == CAREYE_CODEC_NONE)
	{
		// 至少包含一项解码需求
		return NULL;
	}

	CarEyeDecoder *decoder = new CarEyeDecoder;
	if (decoder == NULL)
	{
		return NULL;
	}

	memset(decoder, 0x00, sizeof(CarEyeDecoder));

	// 媒体解码器
	AVCodec *pCodec;
	if (aInfo.VCodec != CAREYE_CODEC_NONE)
	{
		// 请求视频解码器
		pCodec = avcodec_find_decoder((AVCodecID)aInfo.VCodec);
		if (pCodec == NULL)
		{
			printf("Could not find video decoder.\n");
			CarEye_DecoderRelease(decoder);
			return NULL;
		}
		// 申请解码器上下文
		decoder->VDecoder = avcodec_alloc_context3(pCodec);
		if (decoder->VDecoder == NULL)
		{
			printf("Could not alloc video decoder.\n");
			CarEye_DecoderRelease(decoder);
			return NULL;
		}
		decoder->VDecoder->time_base.num = 1;
		// 帧率
		decoder->VDecoder->time_base.den = aInfo.FramesPerSecond;
		// 每包一个视频帧
		decoder->VDecoder->frame_number = 1;
		// 媒体类型为视频
		decoder->VDecoder->codec_type = AVMEDIA_TYPE_VIDEO;
		decoder->VDecoder->bit_rate = aInfo.VideoBitrate;
		// 视频分辨率
		decoder->VDecoder->width = aInfo.Width;
		decoder->VDecoder->height = aInfo.Height;
		decoder->VDecoder->pix_fmt = AV_PIX_FMT_YUV420P;
		decoder->PixelSize = decoder->VDecoder->width * decoder->VDecoder->height;
		if (avcodec_open2(decoder->VDecoder, pCodec, NULL) < 0)
		{
			printf("Could not open video decoder.\n");
			CarEye_DecoderRelease(decoder);
			return NULL;
		}

		decoder->VFrame = av_frame_alloc();
		if (decoder->VFrame == NULL)
		{
			printf("Alloc video frame faile!\n");
			CarEye_DecoderRelease(decoder);
			return NULL;
		}
	}
	if (aInfo.ACodec != CAREYE_CODEC_NONE)
	{
		// 请求音频解码器
		pCodec = avcodec_find_decoder((AVCodecID)aInfo.ACodec);
		if (pCodec == NULL)
		{
			printf("Could not find audio decoder.\n");
			CarEye_DecoderRelease(decoder);
			return NULL;
		}
		// 申请解码器上下文
		decoder->ADecoder = avcodec_alloc_context3(pCodec);
		if (decoder->ADecoder == NULL)
		{
			printf("Could not alloc audio decoder.\n");
			CarEye_DecoderRelease(decoder);
			return NULL;
		}

		// 参数赋值
		decoder->ADecoder->codec_type = AVMEDIA_TYPE_AUDIO;
		decoder->ADecoder->sample_rate = aInfo.SampleRate;
		decoder->ADecoder->channels = aInfo.Channels;
		decoder->ADecoder->bit_rate = aInfo.AudioBitrate;
		decoder->ADecoder->channel_layout = AV_CH_LAYOUT_STEREO;
		if (avcodec_open2(decoder->ADecoder, pCodec, NULL) < 0)
		{
			printf("Could not open audio decoder.\n");
			CarEye_DecoderRelease(decoder);
			return NULL;
		}

		decoder->AFrame = av_frame_alloc();
		if (decoder->AFrame == NULL)
		{
			printf("Alloc audio frame fail!\n");
			CarEye_DecoderRelease(decoder);
			return NULL;
		}
	}

	return decoder;
}

/*
* Comments: 释放解码器资源
* Param aDecoder: 要释放的解码器
* @Return None
*/
CE_API void CE_APICALL CarEye_DecoderRelease(CarEye_Decoder_Handle aDecoder)
{
	CarEyeDecoder *decoder = (CarEyeDecoder *)aDecoder;

	if (decoder == NULL)
	{
		return;
	}
	if (decoder->VDecoder != NULL)
	{
		avcodec_close(decoder->VDecoder);
		decoder->VDecoder = NULL;
	}
	if (decoder->ADecoder != NULL)
	{
		avcodec_close(decoder->ADecoder);
		decoder->ADecoder = NULL;
	}
	if (decoder->VFrame != NULL)
	{
		av_frame_free(&decoder->VFrame);
		decoder->VFrame = NULL;
	}
	if (decoder->AFrame != NULL)
	{
		av_frame_free(&decoder->AFrame);
		decoder->AFrame = NULL;
	}

	delete decoder;
	decoder = NULL;
}

/*
* Comments: 获取YUV输出缓冲区的字节大小
* Param aDecoder: 解码器
* @Return int 输出缓冲区大小 < 0失败
*/
CE_API int CE_APICALL CarEye_GetYUVSize(CarEye_Decoder_Handle aDecoder)
{
	CarEyeDecoder *decoder = (CarEyeDecoder *)aDecoder;
	if (decoder == NULL || decoder->VDecoder == NULL)
	{
		return -1;
	}

	int y_size = decoder->PixelSize;

	return y_size + y_size / 2;
}

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
	unsigned char *aYuv)
{
	CarEyeDecoder *decoder = (CarEyeDecoder *)aDecoder;
	if (decoder == NULL || decoder->VDecoder == NULL)
	{
		return 0;
	}
	if (aBytes == NULL || aSize < 1 || aYuv == NULL)
	{
		return 0;
	}

	int ret;
	int y_size;
	int out_size = 0;
	AVPacket packet = { 0 };

	packet.data = aBytes;
	packet.size = aSize;
	ret = Decode(decoder->VDecoder, &packet, decoder->VFrame);
	if (ret < 0)
	{
		printf("Decode video error.\n");
		av_packet_unref(&packet);
		return ret;
	}


	y_size = decoder->VDecoder->width * decoder->VDecoder->height;

	// 赋值Y值
	memcpy(aYuv, decoder->VFrame->data[0], y_size);
	out_size += y_size;
	memcpy(aYuv + out_size, decoder->VFrame->data[1], y_size / 4);
	out_size += (y_size / 4);
	memcpy(aYuv + out_size, decoder->VFrame->data[2], y_size / 4);
	out_size += (y_size / 4);

	av_packet_unref(&packet);

	return out_size;
}

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
	unsigned char *aPcm)
{
	CarEyeDecoder *decoder = (CarEyeDecoder *)aDecoder;
	if (decoder == NULL || decoder->ADecoder == NULL)
	{
		return 0;
	}
	if (aBytes == NULL || aSize < 1 || aPcm == NULL)
	{
		return 0;
	}

	int ret;
	int out_size = 0;
	AVPacket packet = { 0 };

	packet.data = aBytes;
	packet.size = aSize;

	ret = Decode(decoder->ADecoder, &packet, decoder->AFrame);
	if (ret < 0)
	{
		printf("Decode audio error.\n");
		av_packet_unref(&packet);
		return ret;
	}

	int data_size = av_get_bytes_per_sample(decoder->ADecoder->sample_fmt);

	for (int i = 0; i < decoder->AFrame->nb_samples; i++)
	{
		for (int ch = 0; ch < decoder->ADecoder->channels; ch++)
		{
			memcpy(aPcm, decoder->AFrame->data[ch] + data_size * i, data_size);
			aPcm += data_size;
			out_size += data_size;
		}
	}

	av_packet_unref(&packet);

	return out_size;
}

