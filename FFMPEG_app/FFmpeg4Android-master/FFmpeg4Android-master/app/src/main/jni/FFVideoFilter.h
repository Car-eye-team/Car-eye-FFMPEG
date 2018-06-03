/*
 * Car eye 车辆管理平台: www.car-eye.cn
 * Car eye 开源网址: https://github.com/Car-eye-team
 * FFVideoFilter.h
 *
 * Author: Wgj
 * Date: 2018-05-16 22:46
 * Copyright 2018
 *
 * 视频水印添加实体类声明
 */

#pragma once

#define __STDC_CONSTANT_MACROS
#include "CarEyeEncoderAPI.h"

#ifdef _WIN32

extern "C"
{
#include "libavcodec/avcodec.h"
#include "libavfilter/avfiltergraph.h"
#include "libavfilter/buffersink.h"
#include "libavfilter/buffersrc.h"
#include "libavutil/avutil.h"
#include "libavutil/imgutils.h"

};
#else
 //Linux...
#ifdef __cplusplus
extern "C"
{
#endif
#include "libavcodec/avcodec.h"
#include <libavfilter/avfiltergraph.h>
#include <libavfilter/buffersink.h>
#include <libavfilter/buffersrc.h>
#include <libavutil/avutil.h>
#include <libavutil/imgutils.h>
#ifdef __cplusplus
};
#endif
#endif

#define MAX_FILTER_DESCR 512
#define TXT_MAX_LEN			 256

class FFVideoFilter
{
public:
	FFVideoFilter(void);
	~FFVideoFilter(void);

	// Api Interface [4/18/2018 SwordTwelve]
	int InitFilters(CarEye_OSDParam *aParam);
	int BlendFilters(AVFrame *aFrame, char* txt);

private:
	char m_filter_descr[MAX_FILTER_DESCR];	//= "movie=my_logo.png[wm];[in][wm]overlay=5:5[out]";

	AVFilterContext *	m_pBuffersink_ctx;
	AVFilterContext *	m_pBuffersrc_ctx;
	AVFilterGraph *		m_pFilter_graph;
	int video_stream_index;//= -1;
	int audio_stream_index;//= -1;
	AVFilterInOut *   m_outputs;
	AVFilterInOut *   m_inputs;
	int m_PicWidth;
	int m_PicHeight;
	int m_StartX;
	int m_startY;
	char m_FontName[128];
	int m_FramesPerSecond;
	int m_FontSize;
	float m_Transparency;
	int m_FontColor;
	
	
};
