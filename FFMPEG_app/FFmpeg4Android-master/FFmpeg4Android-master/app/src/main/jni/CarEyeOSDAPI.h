/*
 * Car eye 车辆管理平台: www.car-eye.cn
 * Car eye 开源网址: https://github.com/Car-eye-team
 * CarEyeOSDAPI.h
 *
 * Author: Wgj
 * Date: 2018-05-18 21:48
 * Copyright 2018
 *
 * Car eye视频水印添加接口声明
 */

#ifndef __CarEyeOSDAPI_H_
#define __CarEyeOSDAPI_H_

#include "CarEyeTypes.h"
#ifdef __cplusplus
extern "C"
{
#endif
 /*
 * Comments: 打开水印资源
 * Param aParam: 水印参数
 * @Return int 是否成功，0成功，其他失败
 */
CE_API CarEye_OSD_Handle CE_APICALL CarEye_OSD_Create(CarEye_OSDParam *aParam);

/*
* Comments: 对输入YUV帧信息添加水印
* Param : aFrame: [输入/输出] 要添加水印的YUV帧数据
* @Return void
*/
CE_API int CE_APICALL CarEye_OSD_Encode(CarEye_OSD_Handle aFilter, CarEye_YUVFrame *aFrame, char* txtoverlay);

/*
* Comments: 释放水印资源
* Param aFilter: 水印编码器对象
* @Return int 关闭成功与否 0成功
*/
CE_API void CE_APICALL CarEye_OSD_Release(CarEye_OSD_Handle aFilter);
#ifdef __cplusplus
}
#endif
#endif // __CarEyeFilterAPI_H_
