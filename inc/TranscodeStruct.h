#pragma once
#include "QMediaInfo.h"
#include <vector>

typedef std::wstring FString;
typedef __int64				REFERENCE_TIME;
typedef double				REFTIME;
typedef REFERENCE_TIME		XREFTIME;
#define INDEX_TAG_DEF(p)	struct p{};


enum EDIT_STATUS
{
	STATUS_PLAY,
	STATUS_PAUSE,
	STATUS_STOP,
};

enum TRACK_TYPE
{ 
	TT_VIDEO,
	TT_AUDIO,
};

enum CLIP_TYPE
{
	CLIP_VIDEO,
	CLIP_AUDIO,
	CLIP_BITMAP,
	CLIP_TEXT,
};

enum EDIT_SOURCE_BUFFER_TYPE
{
	VIDEO_BUFFER,
	AUDIO_BUFFER,
};

struct TM_PAIR
{
	TM_PAIR()
		:tmLeft(0)
		,tmRight(0)
	{

	};

	TM_PAIR( XREFTIME lt,XREFTIME rt )
		:tmLeft(lt)
		,tmRight(rt)
	{

	};

	XREFTIME tmLeft;
	XREFTIME tmRight;

	bool operator <( const TM_PAIR& e ) const
	{
		return tmLeft < e.tmLeft && tmRight <= e.tmRight;
	};

	bool isValid() const
	{
		return tmLeft < tmRight;
	}
};


enum  EVIDEO_CHROMA_FMT
{
	CHROMA_I420       = 0,
	CHROMA_I422       = 1,// YUV422格式，又分为很多小类，按照U、V的排列可以有YUYV,YVYU,UYVY,VYUY四种
	CHROMA_I444       = 2,
	CHROMA_YV12       = 3,
	CHROMA_444RGB     = 4,
	CHROMA_444RGBA    = 5
};

enum  EAUDIO_PCM_FMT
{
	FAUDIO_U8         = 0x00,   
	FAUDIO_S16        = 0x01,
	FAUDIO_S24        = 0x02,
	FAUDIO_S32        = 0x03
};

enum ECOLOR_CHANNEL
{ 
	CHANNEL_Y,
	CHANNEL_U,
	CHANNEL_V,
	CHANNEL_R,
	CHANNEL_G,
	CHANNEL_B,
	CHANNEL_A,
};

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
	_E_CON_WMV,		//ASF/WMV/WMP
	_E_CON_WMA,		//WMA
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
	_E_CON_QMVB,
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
	_V_H265,

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
	FString filepath;
	REFERENCE_TIME srcstartts;
	REFERENCE_TIME srcstopts;
	REFERENCE_TIME dststartts;
	REFERENCE_TIME dststopts;
	bool bVideoenable;
	bool bAudioenable;
	bool bSubtitleenable;
	CLIP_CONFIG():srcstartts(0),srcstopts(0),dststartts(0),dststopts(0),bVideoenable(false),bAudioenable(false), bSubtitleenable(false){}
};

struct TRANSCODE_CONFIG
{
	FString		DestURL;
	E_CONTAINER ContainerType;
	VIDEO_CODEC VideoCodec;
	AUDIO_CODEC AudioCodec;
	long AudioVolume;
	bool KeepAspectRatio;
	bool Accel;//硬件加速
	std::vector<CLIP_CONFIG> ClipConfig_List;
};

struct MUX_CONFIG
{
	E_CONTAINER ContainerType;
	VIDEO_CODEC VideoCodec;
	AUDIO_CODEC AudioCodec;
	long AudioVolume;//音量放大，小于100表示音量减小，大于100表示音量放大，按音量/100的比例进行
};

struct MUX_CONFIG_EX : public MUX_CONFIG
{
	DWORD	maxThreadCount;//多线程编码
	REFERENCE_TIME rtStart;
	REFERENCE_TIME rtStop;
	void*	callBack;
};

enum e_TRANSCODE_STATE
{
	E_TSC_INIT,
	E_TSC_STOP,
	E_TSC_READY,
	E_TSC_RUNNING,
	E_TSC_PAUSE
};