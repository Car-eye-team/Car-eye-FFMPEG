/*
 * Car eye 车辆管理平台: www.car-eye.cn
 * Car eye 开源网址: https://github.com/Car-eye-team
 * CarEyeOSDAPI.cpp
 *
 * Author: Wgj
 * Date: 2018-05-18 21:47
 * Copyright 2018
 *
 * Car eye视频水印添加接口实现
 */
#include "CarEyeOSDAPI.h"
#include "FFVideoFilter.h"

 // 水印编码器结构体定义
typedef struct
{
	// 视频编码帧暂存
	AVFrame *VFrame;
	// 视频字幕对象
	FFVideoFilter *VideoFilter;
}CarEyeFilter;

  /*
 * Comments: 打开水印资源
 * Param aParam: 水印参数
 * @Return int 是否成功，0成功，其他失败
 */
CE_API CarEye_OSD_Handle CE_APICALL CarEye_OSD_Create(CarEye_OSDParam *aParam)
{
	CarEyeFilter *filter = new CarEyeFilter;
	if (filter == NULL)
	{
		return NULL;
	}

	filter->VFrame = av_frame_alloc();
	if (filter->VFrame == NULL)
	{
		printf("Alloc video frame faile!\n");
		CarEye_OSD_Release(filter);
		return NULL;
	}

	filter->VFrame->format = (AVPixelFormat)aParam->YUVType;
	filter->VFrame->width = aParam->Width;
	filter->VFrame->height = aParam->Height;

	if (av_image_alloc(filter->VFrame->data, filter->VFrame->linesize,
		filter->VFrame->width, filter->VFrame->height,
		(AVPixelFormat)filter->VFrame->format, 16) < 0)
	{
		printf("Could not allocate raw picture buffer!\n");
		CarEye_OSD_Release(filter);
		return NULL;
	}

	filter->VideoFilter = new FFVideoFilter();
	if (filter->VideoFilter == NULL)
	{
		CarEye_OSD_Release(filter);
		return NULL;
	}
	if (filter->VideoFilter->InitFilters(aParam) != 0)
	{
		CarEye_OSD_Release(filter);
		return NULL;
	}

	return filter;
}

/*
* Comments: 对输入YUV帧信息添加水印
* Param : aFrame: [输入/输出] 要添加水印的YUV帧数据
* @Return void
*/
CE_API int CE_APICALL CarEye_OSD_Encode(CarEye_OSD_Handle aFilter, CarEye_YUVFrame *aFrame, char* txtoverlay)
{
	CarEyeFilter *filter = (CarEyeFilter *)aFilter;

	if (filter == NULL)
	{
		return -1;
	}

	memcpy(filter->VFrame->data[0], aFrame->Y, aFrame->YSize);
	memcpy(filter->VFrame->data[1], aFrame->U, aFrame->USize);
	memcpy(filter->VFrame->data[2], aFrame->V, aFrame->VSize);
	filter->VFrame->pts++;
	if (filter->VideoFilter->BlendFilters(filter->VFrame, txtoverlay) < 0)
	{
		printf("Filter video error.\n");
		return -1;
	}
	memcpy(aFrame->Y, filter->VFrame->data[0], aFrame->YSize);
	memcpy(aFrame->U, filter->VFrame->data[1], aFrame->USize);
	memcpy(aFrame->V, filter->VFrame->data[2], aFrame->VSize);
	return 0;
}


/*
* Comments: 释放水印资源
* Param aFilter: 水印编码器对象
* @Return int 关闭成功与否 0成功
*/
CE_API void CE_APICALL CarEye_OSD_Release(CarEye_OSD_Handle aFilter)
{
	CarEyeFilter *filter = (CarEyeFilter *)aFilter;

	if (filter == NULL)
	{
		return;
	}

	if (filter->VFrame != NULL)
	{
		av_frame_free(&filter->VFrame);
		filter->VFrame = NULL;
	}
	if (filter->VideoFilter != NULL)
	{
		delete filter->VideoFilter;
		filter->VideoFilter = NULL;
	}

	delete filter;
}
