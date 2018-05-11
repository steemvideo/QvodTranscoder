#ifndef __H_ffmpegMuxGuids__
#define __H_ffmpegMuxGuids__


enum E_CONTAINER
{
	_E_CON_UNKNOW = 0,
	_E_CON_RMVB = 1,
	_E_CON_AVI,
	_E_CON_MKV,
	_E_CON_MP4,		//MPEG-4 Part 14,3GP系
	_E_CON_3GP,		//3GP
	_E_CON_MOV,		//Apple系 包含MOV，QT

	//_E_CON_QT
	_E_CON_MPEG,	//MPEG1/2/4 标准容器格式
	_E_CON_DAT,
	_E_CON_MPEG4ES,	//MPEG4基本流
	_E_CON_MPEGTS,	//TS
	_E_CON_MPEGPS,	//VOB EVO HD-DVD
	_E_CON_HKMPG,	//Hikvision mp4
	_E_CON_DIVX,	//Divx Media Format(DMF)
	_E_CON_WM,		//ASF/WMV/WMA/WMP
	_E_CON_TS,		//TS
	_E_CON_M2TS,	//M2TS/MTS,支持DBAV 与 AVCHD
	_E_CON_DVD,		//DVD盘，特殊处理（DVD导航）
	_E_CON_QMV,
	_E_CON_VP5,
	_E_CON_VP6,
	_E_CON_VP7,
	_E_CON_WEBM,//VP8
	_E_CON_FLV,
	_E_CON_PMP,
	_E_CON_BIK,
	_E_CON_CSF,
	_E_CON_DV,	
	_E_CON_XLMV,//迅雷格式
	_E_CON_M2V,
	_E_CON_SWF,
	_E_CON_AMV, //Anime Music Video File
	_E_CON_IVF,//Indeo Video Format file
	_E_CON_IVM,
	_E_CON_SCM,
	_E_CON_AOB,
	_E_CON_AMR,
	_E_CON_DC,
	_E_CON_DSM,
	_E_CON_MP2,
	_E_CON_MP3,
	_E_CON_OGG,
	_E_CON_DTS,
	_E_CON_MIDI,
	_E_CON_WVP,
	_E_CON_AAC,
	_E_CON_AC3,
	_E_CON_AIFF,
	_E_CON_ALAW,
	_E_CON_APE,
	_E_CON_AU,
	_E_CON_AWB,//AMR-WB
	_E_CON_CDA,
	_E_CON_FLAC,
	_E_CON_MOD,
	_E_CON_MPA,//MPEG-2 Audio File
	_E_CON_MPC,//Musepack Audio
	_E_CON_RA,
	_E_CON_TTA,
	_E_CON_WAV,
	_E_CON_CDXA,
	_E_CON_SKM,
	_E_CON_MTV,
	_E_CON_MXF,
	_E_CON_OTHER = 0x100
};

enum E_V_CODEC
{
	/*MPEG 系列*/
	_V_MPEG_1,
	_V_MPEG_2,

	_V_MPEG_4_Visual,//Simple
	_V_MPEG_4_Advanced_Simple,//Advanced Simple

	_V_XVID,
	_V_DIVX,
	//_V_AVC,//AVC1/H264/X264

	/*ITU 系列*/
	_V_H261,
	_V_H263,
	_V_H263_PLUS,
	_V_H264,

	/*Real Media系列*/
	_V_RV10,
	_V_RV20,
	_V_RV30,
	_V_RV40,			

	/*Windows 系列*/
	_V_WM_MPEG4_V1,//Microsoft ISO MPEG-4 Video Codec
	_V_WM_MPEG4_V2,//Microsoft MPEG-4 Video Codec v2	MP42
	_V_WM_MPEG4_V3,//Microsoft MPEG-4 Video Codec v3	MP43

	_V_WMV7,//Windows Media Video v7	WMV1
	_V_WMV8,//Windows Media Video v8	WMV2
	_V_WMV9,//Windows Media Video v9	WMV3 
	_V_WVC1,//Windows Media Video v9 Advanced Profile	WVC1
	_V_MSVIDEO1,//MS VIDEO 1
	_V_VP3,
	_V_VP4,
	_V_VP5,
	_V_VP6,
	_V_VP60,  
	_V_VP61,
	_V_VP62,
	_V_VP7,   
	_V_VP8,	

	/*其他*/
	_V_HFYU,		//Huffyuv
	_V_YUV,			//YUV
	_V_YV12,		//YV12
	_V_UYVY,		//UYVY
	_V_JPEG,        //JPEG11
	_V_PNG,

	_V_IV31,		//IndeoV31
	_V_IV32,

	//_V_FLV,			//FLV
	_V_MJPG,		//MOTION JPEG
	_V_LJPG,		//LossLess JPEG

	_V_ZLIB,		//AVIZlib
	_V_TSCC,		//tscc
	_V_CVID,		//Cinepak
	_V_RAW,			//uncompressed Video
	_V_SMC,			//Graphics
	_V_RLE,			//Animation
	_V_APPV,		//Apple Video
	_V_KPCD,		//Kodak Photo CD
	_V_SORV,		//Sorenson video
	_V_SCRN,		//Screen video
	_V_THEORA,		//Ogg Theora         
	_V_ZMP5,
	_V_CSF,
	_V_OTHER	= 0x100	//未知
};

enum E_A_CODEC
{
	_A_AAC	= 0,		//LC-AAC/HE-AAC
	_A_AC3,				//AC3
	_A_E_AC3,			//E-AC-3
	_A_MP2,				//MPEG1 Layer 2
	_A_MP3,				//MPEG1 Layer 3
	_A_MP4,             //MPEG4 Audio
	_A_AMR_NB,			//AMR
	_A_AMR_WB,
	_A_DTS,
	_A_MIDI,
	_A_PCM,				//脉冲编码调制
	_A_DPCM,			//差分脉冲编码调制
	_A_APCM,			//自适应脉冲编码调制
	_A_ADPCM,			//自适应差分脉冲编码调制
	_A_PCM_LE,			//PCM Little Endian
	_A_PCM_BE,			//PCM Big	 Endian
	_A_COOK,			//rmvb AUDIO
	_A_SIPR,			//rmvb AUDIO
	_A_TSCC,			//tscc
	_A_QCELP,			//Qualcomm公司于1993年提出了可变速率的CELP，常称为QCELP,QCELP 语音压缩编码算法包括8K和13K两种
	_A_WMA1,
	_A_WMA2,
	_A_WMA3,

	_A_WMA4,
	_A_WMA5,
	_A_WMA6,

	_A_WMA7,
	_A_WMA8,
	_A_WMA9,            //windows media audio 9.0

	_A_RAW,             //uncompressed audio
	_A_MAC,             //kMACE3Compression
	_A_IMA,             //kIMACompression, Samples have been compressed using IMA 4:1.
	_A_LAW,             //kULawCompression,kALawCompression, uLaw2:1
	_A_ACM,             //kMicrosoftADPCMFormat: Microsoft ADPCM-ACM code 2;kDVIIntelIMAFormat:DVI/Intel IMAADPCM-ACM code 17
	_A_DVA,             //kDVAudioFormat:DV Audio
	_A_QDM,             //kQDesignCompression:QDesign music
	_A_QCA,             //kQUALCOMMCompression:QUALCOMM PureVoice
	_A_G711,            //G.711 A-law logarithmic PCM and G.711 mu-law logarithmic PCM
	_A_NYM,             //Nellymoser
	_A_SPX,             //Speex
	_A_VORBIS,          //Ogg vorbis
	_A_APE,				//Monkey's audio
	_A_FLAC,
	_A_ALAC,
	_A_MLP,				//DVD-Audio MLP
	_A_CSF,
	_A_AIFF,			//AIFF
	_A_TTA,				//True Type Audio
	_A_VP7,
	_A_MPC,				//Musepack audio
	_A_TRUEHD,			//Dolby True HD
	_A_WVP,				//Wavpack
	_A_OTHER   = 0x100	//未知
};

struct VIDEO_CODEC
{
	E_V_CODEC CodecType;
	int width;
	int height;
	int bitrate;
	int framerate;
};

struct AUDIO_CODEC
{
	E_A_CODEC CodecType;
	int samplingrate;
	int channel;
	int bitrate;
};
struct CLIP_CONFIG
{
	//CString filepath;
	wchar_t filepath[512];
	REFERENCE_TIME srcstartts;
	REFERENCE_TIME srcstopts;
	REFERENCE_TIME dststartts;
	REFERENCE_TIME dststopts;
	bool bVideoenable;
	bool bAudioenable;
	CLIP_CONFIG():srcstartts(0),srcstopts(0),dststartts(0),dststopts(0),bVideoenable(false),bAudioenable(false){}
};



struct MUX_CONFIG
{
	E_CONTAINER ContainerType;
	VIDEO_CODEC VideoCodec;
	AUDIO_CODEC AudioCodec;
	long AudioVolume;//音量放大，小于100表示音量减小，大于100表示音量放大，按音量/100的比例进行

};
// {044BAFB4-1908-4fb9-BFB8-75CEA950ECA0}
DEFINE_GUID(CLSID_QvodFFmpegMux, 
			0x44bafb4, 0x1908, 0x4fb9, 0xbf, 0xb8, 0x75, 0xce, 0xa9, 0x50, 0xec, 0xa0);

// {4722DF8D-C7AD-4a43-8F7F-7FB66066F0E8}
DEFINE_GUID(IID_IFFmpegMux, 
			0x4722df8d, 0xc7ad, 0x4a43, 0x8f, 0x7f, 0x7f, 0xb6, 0x60, 0x66, 0xf0, 0xe8);


#ifdef __cplusplus
extern "C" {
#endif

	DECLARE_INTERFACE_(IFFmpegMux, IUnknown)
	{
		STDMETHOD (SetMuxConfig) (MUX_CONFIG* pMuxConfig) = 0;
		STDMETHOD (GetMuxConfig) (MUX_CONFIG* pMuxConfig) = 0;
		STDMETHOD (SetMuxThreadCount) (DWORD Threadcount) = 0;
		STDMETHOD (GetMuxThreadCount) (DWORD &Threadcount) = 0;
		STDMETHOD (SetMuxDuration) (REFERENCE_TIME* rtStart, REFERENCE_TIME* rtStop) = 0;
		STDMETHOD (GetMuxDuration) (REFERENCE_TIME* rtStart, REFERENCE_TIME* rtStop) = 0;
		STDMETHOD (SetSrcFrameRate) (float framerate) = 0;
		STDMETHOD (SetQvodCallBack) (interface IQvodCallBack* pQvodCallBack) = 0;
	};
	//----------------------------------------------------------------------------

#ifdef __cplusplus
}
#endif

#endif // __H_ffmpegMuxGuids__

