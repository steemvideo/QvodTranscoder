#pragma once

/*******************************************************************************
Copyright (C), 1988-1999, QVOD Technology Co.,Ltd.
File name:      QvodFilterManager.h
Version:      1.0.0
Date:2011-3-7 
Description:  ����ʵ��XML��ȡ������������Ⱦ��·�����Filter��Ϣ
Others:        
Function List:  GetFilterByClsid��
History:        
1. Date:
Author:
Modification:
********************************************************************************/



//<!-- Filter Capacity Bitλ˵�� -->
//<!-- ��һλ ��Filter�߱���ȡ������������SYS FileReader��QVodSource,DCBass Source, MIDI Parser -->
//<!-- �ڶ�λ ��Filter�߱���������������Splitter)-->
//<!-- ����λ ��Filter�߱���Ƶ����������Video Decoder�� -->
//<!-- ����λ ��Filter�߱���Ƶ����������Audio Decoder�� -->
//<!-- ����λ ��Filter�߱�Video Trans��������QvodPostVideo��QvodSubTitle,������ĻFilter�� -->
//<!-- ����λ ��Filter�߱�Audio Trans��������Qvod Sound Filter -->
//<!-- ����λ ��Filter�߱�Video Render��������CSF Render, FFMPEG Filter -->
//<!-- �ڰ�λ ��Filter�߱�Audio Render��������CSF Render, FFMPEG Filter -->
#define E_FILTER_CAPACITY_READ			0x01
#define E_FILTER_CAPACITY_SPLIT			0x02
#define E_FILTER_CAPACITY_VIDEO_DEC		0x04
#define E_FILTER_CAPACITY_AUDIO_DEC		0x08
#define E_FILTER_CAPACITY_VIDEO_TRANS	0x10
#define E_FILTER_CAPACITY_AUDIO_TRANS	0x20
#define E_FILTER_CAPACITY_VIDEO_RENDER  0x40
#define E_FILTER_CAPACITY_AUDIO_RENDER	0x80


// Filter�����������Ϣ
typedef struct _ComponentInfo
{
	_ComponentInfo():bReg(false)
	{

	}
	CString path;	// ���·��
	bool	bReg;	// �Ƿ���Ҫע��
	CLSID	clsid;	// ���ID
} COMPONENT_INFO;

// ý������
typedef struct _MediaType
{
	CLSID major;
	CLSID sub;
	CLSID format;
	int   priority;//���ȼ�
} QVOD_MEDIA_TYPE;

// typedef struct _Decoder_Priority
// {
// 	E_V_CODEC encode;
// 	DWORD		priority;
// 	_Decoder_Priority()
// 	{
// 		encode = _V_OTHER;
// 		priority = 5;
// 	}
// } QVOD_FILTER_DECODER_PRIORITY;

// Filter����
enum e_Filter_System_Apply
{
	e_Apply_All,
	e_Apply_XP_Only,
	e_Apply_Vista_Only,//Vista+
};
typedef struct _QvodFilterInfo
{
	_QvodFilterInfo()
	{
		bEnable	= true;
		capacity = 0;	
		clsid = wrapid = catid = CLSID_NULL;
		bDMO = false;
	}
	DWORD		capacity;		//Filter Capacity
	bool		bEnable;		//Filter�ɲ�����
	CString		strName;		//Filter����
	CString		path;			//Filter·��
	CString		wrapperpath;	//Wrapper path
	bool		bDMO;			//DMO Flag
	CLSID		clsid;			// CLSID
	CLSID		wrapid;			// wrap ID
	CLSID		catid;			// ����ID
	//DWORD		version;		// ϵͳ�汾��
	e_Filter_System_Apply eSystem;// ϵͳ����
	std::vector<COMPONENT_INFO>		vComponents;	//Filter�������
	std::vector<QVOD_MEDIA_TYPE>	vInTypes;	// ����ý������
	//std::vector<QVOD_FILTER_DECODER_PRIORITY>	vDecoderPriority;
} QVOD_FILTER_INFO;

// �������� -> Source & Splitter
typedef struct  _container_to_src
{
	E_CONTAINER			container;
	std::vector<CLSID>	vSource;
	std::vector<CLSID>	vSplitters;
} QVOD_CONTAINER_TO_SRC;

// �������� -> Decoder 
typedef struct _v_encoder_to_decoder
{
	E_V_CODEC			encoder;
	std::vector<CLSID>	vDecoders;
} QVOD_VIDEO_DECODER;

typedef struct _a_encoder_to_decoder
{
	E_A_CODEC			encoder;
	std::vector<CLSID>	vDecoders;
} QVOD_AUDIO_DECODER;

// ���ٽ����������
typedef struct QVOD_ACCEL_SOLUTION
{
	CString						solution_name;		// ��������
	CString						solution_comment;	// ��������

	std::vector<QVOD_CONTAINER_TO_SRC>	vContainers;		// ��������
	std::vector<QVOD_VIDEO_DECODER>		vVDecoders;		// ���ٽ�����
	std::vector<CLSID>					vRenders;		// ��Ⱦ��
} QVOD_ACCEL_SOLUTION;

// CheckByte��Suffix��Filters��ӳ��
typedef struct _FilterMap
{
	E_CONTAINER					container;
	std::vector<CString>		vCheckBytes;	// CheckByte
	std::vector<CString>		vSuffix;		// ��׺��
	std::vector<CLSID>			vectFilters;	// ʹ�õ�Filters
} QVOD_FILTER_MAP;

class TiXmlElement;

class CXMLElement;
// Filter������
class QvodFilterManager
{

protected:
	typedef struct _BestGraph
	{
		E_CONTAINER					container;		// ��������
		E_V_CODEC					vencoder;		// ��Ƶ��������
		E_A_CODEC					aencoder;		// ��Ƶ��������

		std::vector<CLSID>			vectFilters;	// ���ŷ���

	} INNER_BEST_GRAPH;

public:
	// ��ȡQvodFilterManagerʵ��
	static QvodFilterManager& GetInstance();
	
	bool	LoadXMLs(CString path);
	
	// ��ȡĬ�ϼ��ٷ���
	bool	GetAccelSolution_Default(QVOD_ACCEL_SOLUTION& solution) const;
	
	// ��ȡ���õ����м��ٷ���
	bool	GetAccelSolutionList(std::vector<QVOD_ACCEL_SOLUTION>& solutions) const;
	
	// ��ȡָ�����ٷ���
	bool	GetAccelSolutionByName(const CString& name,QVOD_ACCEL_SOLUTION& solution) const;

	// ��ȡָ���������͡�ָ��capacity��Filter
	void	GetFilterByMediaType(DWORD capacity, QVOD_MEDIA_TYPE& type, std::vector<CLSID>&) const;
	
	// ��ȡָ��capacity��Filter
	void	GetFilterByCapacity(DWORD capacity, std::vector<CLSID>& vectClsid) const;
	
	// ��ȡFilter�Ľ������ȼ�
	DWORD	GetFilterPriority(const CLSID& clsid, QVOD_MEDIA_TYPE& mt) const;

	// ��ȡFilter
	bool	GetFilterByCLSID(const CLSID& clsid, QVOD_FILTER_INFO& node) const;
	
	// ��ȡFilter Capacity
	DWORD	GetFilterCapacity(const CLSID& clsid) const;

	// �����������͡���Ƶ�������͡���Ƶ�������ͻ�ȡ������·
	bool	GetBestGraph(E_CONTAINER c, E_V_CODEC ve, E_A_CODEC ae, std::vector<CLSID>& filters) const;

	// ����CheckByte��Suffix��Filter��ӳ���
	const std::vector<QVOD_FILTER_MAP>& GetFilterMaps() const;
private:
	QvodFilterManager(void);
	
	bool		LoadDataFromXML(const CString& path);	// ��XML�ж�ȡ
//	bool		ReadFromBin();							// ����Դ�ļ�������������
	bool		ParserXML(BYTE* src);					// ����XML
	bool		LoadXML_Filters(CXMLElement* );
	bool		LoadXML_BestGraph(CXMLElement*);
	bool		LoadXML_AccelSolution(CXMLElement*);
	bool		LoadXML_FilterMap(CXMLElement*);
private:
	std::vector<QVOD_FILTER_INFO>		mFilters;
	std::vector<QVOD_ACCEL_SOLUTION>	mAccelSolutions;// ����������
	std::vector<QVOD_FILTER_MAP>		mFilterMaps;	// CheckByte/Suffix -> Filter CLSID
	std::vector<INNER_BEST_GRAPH>		mBestGraphs;	// ������·��
	bool								mbLoaded;
};
