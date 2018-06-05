/*
 * Car eye 车辆管理平台: www.car-eye.cn
 * Car eye 开源网址: https://github.com/Car-eye-team
 * CarEyeMPEGAPI.cpp
 *
 * Author: Wgj
 * Date: 2018-05-18 23:25
 * Copyright 2018
 *
 * Car eye MPEG库基础接口实现
 */

#include "CarEyeTypes.h"

extern "C"
{
#include "libavformat/avformat.h"
#include "libavfilter/avfilter.h"
};
/*
* Comments: 使用本库之前必须调用一次本方法
* Param : None
* @Return void
*/
CE_API void CE_APICALL CarEye_MPEG_Init(void)
{
	// 注册编码器
	av_register_all();
	// 注册OSD层
	avfilter_register_all();
}
