#include "FFVideoFilter.h"

#include "public.h"


FFVideoFilter::FFVideoFilter(void)
{
	memset(m_filter_descr, 0x00, MAX_FILTER_DESCR);
	memset(m_FontName, 0, sizeof(m_FontName));
	m_pBuffersink_ctx = NULL;
	m_pBuffersrc_ctx = NULL;
	m_pFilter_graph = NULL;
	m_outputs = m_inputs = NULL;
	video_stream_index = -1 ;
	audio_stream_index  = -1;
}

FFVideoFilter::~FFVideoFilter(void)
{
	if (m_pFilter_graph != NULL)
	{
		avfilter_graph_free(&m_pFilter_graph);
		m_pFilter_graph = NULL;
	}
	if(m_outputs)
	{
		avfilter_inout_free(&m_outputs);
		m_outputs = NULL;
	}
	if(m_inputs)
	{
		avfilter_inout_free(&m_inputs);
		m_inputs = NULL;
	}
	CarEyeLog("~FFVideoFilter\n");		
	
}

static void EncodeCode(char* pSrc, char* pDest)
{
	int Count;
	Count = strlen(pSrc);
	int j = 0;
	for(int i = 0;  i < Count; i++)
	{
		if(pSrc[i] == ':')
		{
			pDest[j++] = 0x5c;
			pDest[j++] = 0x5c;
			pDest[j++] = 0x5c;
			pDest[j++] = ':';			
		}else
		{
			pDest[j++] = pSrc[i];
		}				
	}	
}

int FFVideoFilter::InitFilters( CarEye_OSDParam* param)
{	
	if(param->Width<= 0 || param->Height <= 0)
	{
		return -1;
	}
	if(param->X >= param->Width || param->Y>= param->Height)
	{
		return -1;
  }
  if(strlen(param->FontName)<=0)
  	return -1;
	m_PicWidth = param->Width;
	m_PicHeight = param->Height;
	m_StartX = param->X;
	m_startY = param->Y;	
	EncodeCode(param->FontName, m_FontName);
	m_FramesPerSecond =  param->FramesPerSecond;
	m_FontSize = param->FontSize;
	m_Transparency = param->Transparency;
	m_FontColor= param->FontColor;	
	return 0;
}



int FFVideoFilter::BlendFilters(AVFrame *aFrame, char* txt )
{
 	int ret = 0; 	 	
 	char args[MAX_FILTER_DESCR];
 	char txtoverlay[TXT_MAX_LEN];
 	memset(txtoverlay,0,sizeof(txtoverlay));
	if(strlen(txt)<=0)
  	return -1;
    
	AVFilter *buffersrc  = avfilter_get_by_name("buffer");
	AVFilter *buffersink = avfilter_get_by_name("buffersink");
	m_outputs = avfilter_inout_alloc();
	m_inputs  = avfilter_inout_alloc();
	enum AVPixelFormat pix_fmts[] = { AV_PIX_FMT_YUV420P, AV_PIX_FMT_NONE };
	AVBufferSinkParams *buffersink_params;
	m_pFilter_graph = avfilter_graph_alloc();
	/* buffer video source: the decoded frames from the decoder will be inserted here. */
	snprintf(args, sizeof(args),
		"video_size=%dx%d:pix_fmt=%d:time_base=%d/%d:pixel_aspect=%d/%d",
		m_PicWidth, m_PicHeight, AV_PIX_FMT_YUV420P,
		1, m_FramesPerSecond, 1, 1);
	//	pCodecCtx->time_base.num, pCodecCtx->time_base.den,
	// 		pCodecCtx->sample_aspect_ratio.num, pCodecCtx->sample_aspect_ratio.den);
	ret = avfilter_graph_create_filter(&m_pBuffersrc_ctx, buffersrc, "src",
		args, NULL, m_pFilter_graph);
	if (ret < 0) {
		printf("Cannot create buffer source\n");
		goto the_end1;
	}

	/* buffer video sink: to terminate the filter chain. */
	buffersink_params = av_buffersink_params_alloc();
	buffersink_params->pixel_fmts = pix_fmts;
	ret = avfilter_graph_create_filter(&m_pBuffersink_ctx, buffersink, "out",
		NULL, buffersink_params, m_pFilter_graph);
	av_free(buffersink_params);
	if (ret < 0) {
		printf("Cannot create buffer sink\n");
		goto the_end1;
	}

	/* Endpoints for the filter graph. */
	m_outputs->name       = av_strdup("in");
	m_outputs->filter_ctx = m_pBuffersrc_ctx;
	m_outputs->pad_idx    = 0;
	m_outputs->next       = NULL;

	m_inputs->name       = av_strdup("out");
	m_inputs->filter_ctx = m_pBuffersink_ctx;
	m_inputs->pad_idx    = 0;
	m_inputs->next       = NULL;	
	
 	// ֱ½Ԋ¹ԃOSD
 	
 EncodeCode(txt, txtoverlay);
 sprintf(m_filter_descr, "drawtext=fontfile=%s:fontcolor=%06X@%.2f:x=%d:y=%d:fontsize=%d:text='%s'",
			m_FontName, m_FontColor, m_Transparency,
			m_StartX, m_startY, m_FontSize, txtoverlay);
			printf("SubTitle %s\n",m_filter_descr);
	
	if ((ret = avfilter_graph_parse_ptr(m_pFilter_graph, m_filter_descr,
		&m_inputs, &m_outputs, NULL)) < 0)
	{		
		printf("Cannot avfilter_graph_parse_ptr param.FontName:%s, FontColor:%d,x:%d,Y:%d\n",m_FontName, m_FontColor,m_StartX,m_startY);
		goto the_end;
	}
	if ((ret = avfilter_graph_config(m_pFilter_graph, NULL)) < 0)
	{
		
			printf("Cannot avfilter_graph_config\n");
			goto the_end;
	}
	if (av_buffersrc_add_frame(m_pBuffersrc_ctx, aFrame) < 0) {
		printf("Cannot av_buffersrc_add_frame\n");
		goto the_end;
	}
	/* pull filtered pictures from the filtergraph */
	ret = av_buffersink_get_frame(m_pBuffersink_ctx, aFrame);
 the_end:
 	avfilter_inout_free(&m_inputs);
 	avfilter_inout_free(&m_outputs);
 the_end1:	
 	avfilter_graph_free(&m_pFilter_graph);
	return ret;

}
