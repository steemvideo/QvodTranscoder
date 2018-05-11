#include "stdafx.h"
#include "xTranscodeGraph.h"
#include "xUtility.h"
#include "commonfunction.h"
#include <fstream>
#include <amvideo.h>
#include <dvdmedia.h>
#include "clsid.h"
#include "../inc/IDirectVobSub.h"
#include "../inc/IntelSupportDxva/mfxvideo.h"
#include "../../inc/mfx_filter_externals.h"

#define INTEL_VERSION

extern void	CStringtoBin(CString str, BYTE *pdata);

#define BEGIN_FOR_EACH_FILTER_BY_CAPACITY(VECT,CAPACITY,NODE) \
	for (int i = 0; i < (int)VECT.size(); i++) \
	{ \
	NODE= VECT[i];\
	DWORD C = QvodFilterManager::GetInstance().GetFilterCapacity(NODE) << 1;\
	DWORD T = 1; \
	bool conti = false;\
	while((T = T << 1) <= CAPACITY){if (C & T) { conti=true; break;} };\
	if (conti || !(CAPACITY&(C>>1))) {continue;}



#define END_FOR_EACH }


// 排序仿函数
struct PRIORITY_SORT
{
	bool operator()(const CLSID& filter1, const CLSID& filter2) const
	{
		QVOD_FILTER_INFO node1, node2;
		DWORD p1 = QvodFilterManager::GetInstance().GetFilterPriority(filter1, mt);
		DWORD p2 = QvodFilterManager::GetInstance().GetFilterPriority(filter2, mt);
		return p1 > p2;
	}
	static QVOD_MEDIA_TYPE mt;
};
QVOD_MEDIA_TYPE PRIORITY_SORT::mt;

// 排序函数
void SortFilterByPriority(std::vector<CLSID>& filters, QVOD_MEDIA_TYPE& mt)
{
	PRIORITY_SORT::mt = mt;
	std::sort(filters.begin(), filters.end(), PRIORITY_SORT());
}

// 检查CheckByte是否出现在pDatas中
// 返回
bool CheckCB(const TCHAR * pchkbytes, const BYTE* pDatas, int dataSize, int& needMoreData)
{
	if (!pchkbytes || !pDatas)
	{
		return false;
	}

	needMoreData = 0;

	std::vector<CString> vcheckbytes;
	int nlen = _tcslen(pchkbytes);
	const TCHAR *pc = pchkbytes;
	CString strtemp;
	while(nlen-- > 0)
	{
		if(*pc == ',')
		{
			if(strtemp.CompareNoCase(L",") == 0)
			{
				strtemp.Empty();
			}
			vcheckbytes.push_back(strtemp);
			strtemp.Empty();
		}
		else
		{
			strtemp += (*pc);
		}
		pc++;
	}
	vcheckbytes.push_back(strtemp);
	if(vcheckbytes.size() < 4)
	{	
		return false;
	}
	for (int i = 0; i < vcheckbytes.size(); i += 4)
	{
		CString offsetstr = vcheckbytes[i];
		CString cbstr = vcheckbytes[i+1];
		CString maskstr = vcheckbytes[i+2];
		CString valstr = vcheckbytes[i+3];
		long cb = _ttol(cbstr);

		if(offsetstr.IsEmpty() || cbstr.IsEmpty() 
			|| valstr.IsEmpty() || (valstr.GetLength() & 1)
			|| cb*2 != valstr.GetLength())
			return false;

		int pos = int(pDatas) + (int)_ttoi64(offsetstr);
		if (pos >  (int)pDatas + dataSize - valstr.GetLength()/2)
		{
			// 数据不够，要求更多数据
			needMoreData = (int)_ttoi64(offsetstr) + 64;
			return false;
		}

		// LAME
		while(maskstr.GetLength() < valstr.GetLength())
			maskstr += 'F';
		valstr.TrimLeft();
		valstr.TrimRight();
		BYTE *pmask = new BYTE[maskstr.GetLength()/2];
		BYTE *pval  = new BYTE[valstr.GetLength()/2];
		int nvallen = valstr.GetLength()/2;
		CStringtoBin(maskstr, pmask);
		CStringtoBin(valstr, pval);

		for(size_t i = 0; i < nvallen; i++, pos++)
		{
			BYTE b = *((BYTE*)pos);
			if( (b & pmask[i]) != pval[i])
			{
				SafeDeleteArray(pmask);
				SafeDeleteArray(pval);
				return false;
			}
		}
		SafeDeleteArray(pmask);
		SafeDeleteArray(pval);
	}
	return true;
}

const GUID MEDIATYPE_OggPacketStream ={0x60891713, 0xc24f, 0x4767, { 0xb6, 0xc9, 0x6c, 0xa0, 0x5b, 0x33, 0x38, 0xfc}};
const GUID MEDIATYPE_Subtitle  =   {0xe487eb08, 0x6b26, 0x4be9, {0x9d, 0xd3, 0x99, 0x34, 0x34, 0xd3, 0x13, 0xfd}};
/************************************************************************/
/* xTranscodeGraph                                                           */
/************************************************************************/
xTranscodeGraph::xTranscodeGraph()
: mHasVideo(false)		// 视频流标志
, mHasAudio(false)		// 音频流标志
, mHasSubTitle(false)	// 字幕流标志
, mVCount(0)		// 视频流数量
, mACount(0)		// 音频流数量
, mTCount(0)				// 字幕流数量
, mConType(_E_CON_OTHER)
, mVideoType(_V_OTHER)
, mAudioType(_A_OTHER)
, mVideoWidth(0)
, mVideoHeight(0)
, mdwAspectRatio_X(0)
, mdwAspectRatio_Y(0)
, mGraphType(e_GraphTransCode)
{
	
}

xTranscodeGraph::~xTranscodeGraph(void)
{
}

CComPtr<IGraphBuilder> xTranscodeGraph::BuildGraph(build_graph_parameter& param, E_GraphType graphtype)
{
	mGraphType = graphtype;
	mParam = param;
	mParam.bNeedAudio = true;
	
	if (!ParserMediaInfo(&mParam.media_info))
	{
		Trace(-2, L"ParserMediaInfo Failed\n");
		return NULL;
	}
	if(e_GraphPreview == graphtype && !mHasVideo)
	{
		return NULL;
	}

	if (!mpGraph)
	{
		mpGraph = LoadFilterGraph();
	}

	// 硬件加速?
#ifdef INTEL_VERSION
	if (can_accel() && BuildingWithAccelSolution())
	{
		goto _OK;
	}
#endif
	
	// 释放
	ResetFilters();

	// 最优链路?
	if (BuildingWithBestGraph())
	{
		Trace(1, L"BuildingWithBestGraph success\n");
		goto _OK;
	}
	else
	{
		Trace(1, L"BuildingWithBestGraph failed\n");
	}

	// 释放
	ResetFilters();
	
	// CheckByte建链路
	if (BuildingWithCheckBytes())
	{
		Trace(1, L"BuildingWithCheckBytes success\n");
		goto _OK;
	}
	else
	{
		//MessageBox(NULL, NULL, NULL, MB_TOPMOST);
		Trace(-2, L"BuildingWithCheckBytes failed\n");
	}

	// 释放
	//ResetFilters();
	DestroyGraph();

	// 尝试QVOD FFmpeg Source
// 	if (BuildingWithQvodFFmpegFilter())
// 	{
// 		goto _OK;
// 	}
	Trace(-2, L"Building Graph failed\n");
	return NULL;
_OK:
	return mpGraph;
}

void xTranscodeGraph::DestroyGraph()
{
	Trace(2, L"before xTranscodeGraph::DestroyGraph\n");
	mpFilterSource.Release();
	mpFilterSplitter.Release();
	mpFilterNullRender.Release();
	mpISubTrans.Release();
	mpIPostVideoEx.Release();
	mpISoundTrans.Release();
	mIFFmpegMux.Release();
	//mIQVODH265Encode.Release();
	

	mpFilterVobSubTitle.Release();
	mpFilterQvodSubTitle.Release();
	mpFilterQvodPostVideo.Release();
	mpFilterQvodSound.Release();
	mpFilterColorSpaceTrans_1.Release();
	mpFilterAudioSwitch.Release();
	mCurVideoOutputPin.Release();
	mpFilterSource.Release();
	/*for(int i = 0; i < mpFilterAudioDecoders.size();i++)
	mpFilterAudioDecoders[i].Release();*/
	mpFilterAudioDecoders.clear();
	mpFilterVideoDecoder.Release();
	mpFilterAudioEncoder.Release();
	mpFilterVideoEncoder.Release();
	mpFilterMux.Release();
	mpFilterWriter.Release();
	xMainGraph::DestroyGraph();
	Trace(2, L"after xTranscodeGraph::DestroyGraph\n");
}

void xTranscodeGraph::ResetFilters()
{		
	// 接口释放
	mpISubTrans.Release();
	mpIPostVideoEx.Release();
	mpISoundTrans.Release();
	mIFFmpegMux.Release();
	//mIQVODH265Encode.Release();
	mCurVideoOutputPin.Release();
	mpVMRWindowless.Release();

	ReleaseFilter(mpFilterSource);
	ReleaseFilter(mpFilterSplitter);
	ReleaseFilter(mpFilterNullRender);
	ReleaseFilter(mpFilterVobSubTitle);
	ReleaseFilter(mpFilterQvodSubTitle);
	ReleaseFilter(mpFilterQvodPostVideo);
	ReleaseFilter(mpFilterQvodSound);
	ReleaseFilter(mpFilterColorSpaceTrans_1);
	ReleaseFilter(mpFilterAudioSwitch);
	ReleaseFilter(mpFilterSource);
	for(int i = 0; i < mpFilterAudioDecoders.size();i++)
	ReleaseFilter(mpFilterAudioDecoders[i]);
	mpFilterAudioDecoders.clear();
	ReleaseFilter(mpFilterVideoDecoder);
	ReleaseFilter(mpFilterAudioEncoder);
	ReleaseFilter(mpFilterVideoEncoder);
	ReleaseFilter(mpFilterMux);
	ReleaseFilter(mpFilterWriter);
}

void xTranscodeGraph::ResetFlags()
{
	// Reset flags
	mVCount = 0;
	mACount = 0;
	mTCount = 0;
	mHasAudio = false;
	mHasVideo = false;
	mHasSubTitle = false;
}

bool xTranscodeGraph::IsNeedAudioDecoder()
{
	if (!mParam.bNeedAudio)
	{
		return false;
	}

	if (!mHasAudio)
	{
		return false;
	}
	if (mConType == _E_CON_BIK)
	{
		return false;
	}

	CLSID clsid = CLSID_NULL;
	CComPtr<IBaseFilter> filter = mpFilterSplitter ? mpFilterSplitter : mpFilterSource;
	if (filter)
	{
		filter->GetClassID(&clsid);
		return !(bool)(QvodFilterManager::GetInstance().GetFilterCapacity(clsid) & E_FILTER_CAPACITY_AUDIO_DEC);
	}
	return false;
}

bool xTranscodeGraph::IsNeedVideoDecoder()
{
	if (!mParam.bNeedVideo)
	{
		return false;
	}

	if (!mHasVideo)
	{
		return false;
	}

	if (mConType == _E_CON_BIK)
	{
		return false;
	}
	CLSID clsid = CLSID_NULL;
	CComPtr<IBaseFilter> filter = mpFilterSplitter ? mpFilterSplitter : mpFilterSource;
	if (filter)
	{
		filter->GetClassID(&clsid);
		return !(bool)(QvodFilterManager::GetInstance().GetFilterCapacity(clsid) & E_FILTER_CAPACITY_VIDEO_DEC);
	}
	return false;
}

bool xTranscodeGraph::IsNeedSubTitle()
{
	//if(mParam.bNeedSubTitle&& mHasVideo)
	//{
	//	FString str = mParam.subpath;
	//	transform(str.begin(), str.end(), str.begin(), tolower);
	//	if(wcsstr(str.c_str(), L".idx") || wcsstr(str.c_str(), L".sub") )
	//	{
	//		return false;
	//	}
	//	return true;
	//}
	
	
	return false; 
}

bool xTranscodeGraph::IsNeedVobSub()
{
	if(mParam.bNeedSubTitle&& mHasVideo)
	{
		FString str = mParam.subpath;
		transform(str.begin(), str.end(), str.begin(), tolower);
		if(wcsstr(str.c_str(), L".idx") || wcsstr(str.c_str(), L".sub") )
		{
			return true;
		}
		
	}

	if(mParam.media_info.nSubtitleCount >= 1)
	{
		return true;
	}
	return false;
}

bool xTranscodeGraph::IsNeedPostVideo()
{
//	return mHasVideo&&mParam.bNeedVideo;
	return false;
}

bool xTranscodeGraph::IsNeedQvodSound()
{
	return true;
}

HRESULT xTranscodeGraph::Load_ColorSpaceTrans1()
{
	X_ASSERT(!mpFilterColorSpaceTrans_1);
	if (mpFilterColorSpaceTrans_1)
	{
		return false;
	}
	QVOD_FILTER_INFO info;
	CComQIPtr<IBaseFilter> pBaseFilter = LoadFilter(CLSID_ElcardCSC, info);
	if (!pBaseFilter)
	{
		pBaseFilter = LoadFilter(CLSID_ColorSpaceConvert, info);
	}

	if (!pBaseFilter)
	{
		return false;
	}

	CComQIPtr<IBaseFilter> pSrc = GetPreFilterOnPath(_EFT_V_TRANS);
	if(FAILED(ConnectFilterDirect(pSrc, pBaseFilter)))
	{
		Trace(-1, L"Connect to color space converter 1 failed!\n");
		ReleaseFilter(pBaseFilter);
		//SafeRelease(pBaseFilter);
		pBaseFilter.Release();
		return false;
	}

	mpFilterColorSpaceTrans_1 = pBaseFilter;
	return true;
}
HRESULT	xTranscodeGraph::Load_AudioSwitch()
{
	if(mpFilterAudioDecoders.size() > 1)
	{
		QVOD_FILTER_INFO info;
		mpFilterAudioSwitch = LoadFilter(CLSID_AudioSwtich,info);
		if (mpFilterAudioSwitch)
		{
			for (int i = 0; i < mpFilterAudioDecoders.size(); i++)
			{
				if(FAILED(ConnectFilterDirect(mpFilterAudioDecoders[i], mpFilterAudioSwitch)))
				{
					ReleaseFilter(mpFilterAudioSwitch);
					mpFilterAudioSwitch.Release();
					return E_FAIL;
				}
				//DeleteMediaType(&type);
			}
		}
	}
	return S_OK;
}

HRESULT	xTranscodeGraph::Load_Sound()
{
	if (IsNeedQvodSound())
	{
		QVOD_FILTER_INFO info;
		CComQIPtr<IBaseFilter> filter = LoadFilter(CLSID_QvodSound, info);
		CComQIPtr<IBaseFilter> pSrc = GetPreFilterOnPath(_EFT_A_TRANS);
		X_ASSERT(pSrc);
		if (filter && pSrc)
		{
			if (FAILED(ConnectFilterDirect(pSrc, filter)))
			{
				Trace(-1, L"Connect Qvod sound filter failed!");
				ReleaseFilter(filter);
				filter.Release();
			}
			else
			{
				mpFilterQvodSound = filter;
				mpFilterQvodSound->QueryInterface(IID_ISoundtrans, (void**)&mpISoundTrans);
			}
			//DeleteMediaType(&type);
		}
		return filter?S_OK:E_FAIL;
	}
	return S_OK;
}

HRESULT	xTranscodeGraph::Load_SubTitle()
{
	if (IsNeedSubTitle())
	{
		// 加载QVOD SutTitle?
		QVOD_FILTER_INFO info;
		CComQIPtr<IBaseFilter> filter = LoadFilter(CLSID_QvodSubTitle, info);
		if (filter)
		{
			CComQIPtr<IBaseFilter> pSrc = GetPreFilterOnPath(_EFT_V_TRANS);
			if (FAILED(ConnectFilterDirect(pSrc, filter)))
			{
				Trace(-1, L"Connect Qvod SubTitle FAILED!Try to load color space converter!");
				if(!Load_ColorSpaceTrans1())
				{
					Trace(-1, L"Load Load_ColorSpaceTrans1 Failed!");
					ReleaseFilter(filter);
					//SafeRelease(filter);
					filter.Release();
				}

				pSrc = GetPreFilterOnPath(_EFT_V_TRANS);
				if (FAILED(ConnectFilterDirect(pSrc, filter)))
				{
					Trace(-1, L"Connect Qvod SubTitle to color space converter failed!");
					ReleaseFilter(filter);
					//SafeRelease(filter);
					filter.Release();
				}
			}

			if(filter)
			{
				mpFilterQvodSubTitle = filter;

				// IColortrans /ISubTitle获取
				mpFilterQvodSubTitle->QueryInterface(IID_COLORFILTER_SUB, (void**)&mpISubTrans);
				if(mpISubTrans)
				{
					mpISubTrans->put_path(mParam.subpath.c_str());
				}
			}
		}
		return filter?S_OK:E_FAIL;
	}
		return S_OK;
}

HRESULT	xTranscodeGraph::Load_VobSub()
{
	if (IsNeedVobSub())
	{
		// 加载QVOD SutTitle?
		QVOD_FILTER_INFO info;
		CComQIPtr<IBaseFilter> filter = LoadFilter(CLSID_DirectVobSub, info);
		if (filter)
		{
			CComQIPtr<IBaseFilter> pSrc = GetPreFilterOnPath(_EFT_V_TRANS);
			if (FAILED(ConnectFilterDirect(pSrc, filter)))
			{
				Trace(-1, L"Connect Vob subtitle  failed!");
				ReleaseFilter(filter);
				//SafeRelease(filter);
				filter.Release();
			}

			if(filter && mParam.media_info.nSubtitleCount > 0)
			{
				CComQIPtr<IBaseFilter> splt = GetPreFilterOnPath(_EFT_V_DEC);
				if(!ConnectSubTitilePins(splt, filter))
				{
					Trace(-1, L"Connect Vob subtitle failed!");
					ReleaseFilter(filter);
					//SafeRelease(filter);
					filter.Release();
				}
			}

			if(filter)
			{
				mpFilterQvodSubTitle = filter;

				// IColortrans /ISubTitle获取
				CComPtr<IDirectVobSub> pvobsub;
				mpFilterQvodSubTitle->QueryInterface(__uuidof(IDirectVobSub), (void**)&pvobsub);
				if(pvobsub)
				{
					pvobsub->put_FileName(mParam.subpath.c_str());
					//mpISubTrans->put_path(mParam.subpath.c_str());
				}
			}
		}
		return filter?S_OK:E_FAIL;
	}
	return S_OK;
}

bool xTranscodeGraph::ConnectSubTitilePins(CComQIPtr<IBaseFilter>& pSrc, CComQIPtr<IBaseFilter>& pVobSubFilter)
{
	// 获取Dest Filter的输入媒体类型
	if (!pSrc || !pVobSubFilter)
	{
		return false;
	}

	bool ret = false;
	// 循环每个输出PIN
	BeginEnumPins(pSrc, pEnumPins, pOutPin)
	{	
		PIN_DIRECTION out_direction;
		HRESULT hr = pOutPin->QueryDirection(&out_direction);
		if(SUCCEEDED(hr) && out_direction == PINDIR_OUTPUT)
		{
			// 循环输出PIN的媒体类型
			BeginEnumMediaTypes(pOutPin, pEnumMediaTypes, pOutMediaType)
			{
				if (pOutMediaType->majortype == MEDIATYPE_Subtitle 
					|| (pOutMediaType->majortype == MEDIATYPE_DVD_ENCRYPTED_PACK && pOutMediaType->subtype == MEDIASUBTYPE_DVD_SUBPICTURE))
				{			
					// 找到输入PIN
					BeginEnumPins(pVobSubFilter, pDecoderEnumPins, pInPin)
					{
						hr = pInPin->QueryDirection(&out_direction);
						// 找到
						if (SUCCEEDED(hr) && out_direction == PINDIR_INPUT && SUCCEEDED(pInPin->QueryAccept(pOutMediaType)))
						{
							// 未连接？
							CComQIPtr<IPin> pConnectto;
							pInPin->ConnectedTo(&pConnectto);
							if(pConnectto)
							{
								pConnectto.Release();
								continue;
							}

							// 连接两个PIN
							hr =  mpGraph->ConnectDirect(pOutPin,pInPin,pOutMediaType);
							if(SUCCEEDED(hr))
							{
// 								if(pOutMediaType) 
// 									DeleteMediaType(pOutMediaType); 
// 								return true;
								ret = true;
							}
						}
					}
					EndEnumPins
				}
			}
			EndEnumMediaTypes(pOutMediaType)
		}
	}
	EndEnumPins
	return ret;
}

HRESULT	xTranscodeGraph::Load_PostVideo()
{
	if(IsNeedPostVideo())
	{
	// 加载QVOD PostVideo
		QVOD_FILTER_INFO info;
		CComQIPtr<IBaseFilter> filter = LoadFilter(CLSID_QvodPostVideo, info);

		// 设置POST VIDEO 参数，在连接之前设置
		//if(filter)
		//{
		//	CComPtr<IPostVideoEffectEx> pIPostVideoEx;
		//	filter->QueryInterface(IID_IPostVideoEffectEx, (void**)&pIPostVideoEx);
		//	if (pIPostVideoEx)//做缩放处理
		//	{	
		//		//mPlayMedia->SetVideoPostParamter(pIPostVideo); 
		//		//SafeRelease(pIPostVideo);
		//	}
		//}

		CComQIPtr<IBaseFilter> pSrc = GetPreFilterOnPath(_EFT_V_TRANS);
		if (filter && pSrc)
		{
			if (FAILED(ConnectFilterDirect(pSrc, filter)))
			{
				if (pSrc != mpFilterQvodSubTitle)
				{
					Trace(-1, L"Connect Qvod postvideo FAILED!Try to load color space converter!");
					if(!Load_ColorSpaceTrans1())
					{
						Trace(-2, L"Load Load_ColorSpaceTrans1 Failed!");
						ReleaseFilter(filter);
						//SafeRelease(filter);
						filter.Release();
					}

					pSrc = GetPreFilterOnPath(_EFT_V_TRANS);
					if (FAILED(ConnectFilterDirect(pSrc, filter)))
					{
						Trace(-2, L"Connect Qvod postvideo to color space converter failed!");
						ReleaseFilter(filter);
						//SafeRelease(filter);
						filter.Release();
					}

				}
				
			}

			if(filter)
			{
				mpFilterQvodPostVideo = filter;
				mpFilterQvodPostVideo->QueryInterface(IID_IPostVideoEffectEx, (void**)&mpIPostVideoEx);
				if(mpIPostVideoEx)
				{
					mpIPostVideoEx->Scale(mParam.dstWidth, mParam.dstHeight);
				}
			}
		}
		return filter?S_OK:E_FAIL;
	}
	
	return S_OK;
}

HRESULT xTranscodeGraph::Load_NULL_Render()
{
	// 加载QVOD PostVideo
	QVOD_FILTER_INFO info;
	CComQIPtr<IBaseFilter> filter = LoadFilter(CLSID_NULLRender, info);

	CComQIPtr<IBaseFilter> pSrc = GetPreFilterOnPath(_EFT_V_TRANS);
	if (filter && pSrc)
	{
		if(FAILED(ConnectFilterDirect(pSrc, filter)))
		{
			ReleaseFilter(filter);
			filter.Release();
		}
	}
	mpFilterNullRender = filter;
	return filter?S_OK:E_FAIL;
}
// bool xTranscodeGraph::IsNeedVideoRender()
// {
// 	if (!mParam.bNeedVideo)
// 	{
// 		return false;
// 	}
// 	if (!mHasVideo)
// 	{
// 		return false;
// 	}	
// 	return true;
// }

// bool xTranscodeGraph::IsNeedAudioRender()
// {
// 	// 抓帧
// 	if (mConType == _E_CON_CSF)
// 	{
// 		return false;
// 	}
// 	if (!mParam.bNeedAudio)
// 	{
// 		return false;
// 	}
// 	if (!mHasAudio)
// 	{
// 		return false;
// 	}
// 	return true;
// }


bool xTranscodeGraph::can_accel()
{
	bool bRet = true;
	if(!mParam.bAccel)
	{
		bRet = false;
		goto _ret;
	}

	if (mGraphType != e_GraphTransCode)
	{
		bRet = false;
		goto _ret;
	}

	// 检查媒体类型
	if (mVideoType != _V_H264
		&& mVideoType != _V_WVC1
		&& mVideoType != _V_MPEG_2)
	{
		bRet = false;
		goto _ret;
	}

	//目标不是H264
	if (mParam.dstVideo != _V_H264)
	{
		bRet = false;
		goto _ret;
	}

	//UI加了字幕文件
	if(mParam.subpath.size())
	{
		bRet = false;
		goto _ret;
	}

	//Intel显卡
	if(!IntelDXVASupport())
	{
		bRet = false;
	}
_ret:
	Trace(-2, L"!!!can_accel = %d\n", bRet?1:0);
	return bRet;
}

bool xTranscodeGraph::can_h265Encoder()
{
	bool bRet = true;
	if (mGraphType != e_GraphTransCode)
	{
		bRet = false;
		goto _ret;
	}

	//目标不是H265
	if (mParam.dstVideo != _V_H265)
	{
		bRet = false;
		goto _ret;
	}
	if(mpFilterVideoEncoder)
	{
		bRet = false;
		goto _ret;
	}
_ret:
	Trace(-2, L"!!!can_h265Encoder = %d\n", bRet?1:0);
	return bRet;
}

bool xTranscodeGraph::BuildingWithAccelSolution()
{
	Trace(-1, L"BuildingWithAccelSolution enter\n");

	// 获取加速方案
	QVOD_ACCEL_SOLUTION solution;
	if(!QvodFilterManager::GetInstance().GetAccelSolution_Default(solution))
	{
		Trace(-1, L"BuildingWithAccelSolution GetAccelSolution_Default error\n");
		return false;
	}

	int idx_vencoder = 0;
	// 检查容器类型是否符合加速条件
	bool bOK = false;
	for(int i = 0; i < solution.vVDecoders.size(); i++)
	{
		if (solution.vVDecoders[i].encoder == mVideoType)
		{
			idx_vencoder = i;
			bOK = true;
			break;
		}
	}
	if (!bOK)
	{
		Trace(-1, L"BuildingWithAccelSolution check encode type failed\n");
		return false;
	}

	//---------------------------------------------------------- 加载Source Filter
	// 本地播放，先尝试指定的SOURCE，否则尝试FileReader
	std::vector<CLSID> vectFilters;
	if(!QvodFilterManager::GetInstance().GetBestGraph(mConType, mVideoType , _A_OTHER, vectFilters))
	{	
		Trace(-1, L"BuildingWithAccelSolution GetBestGraph failed\n");
		return false;
	}

	// 指定了Source？
	CLSID source = CLSID_NULL;
	{
		BEGIN_FOR_EACH_FILTER_BY_CAPACITY(vectFilters, E_FILTER_CAPACITY_READ, source)
		{
			mpFilterSource = Load_SourceFilter(source);
			if (mpFilterSource)
			{
				break;
			}
		}
		END_FOR_EACH
	}

	// 没创建Source，创建Filter Reader
	if (!mpFilterSource)
	{	
		mpFilterFileReader = Load_ReaderFilter();
		if (!mpFilterFileReader)
		{
			Trace(-1, L"BuildingWithAccelSolution Load_ReaderFilter failed\n");
			return false;
		}
		
		// 创建Splitter
		CLSID splitter = CLSID_NULL;
		{
			BEGIN_FOR_EACH_FILTER_BY_CAPACITY(vectFilters, E_FILTER_CAPACITY_SPLIT, splitter)
			{
				if (splitter != CLSID_NULL)
				{
					mpFilterSplitter = Load_SplitterFilter(splitter);
					if (mpFilterSplitter)
					{
						break;
					}
				}
			}
			END_FOR_EACH

			if (!mpFilterSplitter)
			{
				Trace(-1, L"BuildingWithAccelSolution Load_SplitterFilter failed\n");
				return false;
			}
		}
	}

	// 没有Source也没有Splitter?
	if (!mpFilterSplitter && !mpFilterSource)
	{
		Trace(-1, L"BuildingWithAccelSolution no splitter & source error\n");
		return false;
	}

	//Reset
	ResetFlags();

	// 检查视频流，音频流，字幕流
	CheckChannelInfo();

	if (!mHasVideo && !mHasAudio)
	{
		Trace(-1, L"BuildingWithAccelSolution no video & audio error\n");
		return false;
	}

	//OutputDebugString(L"before load v decoder filter\n");
	//---------------------------------------------------------- 加载指定Video Decoder
	for (int i = 0 ; i < solution.vVDecoders[idx_vencoder].vDecoders.size(); i++)
	{
		mpFilterVideoDecoder = Load_VideoDecoderByCLSID(solution.vVDecoders[idx_vencoder].vDecoders[i]);
		if (mpFilterVideoDecoder)
			break;
	}

	if(mpFilterVideoDecoder == NULL)
	{
		Trace(-1, L"BuildingWithAccelSolution no video decoder error\n");
		return false;
	}

	CLSID adec = CLSID_NULL;
	if (mACount == 1)
	{
		BEGIN_FOR_EACH_FILTER_BY_CAPACITY(vectFilters, E_FILTER_CAPACITY_AUDIO_DEC, adec)
		{
			CComQIPtr<IBaseFilter> p = Load_AudioDecoderByCLSID(adec);
			if (p)
			{
				mpFilterAudioDecoders.push_back(p);
				break;
			}
		}
		END_FOR_EACH
	}
	//---------------------------------------------------------- 加载指定Audio Decoder
	if(mpFilterAudioDecoders.size() == 0 && !Load_AudioDecoderAuto())
	{
	
	}

	//----------------------------------------------------------- 加载其他
	Load_AudioSwitch();
	Load_Sound();


	//----------------------------------------------------------- 加载Intel Accel Encoder
	load_AccelEncoder();
	

	//----------------------------------------------------------- 加载MUX
	if(mGraphType == e_GraphTransCode)
	{
		if(Load_FFmpegMux() == NULL)
		{
			Trace(-1, L"BuildingWithAccelSolution Load_FFmpegMux failed\n");
			return false;
		}
	}
	
	Trace(-1, L"BuildingWithAccelSolution out\n");
	return true;
}


bool xTranscodeGraph::BuildingWithBestGraph()
{
	Trace(-1, L"BuildingWithBestGraph enter\n");
	// 检查容器类型,视频编码类型是否明确
	if (mConType == _E_CON_UNKNOW ||  mConType == _E_CON_OTHER)
	{
		Trace(-1, L"BuildingWithBestGraph _E_CON_UNKNOW error\n");
		return false;
	}

	// 无音频无视频流
	if (mVideoType == _V_OTHER && mAudioType == _A_OTHER)
	{
		Trace(-1, L"BuildingWithBestGraph _V_OTHER error\n");
		return false;
	}


	// 获取最优链路
	std::vector<CLSID> vectFilters;
	if(!QvodFilterManager::GetInstance().GetBestGraph(mConType, mVideoType, mAudioType, vectFilters))
	{
		Trace(-1, L"BuildingWithBestGraph GetBestGraph failed\n");
		return false;
	}

	// 指定了Source？
	CLSID source = CLSID_NULL;
	if (!mpFilterSource)
	{
		BEGIN_FOR_EACH_FILTER_BY_CAPACITY(vectFilters, E_FILTER_CAPACITY_READ, source)
		{
			mpFilterSource = Load_SourceFilter(source);
			if (mpFilterSource)
			{
				break;
			}
		}
		END_FOR_EACH
	}

	// 没创建Source，创建Filter Reader
	if (!mpFilterSource)
	{
		mpFilterFileReader = Load_ReaderFilter(CLSID_NULL);
		if (!mpFilterFileReader)
		{
			Trace(-1, L"BuildingWithBestGraph Load_ReaderFilter failed\n");
			return false;
		}
	}

	// 创建Splitter
	CLSID splitter = CLSID_NULL;
	if (!mpFilterSource)
	{
		BEGIN_FOR_EACH_FILTER_BY_CAPACITY(vectFilters, E_FILTER_CAPACITY_SPLIT, splitter)
		{
			if (splitter != CLSID_NULL)
			{
				mpFilterSplitter = Load_SplitterFilter(splitter);
				if (mpFilterSplitter)
				{
					break;
				}
			}
		}
		END_FOR_EACH

		if (!mpFilterSplitter)
		{
			Trace(-1, L"BuildingWithBestGraph Load_SplitterFilter failed\n");
			return false;
		}
	}

	// 没有Source也没有Splitter?
	if (!mpFilterSplitter && !mpFilterSource)
	{
		Trace(-1, L"BuildingWithBestGraph no splitter & source error\n");
		return false;
	}

	//Reset
	ResetFlags();

	// 检查视频流，音频流，字幕流
	CheckChannelInfo();

	// 截图且没有视频流
	if (!mHasVideo && !mHasAudio)
	{
		Trace(-1, L"BuildingWithBestGraph no video & audio error\n");
		return false;
	}

	
	// 创建视频解码器
	if (IsNeedVideoDecoder())
	{
		// 指定了Video Decoder
		CLSID vdec = CLSID_NULL;
		BEGIN_FOR_EACH_FILTER_BY_CAPACITY(vectFilters, E_FILTER_CAPACITY_VIDEO_DEC, vdec)
		{
			//WMVideo decoder dmo 不能连ffmpegmux
			if (vdec != CLSID_NULL && (vdec != CLSID_WMVideo_Decoder_DMO || mVideoType != _V_WVC1))
			{
				mpFilterVideoDecoder = Load_VideoDecoderByCLSID(vdec);
				if (mpFilterVideoDecoder)
				{
					break;
				}
			}
		}
		END_FOR_EACH

		if (!mpFilterVideoDecoder)
		{
			mpFilterVideoDecoder = Load_VideoDecoderAuto();
		}
	}

	// 创建音频解码器
	if (IsNeedAudioDecoder())
	{
		// 指定了AUDIO Decoder
		CLSID adec = CLSID_NULL;
		BEGIN_FOR_EACH_FILTER_BY_CAPACITY(vectFilters, E_FILTER_CAPACITY_AUDIO_DEC, adec)
		{
			if (adec != CLSID_NULL)
			{
				CComPtr<IBaseFilter> pFilterAudioDecoder = Load_AudioDecoderByCLSID(adec);
				if (pFilterAudioDecoder)
				{
					mpFilterAudioDecoders.push_back(pFilterAudioDecoder);
					break;
				}
			}
		}
		END_FOR_EACH

		if (mpFilterAudioDecoders.size() <= 0)
		{
			if(FAILED(Load_AudioDecoderAuto()))
			{
				Trace(-1, L"BuildingWithBestGraph Load_AudioDecoderAuto failed\n");
				return false;
			}
		}
	}


	Load_AudioSwitch();
	Load_Sound();
	Load_VobSub();
	Load_SubTitle();
	Load_PostVideo();

	
#ifdef INTEL_VERSION
	//尝试软件+硬编
	if(can_accel())
	{
		load_AccelEncoder();
	}
#endif

	if(can_h265Encoder())
	{
		Load_VideoH265EncoderAuto();
		
	}

	if(mGraphType == e_GraphTransCode)
	{
		if(Load_FFmpegMux() == NULL)
		{
			//OutputDebugString(L"BuildingWithBestGraph Load_FFmpegMux failed\n");
			return false;
		}
	}
	else if(mGraphType == e_GraphPlay)
	{

	}
	else if(mGraphType == e_GraphPreview)
	{
		Load_NULL_Render();
	}

	Trace(-1, L"BuildingWithBestGraph out\n");
	return true;
}

bool xTranscodeGraph::BuildingWithCheckBytes()
{
	// 判断CheckByte获取相关filter
	QVOD_FILTER_MAP filterMap;
	if(!GetFilterMapByCheckByte(filterMap))
	{
		Trace(-1, L"BuildingWithCheckBytes failed:GetFilterMapByCheckByte\n");
		return false;
	}

	if (_E_CON_UNKNOW == mConType)
		mConType = filterMap.container;

	// 存放Source & Splitter
	std::vector<CLSID> vectFilters = filterMap.vectFilters;

	if(!mpFilterFileReader)
	{
		// 创建Source
		CLSID sourceID;
		BEGIN_FOR_EACH_FILTER_BY_CAPACITY(vectFilters, E_FILTER_CAPACITY_READ, sourceID)
		{
			mpFilterSource = Load_SourceFilter(sourceID);
			if (mpFilterSource)
			{
				break;
			}
		}
		END_FOR_EACH

		if (!mpFilterSource)
		{
			mpFilterFileReader = Load_ReaderFilter(CLSID_NULL);
			if (!mpFilterFileReader)
			{
				Trace(-1, L"BuildingWithCheckBytes:Load_ReaderFilter failed\n");
				return false;
			}
		}
	}

	//--------------------------------------------------------- 加载Splitter Filter
	if(!mpFilterSource)
	{
		CLSID splitterID;
		BEGIN_FOR_EACH_FILTER_BY_CAPACITY(vectFilters, E_FILTER_CAPACITY_SPLIT, splitterID)
		{
			// 创建Splitter
			mpFilterSplitter = Load_SplitterFilter(splitterID);
			if (mpFilterSplitter)
			{
				break;
			}
		}
		END_FOR_EACH

		if (!mpFilterSplitter)
		{
			Trace(-1, L"BuildingWithCheckBytes:Load_SplitterFilter failed\n");
			return false;
		}
	}

	// 没有Source也没有Splitter
	if (!mpFilterSplitter && !mpFilterSource)
	{
		return false;
	}

	//Reset
	ResetFlags();

	// 检查视频流，音频流，字幕流
	CheckChannelInfo();

	// 没有视频也没有音频
	if (!mHasVideo && !mHasAudio)
	{
		Trace(-1, L"BuildingWithCheckBytes:!mHasVideo && !mHasAudio failed\n");
		return false;
	}

	bool bNeedVDec = IsNeedVideoDecoder();
	bool bNeedADec = IsNeedAudioDecoder();	
	bool bVDecOK = true;
	bool bADecOK = true;

	// 自动加载视频解码器并连接到Splitter或Source
	if (bNeedVDec)
	{
		CLSID vdec = CLSID_NULL;
		BEGIN_FOR_EACH_FILTER_BY_CAPACITY(vectFilters, E_FILTER_CAPACITY_VIDEO_DEC, vdec)
		{
			if (vdec != CLSID_NULL&& vdec != CLSID_WMVideo_Decoder_DMO)
			{
				mpFilterVideoDecoder = Load_VideoDecoderByCLSID(vdec);
				if (mpFilterVideoDecoder)
				{
					break;
				}
			}
		}
		END_FOR_EACH

		if(!mpFilterVideoDecoder)
		{
			mpFilterVideoDecoder = Load_VideoDecoderAuto();
		}

		bVDecOK = mpFilterVideoDecoder != NULL;
	}


	//---------------------------------------------------------- 加载指定Audio Decoder
	if (bNeedADec)
	{
		if(!Load_AudioDecoderAuto())
		{
			Trace(-1, L"BuildingWithCheckBytes:Load_AudioDecoderAuto failed\n");
			bADecOK = false;
		}
	}

	if (!bVDecOK && !bADecOK)
	{
		Trace(-1, L"BuildingWithCheckBytes:!bVDecOK && !bADecOK failed\n");
		return false;
	}

	Load_AudioSwitch();
	Load_Sound();
	Load_VobSub();
	Load_SubTitle();
	Load_PostVideo();

#ifdef INTEL_VERSION
	//尝试软件+硬编
	if(can_accel())
	{
		load_AccelEncoder();
	}
#endif

	if(can_h265Encoder())
	{
		Load_VideoH265EncoderAuto();
	}

	if(mGraphType == e_GraphTransCode)
	{
		if(Load_FFmpegMux() == NULL)
		{
			return false;
		}
	}
	else if(mGraphType == e_GraphPlay)
	{

	}
	else if(mGraphType == e_GraphPreview)
	{
		Load_NULL_Render();
	}
	return true;
}


CComPtr<IBaseFilter> xTranscodeGraph::Load_SourceFilter(const CLSID& clsid)
{
	QVOD_FILTER_INFO info;
	CComPtr<IBaseFilter> pibasefilter = LoadFilter(clsid, info);
	if(pibasefilter)
	{
		//加载媒体文件
		CComPtr<IFileSourceFilter> pfilesoruce;
		HRESULT hr = pibasefilter->QueryInterface(IID_IFileSourceFilter, (void **)&pfilesoruce);
		if(FAILED(hr))
		{
			ReleaseFilter(pibasefilter);
			pibasefilter.Release();
			return CComPtr<IBaseFilter>();
		}

		bool bOK = false;
		//HRESULT hr = S_OK;
		if(FAILED(hr = pfilesoruce->Load(mParam.srcurl.c_str(), NULL)))
		{
			ReleaseFilter(pibasefilter);
			pfilesoruce.Release();
			pibasefilter.Release();
			return CComPtr<IBaseFilter>();
		}
		pfilesoruce.Release();
		
		// 获取接口
// 		if (clsid != CLSID_DCBassFilter)
// 		{
// 			mpKeyFrameInfo.Release();
// 			mpKeyFrameInfoEx.Release();
// 			pibasefilter->QueryInterface(IID_IKeyFrameInfo, (void**)&mpKeyFrameInfo);
// 			pibasefilter->QueryInterface(IID_IKeyFrameInfoEx, (void**)&mpKeyFrameInfoEx);
// 		}


		return pibasefilter;
	}
	return CComPtr<IBaseFilter>();
}

CComPtr<IBaseFilter> xTranscodeGraph::Load_SplitterFilter(const CLSID& clsid)
{
	X_ASSERT(mpFilterSplitter == NULL);
	X_ASSERT(mpFilterFileReader);

	QVOD_FILTER_INFO info;
	CComPtr<IBaseFilter> pbasefilter = LoadFilter(clsid, info);
	if(!pbasefilter)
	{
		return pbasefilter;
	}

	// 连接Splitter与前一个Filter
	CComPtr<IBaseFilter> pSrcFilter = GetPreFilterOnPath(_EFT_SPL);
	CComPtr<IPin> pOutPin = GetUnconnectedPin(pSrcFilter, PINDIR_OUTPUT);
	if(pOutPin)
	{
		if(SUCCEEDED(ConnectPinToFilers(pOutPin, pbasefilter)))
		{
			pOutPin.Release();

			// 获取接口
// 			if (clsid != CLSID_DCBassFilter)
// 			{
// 				mpKeyFrameInfo.Release();
// 				mpKeyFrameInfoEx.Release();
// 
// 				pbasefilter->QueryInterface(IID_IKeyFrameInfo, (void**)&mpKeyFrameInfo);
// 				pbasefilter->QueryInterface(IID_IKeyFrameInfoEx, (void**)&mpKeyFrameInfoEx);
// 			}
			return pbasefilter;
		}
		pOutPin.Release();
	}

	ReleaseFilter(pbasefilter);
	pbasefilter.Release();
	return CComPtr<IBaseFilter>();
}	

CComPtr<IBaseFilter> xTranscodeGraph::Load_VideoDecoderByCLSID(const CLSID& clsid)
{
	X_ASSERT(mpFilterSplitter || mpFilterSource);

	QVOD_FILTER_INFO info;
	CComPtr<IBaseFilter> pbasefilter = LoadFilter(clsid, info);
	if(!pbasefilter)
	{
		return CComPtr<IBaseFilter>();
	}

	X_ASSERT(mCurVideoOutputPin);
	if (mCurVideoOutputPin && SUCCEEDED(ConnectPinToFilers(mCurVideoOutputPin, pbasefilter)))
	{
		return pbasefilter;
	}

	ReleaseFilter(pbasefilter);
	pbasefilter.Release();
	return CComPtr<IBaseFilter>();
}

CComPtr<IBaseFilter> xTranscodeGraph::Load_AudioDecoderByCLSID(const CLSID& clsid)
{
	X_ASSERT(mpFilterSplitter || mpFilterSource);

	QVOD_FILTER_INFO info;
	CComPtr<IBaseFilter>pbasefilter = LoadFilter(clsid,info);
	if(!pbasefilter)
	{
		return CComPtr<IBaseFilter>();
	}

	CComPtr<IBaseFilter> pSrc = GetPreFilterOnPath(_EFT_A_DEC);
	bool bOK = ConnectFilers(pSrc, pbasefilter);
	if(!bOK)
	{
		ReleaseFilter(pbasefilter);
		pbasefilter.Release();
		return CComPtr<IBaseFilter>();
	}
	return pbasefilter;
}

CComPtr<IBaseFilter> xTranscodeGraph::Load_VideoDecoderAuto()
{
	X_ASSERT(mCurVideoOutputPin);
	if (!mCurVideoOutputPin)
	{
		return CComPtr<IBaseFilter>();
	}
	BeginEnumMediaTypes(mCurVideoOutputPin, pEnumMediaTypes, pOutMediaType)
	{
		QVOD_MEDIA_TYPE type = {pOutMediaType->majortype , pOutMediaType->subtype};

		// 获取合适的Filter
		std::vector<CLSID> video_filters;
		QvodFilterManager::GetInstance().GetFilterByMediaType(E_FILTER_CAPACITY_VIDEO_DEC, type, video_filters);

		// 按解码优先级排序
		SortFilterByPriority(video_filters, type);

		for (size_t j = 0; j < video_filters.size(); j++)
		{
			//WMVideo decoder dmo 不能连ffmpegmux
			if(CLSID_WMVideo_Decoder_DMO == video_filters[j])
			{
				continue;
			}
			QVOD_FILTER_INFO info;
			CComPtr<IBaseFilter> pbasefilter = LoadFilter(video_filters[j], info);
			if(!pbasefilter)
			{
				continue;
			}

			if(SUCCEEDED(ConnectPinToFilers(mCurVideoOutputPin, pbasefilter)))
			{	
				DeleteMediaType(pOutMediaType);
				return pbasefilter;
			}
			else
			{
				ReleaseFilter(pbasefilter);
				pbasefilter.Release();
				continue;
			}
		}

	}
	EndEnumMediaTypes(pOutMediaType)

	return CComPtr<IBaseFilter>();
}

CComPtr<IBaseFilter> xTranscodeGraph::Load_VideoH265EncoderAuto()
{
	const GUID clsid_h265encoder = { 0xa56994f8, 0x1ba9, 0x4afc, { 0x8a, 0x21, 0xb1, 0x5c, 0x19, 0xda, 0x6, 0x70 } };//{0xBEFF6FA5,0x87E3,0x44B5,{0x86,0xC8,0x95,0xF9,0x31,0xBD,0xF5,0x90}};
	QVOD_FILTER_INFO info;
	CComQIPtr<IBaseFilter>  pVtrans = GetPreFilterOnPath(_EFT_V_TRANS);
	mpFilterVideoEncoder = LoadFilter(clsid_h265encoder, info);

	CComPtr<IScale> configInterface;
	mpFilterVideoEncoder->QueryInterface(IID_IScale, (void**)&configInterface);
	if(configInterface)
	{
		configInterface->SetScale(mParam.dstWidth, mParam.dstHeight);
	}

	if(SUCCEEDED(ConnectFilterDirect(pVtrans, mpFilterVideoEncoder)))
	{	
		if(mpFilterVideoEncoder)
		{
			//mpFilterVideoEncoder->QueryInterface(IID_IQVODH265Encode, (void**)&mIQVODH265Encode);
		}
		//if(mIQVODH265Encode)
		{
			//mIQVODH265Encode->SetBitrate(mParam.)

		}
		return mpFilterVideoEncoder;
	}

	ReleaseFilter(mpFilterVideoEncoder);
	return mpFilterVideoEncoder;
}

HRESULT xTranscodeGraph::Load_AudioDecoderAuto()
{
	X_ASSERT(mpFilterSplitter || mpFilterSource);
	CComPtr<IBaseFilter> pSrc = GetPreFilterOnPath(_EFT_A_DEC);
	CComPtr<IBaseFilter> pbasefilter;
	BeginEnumPins(pSrc, pEnumPins, pOutPin)
	{
		bool bBreakFor = false;
		PIN_DIRECTION pin_direction;
		HRESULT hr = pOutPin->QueryDirection(&pin_direction);
		if(SUCCEEDED(hr) && pin_direction == PINDIR_OUTPUT)
		{
			BeginEnumMediaTypes(pOutPin, pEnumMediaTypes, pOutMediaType)
			{
				if (pOutMediaType->majortype == MEDIATYPE_Video)
				{
					continue;
				}

				//// 不需要解码器
				if(pOutMediaType->subtype == MEDIASUBTYPE_PCM)
				{
					continue;
				}

				if (bBreakFor)
				{
					break;
				}
				QVOD_MEDIA_TYPE type = {pOutMediaType->majortype, pOutMediaType->subtype};

				// 获取合适的Filter
				std::vector<CLSID> audio_filters;
				QvodFilterManager::GetInstance().GetFilterByMediaType(E_FILTER_CAPACITY_AUDIO_DEC, type, audio_filters);
				// 按解码优先级排序
				SortFilterByPriority(audio_filters, type);
				for (size_t i = 0; i < audio_filters.size(); i++)
				{
					QVOD_FILTER_INFO info;
					pbasefilter = LoadFilter(audio_filters[i], info);
					if(pbasefilter)
					{
						if(ConnectFilers(pSrc, pbasefilter))
						{	
							//mpFilterAudioDecoder = pbasefilter;
							mpFilterAudioDecoders.push_back(pbasefilter);
							bBreakFor = true;
							break;
						}
						else
						{
							ReleaseFilter(pbasefilter);
							pbasefilter.Release();
						}
					}
				}
			}
			EndEnumMediaTypes(pOutMediaType)
		}
	}
	EndEnumPins

		return mpFilterAudioDecoders.size() > 0 ? S_OK:E_FAIL;
}

bool xTranscodeGraph::GetFilterMapByCheckByte(QVOD_FILTER_MAP& filters)
{
	filters.container = _E_CON_UNKNOW;
	filters.vCheckBytes.clear();
	filters.vSuffix.clear();
	filters.vectFilters.clear();

	const std::vector<QVOD_FILTER_MAP>& filterMaps = QvodFilterManager::GetInstance().GetFilterMaps();
	if (filterMaps.empty())
	{
		Trace(-1, L"GetFilterMapByCheckByte::filterMaps.empty() == true\n");
		return false;
	}

	// 读取一段字节
	int size_data = 32;
	BYTE* datas = new BYTE[size_data];
	memset(datas,0,size_data);
	if(!ReadCheckBytes(datas, size_data))
	{
		Trace(-1, L"GetFilterMapByCheckByte:ReadCheckBytes failed\n");
		return false;
	}

	// 对比每个CheckByte,查找类型类型
	for (size_t i = 0; i < filterMaps.size(); i++)
	{
		for (size_t j = 0; j < filterMaps[i].vCheckBytes.size(); j++)
		{
			CString checkBytes = filterMaps[i].vCheckBytes[j];
			bool bOK = false;
			int needMoreData = 0;
			while (!(bOK = CheckCB(LPCTSTR(checkBytes), datas, size_data, needMoreData)) && needMoreData)
			{
				delete [] datas;
				datas = new BYTE[needMoreData];
				size_data = needMoreData;
				if (!ReadCheckBytes(datas, size_data))
				{
					delete [] datas;
					Trace(-1, L"GetFilterMapByCheckByte::for ReadCheckBytes failed\n");
					return false;
				}
			}
			if(bOK)
			{
				filters = filterMaps[i];
				delete [] datas;
				return true;
			}
		}
	}
	Trace(-1, L"GetFilterMapByCheckByte::Unknow media type, filtermapsize:%d\n", filterMaps.size());
	return false;
}

bool xTranscodeGraph::ReadCheckBytes(BYTE* buffer, int size_to_read)
{
	if(mpFilterFileReader)
	{
		if (mConType == _E_CON_XLMV)
		{
			BeginEnumPins(mpFilterFileReader, pEnumPins, pPin)
			{
				PIN_DIRECTION pin_dir;
				HRESULT hr = pPin->QueryDirection(&pin_dir);
				if(SUCCEEDED(hr)&& pin_dir == PINDIR_OUTPUT)
				{
					CComPtr<IAsyncReader> pAsynReader;
					HRESULT hr = pPin->QueryInterface(IID_IAsyncReader, (void**)&pAsynReader);
					if(pAsynReader && SUCCEEDED(pAsynReader->SyncRead(0, size_to_read, buffer)))
					{
						return true;	
					}
				}
			}
			EndEnumPins
		}
		else
		{
			CComPtr<IAsyncReader> pAsynReader;
			HRESULT hr = mpFilterFileReader->QueryInterface(IID_IAsyncReader, (void**)&pAsynReader);
			if(pAsynReader && SUCCEEDED(pAsynReader->SyncRead(0, size_to_read, buffer)))
			{
				return true;
			}
		}
	}
	else
	{
		std::ifstream ifile(mParam.srcurl.c_str(), std::ios::in | std::ios::binary);
		if (ifile.is_open())
		{
			ifile.read((char*)buffer, size_to_read);
			int read_size = ifile.gcount();
			if(read_size == size_to_read)
				return true;
		}
	}
	return false;
}

CComPtr<IBaseFilter> xTranscodeGraph::GetPreFilterOnPath(e_FilterType t)
{
	if (t == _EFT_SPL)
	{
		return mpFilterFileReader;
	}
	else if(t == _EFT_V_DEC)
	{
		return mpFilterSplitter?mpFilterSplitter:mpFilterSource;
	}
	else if(t == _EFT_A_DEC)
	{
		return mpFilterSplitter?mpFilterSplitter:mpFilterSource;
	}
	else if(t == _EFT_V_TRANS)
	{

		 		/*if (mpFilterColorSpaceTrans_2)
		 			return mpFilterColorSpaceTrans_2;*/
		
		if (mpFilterVideoEncoder)
			return mpFilterVideoEncoder;

		if (mpFilterQvodPostVideo)
			return mpFilterQvodPostVideo;

		if (mpFilterQvodSubTitle)
			return mpFilterQvodSubTitle;

		 		if (mpFilterColorSpaceTrans_1)
		 			return mpFilterColorSpaceTrans_1;




		if (mpFilterVobSubTitle)
			return mpFilterVobSubTitle;

		if (mpFilterVideoDecoder)
			return mpFilterVideoDecoder;

		if (mpFilterSplitter)
			return mpFilterSplitter;

		return mpFilterSource;

	}
	else if(t == _EFT_A_TRANS)
	{
		if (mpFilterAudioEncoder)
			return mpFilterAudioEncoder;

		if (mpFilterQvodSound)
			return mpFilterQvodSound;

		if(mpFilterAudioSwitch)
			return mpFilterAudioSwitch;

		CComPtr<IBaseFilter> filter = mpFilterAudioDecoders.size()> 0 ? mpFilterAudioDecoders[0]: mpFilterSplitter;
		if (filter)
			return filter;

		return mpFilterSource;
	}
	else if(t == _EFT_VIDEO)
	{
		// 		if (mpFilterColorSpaceTrans_2)
		// 			return mpFilterColorSpaceTrans_2;
		return mpFilterQvodPostVideo?mpFilterQvodPostVideo : GetPreFilterOnPath(_EFT_V_TRANS);
	}
	else if(t == _EFT_AUDIO)
	{
		return mpFilterQvodSound ? mpFilterQvodSound : GetPreFilterOnPath(_EFT_A_TRANS);
	}
	return CComPtr<IBaseFilter>();
}

void xTranscodeGraph::CheckChannelInfo()
{
	// 检查视频流，音频流，字幕流
	CComPtr<IBaseFilter> pCheckFilter = mpFilterSource ? mpFilterSource : mpFilterSplitter;
	if (!pCheckFilter)
	{
		return;
	}

	int maxVideoRate = 0;
	int maxAudioRate = 0;
	int idx = 0;
	BeginEnumPins(pCheckFilter, pEnumPins, pOutPin)
	{
		PIN_DIRECTION pin_direction;
		HRESULT hr = pOutPin->QueryDirection(&pin_direction);
		if(SUCCEEDED(hr) && pin_direction == PINDIR_OUTPUT)
		{
			bool bAddVideo = false;
			bool bAddAudio = false;
			bool bAddTitle = false;
			BeginEnumMediaTypes(pOutPin, pEnumMediaTypes, pOutMediaType)
			{
				if (pOutMediaType->majortype == MEDIATYPE_Video || 
					pOutMediaType->formattype == FORMAT_VideoInfo || 
					pOutMediaType->formattype == FORMAT_VideoInfo2 ||
					pOutMediaType->formattype == FORMAT_MPEGVideo)
				{
					// 初值
					if (!mCurVideoOutputPin)
					{
						mCurVideoOutputPin = pOutPin;
					}

					if(pOutMediaType->formattype == FORMAT_VideoInfo)
					{
						VIDEOINFOHEADER *pVih = (VIDEOINFOHEADER*)pOutMediaType->pbFormat;
						mVideoWidth = pVih->bmiHeader.biWidth;
						mVideoHeight = pVih->bmiHeader.biHeight;

						mdwAspectRatio_X = abs(pVih->bmiHeader.biWidth);
						mdwAspectRatio_Y = abs(pVih->bmiHeader.biHeight);

						if (pVih->dwBitRate > maxVideoRate)
						{
							maxVideoRate = pVih->dwBitRate;
							mCurVideoOutputPin = pOutPin;// 最高码率视频流
						}
					}
					else if(pOutMediaType->formattype == FORMAT_VideoInfo2)
					{
						VIDEOINFOHEADER2 *pvih2 = (VIDEOINFOHEADER2*)pOutMediaType->pbFormat;
						mVideoWidth = pvih2->bmiHeader.biWidth;
						mVideoHeight = pvih2->bmiHeader.biHeight;

						mdwAspectRatio_X = pvih2->dwPictAspectRatioX;
						mdwAspectRatio_Y = pvih2->dwPictAspectRatioY;

						if (pvih2->dwBitRate > maxVideoRate)
						{
							maxVideoRate = pvih2->dwBitRate;
							mCurVideoOutputPin = pOutPin;// 最高码率视频流
						}
					}
					else if(pOutMediaType->formattype == FORMAT_MPEGVideo)
					{
						MPEG1VIDEOINFO *pMVI = (MPEG1VIDEOINFO*)pOutMediaType->pbFormat;

						mVideoWidth = pMVI->hdr.bmiHeader.biWidth;
						mVideoHeight = pMVI->hdr.bmiHeader.biHeight;

					}
					else if(pOutMediaType->formattype == FORMAT_MPEG2_VIDEO)
					{
						MPEG2VIDEOINFO *pmvi2 = (MPEG2VIDEOINFO*)pOutMediaType->pbFormat;
						mVideoWidth = pmvi2->hdr.bmiHeader.biWidth;
						mVideoHeight = pmvi2->hdr.bmiHeader.biHeight;

					}

					bAddVideo = true;
				}
				else if(pOutMediaType->majortype == MEDIATYPE_Audio ||
					pOutMediaType->majortype == MEDIATYPE_OggPacketStream ||
					pOutMediaType->majortype == MEDIATYPE_Midi ||
					pOutMediaType->formattype == FORMAT_WaveFormatEx)
				{
					bAddAudio = true;

					//WAVEFORMATEX *pwav = (WAVEFORMATEX*)pOutMediaType->pbFormat;
				}
				else if(pOutMediaType->majortype == MEDIATYPE_Stream)
				{
					// 初值
					if (!mCurVideoOutputPin)
					{
						mCurVideoOutputPin = pOutPin;
					}

					bAddVideo = true;

				}
				else if(pOutMediaType->majortype == MEDIATYPE_Subtitle ||
					pOutMediaType->majortype == MEDIATYPE_Text )
				{	
					bAddTitle = true;

				}
				else if(pOutMediaType->majortype == MEDIATYPE_MPEG2_PES ||
					pOutMediaType->majortype == MEDIATYPE_DVD_ENCRYPTED_PACK ||
					pOutMediaType->majortype == MEDIATYPE_MPEG2_PACK)
				{
					if (pOutMediaType->subtype == MEDIASUBTYPE_MPEG2_VIDEO )
					{

						// 初值
						if (!mCurVideoOutputPin)
						{
							mCurVideoOutputPin = pOutPin;
						}

						bAddVideo = true;

						MPEG2VIDEOINFO *pmvi2 = (MPEG2VIDEOINFO*)pOutMediaType->pbFormat;
						mVideoWidth = pmvi2->hdr.bmiHeader.biWidth;
						mVideoHeight = pmvi2->hdr.bmiHeader.biHeight;

						if (pmvi2->hdr.dwBitRate > maxVideoRate)
							maxVideoRate = pmvi2->hdr.dwBitRate;
					}
					else
					{
						bAddAudio = true;

					}
				}
			}
			EndEnumMediaTypes(pOutMediaType)

				// 增加流数目
				if (bAddVideo)
					mVCount++;
				else if(bAddAudio)
					mACount++;
				else if(bAddTitle)
					mTCount++;

		}
		idx++;
	}
	EndEnumPins

	mHasVideo = mVCount ? true : false;
	mHasAudio = mACount ? true : false;
	mHasSubTitle = mTCount ? true : false;

	if(mGraphType == e_GraphPreview)
	{
		mHasAudio = false;
	}
}

CComPtr<IBaseFilter> xTranscodeGraph::Load_ReaderFilter(const CLSID& clsid)
{
	CLSID id = (clsid == CLSID_NULL) ? CLSID_AsyncReader : clsid;
	return Load_SourceFilter(id);
}

CComPtr<IPin> xTranscodeGraph::GetUnconnectedPin(CComPtr<IBaseFilter>& pFilter,PIN_DIRECTION pinDir)
{
	CComPtr<IEnumPins> pEnum;
	CComPtr<IPin> pPin;
	HRESULT hr = pFilter->EnumPins(&pEnum);
	if (FAILED(hr))
	{
		return CComPtr<IPin>();
	}

	while(pPin.Release(), pEnum->Next(1,&pPin, NULL) == S_OK)
	{
		PIN_DIRECTION thisPinDir;
		pPin->QueryDirection(&thisPinDir);
		if (thisPinDir == pinDir)
		{
			CComPtr<IPin> pTmp;
			hr = pPin->ConnectedTo(&pTmp);
			if (SUCCEEDED(hr))
			{
				pTmp.Release();
			}
			else
			{
				//pEnum.Release();
				return pPin;
				//return S_OK;
			}
		}
		pPin.Release();
	}
	//pEnum->Release();
	return CComPtr<IPin>();

}

bool xTranscodeGraph::ParserMediaInfo(Qvod_DetailMediaInfo* info)
{
	// 初始化
	mConType = _E_CON_UNKNOW;
	mVideoType = _V_OTHER;
	mVCount = 0;
	mACount = 0;
	mTCount = 0;
	mHasAudio = false;
	mHasVideo = false;
	mHasSubTitle = false;
	mVideoHeight = 0;


	// 容器类型
	CString con = info->container;
	mConType = ContainerFromString(con);

	if (mConType == _E_CON_UNKNOW || mConType == _E_CON_OTHER)
	{
		return false;
	}
	// 视频编码类型
	if (info->nVideoCount > 0)
	{
		CString video = info->videoInfo[0]->Codec;
		mVideoType = VideoEncoderFromString(video);
	}
	mHasVideo = mVideoType != _V_OTHER;
	mVCount = mHasVideo ? 1 : 0;

	// 音频编码类型
	for (int i = 0; i < info->nAudioCount && i == mParam.nAudioStreamID; i++)
	{
		CString audio = info->audioInfo[i]->Codec;
		mAudioType = AudioEncoderFromString(audio);
		
	}
	mHasAudio = mAudioType != _A_OTHER;
	mACount = 1;

	// 字幕
	mHasSubTitle = info->nSubtitleCount ? true : false;
	mTCount = info->nSubtitleCount;


	mVideoHeight = info->videoInfo[0] ? info->videoInfo[0]->nHeight : 0;

	return true;
}




CComPtr<IBaseFilter> xTranscodeGraph::Load_AudioEncoderAuto()
{
	const GUID clsid_aacencoder = {0x88F36DB6, 0xD898, 0x40B5, {0xB4, 0x09, 0x46, 0x6A, 0x0E, 0xEC, 0xC2, 0x6A}};//{0x88F36DB6,0xD898,0x40B5,{0xB4,0x09,0x46,0x6A,0x0E,0xEC,0xC2,0x6A}};
	QVOD_FILTER_INFO info;
	mpFilterAudioEncoder = LoadFilter(clsid_aacencoder, info);

	CComQIPtr<IBaseFilter>  patrans = GetPreFilterOnPath(_EFT_V_TRANS);
	if(SUCCEEDED(ConnectFilterDirect(patrans, mpFilterAudioEncoder)))
	{	
		return mpFilterAudioEncoder;
	}

	ReleaseFilter(mpFilterAudioEncoder);
	return mpFilterAudioEncoder;
}

CComPtr<IBaseFilter> xTranscodeGraph::Load_VideoEncodeAuto()
{
	const GUID clsid_h264encoder = {0xE2B7DF52, 0x38C5, 0x11D5, {0x91, 0xF6, 0x00, 0x10, 0x4B, 0xDB, 0x8F, 0xF9}};//{0xBEFF6FA5,0x87E3,0x44B5,{0x86,0xC8,0x95,0xF9,0x31,0xBD,0xF5,0x90}};
	QVOD_FILTER_INFO info;
	mpFilterVideoEncoder = LoadFilter(clsid_h264encoder, info);

	CComQIPtr<IBaseFilter>  pVtrans = GetPreFilterOnPath(_EFT_V_TRANS);
	if(SUCCEEDED(ConnectFilterDirect(pVtrans, mpFilterVideoEncoder)))
	{	
		return mpFilterVideoEncoder;
	}

	ReleaseFilter(mpFilterVideoEncoder);
	return mpFilterVideoEncoder;
}


HRESULT xTranscodeGraph::load_AccelEncoder()
{
	Trace(-2, L"!!!load_AccelEncoder begin\n");
	const GUID classid_IntelH264Encoder = {0x3CE2DF04,0xB62E,0x4B14,{0x8F,0x11,0x05,0x10,0xDF,0xEB,0x04,0xFF}};
	QVOD_FILTER_INFO info;
	CComPtr<IBaseFilter> pFilterEncoder = LoadFilter(classid_IntelH264Encoder, info);
	if(pFilterEncoder == NULL)
	{
		Trace(-1, L"load_AccelEncoder LoadFilter failed\n");
		return E_FAIL;
	}

	if(mParam.bNeedVideo && mHasVideo)
	{
		CComQIPtr<IBaseFilter>  pvtrans = GetPreFilterOnPath(_EFT_V_TRANS);
		if(SUCCEEDED(ConnectFilterDirect(pvtrans, pFilterEncoder)))
		{	
			mpFilterVideoEncoder = pFilterEncoder;	
			return S_OK;
		}
	}

	Trace(-1, L"load_AccelEncoder error\n");
	return E_FAIL;
}

bool xTranscodeGraph::IsAccelTrans()
{
	FILTER_INFO Info ;
	if(mpFilterVideoEncoder)
	{
		HRESULT hr = mpFilterVideoEncoder->QueryFilterInfo(&Info);
		if(hr == S_OK)
		{
			if(Info.pGraph)
			{
				Info.pGraph->Release();
				Info.pGraph = NULL;
			}
			if(!wcscmp(Info.achName,L"Qvod H265 Encoder"))
			{
				return false;
			}
		}
	}
	return mpFilterVideoEncoder?true:false;
}

const GUID IID_IFFmpegMux = {0x4722df8d, 0xc7ad, 0x4a43, {0x8f, 0x7f, 0x7f, 0xb6, 0x60, 0x66, 0xf0, 0xe8}};

CComPtr<IBaseFilter> xTranscodeGraph::Load_FFmpegMux()
{
	const GUID clsid_ffmpegmux = {0x044BAFB4,0x1908,0x4FB9,{0xBF,0xB8,0x75,0xCE,0xA9,0x50,0xEC,0xA0}};//{0x5FD85181, 0xE542, 0x4E52, {0x8D, 0x9D, 0x5D, 0x61, 0x3C, 0x30, 0x13, 0x1B}};//{0x36B426DE,0xB820,0x49C8,{0x94,0xF8,0x47,0x54,0xC0,0xC6,0xE6,0x04}};
	QVOD_FILTER_INFO info;
	 CComPtr<IBaseFilter> pFilterMux = LoadFilter(clsid_ffmpegmux, info);
	if(pFilterMux == NULL)
	{
		Trace(-1, L"Load_FFmpegMux LoadFilter failed\n");
		return CComPtr<IBaseFilter>();
	}
	
	CComPtr<IFileSinkFilter>	sink_filter;
	pFilterMux->QueryInterface(IID_IFileSinkFilter, (void**)&sink_filter);
	if (sink_filter)
	{
		HRESULT hr = S_OK;
		if(FAILED(hr = sink_filter->SetFileName(mParam.dstcurl.c_str(), NULL)))
		{
			Trace(-2, L"Set FileName failed Errorcode:%0x\n");
			Trace(-2, mParam.dstcurl.c_str());
			return CComPtr<IBaseFilter>();
		}	
	}
	mpFilterMux = pFilterMux;
	if(mpFilterMux)
	{
		mpFilterMux->QueryInterface(IID_IFFmpegMux, (void**)&mIFFmpegMux);
	}
	
	if(mParam.bNeedVideo && mHasVideo)
	{
		CComQIPtr<IBaseFilter>  pvtrans = GetPreFilterOnPath(_EFT_V_TRANS);
		if(FAILED(ConnectFilterDirect(pvtrans, mpFilterMux)))
		{	
			Trace(-2, L"ffmpeg ConnectFilterDirect v failed\n");
			return CComPtr<IBaseFilter>();
		}
	}

	if(mParam.bNeedAudio && mHasAudio)
	{
		CComQIPtr<IBaseFilter> patrans = GetPreFilterOnPath(_EFT_A_TRANS);
		if(FAILED(ConnectFilterDirect(patrans, mpFilterMux)))
		{
			Trace(-2, L"ffmpeg ConnectFilterDirect a failed\n");
			return CComPtr<IBaseFilter>();
		}
	}
	return mpFilterMux;
}

// CComPtr<IBaseFilter> xTranscodeGraph::Load_FileWriter()
// {
// 	const GUID clsid_filewriter = {0x8596E5F0,0x0DA5,0x11D0,{0xBD,0x21,0x00,0xA0,0xC9,0x11,0xCE,0x86}};
// 	QVOD_FILTER_INFO info;
// 	mpFilterWriter = LoadFilter(clsid_filewriter, info);
// 
// 	CComPtr<IFileSinkFilter>	sink_filter;
// 	mpFilterWriter->QueryInterface(IID_IFileSinkFilter, (void**)&sink_filter);
// 	if (sink_filter)
// 	{
// 		sink_filter->SetFileName(L"C:\\test.mp4",NULL);
// 	}
// 	if(SUCCEEDED(ConnectFilterDirect(mpFilterMux, mpFilterWriter)))
// 	{
// 
// 	}
// 	return mpFilterWriter;
// }

bool xTranscodeGraph::IntelDXVASupport()
{
	mfxSession HWsession = 0;
	mfxVersion HWversion = {0, 1};
	if(MFX_ERR_NONE == MFXInit(MFX_IMPL_HARDWARE, &HWversion, &HWsession))//试图创建一个最低版本的硬件session
	{
		MFXClose(HWsession);
		return true;
	}
	else
	{
		MFXClose(HWsession);
		return false;
	}
}

HRESULT xTranscodeGraph::SetMuxConfigure(MUX_CONFIG_EX& config,float srcFrameRate)
{
	
	if (mpFilterVideoEncoder)
	{
		//设置编码参数 、硬编
		if (mParam.dstVideo == _V_H264)
		{
			CComPtr<IConfigureVideoEncoder> configInterface;
			mpFilterVideoEncoder->QueryInterface(IID_IConfigureVideoEncoder, (void**)&configInterface);
			if (configInterface)
			{
				IConfigureVideoEncoder::Params p;

				configInterface->GetParams(&p);

				p.level_idc             = IConfigureVideoEncoder::Params::LL_AUTOSELECT;
				p.profile_idc                    =  IConfigureVideoEncoder::Params::PF_H264_BASELINE;

				p.pc_control                     =  IConfigureVideoEncoder::Params::PC_FRAME;
				p.ps_control.GopPicSize          = 30;
				p.ps_control.GopRefDist          = 3;
				p.ps_control.NumSlice            = 0; // autoselect

				p.rc_control.bitrate = config.VideoCodec.bitrate / 1000;
				p.rc_control.rc_method =  IConfigureVideoEncoder::Params::RCControl::RC_CBR;
				// 			p.target_usage = MFX_TARGETUSAGE_BALANCED;
				p.preset = 0/*CodecPreset::PRESET_LOW_LATENCY*/;

				p.throttle_control.qp_up=2;
				p.throttle_control.qp_down=2;
				p.throttle_control.bitrate_up=100;
				p.throttle_control.bitrate_down=100;

				//if rc_method == RC_VBR then


				p.frame_control.width = config.VideoCodec.width;
				p.frame_control.height = config.VideoCodec.height;

				configInterface->SetParams(&p);
			}
		}
		if (mParam.dstVideo == _V_H265)
		{
			//设置编码参数 、软编
			CComPtr<IQVODH265ENCODER> configInterface;
			mpFilterVideoEncoder->QueryInterface(IID_IQVODH265Encode, (void**)&configInterface);
			if(configInterface)
			{
				configInterface->SetBitrate(config.VideoCodec.bitrate / 1000);
			}
		}	
	}


	if(mIFFmpegMux)
	{
		HRESULT hr = mIFFmpegMux->SetMuxConfig((MUX_CONFIG*)&config);
		mIFFmpegMux->SetSrcFrameRate(srcFrameRate);
		mIFFmpegMux->SetMuxThreadCount(config.maxThreadCount);
		mIFFmpegMux->SetMuxDuration(&config.rtStart, &config.rtStop);
		mIFFmpegMux->SetQvodCallBack((IQvodCallBack *)config.callBack);
		

		return hr;
	}
	
	return E_FAIL;
}