
/********************************************************************
	created:	2013/11/07
	created:	7:11:2013   15:16
	filename: 	e:\my_work\project\opencv_project\QvodH265Encoder_2008\QvodH265Encoder\IQVODH265Encoder.h
	file path:	e:\my_work\project\opencv_project\QvodH265Encoder_2008\QvodH265Encoder
	file base:	IQVODH265Encoder
	file ext:	h
	
	purpose:	
*********************************************************************/

#ifndef IQVODH265Encoder_h__
#define IQVODH265Encoder_h__



#ifdef __cplusplus
extern "C"{
#endif

	// {213A2AE1-4212-4529-A7DC-2F3927C6081A}
	static const GUID IID_IQVODH265Encode = 
	{ 0x213a2ae1, 0x4212, 0x4529, { 0xa7, 0xdc, 0x2f, 0x39, 0x27, 0xc6, 0x8, 0x1a } };


	static const GUID IID_IScale = 
	{ 0x475c34fc, 0x1f9c, 0x4564, { 0xae, 0xf1, 0xae, 0x71, 0xa1, 0xc, 0xa1, 0x60 } };

	DECLARE_INTERFACE_(IScale, IUnknown)
	{
		STDMETHOD(SetScale)(int widht_out,int height_out)	PURE;
	};



#define QVODH265_FRAMERATE_60		26
#define QVODH265_FRAMERATE_5994		27
#define QVODH265_FRAMERATE_50		28
#define QVODH265_FRAMERATE_30		29
#define QVODH265_FRAMERATE_2997		30
#define QVODH265_FRAMERATE_25		31
#define QVODH265_FRAMERATE_24		32
#define QVODH265_FRAMERATE_2397		33
#define QVODH265_FRAMERATE_15		34
#define QVODH265_FRAMERATE_1225		35



	//speed
	enum  Speed
	{
		speed_ultrafast	,
		speed_superfast	,
		speed_veryfast	,
		speed_fast		,
		speed_medium	,
		speed_slow		,
		speed_slower	,
		speed_veryslow	,
		speed_placebo	,
		speed_zero	
	};

	//profile

	enum	Profile
	{
		main			,
		main10			,
		mainstillpicture
	};



	//tune
	enum	Tune
	{
		psnr			,			
		ssim			,		
		zero_latency	,
		Tune_zero
	};


	//RC-control mehtod

	enum	RC_method
	{
		ABR		,		
		CBR		,		
		CQP			
	};



DECLARE_INTERFACE_(IQVODH265ENCODER, IUnknown)
{


	STDMETHOD(SetBitrate) (THIS_	int Bitrate  ) PURE;   //设置码率
		
	STDMETHOD(Set_I_interval) (THIS_	int Num  ) PURE;   //设置I帧间隔

	STDMETHOD(SetRefNumber) (THIS_	int Ref ) PURE;   //设置参考帧

	STDMETHOD(subpelRefine) (THIS_	int level  ) PURE;   //设置level等

	STDMETHOD(SetEnableDeblock) (THIS_	BOOL flag  ) PURE;   //设置level等

	STDMETHOD(Set_B_Num) (THIS_	int	B_Num  ) PURE;   //设置B帧数目，默认3，最大<16

	STDMETHOD(SetEncoder_Speed_tune) (THIS_	enum  Speed	,	THIS_	enum	Tune	) PURE;   //设置level等

	STDMETHOD(SetProfile) (THIS_	enum	Profile  ) PURE;   //设置Profile等

	STDMETHOD(Set_default_Params_fast)()PURE;		//码率W*h*1.5*fps*8/500/2; 码率控制为CBR  速度最快，编码质量略有下降

	STDMETHOD(Set_default_Params_recommend)()PURE;		//码率W*h*1.5*fps*8/500/2; 码率控制为CBR  编码参数比较复杂，编码质量有所提升，编码时间比较长；

	STDMETHOD(Set_default_Params_slow)()PURE;

};

#ifdef __cplusplus
};
#endif

#endif // IQVODH265Encoder_h__