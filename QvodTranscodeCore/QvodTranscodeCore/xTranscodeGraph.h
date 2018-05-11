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
		int						nAudioStreamID;	//����Ƶ������µ���Ƶ�����
		bool					bNeedSubTitle; 
		long					dstWidth;
		long					dstHeight;
		long					AudioVolume;
		FString					subpath;
		FString					srcurl;
		FString					dstcurl;
		Qvod_DetailMediaInfo	media_info;		//source file media info
		
		bool					bAccel;			//Ӳ������
		E_V_CODEC				dstVideo;		//��������
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
	bool					BuildingWithAccelSolution();	//Ӳ��+Ӳ��
	bool					BuildingWithBestGraph();		//���+��ࡢӲ��
	bool					BuildingWithCheckBytes();		//���+��ࡢӲ��

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
	bool					mHasVideo;		// ��Ƶ����־
	bool					mHasAudio;		// ��Ƶ����־
	bool					mHasSubTitle;	// ��Ļ����־
	int						mVCount;		// ��Ƶ������
	int						mACount;		// ��Ƶ������
	int						mTCount;		// ��Ļ������

	// Filters
	CComPtr<IBaseFilter>			mpFilterFileReader;		// SYS FileReader/XLMV Reader/...
	CComPtr<IBaseFilter>			mpFilterSource;			// ���ز��ŵ�Source
	CComPtr<IBaseFilter>			mpFilterSplitter;		// ���ء����粥�ŵ�Splitter
	CComPtr<IBaseFilter>			mpFilterVideoDecoder;	// ѡ�õ�Video Decoder Filter
	std::vector<CComPtr<IBaseFilter>>	 mpFilterAudioDecoders;// Audio Decoder Filters
	//CComPtr<IBaseFilter>			mpFilterAudioDecoder;	// ѡ�õ�Audio Decoder Filter
	CComPtr<IBaseFilter>			mpFilterVobSubTitle;	// ��Ļ������
	CComPtr<IBaseFilter>			mpFilterQvodSubTitle;	// QVOD SubTitle
	CComPtr<IBaseFilter>			mpFilterQvodPostVideo;	// QVOD PostVideo
	CComPtr<IBaseFilter>			mpFilterQvodSound;		// QVOD Sound filter
	CComPtr<IBaseFilter>			mpFilterAudioSwitch;	// ��Ƶ���л���
	CComPtr<IBaseFilter>			mpFilterColorSpaceTrans_1;// Color Space trans 1 ��VIDEO Decoder -> QvodSubTitle/QvodPostVideo �䣩
	CComPtr<IBaseFilter>			mpFilterNullRender;
//	CComPtr<IBaseFilter>			mpFilterColorSpaceTrans_2;// Color Space trans 2 ��POST VIDEO->Render�䣩

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
