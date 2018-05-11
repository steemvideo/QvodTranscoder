
#include <vector>
#include <atlbase.h>
#include <atlcom.h>
#include <atlcoll.h>
#include <atlstr.h>
#include <streams.h>
#include <dvdmedia.h>
#include <assert.h>

#include <initguid.h>
#include "QvodCallBack.h"
#include "IFFmpegMux.h"

typedef struct AVOutputStream 
{
	int file_index;          /* file index */
	int index;               /* stream index in the output file */
	int source_index;        /* AVInputStream index */
	AVStream *st;            /* stream in the output file */
	int encoding_needed;     /* true if encoding needed for this stream */
	int frame_number;
	//struct AVInputStream *sync_ist; /* input stream to sync against */
	int64_t sync_opts;       /* output frame counter, could be changed to some true timestamp */ //FIXME look at frame_number
	/* video only */
	int video_resample;
	AVFrame pict_tmp;      /* temporary image for resampling */
	struct SwsContext *img_resample_ctx; /* for image resampling */
	int resample_height;
	int resample_width;
	int resample_pix_fmt;

	int64_t *forced_kf_pts;
	int forced_kf_count;
	int forced_kf_index;

	/* full frame size of first frame */
	int original_height;
	int original_width;

	/* cropping area sizes */
	int video_crop;
	int topBand;
	int bottomBand;
	int leftBand;
	int rightBand;

	/* cropping area of first frame */
	int original_topBand;
	int original_bottomBand;
	int original_leftBand;
	int original_rightBand;

	/* padding area sizes */
	int video_pad;
	int padtop;
	int padbottom;
	int padleft;
	int padright;

	/* audio only */
	int audio_resample;
	ReSampleContext *resample; /* for audio resampling */
	int reformat_pair;
	AVAudioConvert *reformat_ctx;
	AVFifoBuffer *fifo;     /* for compression: one audio fifo per codec */
	//FILE *logfile;
} AVOutputStream;

enum StreamType
{
	Video_Type = 0,
	Audio_Type,
	Other_Type
};
class BLOCK
{
public:
	REFERENCE_TIME start;
	REFERENCE_TIME stop;
	AVPacket  packet ;
	StreamType packettype;
	long packetsize;
	BLOCK()
	{
		start = 0;
		stop = 0;
		//packet ;
		packettype = Other_Type;
		long packetsize = 0;
	}
	//~BLOCK(){}
};


typedef struct {
	const char *name;
	int flags;
#define HAS_ARG    0x0001
#define OPT_BOOL   0x0002
#define OPT_EXPERT 0x0004
#define OPT_STRING 0x0008
#define OPT_VIDEO  0x0010
#define OPT_AUDIO  0x0020
#define OPT_GRAB   0x0040
#define OPT_INT    0x0080
#define OPT_FLOAT  0x0100
#define OPT_SUBTITLE 0x0200
#define OPT_FUNC2  0x0400
#define OPT_INT64  0x0800
#define OPT_EXIT   0x1000
	union {
		void* void_arg;
		int *int_arg;
		char **str_arg;
		float *float_arg;
		int64_t *int64_arg;
		void (*func_arg)(const char *); //FIXME passing error code as int return would be nicer then exit() in the func
		int (*func2_arg)(const char *, const char *);
	} u;
	const char *help;
	const char *argname;
} OptionDef;

class CFFmpegMuxFilter;
class CFFmpegMuxInputPin :	public CBaseInputPin
	//,public IPinConnection
{
	CCritSec m_csReceive;
	
	
	BOOL m_bCheckMediaType;

protected:
	CFFmpegMuxFilter* m_ParentFilter;
public:
	CFFmpegMuxInputPin(LPCWSTR pName, CFFmpegMuxFilter* pFilter, CCritSec* pLock, HRESULT* phr);
	virtual ~CFFmpegMuxInputPin();

	DECLARE_IUNKNOWN;
	CAutoPtrList<BLOCK>  m_block;
	//CCritSec m_csQueue;
	BOOL m_fActive;
	BOOL m_fEndOfStreamReceived;

	HRESULT CheckMediaType(const CMediaType* pmt);
	CMediaType& CurrentMediaType();
	HRESULT BreakConnect();
	HRESULT CompleteConnect(IPin* pPin);
	HRESULT Active(), Inactive();
	IBaseFilter* GetFilterFromPin(IPin* pPin);
	BITMAPINFOHEADER GetVideoInfo();
	WAVEFORMATEX GetAudioInfo();
	BYTE * GetExtradata();
	int   GetExtradataLength();
	//double GetVideoBitrate();

	STDMETHODIMP NewSegment(REFERENCE_TIME tStart, REFERENCE_TIME tStop, double dRate);
	STDMETHODIMP BeginFlush();
	STDMETHODIMP EndFlush();

	STDMETHODIMP Receive(IMediaSample* pSample);
	STDMETHODIMP EndOfStream();
	STDMETHODIMP ReceiveCanBlock();
private:
	AVFrame *alloc_picture(int pix_fmt, int width, int height);
	bool GetBmpHeader(const CMediaType &mt,BITMAPINFOHEADER & bph);
	bool GetWaveFormat(const CMediaType &mt,WAVEFORMATEX & p);
	void FillYuvImage(AVFrame *pict, BYTE* pData, int width, int height,int yPitch,int uvPitch);
	void SaveFrame(AVFrame *pFrame, int width, int height, int iFrame);
	void YV12ToAVFrame(BYTE * pData,int nLen,int nWidth,int nHeight,AVFrame * pFrame);
	bool GetDiskSpace(wchar_t *strDir,DWORD *nFreeDisk);
	
private:
	
	BYTE *video_buf;
	unsigned int allocated_audio_out_size;
	unsigned int allocated_audio_buf_size;
	uint8_t *audio_buf;
	uint8_t *audio_out;
	int video_sync_method;
	AVOutputStream *ost;
	BITMAPINFOHEADER video_fmt;
	WAVEFORMATEX audio_fmt;
	REFERENCE_TIME nShizhuanDur;
	BYTE *m_pExtradata;
	int  m_nExtradataLen;
	//double m_vBitrate;
public:
	BOOL m_bThreadActive;
	BOOL m_bCanDecoder;
	BOOL m_bVideo;
	BOOL m_bAudio;
};

class CFFmpegMuxFilter
	: public CBaseFilter
	, public IFileSinkFilter
	, public CCritSec
	, public CAMThread
	, public IFFmpegMux
	, public IQvodCallBack
{
protected:
	CAutoPtrList<CFFmpegMuxInputPin>  m_pInputs;
	REFERENCE_TIME m_rtCurrent;
	enum {CMD_EXIT, CMD_RUN};
	DWORD ThreadProc();

public:
	CCritSec m_csQueue;
	bool m_bEnableAudio;
	bool m_bError;
	static CUnknown * WINAPI CreateInstance(LPUNKNOWN lpunk, HRESULT *phr);
	CFFmpegMuxFilter(LPUNKNOWN pUnk, HRESULT* phr);
	virtual ~CFFmpegMuxFilter();

	DECLARE_IUNKNOWN;
	STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void** ppv);

	int GetPinCount();
	CBasePin* GetPin(int n);

	STDMETHODIMP Stop();
	STDMETHODIMP Pause();
	STDMETHODIMP Run(REFERENCE_TIME tStart);

	// IFFmpegMux
	STDMETHODIMP SetMuxConfig(MUX_CONFIG* pMuxConfig);
	STDMETHODIMP GetMuxConfig(MUX_CONFIG* pMuxConfig);
	STDMETHODIMP SetMuxThreadCount(DWORD Threadcount) ;
	STDMETHODIMP GetMuxThreadCount(DWORD &Threadcount) ;
	STDMETHODIMP SetMuxDuration(REFERENCE_TIME* rtStart, REFERENCE_TIME* rtStop);
	STDMETHODIMP GetMuxDuration(REFERENCE_TIME* rtStart, REFERENCE_TIME* rtStop) ;
	STDMETHODIMP SetSrcFrameRate(float framerate);
	STDMETHODIMP SetQvodCallBack(interface IQvodCallBack* pQvodCallBack);
	STDMETHODIMP GetDuration ( LONGLONG *lt) ;

	// ******************************************
	// ***** IFileSinkFilter Implementation *****
	// ******************************************
	STDMETHODIMP SetFileName(LPCOLESTR inFileName, const AM_MEDIA_TYPE* inMediaType);
	STDMETHODIMP GetCurFile(LPOLESTR* outFileName, AM_MEDIA_TYPE* outMediaType);
	
public:
	AVFormatContext * GetContext();
	AVOutputStream * GetOutStream(int num);
	HRESULT OutputFile(const char *filename);
	 
	void SetWidth(int *width);
	void SetHeight(int *height);
	void GetFileDuration(REFERENCE_TIME *ll);

	void AddInput();
	long EventNotify_(UINT uMsg, long wparam, long lparam);
	IQvodCallBack *GetQvodCallBack();
	REFERENCE_TIME GetFileDuration();
	REFERENCE_TIME GetShizhuanDuration();
	wchar_t *GetFileURL();



public:
	int m_nStreamNum;
	


private:
	void NewVideoStream(AVFormatContext *oc);
	void NewAudioStream(AVFormatContext *oc);
	enum CodecID find_codec_or_die(const char *name, int type, int encoder, int s);
	void choose_pixel_fmt(AVStream *st, AVCodec *codec);
	void choose_sample_fmt(AVStream *st, AVCodec *codec);
	void choose_sample_rate(AVStream *st, AVCodec *codec);
	void ReleaseSource();
	BOOL IsAsc(char *pSource, int nlen);


private:
	wchar_t mFileName[512];
	char m_sFilePath[MAX_PATH];
	AVFormatContext *m_oc;

	int audio_stream_copy ;
	int video_stream_copy ;

	int audio_able;
	int video_able ;

	float mux_preload;
	float mux_max_delay;
	int loop_output ;

	AVRational frame_rate;

	 int frame_width  ;
	 int frame_height ;
	 int audio_sample_rate ;
	 int audio_channels ;

	 const char **opt_names;
	 int opt_name_count;
	 int thread_count;
	 PixelFormat frame_pix_fmt;
	 int video_global_header;
	 float frame_aspect_ratio ;
	 int frame_padtop;
	 int frame_padbottom ;
	 int frame_padleft  ;
	 int frame_padright ;

	 int me_threshold;
     int intra_dc_precision ;

     SampleFormat audio_sample_fmt;
     int64_t channel_layout;
	   
	 int64_t extra_size;
	 AVOutputStream *ost, **ost_table ;

	 int m_Width;
	 int m_Height;
	 IQvodCallBack *m_QvodCallBack; 
	 double  m_Progress;
	 REFERENCE_TIME m_rtDur;
	 REFERENCE_TIME m_TransStopTime;
	 MUX_CONFIG m_TransConfig; 
	 AVDictionary  *video_opts;
	 unsigned int sws_flags ;
	 SwsContext *sws_opts; 
	 REFERENCE_TIME vtemp;
	 REFERENCE_TIME atemp ;
	 CAutoPtrList<BLOCK>  VideoBlock; 
	 CAutoPtrList<BLOCK>  AudioBlock; 
	 CFFmpegMuxInputPin  *pVideoPin ;
	 CFFmpegMuxInputPin  *pAudioPin;
	 float m_vBitrate;
	 
};

