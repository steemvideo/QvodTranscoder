#pragma once

#include "xMainGraph.h"
#include "../../inc/ISub.h"
#include "../../inc/ISound.h"
#include "../../inc/IvideoEffect.h"

#include "Reg.h"

enum E_GraphType
{
	e_GraphPlay,
	e_GraphTransCode,
	e_GraphPreview
};
class xTranscodeGraph : public xMainGraph
{
public:
	struct build_graph_parameter
	{
		build_graph_parameter& operator = (const build_graph_parameter& c)
		{
			bNeedVideo = c.bNeedVideo;
			bNeedAudio = c.bNeedAudio;
			nAudioStreamID = c.nAudioStreamID;	
			bNeedSubTitle = c.bNeedSubTitle;
			dstWidth = c.dstWidth;
			dstHeight = c.dstHeight;
			AudioVolume = c.AudioVolume;
			subpath = c.subpath;
			srcurl = c.srcurl;
			dstcurl = c.dstcurl;
			media_info = c.media_info;
			bAccel = c.bAccel;
			dstVideo = c.dstVideo;
			return *this;
		}
		bool					bNeedVideo;		
		bool					bNeedAudio;
		int						nAudioStreamID;	//多音频流情况下的音频流序号
		bool					bNeedSubTitle; 
		long					dstWidth;
		long					dstHeight;
		long					AudioVolume;
		FString					subpath;
		FString					srcurl;
		FString					dstcurl;
		Qvod_DetailMediaInfo	media_info;		//source file media info
		
		bool					bAccel;			//硬件加速
		E_V_CODEC				dstVideo;		//编码类型
	};
public:
	xTranscodeGraph();
	~xTranscodeGraph(void);

public:
	virtual CComPtr<IGraphBuilder>	BuildGraph(build_graph_parameter& param, E_GraphType graphtype = e_GraphTransCode);
	virtual void					DestroyGraph();
	CComPtr<IPostVideoEffectEx>		GetIPostVideoEx(){return mpIPostVideoEx;}
	HRESULT							SetMuxConfigure(MUX_CONFIG_EX& config,float srcFrameRate);
	bool							IsAccelTrans();
protected:
	bool					BuildingWithAccelSolution();	//硬解+硬编
	bool					BuildingWithBestGraph();		//软解+软编、硬编
	bool					BuildingWithCheckBytes();		//软解+软编、硬编

	CComPtr<IBaseFilter>	Load_ReaderFilter(const CLSID& clsid = CLSID_NULL);
	CComPtr<IBaseFilter>	Load_SourceFilter(const CLSID& clsid);
	CComPtr<IBaseFilter>	Load_SplitterFilter(const CLSID& clsid);
	CComPtr<IBaseFilter>	Load_VideoDecoderByCLSID(const CLSID& clsid);
	CComPtr<IBaseFilter>	Load_AudioDecoderByCLSID(const CLSID& clsid);
	CComPtr<IBaseFilter>	Load_VideoDecoderAuto();
	CComPtr<IBaseFilter>	Load_VideoH265EncoderAuto();
	HRESULT	Load_AudioDecoderAuto();

	// for test only
	CComPtr<IBaseFilter>	Load_AudioEncoderAuto();
	CComPtr<IBaseFilter>	Load_VideoEncodeAuto();
	CComPtr<IBaseFilter>	Load_FFmpegMux();
	//CComPtr<IBaseFilter>	Load_FileWriter();

	CComPtr<IPin>			GetUnconnectedPin(CComPtr<IBaseFilter>& pFilter,PIN_DIRECTION pinDir);

	void					CheckChannelInfo();
	CComPtr<IBaseFilter>	GetPreFilterOnPath(e_FilterType t);
	bool					GetFilterMapByCheckByte(QVOD_FILTER_MAP& filters);
	bool					ReadCheckBytes(BYTE* buffer, int size_to_read);
	void					ResetFlags();
	void					ResetFilters();
	HRESULT					Load_ColorSpaceTrans1();
	bool					IsNeedAudioDecoder();
	bool					IsNeedVideoDecoder();
	bool					IsNeedSubTitle();
	bool					IsNeedVobSub();
	bool					IsNeedPostVideo();
	bool					IsNeedQvodSound();
	bool					can_accel();
	bool					can_h265Encoder();
	HRESULT					Load_AudioSwitch();
	HRESULT					Load_Sound();
	HRESULT					Load_SubTitle();
	HRESULT					Load_VobSub();
	bool					ConnectSubTitilePins(CComQIPtr<IBaseFilter>& pSrc, CComQIPtr<IBaseFilter>& pVobSubFilter);
	HRESULT					Load_PostVideo();
	HRESULT					Load_NULL_Render();
	HRESULT					Load_Video_Render();
	HRESULT					Load_Aideo_Render();
	HRESULT					load_AccelEncoder();

	bool					ParserMediaInfo(Qvod_DetailMediaInfo* info);
	bool					IntelDXVASupport();
protected:
	build_graph_parameter			mParam;
	E_CONTAINER						mConType;
	E_V_CODEC						mVideoType;
	E_A_CODEC						mAudioType;
	E_GraphType						mGraphType;

	LONG					mVideoWidth;
	LONG					mVideoHeight;
	LONG					mdwAspectRatio_X;
	LONG					mdwAspectRatio_Y;
	bool					mHasVideo;		// 视频流标志
	bool					mHasAudio;		// 音频流标志
	bool					mHasSubTitle;	// 字幕流标志
	int						mVCount;		// 视频流数量
	int						mACount;		// 音频流数量
	int						mTCount;		// 字幕流数量

	// Filters
	CComPtr<IBaseFilter>			mpFilterFileReader;		// SYS FileReader/XLMV Reader/...
	CComPtr<IBaseFilter>			mpFilterSource;			// 本地播放的Source
	CComPtr<IBaseFilter>			mpFilterSplitter;		// 本地、网络播放的Splitter
	CComPtr<IBaseFilter>			mpFilterVideoDecoder;	// 选用的Video Decoder Filter
	std::vector<CComPtr<IBaseFilter>>	 mpFilterAudioDecoders;// Audio Decoder Filters
	//CComPtr<IBaseFilter>			mpFilterAudioDecoder;	// 选用的Audio Decoder Filter
	CComPtr<IBaseFilter>			mpFilterVobSubTitle;	// 字幕解析器
	CComPtr<IBaseFilter>			mpFilterQvodSubTitle;	// QVOD SubTitle
	CComPtr<IBaseFilter>			mpFilterQvodPostVideo;	// QVOD PostVideo
	CComPtr<IBaseFilter>			mpFilterQvodSound;		// QVOD Sound filter
	CComPtr<IBaseFilter>			mpFilterAudioSwitch;	// 音频流切换器
	CComPtr<IBaseFilter>			mpFilterColorSpaceTrans_1;// Color Space trans 1 （VIDEO Decoder -> QvodSubTitle/QvodPostVideo 间）
	CComPtr<IBaseFilter>			mpFilterNullRender;
//	CComPtr<IBaseFilter>			mpFilterColorSpaceTrans_2;// Color Space trans 2 （POST VIDEO->Render间）

	CComPtr<IPin>					mCurVideoOutputPin;
	CComPtr<ISUBTITLE>				mpISubTrans;
	CComPtr<IPostVideoEffectEx>		mpIPostVideoEx;
	CComPtr<ISoundtrans>			mpISoundTrans;


	// for test
	CComPtr<IBaseFilter>			mpFilterAudioEncoder;
	CComPtr<IBaseFilter>			mpFilterVideoEncoder;
	CComPtr<IBaseFilter>			mpFilterMux;

	CComPtr<IBaseFilter>			mpFilterWriter;
};
