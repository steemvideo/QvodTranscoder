// QvodFFmpegMux.cpp : 定义 DLL 应用程序的入口点。
//
#pragma once
#include "stdafx.h"
#include "QvodFFmpegMux.h"

#define MAXBLOCKS 100000

int max_frames[4] = {INT_MAX, INT_MAX, INT_MAX, INT_MAX};

#define lrintf(f) (f>=0?(int32_t)(f+(float)0.5):(int32_t)(f-(float)0.4))
#pragma comment(lib, "lib0.6/avcodec-52.lib")
#pragma comment(lib, "lib0.6/avformat-52.lib")
#pragma comment(lib, "lib0.6/avdevice-52.lib")
#pragma comment(lib, "lib0.6/avutil-50.lib")
#pragma comment(lib, "lib0.6/avfilter-1.lib")
#pragma comment(lib, "lib0.6/postproc-51.lib")
#pragma comment(lib, "lib0.6/swscale-0.lib")

AVRational _AVRational(int num, int den) 
{
	AVRational r = {num, den};
	return r;
}

const AMOVIESETUP_MEDIATYPE sudPinTypes =
{
	&MEDIATYPE_NULL,            // Major type
	&MEDIASUBTYPE_NULL          // Minor type
};

const AMOVIESETUP_PIN sudPins =
{
	L"Input",                   // Pin string name
	FALSE,                      // Is it rendered
	FALSE,                      // Is it an output
	FALSE,                      // Allowed none
	FALSE,                      // Likewise many
	&CLSID_NULL,                // Connects to filter
	L"Output",                  // Connects to pin
	1,                          // Number of types
	&sudPinTypes                // Pin information
};


const AMOVIESETUP_FILTER sudFilter =
{
	&CLSID_QvodFFmpegMux, L"QvodFFmpegMuxer", MERIT_DO_NOT_USE, 1, &sudPins
};

CFactoryTemplate g_Templates[] =
{
	
		L"QvodFFmpegMuxer",
			&CLSID_QvodFFmpegMux,
			CFFmpegMuxFilter::CreateInstance,
			NULL,
			&sudFilter
	
};

int g_cTemplates = sizeof(g_Templates) / sizeof(g_Templates[0]);

STDAPI DllRegisterServer()
{
	OutputDebugString(L"11111111111111/n");
	return AMovieDllRegisterServer2(TRUE);
}

STDAPI DllUnregisterServer()
{
	return AMovieDllRegisterServer2(FALSE);
}

extern "C" BOOL WINAPI DllEntryPoint(HINSTANCE, ULONG, LPVOID);//APIENTRY

BOOL WINAPI DllMain(HANDLE hModule, 
					  DWORD  dwReason, 
					  LPVOID lpReserved)
{
	OutputDebugString(L"222222222/n");
	return DllEntryPoint((HINSTANCE)(hModule), dwReason, lpReserved);
}

CFFmpegMuxInputPin::CFFmpegMuxInputPin(LPCWSTR pName, CFFmpegMuxFilter* pFilter, CCritSec* pLock, HRESULT* phr)
: CBaseInputPin(NAME("CFFmpegMuxInputPin"), pFilter, pLock, phr, pName)
, m_fActive(false)
, m_ParentFilter(pFilter)
, m_fEndOfStreamReceived(false)
, m_rtDur(0)
{
	m_bVideo = true;
	m_bAudio = true;
	allocated_audio_out_size = 0;
	allocated_audio_buf_size = 0;
	picture = NULL;
	img_convert_ctx = NULL;
	m_voutbuffer = NULL;
	m_aoutbuffer = NULL;
	i = 0;
	audio_buf = NULL;
	audio_out = NULL;
	allocated_audio_out_size = 0;
	allocated_audio_buf_size = 0;
	memset(&bih,0,sizeof(BITMAPINFOHEADER));
	memset(&audioformat,0,sizeof(WAVEFORMATEX));
	video_sync_method = -1;
	//audioformat = 0;
}

CFFmpegMuxInputPin::~CFFmpegMuxInputPin()
{
	if(m_voutbuffer)
	{
		av_free(m_voutbuffer);
		m_voutbuffer = NULL;
	}
}

//HRESULT CFFmpegMuxInputPin::GetMediaType(int iPosition, CMediaType* pmt)
//{
//	if(pmt)
//		if(pmt->pbFormat == NULL)
//		{
//			OutputDebugString(L"GetMediaType is NULL!\n");
//		}
//	OutputDebugString(L"GetMediaType\n");
//	switch(iPosition) 
//	{
//	case 0:
//		pmt->majortype = MEDIATYPE_Video;
//		pmt->subtype = MEDIASUBTYPE_YV12;
//		return S_OK;
//	case 1:
//		pmt->majortype = MEDIATYPE_Audio;
//		pmt->subtype = MEDIASUBTYPE_PCM;
//		return S_OK;
//	default:
//		return VFW_S_NO_MORE_ITEMS;
//	}
//}


CMediaType& CFFmpegMuxInputPin::CurrentMediaType()
{
	return m_mt;
}

HRESULT CFFmpegMuxInputPin::CheckMediaType(const CMediaType* pmt)
{
	if(pmt)
		if(pmt->pbFormat == NULL)
		{
			OutputDebugString(L"CheckMediaType is NULL!\n");
		}
	OutputDebugString(L"CheckMediaType\n");
	if	((pmt->majortype == MEDIATYPE_Video 
		&& pmt->subtype == MEDIASUBTYPE_YV12 ) 
		||
		(pmt->majortype == MEDIATYPE_Audio 
		&& pmt->subtype == MEDIASUBTYPE_PCM )) 
	{
		bool b = GetBmpHeader(*pmt,bih);
	 	b  = GetWaveFormat(*pmt,audioformat);
		return S_OK;
	} 
	else 
	{
		return E_FAIL;
	}
}

HRESULT CFFmpegMuxInputPin::BreakConnect()
{
	OutputDebugString(L"BreakConnect\n");
	HRESULT hr;

	if(FAILED(hr = __super::BreakConnect()))
		return hr;
	return hr;
}

HRESULT CFFmpegMuxInputPin::CompleteConnect(IPin* pPin)
{
	OutputDebugString(L"CompleteConnect\n");
	m_ParentFilter->AddInput();
	return S_OK;
}

HRESULT CFFmpegMuxInputPin::Active()
{
	m_fActive = true;
	m_rtLastStart = m_rtLastStop = -1;
	//m_fEndOfStreamReceived = false;
	return __super::Active();
}

HRESULT CFFmpegMuxInputPin::Inactive()
{
	m_fActive = false;
	m_fEndOfStreamReceived = true;
	CAutoLock cAutoLock(&m_csQueue);
	m_block.RemoveAll();
	return __super::Inactive();
}

STDMETHODIMP CFFmpegMuxInputPin::EndOfStream()
{
	HRESULT hr;

	if(FAILED(hr = __super::EndOfStream()))
		return hr;


	CAutoLock cAutoLock(&m_csQueue);

	//m_fActive = false;

	m_fEndOfStreamReceived = true;

	return hr;
}

STDMETHODIMP CFFmpegMuxInputPin::ReceiveCanBlock()
{
	return S_FALSE;
}

STDMETHODIMP CFFmpegMuxInputPin::NewSegment(REFERENCE_TIME tStart, REFERENCE_TIME tStop, double dRate)
{
	//return S_OK;
	CAutoLock cAutoLock(&m_csReceive);
	return __super::NewSegment(tStart, tStop, dRate);
}

STDMETHODIMP CFFmpegMuxInputPin::BeginFlush()
{
	return __super::BeginFlush();
}

STDMETHODIMP CFFmpegMuxInputPin::EndFlush()
{
	return __super::EndFlush();
}



void CFFmpegMuxInputPin::SaveFrame(AVFrame *pFrame, int width, int height, int iFrame) 
{
	FILE *pFile;
	char szFilename[32];
	int  y;

	// Open file
	sprintf(szFilename, "frame%d.ppm", iFrame);
	pFile=fopen(szFilename, "wb");
	if(pFile==NULL)
		return;

	// Write header
	fprintf(pFile, "P6\n%d %d\n255\n", width, height);

	// Write pixel data
	for(y=0; y<height; y++)
		fwrite(pFrame->data[0]+y*pFrame->linesize[0], 1, width*3, pFile);

	// Close file
	fclose(pFile);
}

void CFFmpegMuxInputPin::YV12ToAVFrame(BYTE * pData,int nLen,int nWidth,int nHeight,AVFrame * pFrame)
{
	avcodec_get_frame_defaults(pFrame);
	int size=avpicture_get_size(PIX_FMT_YUV420P,nWidth ,nHeight );
	ASSERT(size==nLen);
	avpicture_fill( (AVPicture*) pFrame,pData,PIX_FMT_YUV420P , nWidth ,nHeight );
	BYTE * ptmp=pFrame->data [1];//U,V互换
	pFrame->data [1]=pFrame->data [2];
	pFrame->data [2]=ptmp;
}

STDMETHODIMP CFFmpegMuxInputPin::Receive(IMediaSample* pSample)
{
	if(m_fEndOfStreamReceived) 
	{
		return S_FALSE;
	}


	CAutoLock locLock(&m_csReceive);
	while(m_fActive)
	{
		{
			CAutoLock cAutoLock2(&m_csQueue);
			if(m_block.GetCount() < MAXBLOCKS)
				break;
		}
	}


	if(!m_fActive) return S_FALSE;

	HRESULT hr;
	int out_size, ret;
	

	if(FAILED(hr = __super::Receive(pSample)))
		return hr;

	
	BYTE* SampleBuff;
	pSample->GetPointer(&SampleBuff);
	long len = pSample->GetActualDataLength();

	LONGLONG locStart = -1,locEnd = -1;
	hr = pSample->GetTime(&locStart, &locEnd);
	if(FAILED(hr) || locStart == -1 || locEnd == -1)
	{
		m_ParentFilter->NotifyEvent(EC_ERRORABORT, VFW_E_SAMPLE_TIME_NOT_SET, 0);
		return VFW_E_SAMPLE_TIME_NOT_SET;
	}

	if(m_mt.subtype == MEDIASUBTYPE_YV12)
	{
		double sync_ipts;

		AVOutputStream *ost = m_ParentFilter->GetOutStream(0);
		AVCodecContext *enc= ost->st->codec;
		enc->gop_size = 30;
		sync_ipts =  (double)locStart/10/AV_TIME_BASE / av_q2d(enc->time_base);
		int nb_frames = 1;

		if(video_sync_method)
		{
			double vdelta = sync_ipts - ost->sync_opts;
			//FIXME set to 0.5 after we fix some dts/pts bugs like in avidec.c GetContext
			if (vdelta < -1.1)
				nb_frames = 0;
			else if (video_sync_method == 2 || (video_sync_method<0 && (m_ParentFilter->GetContext()->oformat->flags & AVFMT_VARIABLE_FPS)))
			{
				if(vdelta<=-0.6)
				{
					nb_frames=0;
				}
				else if(vdelta>0.6)
					ost->sync_opts= lrintf(sync_ipts);
			}
			else if (vdelta > 1.1)
				nb_frames = lrintf(vdelta);
			//if (nb_frames == 0){
			//	++nb_frames_drop;
			//}else if (nb_frames > 1) {
			//	nb_frames_dup += nb_frames - 1;
			//}
		}
		else
			ost->sync_opts= lrintf(sync_ipts);

		nb_frames= FFMIN(nb_frames, max_frames[AVMEDIA_TYPE_VIDEO] - ost->frame_number);
		if (nb_frames <= 0)
			return S_OK;
		
		//ost->sync_opts= lrintf(sync_ipts);
		//ost->st->codec->codec_type = AVMEDIA_TYPE_VIDEO;
		//ost->st->codec->bit_rate = 400000;
		//ost->st->codec->width = bih.biWidth;
		//ost->st->codec->height = bih.biHeight;

		int sws_flags = SWS_BICUBIC;
		if(m_bVideo)
		{		
			m_bVideo = false;
			m_voutbuffer = (BYTE*)av_malloc(bih.biWidth*bih.biHeight*6 +200);			
		}
		
		//int yPitch = len/ bih.biHeight * 2 / 3;
		//int uvPitch = yPitch>>1;
		AVFrame p;
		YV12ToAVFrame(SampleBuff,len,bih.biWidth, bih.biHeight,&p);
		p.pts = ost->sync_opts;
		p.pict_type = 0;
		//SaveFrame(&p, bih.biWidth, bih.biHeight, 0) ;
		out_size = avcodec_encode_video(ost->st->codec, m_voutbuffer, bih.biWidth*bih.biHeight*6 +200, &p);
		//av_free(&p);
		/* if zero size, it means the image was buffered */
		if (out_size > 0)
		{
			CAutoPtr<BLOCK> b(new BLOCK());
			av_init_packet(&b->packet);
			if (ost->st->codec->coded_frame->pts != AV_NOPTS_VALUE)
				b->packet.pts= av_rescale_q(ost->st->codec->coded_frame->pts, ost->st->codec->time_base, m_ParentFilter->GetContext()->streams[0]->time_base);

			///b->packet.pts = ost->sync_opts;
			b->start = locStart;
			b->stop = locEnd;
			if(ost->st->codec->coded_frame->key_frame)
				b->packet.flags |= AV_PKT_FLAG_KEY;
			b->packet.stream_index= m_ParentFilter->GetContext()->streams[0]->index;
			b->packet.data= m_voutbuffer;
			b->packet.size= out_size;
			b->packettype = Video_Type;
			CAutoLock cAutoLock2(&m_csQueue);
			m_block.AddTail(b);
			ost->sync_opts++;
			ost->frame_number++;
		}
		else
		{
			ost->sync_opts++;
			ost->frame_number++;
			return S_OK;
		}
	}
	if(m_mt.subtype == MEDIASUBTYPE_PCM)
	{
		
		//pSample->SetSyncPoint(TRUE); 
		uint8_t *buftmp;
		int64_t audio_out_size, audio_buf_size;
		int64_t allocated_for_size= len;

		int size_out, frame_bytes, ret;
		AVOutputStream *ost = m_ParentFilter->GetOutStream(1);
		AVCodecContext *enc= ost->st->codec;

		if(m_bAudio)
		{
			//audioformat = (WAVEFORMATEX *)m_mt.pbFormat;
			enc->sample_rate = audioformat.nSamplesPerSec; 
			enc->time_base= _AVRational(1, enc->sample_rate);
			m_bAudio = FALSE;
		}
		int osize= av_get_bits_per_sample_format(enc->sample_fmt)/8;
		int isize=2 /*av_get_bits_per_sample_format(enc->sample_fmt)/8*/;
		const int coded_bps = av_get_bits_per_sample(enc->codec->id);

need_realloc:
		audio_buf_size= (allocated_for_size + isize*audioformat.nChannels - 1) / (isize*audioformat.nChannels);
		audio_buf_size= (audio_buf_size*enc->sample_rate + enc->sample_rate) / enc->sample_rate;
		audio_buf_size= audio_buf_size*2 + 10000; //safety factors for the deprecated resampling API
		audio_buf_size*= osize*enc->channels;

		audio_out_size= FFMAX(audio_buf_size, enc->frame_size * osize * enc->channels);
		if(coded_bps > 8*osize)
			audio_out_size= audio_out_size * coded_bps / (8*osize);
		audio_out_size += FF_MIN_BUFFER_SIZE;

		if(audio_out_size > INT_MAX || audio_buf_size > INT_MAX)
		{
			return S_FALSE;
			//return S_OK;
		}

		av_fast_malloc(&audio_buf, &allocated_audio_buf_size, audio_buf_size);
		av_fast_malloc(&audio_out, &allocated_audio_out_size, audio_out_size);
		if (!audio_buf || !audio_out)
		{
			fprintf(stderr, "Out of memory in do_audio_out\n");
		}

#define MAKE_SFMT_PAIR(a,b) ((a)+SAMPLE_FMT_NB*(b))
		if (!ost->audio_resample && enc->sample_fmt!=enc->sample_fmt &&
			MAKE_SFMT_PAIR(enc->sample_fmt,enc->sample_fmt)!=ost->reformat_pair) 
		{
		}
		locStart/=10;
		ost->sync_opts= lrintf(/*get_sync_ipts(ost)*/ (double)locStart/AV_TIME_BASE* enc->sample_rate)
			- av_fifo_size(ost->fifo)/(ost->st->codec->channels * 2); //FIXME wrong
		{
			buftmp = SampleBuff;
			size_out = len;
		}
		/* now encode as many frames as possible */
		if (enc->frame_size > 1)
		{
			/* output resampled raw samples */
			if (av_fifo_realloc2(ost->fifo, av_fifo_size(ost->fifo) + size_out) < 0)
			{
				return S_FALSE;
				//return S_OK;
			}
			av_fifo_generic_write(ost->fifo, buftmp, size_out, NULL);

			frame_bytes = enc->frame_size * osize * enc->channels;

			while (av_fifo_size(ost->fifo) >= frame_bytes)
			{
				av_fifo_generic_read(ost->fifo, audio_buf, frame_bytes, NULL);

				ret = avcodec_encode_audio(enc, audio_out, audio_out_size,(short *)audio_buf);
				if (ret < 0) 
				{
					return S_FALSE;
					//return S_OK;
				}
				if(ret == 0)
				{
					continue;
				}
				CAutoPtr<BLOCK> b(new BLOCK());
				av_init_packet(&b->packet);

				if (ost->st->codec->coded_frame->pts != AV_NOPTS_VALUE)
					b->packet.pts=av_rescale_q(ost->st->codec->coded_frame->pts, ost->st->codec->time_base, m_ParentFilter->GetContext()->streams[1]->time_base);
				//b->packet.pts=ost->sync_opts;
				b->start = locStart;
				b->stop = locEnd;
				b->packet.flags |= AV_PKT_FLAG_KEY;
				b->packet.stream_index= m_ParentFilter->GetContext()->streams[1]->index;
				b->packet.data= audio_out;
				b->packet.size= ret;
				b->packettype = Audio_Type;
				CAutoLock cAutoLock2(&m_csQueue);
				m_block.AddTail(b);
				ost->sync_opts += enc->frame_size;
			}
		} 
	}
	return S_OK;
}





bool CFFmpegMuxInputPin::GetBmpHeader(const CMediaType &mt,BITMAPINFOHEADER & bph)
{
	if(*mt.FormatType() == FORMAT_VideoInfo) 
	{
		VIDEOINFOHEADER *pvi = (VIDEOINFOHEADER *) mt.Format();
		bph = pvi->bmiHeader;
		return true;
	}
	else if(*mt.FormatType() == FORMAT_VideoInfo2) 
	{
		VIDEOINFOHEADER2 *pvi = (VIDEOINFOHEADER2 *) mt.Format();
		bph = pvi->bmiHeader;
		return true;
	}
	else
	{
		return false;
	}
}

bool CFFmpegMuxInputPin::GetWaveFormat(const CMediaType &mt,WAVEFORMATEX & p)
{
	if(*mt.FormatType() == FORMAT_WaveFormatEx) 
	{
		WAVEFORMATEX *pai = (WAVEFORMATEX *) mt.Format();
		p = *pai;
		return true;
	}
	else
	{
		return false;
	}
}

void CFFmpegMuxInputPin::write_video_frame(AVFormatContext *oc, AVStream *st,BYTE* pBuffer)
{

}

void CFFmpegMuxInputPin::fill_yuv_image(AVFrame *pict, BYTE* pData, int width, int height,int yPitch,int uvPitch)
{
	for(int i = 0;i < height; i++)
	{
		CopyMemory(pict->data[0],pData,pict->linesize[0]);
		pData += yPitch;
		pict->data[0] += pict->linesize[0];

	}
	for(int i = 0;i < height/2; i++)
	{
		CopyMemory(pict->data[2],pData,pict->linesize[2]);
		pData += uvPitch;
		pict->data[2] += pict->linesize[2];
	}
	for(int i = 0;i < height/2; i++)
	{
		CopyMemory(pict->data[1],pData,pict->linesize[1]);
		pData += uvPitch;
		pict->data[1] += pict->linesize[1];
	}	    
}
AVFrame *CFFmpegMuxInputPin::alloc_picture(int pix_fmt, int width, int height)
{
	AVFrame *picture;
	BYTE *picture_buf;
	int size;

	picture = avcodec_alloc_frame();
	if (!picture)
		return NULL;
	size = avpicture_get_size((PixelFormat)pix_fmt, width, height);
	picture_buf = (BYTE*)av_malloc(size);
	if (!picture_buf) {
		av_free(picture);
		return NULL;
	}
	avpicture_fill((AVPicture *)picture, picture_buf,
		(PixelFormat)pix_fmt, width, height);
	return picture;
}

CUnknown * WINAPI CFFmpegMuxFilter::CreateInstance(LPUNKNOWN punk, HRESULT *phr) 
{
	CFFmpegMuxFilter *pNewObject = new CFFmpegMuxFilter(punk, phr);
	if (pNewObject == NULL) {
		*phr = E_OUTOFMEMORY;
	}
	return pNewObject;
} 

CFFmpegMuxFilter::CFFmpegMuxFilter(LPUNKNOWN pUnk, HRESULT* phr)
: CBaseFilter(NAME("QvodFFmpegMuxer"), NULL, this, CLSID_QvodFFmpegMux)
, m_rtCurrent(0)
, m_fNegative(true), m_fPositive(false)
{
	//CAutoLock cAutoLock(this);
	//AddInput();
	audio_stream_copy = 0;
	video_stream_copy = 0;
	subtitle_stream_copy = 0;
	video_codec_name = NULL;
	audio_codec_name = NULL;
	subtitle_codec_name = NULL;
	audio_disable = 0;
	video_disable = 0;
	subtitle_disable = 0;
	mux_preload= 0.5;
	mux_max_delay= 0.7;
	loop_output = AVFMT_NOOUTPUTLOOP;
	frame_width  = 0;
	frame_height = 0;
	audio_sample_rate = 44100;
	audio_channels = 2;
	thread_count= 1;
	frame_pix_fmt = PIX_FMT_NONE;
	video_global_header = 0;
	frame_aspect_ratio = 0;
	force_fps = 0;
	frame_padtop  = 0;
	frame_padbottom = 0;
	frame_padleft  = 0;
	frame_padright = 0;
	video_qscale = 0;
	video_rc_override_string=NULL;
	nb_ocodecs = 0;
	me_threshold = 0;
	intra_dc_precision = 8;
	intra_matrix = NULL;
	inter_matrix = NULL;
	audio_sample_fmt = SAMPLE_FMT_NONE;
	channel_layout = 0;
	extra_size = 0;
	//mStreamLock = new CCritSec;
	pvideoPin = NULL;
	m_lasttime = 0;
	m_isFirst = TRUE;
	POSITION pos = m_pInputs.GetHeadPosition();
	while(pos)
	{
		CBasePin* pPin = m_pInputs.GetNext(pos);
		if(!pPin->IsConnected()) return;
	}
	CString name;
	name.Format(L"Track %d", m_pInputs.GetCount());
	HRESULT hr;
	pPin = new CFFmpegMuxInputPin(name, this,this, &hr);
	m_pInputs.AddTail(pPin);
	
	if(phr) *phr = S_OK;
	//srand(clock());
}



CFFmpegMuxFilter::~CFFmpegMuxFilter()
{
	//CAutoLock cAutoLock(this);
}

STDMETHODIMP CFFmpegMuxFilter::NonDelegatingQueryInterface(REFIID riid, void** ppv)
{
	CheckPointer(ppv, E_POINTER);
	if (riid == IID_IFFmpegMux)
	{
		return GetInterface((IFFmpegMux *) this, ppv);
	}
	else if(riid == IID_IFileSinkFilter)
	{
		return GetInterface((IFileSinkFilter *) this, ppv);//
	}
	else
	{
		return CBaseFilter::NonDelegatingQueryInterface(riid, ppv);
	}
}

void CFFmpegMuxFilter::AddInput()
{
	POSITION pos = m_pInputs.GetHeadPosition();
	while(pos)
	{
		CBasePin* pPin = m_pInputs.GetNext(pos);
		if(!pPin->IsConnected()) return;
	}

    CString name;
	name.Format(L"Track %d", m_pInputs.GetCount()+1);
	//wchar_t name[256] =L"pin";

	HRESULT hr;
	/*CFFmpegMuxInputPin *pPin*/ pvideoPin= new CFFmpegMuxInputPin(name, this, this, &hr);
	m_pInputs.AddTail(pvideoPin);
}

int CFFmpegMuxFilter::GetPinCount()
{
	return m_pInputs.GetCount();
}

CBasePin* CFFmpegMuxFilter::GetPin(int n)
{
	CAutoLock cAutoLock(this);

	if(n >= 0 && n < (int)m_pInputs.GetCount())
	{
		if(POSITION pos = m_pInputs.FindIndex(n))
			return m_pInputs.GetAt(pos);
	}

	return NULL;
}

STDMETHODIMP CFFmpegMuxFilter::Stop()
{
	CAutoLock cAutoLock(this);

	HRESULT hr;

	if(FAILED(hr = __super::Stop()))
		return hr;

	CallWorker(CMD_EXIT);

	return hr;
}

STDMETHODIMP CFFmpegMuxFilter::Pause()
{
	CAutoLock cAutoLock(this);

	FILTER_STATE fs = m_State;

	HRESULT hr;

	if(FAILED(hr = __super::Pause()))
		return hr;

	if(fs == State_Stopped )
	{
		CAMThread::Create();
		CallWorker(CMD_RUN);
	}

	return hr;
}

STDMETHODIMP CFFmpegMuxFilter::Run(REFERENCE_TIME tStart)
{
	CAutoLock cAutoLock(this);

	HRESULT hr;

	if(FAILED(hr = __super::Run(tStart)))
		return hr;

	return hr;
}

STDMETHODIMP CFFmpegMuxFilter::SetMuxConfig(MUX_CONFIG* pMuxConfig)
{
	return S_OK;
}
STDMETHODIMP CFFmpegMuxFilter::GetMuxConfig(MUX_CONFIG* pMuxConfig)
{
	return S_OK;
}

//IFileSinkFilter Implementation
HRESULT CFFmpegMuxFilter::SetFileName(LPCOLESTR inFileName, const AM_MEDIA_TYPE* inMediaType) 
{
	CAutoLock locLock(m_pLock);
	char temp[512];
	avcodec_register_all();
	av_register_all();

	AVOutputFormat *fmt;
	wcscpy(mFileName,inFileName);
	WideCharToMultiByte( CP_ACP, 0, mFileName, -1,temp, 512, NULL, NULL );
	opt_output_file(temp);
	return S_OK;
}
HRESULT CFFmpegMuxFilter::GetCurFile(LPOLESTR* outFileName, AM_MEDIA_TYPE* outMediaType)
{
	//Return the filename and mediatype of the raw data
	CheckPointer(outFileName, E_POINTER);
	*outFileName = NULL;

	if (mFileName != NULL) 
	{
		*outFileName = (LPOLESTR)
			QzTaskMemAlloc(sizeof(WCHAR) * (1+lstrlenW(mFileName)));

		if (*outFileName != NULL) 
		{
			lstrcpyW(*outFileName, mFileName);
		}
	}

	if(outMediaType) 
	{
		ZeroMemory(outMediaType, sizeof(*outMediaType));
		outMediaType->majortype = MEDIATYPE_NULL;
		outMediaType->subtype = MEDIASUBTYPE_NULL;
	}

	return S_OK;
}

AVStream * CFFmpegMuxFilter::GetVideoStream()
{
	return video_st;
}
AVStream * CFFmpegMuxFilter::GetAudioStream()
{
	return audio_st;
}
AVFormatContext * CFFmpegMuxFilter::GetContext()
{
	return m_oc;
}
AVOutputStream * CFFmpegMuxFilter::GetOutStream(int num)
{
	return ost_table[num];
}

AVCodec *output_codecs[100*10];
DWORD CFFmpegMuxFilter::ThreadProc()
{
	int ret = 0, i, j, k, n, nb_istreams = 0, nb_ostreams = 0;
	AVFormatContext *is, *os;
	AVCodecContext *codec, *icodec;
	ost =NULL;ost_table = NULL;
	char error[1024];
	int key;
	int want_sdp = 1;
	uint8_t no_packet[100]={0};
	int no_packet_count=0;

	nb_ostreams = 0;

	os = m_oc;
	if (!os->nb_streams) 
	{
		dump_format(os, 0, os->filename, 1);
	}
	nb_ostreams += os->nb_streams;

	ost_table = (AVOutputStream **)av_mallocz(sizeof(AVOutputStream *) * nb_ostreams);

	for(i=0;i<nb_ostreams;i++)
	{
		ost = (AVOutputStream*)av_mallocz(sizeof(AVOutputStream));
		ost_table[i] = ost;
	}
	
#define MAKE_SFMT_PAIR(a,b) ((a)+SAMPLE_FMT_NB*(b))
	n = 0;
    os = m_oc;
	for(i=0;i<nb_ostreams;i++,n++)
	{

		ost = ost_table[n];
		ost->file_index = 0;
		ost->index = i;
		ost->st = os->streams[i];

		codec = ost->st->codec;
		{
			switch(codec->codec_type) 
			{
			case AVMEDIA_TYPE_AUDIO:
				ost->fifo= av_fifo_alloc(1024);
				if(!ost->fifo)
					return  0;
				ost->reformat_pair = MAKE_SFMT_PAIR(SAMPLE_FMT_NONE,SAMPLE_FMT_NONE);
				ost->audio_resample =0;
				ost->encoding_needed = 1;
				ost->st->codec->time_base= _AVRational(1,pPin->audioformat.nSamplesPerSec);
				break;
			case AVMEDIA_TYPE_VIDEO:
				if (ost->st->codec->pix_fmt == PIX_FMT_NONE) 
				{
					fprintf(stderr, "Video pixel format is unknown, stream cannot be encoded\n");
				}
				//ost->st->codec->width = pPin->bih.biWidth;
				//ost->st->codec->height = pPin->bih.biHeight;
				ost->video_crop = 0;
				ost->video_pad =0;
				ost->video_resample = 0;
				ost->resample_height = 300 ;
				ost->resample_width  = 400 ;
				ost->resample_pix_fmt= PIX_FMT_YUV420P;
				ost->encoding_needed = 1;
				break;
			default:
				abort();
				break;
			}
		}
	}
    
	
	for(i=0;i<nb_ostreams;i++) 
	{
		ost = ost_table[i];
		if (ost->encoding_needed) 
		{
			AVCodec *codec = output_codecs[i];
			if (!codec)
				codec = avcodec_find_encoder(ost->st->codec->codec_id);
			if (!codec) 
			{
				ret = AVERROR(EINVAL);
			}
			if (avcodec_open(ost->st->codec, codec) < 0) 
			{
				ret = AVERROR(EINVAL);
			}
			extra_size += ost->st->codec->extradata_size;
		}
	}

	/* open files and write file headers */
	for(i=0;i<1;i++) 
	{
		os = m_oc;
		if (av_write_header(os) < 0) 
		{
			ret = AVERROR(EINVAL);
			goto dump_format;
		}
	}
dump_format:
	/* dump the file output parameters - cannot be done before in case
	of stream copy */
	for(i=0;i<1;i++) 
	{
		dump_format(os, i, os->filename, 1);
	}

	//////////////////////////
	CAtlList<CFFmpegMuxInputPin*> pActivePins;

	POSITION pos = m_pInputs.GetHeadPosition();
	while(pos)
	{
		CFFmpegMuxInputPin* pPin = m_pInputs.GetNext(pos);
		if(pPin->IsConnected()) pActivePins.AddTail(pPin);
	}

	BOOL bVideo = FALSE;

	CFFmpegMuxInputPin* pVideoPin = NULL;
	CFFmpegMuxInputPin* pAudioPin = NULL;
	REFERENCE_TIME rtMin = _I64_MAX;

	pos = pActivePins.GetHeadPosition();

	while(pos)	
	{
		CFFmpegMuxInputPin* pTmp = pActivePins.GetNext(pos);

		//CAutoLock cAutoLock(&pTmp->m_csQueue);
		if(pTmp->bih.biHeight != 0)
		{
			pVideoPin = pTmp;
		}
		else
		{
			pAudioPin = pTmp;
		}
	}

	SetThreadPriority(m_hThread, THREAD_PRIORITY_BELOW_NORMAL);


	while(1)
	{
		DWORD cmd = GetRequest();

		switch(cmd)
		{
		default:
		case CMD_EXIT:
			av_write_trailer(m_oc);
			
			CAMThread::m_hThread = NULL;
			Reply(S_OK);
			return 0;

		case CMD_RUN:
			Reply(S_OK);

			while(!CheckRequest(NULL))
			{
				if(m_State == State_Paused)
				{
					Sleep(0);
					continue;
				}

				//{
				//	CAutoLock cAutoLock(&pvideoPin->m_csQueue);
				//	CAutoLock cAutoLock1(&pPin->m_csQueue);
				//	//if(((pvideoPin->m_fEndOfStreamReceived /*|| pvideoPin->m_fActive == false*/ )&& pvideoPin->m_block.GetCount()==0) 
				//	//	&& ((pPin->m_fEndOfStreamReceived /*|| pPin->m_fActive == false*/)&&pPin->m_block.GetCount()==0))
				//	if(((pvideoPin->m_fEndOfStreamReceived ||pvideoPin->m_fActive == false)&& pvideoPin->m_block.GetCount()==0) 
				//		&& (pPin->m_fEndOfStreamReceived || pPin->m_fActive == false))
				//	{
				//		HRESULT hr = NotifyEvent(EC_COMPLETE, S_OK, NULL);
				//		av_write_trailer(m_oc);

				//	}
				//}



				//pos = pActivePins.GetHeadPosition();
		
				//while(pos)	
				//{
				//	CFFmpegMuxInputPin* pTmp = pActivePins.GetNext(pos);

				//	CAutoLock cAutoLock(&pTmp->m_csQueue);

				//	if(pTmp->m_block.GetCount() > 0)
				//	{
				//		pVideoPin = pTmp;
				//	}
				//}

				CAutoPtr<BLOCK> video;
				CAutoPtr<BLOCK> audio;
				{			
					CAutoLock cAudioAutoLock(&pAudioPin->m_csQueue);
					CAutoLock cVideoAutoLock(&pVideoPin->m_csQueue);
					if(pAudioPin->m_block.GetCount() > 0)
					{				
						//video = pVideoPin->m_block.RemoveHead();
						
						if(pVideoPin->m_block.GetCount() > 0)
						{
							video = pVideoPin->m_block.RemoveHead();
							audio = pAudioPin->m_block.RemoveHead();
						}
						else
						{
							audio = pAudioPin->m_block.RemoveHead();
						}
					}
					else
					{
						//Sleep(10);
						continue;
					}
				}

				//{
				//	CAutoLock cAudioAutoLock(&pAudioPin->m_csQueue);
				//	if(pAudioPin->m_block.GetCount() > 0)
				//	{				
				//		//audio = pAudioPin->m_block.RemoveHead();
				//	}
				//	else
				//	{
				//		//Sleep(1);
				//		continue;
				//	}
				//
				//}
				//video = pVideoPin->m_block.RemoveHead();
				//audio = pAudioPin->m_block.RemoveHead();
				//if(video&&audio)
				{
					if(audio)
					{
						//CAutoLock cVideoAutoLock(&pVideoPin->m_csQueue);
						int ret = av_interleaved_write_frame(m_oc, &audio->packet);
						//pVideoPin->m_block.AddHead(video);
					}
					if(video)
					{
						//CAutoLock cAudioAutoLock(&pAudioPin->m_csQueue);
						int ret1 = av_interleaved_write_frame(m_oc, &video->packet);
						//pAudioPin->m_block.AddHead(audio);
					}

				}
				//else
				//{
				//	Sleep(10);
				//	continue;
				//}
				//else
				//{
				//	if(video)
				//	{
				//		int ret1 = av_interleaved_write_frame(m_oc, &video->packet);
				//	}
				//	if(audio)
				//	{
				//		int ret = av_interleaved_write_frame(m_oc, &audio->packet);
				//	}
				//}
				
			
				//{
				//	{
				//		CAutoLock cVideoAutoLock(&pVideoPin->m_csQueue);
				//		CAutoLock cAudioAutoLock(&pAudioPin->m_csQueue);
				//		if(pVideoPin->m_block.GetCount() > 0 || pAudioPin->m_block.GetCount() > 0)
				//		{
				//			CAutoPtr<BLOCK> video,audio;
				//			video = pVideoPin->m_block.RemoveHead();
				//			audio = pAudioPin->m_block.RemoveHead();

				//			//if(&audio->packet.pts> &video->packet.pts)
				//			if(audio->start < video->start)
				//			{
				//				CAutoLock cVideoAutoLock(&pVideoPin->m_csQueue);
				//				int ret = av_interleaved_write_frame(m_oc, &audio->packet);
				//				pVideoPin->m_block.AddHead(video);
				//			}
				//			else
				//			{
				//				CAutoLock cAudioAutoLock(&pAudioPin->m_csQueue);
				//				int ret1 = av_interleaved_write_frame(m_oc, &video->packet);
				//				pAudioPin->m_block.AddHead(audio);
				//			}

				//		}
				//		else
				//		{
				//			continue;			
				//		}
				//	}								
				//}
			
	         }
		}
	}

	ASSERT(0); // we should only exit via CMD_EXIT
	
	CAMThread::m_hThread = NULL;
	return 0;
}

void CFFmpegMuxFilter::opt_output_file(const char *filename)
{
	AVFormatContext *oc;
	int err, use_video, use_audio, use_subtitle;
	int input_has_video, input_has_audio, input_has_subtitle;
	AVFormatParameters params, *ap = &params;
	AVOutputFormat *file_oformat;

	if (!strcmp(filename, "-"))
		filename = "pipe:";

	oc = avformat_alloc_context();
	if (!oc) {
	}


	file_oformat = av_guess_format(NULL, filename, NULL);
	if (!file_oformat) {
		fprintf(stderr, "Unable to find a suitable output format for '%s'\n",
			filename);
	}

	oc->oformat = file_oformat;
	av_strlcpy(oc->filename, filename, sizeof(oc->filename));

	if (!strcmp(file_oformat->name, "ffm") &&
		av_strstart(filename, "http:", NULL)) {
			/* special case for files sent to ffserver: we get the stream
			parameters from ffserver */
	} else {
		use_video = file_oformat->video_codec != CODEC_ID_NONE || video_stream_copy || video_codec_name;
		use_audio = file_oformat->audio_codec != CODEC_ID_NONE || audio_stream_copy || audio_codec_name;
		use_subtitle = file_oformat->subtitle_codec != CODEC_ID_NONE || subtitle_stream_copy || subtitle_codec_name;

		if (audio_disable) {
			use_audio = 0;
		}
		if (video_disable) {
			use_video = 0;
		}
		if (subtitle_disable) {
			use_subtitle = 0;
		}

		if (use_video) {
			new_video_stream(oc);
		}

		if (use_audio) {
			new_audio_stream(oc);
		}
		//oc->timestamp = rec_timestamp;

		//for(; metadata_count>0; metadata_count--){
		//	av_metadata_set2(&oc->metadata, metadata[metadata_count-1].key,
		//		metadata[metadata_count-1].value, 0);
		//}
		//av_metadata_conv(oc, oc->oformat->metadata_conv, NULL);
	}

	

	/* check filename in case of an image number is expected */
	if (oc->oformat->flags & AVFMT_NEEDNUMBER) {
		if (!av_filename_number_test(oc->filename)) {
		}
	}

	if (!(oc->oformat->flags & AVFMT_NOFILE)) 
	{
		/* open the file */
		if ((err = url_fopen(&oc->pb, filename, URL_WRONLY)) < 0) {
		}
	}

	memset(ap, 0, sizeof(*ap)); 
	ap->width = 720;
	ap->height = 396;
	ap->pix_fmt = PIX_FMT_YUV420P;
	ap->time_base.num = 12;
	ap->time_base.den = 1;
	//ap->video_codec_id = 
	if (av_set_parameters(oc, ap) < 0) {
		fprintf(stderr, "%s: Invalid encoding parameters\n",
			oc->filename);
		//av_exit(1);
	}

	oc->preload= (int)(mux_preload*AV_TIME_BASE);
	oc->max_delay= (int)(mux_max_delay*AV_TIME_BASE);
	oc->loop_output = loop_output;
	oc->flags |= AVFMT_FLAG_NONBLOCK;
	m_oc = oc;
	

	//set_context_opts(oc, avformat_opts, AV_OPT_FLAG_ENCODING_PARAM);
}

void CFFmpegMuxFilter::set_context_opts(void *ctx, void *opts_ctx, int flags)
{
	int i;
	for(i=0; i<opt_name_count; i++){
		char buf[256];
		const AVOption *opt;
		const char *str= av_get_string(opts_ctx, opt_names[i], &opt, buf, sizeof(buf));
		/* if an option with name opt_names[i] is present in opts_ctx then str is non-NULL */
		if(str && ((opt->flags & flags) == flags))
			av_set_string3(ctx, opt_names[i], str, 1, NULL);
	}
}

void CFFmpegMuxFilter::Init()
{
	OptionDef para[] = {
		/* video options */
		//{ "b", OPT_FUNC2 | HAS_ARG | OPT_VIDEO, {(void*)opt_bitrate}, "set bitrate (in bits/s)", "bitrate" },
		//{ "vb", OPT_FUNC2 | HAS_ARG | OPT_VIDEO, {(void*)opt_bitrate}, "set bitrate (in bits/s)", "bitrate" },
		//{ "r", OPT_FUNC2 | HAS_ARG | OPT_VIDEO, {(void*)opt_frame_rate}, "set frame rate (Hz value, fraction or abbreviation)", "rate" },
		//{ "s", HAS_ARG | OPT_VIDEO, {(void*)opt_frame_size}, "set frame size (WxH or abbreviation)", "size" },
		//{ "vcodec", HAS_ARG | OPT_VIDEO, {(void*)opt_video_codec}, "force video codec ('copy' to copy stream)", "codec" },
		///* audio options */
		//{ "ab", OPT_FUNC2 | HAS_ARG | OPT_AUDIO, {(void*)opt_bitrate}, "set bitrate (in bits/s)", "bitrate" },
		//{ "ar", HAS_ARG | OPT_FUNC2 | OPT_AUDIO, {(void*)opt_audio_rate}, "set audio sampling rate (in Hz)", "rate" },
		//{ "ac", HAS_ARG | OPT_FUNC2 | OPT_AUDIO, {(void*)opt_audio_channels}, "set number of audio channels", "channels" },
		//{ "acodec", HAS_ARG | OPT_AUDIO, {(void*)opt_audio_codec}, "force audio codec ('copy' to copy stream)", "codec" },

		///* muxer options */
		//{ "muxdelay", OPT_FLOAT | HAS_ARG | OPT_EXPERT, {(void*)&mux_max_delay}, "set the maximum demux-decode delay", "seconds" },
		//{ "muxpreload", OPT_FLOAT | HAS_ARG | OPT_EXPERT, {(void*)&mux_preload}, "set the initial demux-decode delay", "seconds" },

		//{ "default", OPT_FUNC2 | HAS_ARG | OPT_AUDIO | OPT_VIDEO | OPT_EXPERT, {(void*)opt_default}, "generic catch all option", "" },
		{ NULL, },
	};
	//options = para;
}
//DWORD CFFmpegMuxFilter::ThreadProc()
//{
//	return 0;
//}

int CFFmpegMuxFilter::opt_bitrate(const char *opt, const char *arg)
{
	//int codec_type = opt[0]=='a' ? AVMEDIA_TYPE_AUDIO : AVMEDIA_TYPE_VIDEO;

	//opt_default(opt, arg);

	//if (av_get_int(avcodec_opts[codec_type], "b", NULL) < 1000)
	//	fprintf(stderr, "WARNING: The bitrate parameter is set too low. It takes bits/s as argument, not kbits/s\n");

	return 0;
}
int CFFmpegMuxFilter::opt_frame_rate(const char *opt, const char *arg)
{
	//if (av_parse_video_rate(&frame_rate, arg) < 0) {
	//	fprintf(stderr, "Incorrect value for %s: %s\n", opt, arg);
	//}
	return 0;
}
void CFFmpegMuxFilter::opt_frame_size(const char *arg)
{
	//if (av_parse_video_size(&frame_width, &frame_height, arg) < 0) {
	//	fprintf(stderr, "Incorrect frame size\n");
	//}
}
void CFFmpegMuxFilter::opt_video_codec(const char *arg)
{
	opt_codec(&video_stream_copy, &video_codec_name, AVMEDIA_TYPE_VIDEO, arg);
}


int CFFmpegMuxFilter::opt_audio_rate(const char *opt, const char *arg)
{
	audio_sample_rate = parse_number_or_die(opt, arg, OPT_INT64, 0, INT_MAX);
	return 0;
}

double CFFmpegMuxFilter::parse_number_or_die(const char *context, const char *numstr, int type, double min, double max)
{
	char *tail;
	const char *error;
	double d = strtod(numstr, &tail);
	if (*tail)
		error= "Expected number for %s but found: %s\n";
	else if (d < min || d > max)
		error= "The value for %s was %s which is not within %f - %f\n";
	else if(type == OPT_INT64 && (int64_t)d != d)
		error= "Expected int64 for %s but found %s\n";
	else
		return d;
}
int CFFmpegMuxFilter::opt_audio_channels(const char *opt, const char *arg)
{
	audio_channels = parse_number_or_die(opt, arg, OPT_INT64, 0, INT_MAX);
	return 0;
}

void CFFmpegMuxFilter::opt_audio_codec(const char *arg)
{
	opt_codec(&audio_stream_copy, &audio_codec_name, AVMEDIA_TYPE_AUDIO, arg);
}

void CFFmpegMuxFilter::opt_codec(int *pstream_copy, char **pcodec_name,
					  int codec_type, const char *arg)
{
	av_freep(pcodec_name);
	if (!strcmp(arg, "copy")) {
		*pstream_copy = 1;
	} else {
		*pcodec_name = av_strdup(arg);
	}
}

int CFFmpegMuxFilter::opt_default(const char *opt, const char *arg)
{
	return 0;
}

void CFFmpegMuxFilter::new_video_stream(AVFormatContext *oc)
{
	AVStream *st;
	AVCodecContext *video_enc;
	enum CodecID codec_id;

	st = av_new_stream(oc, oc->nb_streams);
	if (!st) 
	{
		fprintf(stderr, "Could not alloc stream\n");
	}
	avcodec_get_context_defaults2(st->codec, AVMEDIA_TYPE_VIDEO);


	avcodec_thread_init(st->codec, thread_count);

	video_enc = st->codec;


	if(   (video_global_header&1)
		|| (video_global_header==0 && (oc->oformat->flags & AVFMT_GLOBALHEADER)))
	{
			video_enc->flags |= CODEC_FLAG_GLOBAL_HEADER;
			//avcodec_opts[AVMEDIA_TYPE_VIDEO]->flags|= CODEC_FLAG_GLOBAL_HEADER;
	}

	if (video_stream_copy) 
	{
		st->stream_copy = 1;
		video_enc->codec_type = AVMEDIA_TYPE_VIDEO;
		video_enc->sample_aspect_ratio =
			st->sample_aspect_ratio = av_d2q(frame_aspect_ratio*frame_height/frame_width, 255);
	} 
	else 
	{
		const char *p;
		int i;
		AVCodec *codec;
		frame_rate.num = 12;
		frame_rate.den = 1;
		AVRational fps= frame_rate.num ? frame_rate : _AVRational(25,1);
		{
			codec_id = av_guess_codec(oc->oformat, NULL, oc->filename, NULL, AVMEDIA_TYPE_VIDEO);
			codec = avcodec_find_encoder(codec_id);
		}

		video_enc->codec_id = codec_id;

		//set_context_opts(video_enc, avcodec_opts[AVMEDIA_TYPE_VIDEO], AV_OPT_FLAG_VIDEO_PARAM | AV_OPT_FLAG_ENCODING_PARAM);

		if (codec && codec->supported_framerates && !force_fps)
			fps = codec->supported_framerates[av_find_nearest_q_idx(fps, codec->supported_framerates)];
		video_enc->time_base.den = fps.num;
		video_enc->time_base.num = fps.den;

		video_enc->width = 720/*frame_width + frame_padright + frame_padleft*/;
		video_enc->height = 396/*frame_height + frame_padtop + frame_padbottom*/;
		video_enc->sample_aspect_ratio = av_d2q(frame_aspect_ratio*video_enc->height/video_enc->width, 255);
		video_enc->pix_fmt = frame_pix_fmt;
		st->sample_aspect_ratio = video_enc->sample_aspect_ratio;
		video_enc->gop_size = 30;

		choose_pixel_fmt(st, codec);

		p= video_rc_override_string;
		for(i=0; p; i++)
		{
			int start, end, q;
			int e=sscanf(p, "%d,%d,%d", &start, &end, &q);
			if(e!=3)
			{
				fprintf(stderr, "error parsing rc_override\n");
			}
			video_enc->rc_override=
				(RcOverride*)av_realloc(video_enc->rc_override,
				sizeof(RcOverride)*(i+1));
			video_enc->rc_override[i].start_frame= start;
			video_enc->rc_override[i].end_frame  = end;
			if(q>0)
			{
				video_enc->rc_override[i].qscale= q;
				video_enc->rc_override[i].quality_factor= 1.0;
			}
			else
			{
				video_enc->rc_override[i].qscale= 0;
				video_enc->rc_override[i].quality_factor= -q/100.0;
			}
			p= strchr(p, '/');
			if(p) p++;
		}
		video_enc->rc_override_count=i;
		if (!video_enc->rc_initial_buffer_occupancy)
			video_enc->rc_initial_buffer_occupancy = video_enc->rc_buffer_size*3/4;
		video_enc->me_threshold= me_threshold;
		video_enc->intra_dc_precision= intra_dc_precision - 8;
	}
	nb_ocodecs++;

	/* reset some key parameters */
	video_disable = 0;
	av_freep(&video_codec_name);
	video_stream_copy = 0;
	frame_pix_fmt = PIX_FMT_NONE;
}

void CFFmpegMuxFilter::new_audio_stream(AVFormatContext *oc)
{
	AVStream *st;
	AVCodecContext *audio_enc;
	enum CodecID codec_id;

	st = av_new_stream(oc, oc->nb_streams);
	if (!st) 
	{
		fprintf(stderr, "Could not alloc stream\n");
	}
	avcodec_get_context_defaults2(st->codec, AVMEDIA_TYPE_AUDIO);

	//bitstream_filters[nb_output_files][oc->nb_streams - 1]= audio_bitstream_filters;
	//audio_bitstream_filters= NULL;

	avcodec_thread_init(st->codec, thread_count);

	audio_enc = st->codec;
	audio_enc->codec_type = AVMEDIA_TYPE_AUDIO;

	if (oc->oformat->flags & AVFMT_GLOBALHEADER) 
	{
		audio_enc->flags |= CODEC_FLAG_GLOBAL_HEADER;
		//avcodec_opts[AVMEDIA_TYPE_AUDIO]->flags|= CODEC_FLAG_GLOBAL_HEADER;
	}
	{
		AVCodec *codec;
		{
			codec_id = av_guess_codec(oc->oformat, NULL, oc->filename, NULL, AVMEDIA_TYPE_AUDIO);
			codec = avcodec_find_encoder(codec_id);
		}
		audio_enc->codec_id = CODEC_ID_AAC/*codec_id*/;

		audio_enc->channels = audio_channels;
		audio_enc->sample_fmt = audio_sample_fmt;
		audio_enc->sample_rate = audio_sample_rate;
		audio_enc->channel_layout = channel_layout;
		//if (avcodec_channel_layout_num_channels(channel_layout) != audio_channels)
		//	audio_enc->channel_layout = 0;
		choose_sample_fmt(st, codec);
		choose_sample_rate(st, codec);
	}
	nb_ocodecs++;
	audio_enc->time_base= _AVRational(1, audio_sample_rate);

	/* reset some key parameters */
	audio_disable = 0;
	av_freep(&audio_codec_name);
	audio_stream_copy = 0;
}

void CFFmpegMuxFilter::choose_sample_fmt(AVStream *st, AVCodec *codec)
{
	if(codec && codec->sample_fmts)
	{
		const enum SampleFormat *p= codec->sample_fmts;
		for(; *p!=-1; p++)
		{
			if(*p == st->codec->sample_fmt)
				break;
		}
		if(*p == -1)
			st->codec->sample_fmt = codec->sample_fmts[0];
	}
}

void CFFmpegMuxFilter::choose_sample_rate(AVStream *st, AVCodec *codec)
{
	if(codec && codec->supported_samplerates)
	{
		const int *p= codec->supported_samplerates;
		int best;
		int best_dist=INT_MAX;
		for(; *p; p++)
		{
			int dist= abs(st->codec->sample_rate - *p);
			if(dist < best_dist)
			{
				best_dist= dist;
				best= *p;
			}
		}
		if(best_dist)
		{
			av_log(st->codec, AV_LOG_WARNING, "Requested sampling rate unsupported using closest supported (%d)\n", best);
		}
		st->codec->sample_rate= best;
	}
}

enum CodecID CFFmpegMuxFilter::find_codec_or_die(const char *name, int type, int encoder, int s)
{
	const char *codec_string = encoder ? "encoder" : "decoder";
	AVCodec *codec;

	if(!name)
		return CODEC_ID_NONE;
	codec = encoder ?
		avcodec_find_encoder_by_name(name) :
	avcodec_find_decoder_by_name(name);
	if(!codec) {
		fprintf(stderr, "Unknown %s '%s'\n", codec_string, name);
	}
	if(codec->type != type) {
		fprintf(stderr, "Invalid %s type '%s'\n", codec_string, name);
	}
	if(codec->capabilities & CODEC_CAP_EXPERIMENTAL &&
		s > FF_COMPLIANCE_EXPERIMENTAL) {
			fprintf(stderr, "%s '%s' is experimental and might produce bad "
				"results.\nAdd '-strict experimental' if you want to use it.\n",
				codec_string, codec->name);
			codec = encoder ?
				avcodec_find_encoder(codec->id) :
			avcodec_find_decoder(codec->id);
			if (!(codec->capabilities & CODEC_CAP_EXPERIMENTAL))
				fprintf(stderr, "Or use the non experimental %s '%s'.\n",
				codec_string, codec->name);
	}
	return codec->id;
}

void CFFmpegMuxFilter::choose_pixel_fmt(AVStream *st, AVCodec *codec)
{
	if(codec && codec->pix_fmts){
		const enum PixelFormat *p= codec->pix_fmts;
		for(; *p!=-1; p++){
			if(*p == st->codec->pix_fmt)
				break;
		}
		if(*p == -1
			&& !(   st->codec->codec_id==CODEC_ID_MJPEG
			&& st->codec->strict_std_compliance <= -1
			&& (   st->codec->pix_fmt == PIX_FMT_YUV420P
			|| st->codec->pix_fmt == PIX_FMT_YUV422P)))
			st->codec->pix_fmt = codec->pix_fmts[0];
	}
}




