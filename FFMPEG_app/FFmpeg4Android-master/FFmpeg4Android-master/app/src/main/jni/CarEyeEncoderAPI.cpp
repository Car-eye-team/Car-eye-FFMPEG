/*
 * Car eye 车辆管理平台: www.car-eye.cn
 * Car eye 开源网址: https://github.com/Car-eye-team
 * CarEyeEncoderAPI.cpp
 *
 * Author: Wgj
 * Date: 2018-05-16 21:35
 * Copyright 2018
 *
 * Car eye基于FFMPEG的音视频编码接口实现
 */
#include "CarEyeTypes.h"
#include "CarEyeEncoderAPI.h"
#include "CarEyeOSDAPI.h"

#ifdef _WIN32
 //Windows
extern "C"
{
#include "libavutil/opt.h"
#include "libavcodec/avcodec.h"
#include "libavutil/imgutils.h"
#include "libavformat/avformat.h"
#include "libswresample/swresample.h"
};
#else
 //Linux...
#ifdef __cplusplus
extern "C"
{
#endif
#include <libavutil/opt.h>
#include <libavcodec/avcodec.h>
#include <libavutil/imgutils.h>
#include <libavformat/avformat.h>
#include <libswresample/swresample.h>
#ifdef __cplusplus
};
#endif
#endif

#include "public.h"

/*
* Comments: 对视频帧添加水印，非对外接口函数
* Param : aFrame: [输入/输出] 要添加水印的视频帧
* @Return void
*/
int CarEye_OSD_Add(CarEye_OSD_Handle aFilter, AVFrame *aFrame);

// ADTS头信息结构 由于不同设备的大小端问题，无法使用该结构进行方便的转换，仅作结构参考
typedef struct
{
	// 固定部分
	// 同步标识，固定为 0xFFF
	unsigned syncword : 12;
	// MPEG Version: 0 for MPEG-4，1 for MPEG-2
	unsigned ID : 1;
	// 总是0
	unsigned layer : 2;
	// 设置1表明没有CRC校验，0则为CRC校验
	unsigned protection_absent : 1;
	// AAC main 1, AAC lc 2, AAC ssr 3
	unsigned profile : 2;
	// 音频采样率的索引
	unsigned sampling_frequency_index : 4;
	unsigned private_bit : 1;
	// 声道数
	unsigned channel_configuration : 3;
	unsigned original_copy : 1;
	unsigned home : 1;

	// 可变部分
	unsigned copyright_identification_bit : 1;
	unsigned copyright_identification_start : 1;
	// 一个ADTS帧的长度包括ADTS头和AAC原始流 有CRC校验则加9，没有则加7 本文中没有CRC校验
	unsigned aac_frame_length : 13;
	// 0x7FF说明是码率可变的码流
	unsigned adts_buffer_fullness : 11;
	// 表示ADTS帧中有number_of_raw_data_blocks_in_frame + 1个AAC原始帧
	unsigned number_of_raw_data_blocks_in_frame : 2;

}ADTS_HEADER;

// 编码器结构体定义
typedef struct
{
	// 视频编码器
	AVCodecContext *VEncoder;
	// 音频编码器
	AVCodecContext *AEncoder;
	// 编码后的视频帧 音视频帧对象分别定义，防止多线程分别编码音视频造成读写冲突
	AVFrame *VFrame;
	// 编码前的音频帧
	AVFrame *AFrame;
	// AAC中需要用到的ADTS头信息
	unsigned char AdtsHeader[7];
	// 接收PCM字节个数上限
	int PcmSize;
	// 每组PCM数据的字节数
	int PerPcmSize;
}CarEyeEncoder;

/*
* Comments: 利用编码器对媒体包进行编码并输出编码后的数据
* Param aEncoder: 有效的编码器
* Param aFrame: 要编码的媒体数据包
* Param aPacket: [输出] 编码后的数据
* @Return int 小于0失败，等于0成功
*/
static int Encode(AVCodecContext *aEncoder, AVFrame *aFrame, AVPacket *aPacket)
{
	int ret;

	ret = avcodec_send_frame(aEncoder, aFrame);
	if (ret < 0)
	{
		printf("Error sending a packet for encoding\n");
		return ret;
	}

	ret = avcodec_receive_packet(aEncoder, aPacket);
	if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
	{
		return 0;
	}
	else if (ret < 0)
	{
		//error during encoding
		return -1;
	}
	return ret;
}

// 音频采样率对应的索引
static const unsigned  samplingFrequencyTable[16] = {
	96000, 88200, 64000, 48000,
	44100, 32000, 24000, 22050,
	16000, 12000, 11025, 8000,
	7350, 0, 0, 0
};

/*
* Comments: 获取音频采样率的索引
* Param : aSample: 音频采样率
* @Return 对应的索引用于ADTS部分
*/
static int GetAACSampleFrequencyIndex(int aSample)
{
	int index = 11;
	int i = 0;
	for (i = 0; i < 16; i++)
	{
		if (samplingFrequencyTable[i] == aSample)
		{
			index = i;
			break;
		}
	}
	return index;
}

/*
* Comments: 创建一个编码器对象
* Param aInfo: 要编码的媒体信息
* @Return CarEye_Encoder_Handle 成功返回编码器对象，否则返回NULL
*/
CE_API CarEye_Encoder_Handle CE_APICALL CarEye_EncoderCreate(CarEye_OriginalStream aInfo)
{
	if (aInfo.OutVideoType == CAREYE_CODEC_NONE
		&& aInfo.OutAudioType == CAREYE_CODEC_NONE)
	{
		// 至少包含一项编码需求
		return NULL;
	}

	CarEyeEncoder *encoder = new CarEyeEncoder;
	if (encoder == NULL)
	{
		return NULL;
	}

	memset(encoder, 0x00, sizeof(CarEyeEncoder));

	// 媒体编码器
	AVCodec *pCodec;
	if (aInfo.OutVideoType != CAREYE_CODEC_NONE)
	{
		// 请求视频编码器
		pCodec = avcodec_find_encoder((AVCodecID)aInfo.OutVideoType);
		if (pCodec == NULL)
		{
			printf("Could not find video encoder.\n");
			CarEye_EncoderRelease(encoder);
			return NULL;
		}
		// 申请编码器上下文
		encoder->VEncoder = avcodec_alloc_context3(pCodec);
		if (encoder->VEncoder == NULL)
		{
			printf("Could not alloc video encoder.\n");
			CarEye_EncoderRelease(encoder);
			return NULL;
		}
		encoder->VEncoder->codec_id = (AVCodecID)aInfo.OutVideoType;
		encoder->VEncoder->time_base.num = 1;
		// 帧率
		encoder->VEncoder->time_base.den = aInfo.FramesPerSecond;
		// 每包一个视频帧
		encoder->VEncoder->frame_number = 1;
		// 媒体类型为视频
		encoder->VEncoder->codec_type = AVMEDIA_TYPE_VIDEO;
		encoder->VEncoder->bit_rate = aInfo.VideoBitrate;
		// 视频分辨率
		encoder->VEncoder->width = aInfo.Width;
		encoder->VEncoder->height = aInfo.Height;
		encoder->VEncoder->gop_size = aInfo.GopSize;
		encoder->VEncoder->max_b_frames = aInfo.MaxBFrames;
		encoder->VEncoder->pix_fmt = (AVPixelFormat)aInfo.InVideoType;

		AVDictionary *param = NULL;
		//H.264
		if (aInfo.OutVideoType == CAREYE_CODEC_H264)
		{
			av_dict_set(&param, "preset", "slow", 0);
			av_dict_set(&param, "tune", "zerolatency", 0);
		}
		//H.265
		if (aInfo.OutVideoType == CAREYE_CODEC_H265)
		{
			av_dict_set(&param, "preset", "ultrafast", 0);
			av_dict_set(&param, "tune", "zero-latency", 0);
		}

		if (avcodec_open2(encoder->VEncoder, pCodec, &param) < 0)
		{
			printf("Could not open video encoder.\n");
			CarEye_EncoderRelease(encoder);
			return NULL;
		}

		encoder->VFrame = av_frame_alloc();
		if (encoder->VFrame == NULL)
		{
			printf("Alloc video frame faile!\n");
			CarEye_EncoderRelease(encoder);
			return NULL;
		}
		encoder->VFrame->format = encoder->VEncoder->pix_fmt;
		encoder->VFrame->width = encoder->VEncoder->width;
		encoder->VFrame->height = encoder->VEncoder->height;

		if (av_image_alloc(encoder->VFrame->data, encoder->VFrame->linesize,
			encoder->VEncoder->width, encoder->VEncoder->height,
			encoder->VEncoder->pix_fmt, 16) < 0)
		{
			printf("Could not allocate raw picture buffer!\n");
			CarEye_EncoderRelease(encoder);
			return NULL;
		}
	}
	if (aInfo.OutAudioType != CAREYE_CODEC_NONE)
	{
		// 请求音频编码器
		pCodec = avcodec_find_encoder((AVCodecID)aInfo.OutAudioType);
		if (pCodec == NULL)
		{
			printf("Could not find audio encoder.\n");
			CarEye_EncoderRelease(encoder);
			return NULL;
		}
		// 申请编码器上下文
		encoder->AEncoder = avcodec_alloc_context3(pCodec);
		if (encoder->AEncoder == NULL)
		{
			printf("Could not alloc audio encoder.\n");
			CarEye_EncoderRelease(encoder);
			return NULL;
		}

		// 参数赋值
		encoder->AEncoder->codec_id = (AVCodecID)aInfo.OutAudioType;
		encoder->AEncoder->codec_type = AVMEDIA_TYPE_AUDIO;
		encoder->AEncoder->sample_fmt = AV_SAMPLE_FMT_FLTP;
		encoder->AEncoder->sample_rate = aInfo.SampleRate;
		encoder->AEncoder->bit_rate = aInfo.AudioBitrate;
		encoder->AEncoder->channel_layout = AV_CH_LAYOUT_STEREO; //AV_CH_LAYOUT_STEREO;
		encoder->AEncoder->channels = av_get_channel_layout_nb_channels(encoder->AEncoder->channel_layout);
		int ret = avcodec_open2(encoder->AEncoder, pCodec, NULL);
		if (ret < 0)
		{
			printf("Could not open audio encoder.\n");
			CarEye_EncoderRelease(encoder);
			return NULL;
		}

		encoder->AFrame = av_frame_alloc();
		if (encoder->AFrame == NULL)
		{
			printf("Alloc audio frame fail!\n");
			CarEye_EncoderRelease(encoder);
			return NULL;
		}
		encoder->AFrame->nb_samples = encoder->AEncoder->frame_size;
		encoder->AFrame->format = encoder->AEncoder->sample_fmt;
		encoder->AFrame->channel_layout = encoder->AEncoder->channel_layout;
		if (av_frame_get_buffer(encoder->AFrame, 0) < 0)
		{
			printf("Failed to allocate the audio frame data\n");
			CarEye_EncoderRelease(encoder);
			return NULL;
		}
		encoder->PerPcmSize = av_get_bytes_per_sample(encoder->AEncoder->sample_fmt);
		encoder->PcmSize = encoder->PerPcmSize * encoder->AEncoder->channels * encoder->AFrame->nb_samples;

		// 		memset(&encoder->AdtsHeader, 0, sizeof(ADTS_HEADER));
		// 		encoder->AdtsHeader.syncword = 0xFFF;
		// 		encoder->AdtsHeader.ID = 0;
		// 		encoder->AdtsHeader.protection_absent = 1;
		// 		encoder->AdtsHeader.profile = 2 - 1;
		// 		encoder->AdtsHeader.sampling_frequency_index = GetAACSampleFrequencyIndex(encoder->AEncoder->sample_rate);
		// 		encoder->AdtsHeader.channel_configuration = 2;
		// 		encoder->AdtsHeader.adts_buffer_fullness = 0x7FF;
		// 		// 就一帧AAC数据
		// 		encoder->AdtsHeader.number_of_raw_data_blocks_in_frame = 0;

		if (encoder->AEncoder->codec_id == AV_CODEC_ID_AAC)
		{
			int profile = 2;	//AAC LC  
			int freqIdx = GetAACSampleFrequencyIndex(encoder->AEncoder->sample_rate);
			encoder->AdtsHeader[0] = 0xFF;      // 11111111     = syncword  
			encoder->AdtsHeader[1] = 0xF1;      // 1111 0 00 1  = syncword MPEG-4 Layer No CRC 
			encoder->AdtsHeader[2] = (((profile - 1) << 6) + (freqIdx << 2) + (encoder->AEncoder->channels >> 2));
			encoder->AdtsHeader[6] = 0xFC;
		}
	}

	return encoder;
}

/*
* Comments: 释放编码器资源
* Param aEncoder: 要释放的编码器
* @Return None
*/
CE_API void CE_APICALL CarEye_EncoderRelease(CarEye_Encoder_Handle aEncoder)
{
	CarEyeEncoder *encoder = (CarEyeEncoder *)aEncoder;

	if (encoder == NULL)
	{
		return;
	}
	if (encoder->VEncoder != NULL)
	{
		avcodec_close(encoder->VEncoder);
		av_free(encoder->VEncoder);
		encoder->VEncoder = NULL;
	}
	if (encoder->AEncoder != NULL)
	{
		avcodec_close(encoder->AEncoder);
		av_free(encoder->AEncoder);
		encoder->AEncoder = NULL;
	}
	if (encoder->VFrame != NULL)
	{
		av_frame_free(&encoder->VFrame);
		encoder->VFrame = NULL;
	}
	if (encoder->AFrame != NULL)
	{
		av_frame_free(&encoder->AFrame);
		encoder->AFrame = NULL;
	}

	delete encoder;
	encoder = NULL;
}

/*
* Comments: 将输入YUV视频编码为设置好的格式数据输出
* Param aEncoder: 申请到的有效编码器
* Param aFilter: 如需添加水印，则传入已创建的水印编码器对象
* Param aYuv: 要编码的YUV数据
* Param aBytes: [输出]编码后的视频流
* @Return int < 0编码失败，> 0为编码后数据字节个数 ==0表示参数无效
*/
CE_API int CE_APICALL CarEye_EncoderYUV(CarEye_Encoder_Handle aEncoder,
                                        CarEye_YUVFrame *aYuv,int aPts,
                                        unsigned char *aBytes)
{
    CarEyeEncoder *encoder = (CarEyeEncoder *)aEncoder;
    if (encoder == NULL || encoder->VEncoder == NULL)
    {
        return 0;
    }
    if (aBytes == NULL)
    {
        return 0;
    }

    int ret;
    int out_size = 0;
    AVPacket packet = { 0 };

    av_init_packet(&packet);
    packet.data = NULL;
    packet.size = 0;
    // 赋值Y值
    memcpy(encoder->VFrame->data[0], aYuv->Y, aYuv->YSize);
    memcpy(encoder->VFrame->data[1], aYuv->U, aYuv->USize);
    memcpy(encoder->VFrame->data[2], aYuv->V, aYuv->VSize);
    encoder->VFrame->pts =aPts;

    ret = Encode(encoder->VEncoder, encoder->VFrame, &packet);
    if (ret < 0)
    {
        CarEyeLog("Encode video error.\n");
        av_packet_unref(&packet);
        return ret;
    }

    out_size = packet.size;
    if (out_size > 0)
    {
        memcpy(aBytes, packet.data, packet.size);
    }
    av_packet_unref(&packet);

    return out_size;
}


/*
* Comments: 获取PCM编码时接受的最大字节数
* Param aEncoder: 申请到的有效编码器
* @Return PCM编码缓冲区最大字节数
*/
CE_API int CE_APICALL CarEye_GetPcmMaxSize(CarEye_Encoder_Handle aEncoder)
{
	CarEyeEncoder *encoder = (CarEyeEncoder *)aEncoder;
	if (encoder == NULL || encoder->AEncoder == NULL)
	{
		return -1;
	}

	return encoder->PcmSize;
}

/*
* Comments: 将输入的PCM音频编码为指定数据格式输出
* Param aEncoder: 申请到的有效编码器
* Param aPcm: 要编码的PCM数据
* Param aSize: 要编码音频流字节数
* Param aPts: 当前视频帧序号
* Param aBytes: [输出] 编码后的音频流
* @Return int < 0编码失败，> 0为编码后PCM的字节个数 ==0表示参数无效
*/
CE_API int CE_APICALL CarEye_EncoderPCM(CarEye_Encoder_Handle aEncoder,
	unsigned char *aPcm, int aSize, int aPts,
	unsigned char *aBytes)
{
	CarEyeEncoder *encoder = (CarEyeEncoder *)aEncoder;
	if (encoder == NULL || encoder->AEncoder == NULL)
	{
		return 0;
	}
	if (aBytes == NULL || aSize < 1 || aPcm == NULL)
	{
		return 0;
	}

	int ret;
	int out_size = 0;
	int i = 0, j = 0;
	int cp_count = 0;
	AVPacket packet = { 0 };

	packet.data = NULL;
	packet.size = 0;
	av_init_packet(&packet);

	for (i = 0; i < encoder->AFrame->nb_samples; i++)
	{
		for (j = 0; j < encoder->AEncoder->channels; j++)
		{
			memcpy(encoder->AFrame->data[j] + i * encoder->PerPcmSize, aPcm, encoder->PerPcmSize);
			cp_count += encoder->PerPcmSize;
			aPcm += encoder->PerPcmSize;
			if (cp_count >= aSize)
			{
				break;
			}
		}
	}

	encoder->AFrame->pts = aPts;
	ret = Encode(encoder->AEncoder, encoder->AFrame, &packet);
	if (ret < 0)
	{
		printf("Decode audio error.\n");
		av_packet_unref(&packet);
		return ret;
	}

	out_size = packet.size;
	if (out_size > 0)
	{
		if (encoder->AEncoder->codec_id == AV_CODEC_ID_AAC)
		{
			//			encoder->AdtsHeader.aac_frame_length = packet.size;

			encoder->AdtsHeader[3] = (((encoder->AEncoder->channels & 3) << 6) + ((7 + packet.size) >> 11));
			encoder->AdtsHeader[4] = (((7 + packet.size) & 0x7FF) >> 3);
			encoder->AdtsHeader[5] = ((((7 + packet.size) & 7) << 5) + 0x1F);

			memcpy(aBytes, (unsigned char *)&encoder->AdtsHeader, 7);
			aBytes += 7;
			out_size += 7;
		}
		memcpy(aBytes, packet.data, packet.size);
	}
	av_packet_unref(&packet);
	return out_size;
}
