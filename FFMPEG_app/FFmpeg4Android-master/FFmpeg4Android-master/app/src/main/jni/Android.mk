

ifeq ($(TARGET_ARCH_ABI), x86)
LIB_NAME_PLUS := x86
else ifeq ($(TARGET_ARCH_ABI), arm64-v8a)
LIB_NAME_PLUS := arm64-v8a
else  
LIB_NAME_PLUS := armeabi-v7a
endif

LOCAL_PATH:= $(call my-dir)

LOCAL_ALLOW_UNDEFINED_SYMBOLS := true

include $(CLEAR_VARS)
LOCAL_MODULE:= avcodec-prebuilt-$(LIB_NAME_PLUS)
LOCAL_SRC_FILES:= prebuilt/$(LIB_NAME_PLUS)/libavcodec.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := swresample-prebuilt-$(LIB_NAME_PLUS)
LOCAL_SRC_FILES := prebuilt/$(LIB_NAME_PLUS)/libswresample.a
include $(PREBUILT_STATIC_LIBRARY)





include $(CLEAR_VARS)
LOCAL_MODULE := x264-prebuilt-$(LIB_NAME_PLUS)
LOCAL_SRC_FILES := prebuilt/$(LIB_NAME_PLUS)/libx264.a
include $(PREBUILT_STATIC_LIBRARY)




include $(CLEAR_VARS)
LOCAL_MODULE := swscale-prebuilt-$(LIB_NAME_PLUS)
LOCAL_SRC_FILES := prebuilt/$(LIB_NAME_PLUS)/libswscale.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE :=  freetype-prebuilt-$(LIB_NAME_PLUS)
LOCAL_SRC_FILES := prebuilt/$(LIB_NAME_PLUS)/libfreetype.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE:= avfilter-prebuilt-$(LIB_NAME_PLUS)
LOCAL_SRC_FILES:= prebuilt/$(LIB_NAME_PLUS)/libavfilter.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE:= avformat-prebuilt-$(LIB_NAME_PLUS)
LOCAL_SRC_FILES:= prebuilt/$(LIB_NAME_PLUS)/libavformat.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE :=  avutil-prebuilt-$(LIB_NAME_PLUS)
LOCAL_SRC_FILES := prebuilt/$(LIB_NAME_PLUS)/libavutil.a
include $(PREBUILT_STATIC_LIBRARY)


include $(CLEAR_VARS)
LOCAL_MODULE :=  postproc-prebuilt-$(LIB_NAME_PLUS)
LOCAL_SRC_FILES := prebuilt/$(LIB_NAME_PLUS)/libpostproc.a
include $(PREBUILT_STATIC_LIBRARY)


include $(CLEAR_VARS)
LOCAL_MODULE :=  libpng17-prebuilt-$(LIB_NAME_PLUS)
LOCAL_SRC_FILES := prebuilt/$(LIB_NAME_PLUS)/libpng17.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)

LOCAL_MODULE := libffmpegjni
LOCAL_SRC_FILES := CarEyeJni.c CarEyeEncoderAPI.cpp FFVideoFilter.cpp CarEyeOSDAPI.cpp CarEyeMPEGAPI.cpp CarEyeDecoderAPI.cpp
LOCAL_LDLIBS := -L$(SYSROOT)/usr/lib -llog -lz -lm 
LOCAL_STATIC_LIBRARIES := avformat-prebuilt-$(LIB_NAME_PLUS) \
													avfilter-prebuilt-$(LIB_NAME_PLUS) \
													avcodec-prebuilt-$(LIB_NAME_PLUS) \
													x264-prebuilt-$(LIB_NAME_PLUS) \
													swresample-prebuilt-$(LIB_NAME_PLUS) \
													freetype-prebuilt-$(LIB_NAME_PLUS) \
													libpng17-prebuilt-$(LIB_NAME_PLUS) \
                          avutil-prebuilt-$(LIB_NAME_PLUS) \
                              swscale-prebuilt-$(LIB_NAME_PLUS) \
                              postproc-prebuilt-$(LIB_NAME_PLUS)                        
                             
LOCAL_C_INCLUDES += -L$(SYSROOT)/usr/include
LOCAL_C_INCLUDES += $(LOCAL_PATH)/include


include $(BUILD_SHARED_LIBRARY)
