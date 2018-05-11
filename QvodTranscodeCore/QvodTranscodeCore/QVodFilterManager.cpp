#include "stdafx.h"
#include "TranscodeStruct.h"
#include "QvodFilterManager.h"
#include "commonfunction.h"
#include <string>
#include <fstream>
#include "xUtility.h"
#include "xXmlparse.h"
using namespace std;


extern CLSID	CStringToClsid(CString strClsid);
extern CString CLSID2String(const CLSID& clsid);
extern void	CStringtoBin(CString str, BYTE *pdata);


//加密解密算法

#define ENCRYPTION

//const char key[32] = {填写一个32位数的数组,随机顺序,范围0< x <32};
const unsigned int g_keys[32] = {2,4,19,31,21,16,7,30,9,18,23,28,1,5,9,19,29,14,12,2,16,23,27,29,17,6,4,14,17,21,22,23};
const unsigned int g_key2 = 16;

void Enc2(const BYTE* pSrc,BYTE* pDes,int len,unsigned int key2)
{
	int i,j,n=0;
	unsigned int k;

	memcpy(pDes,pSrc,len);
	BYTE* pchr = pDes;
	UINT* puint = (UINT*)pchr;
	k = key2;
	for(i = 0;i<len/4;i++)
	{
		puint[i] = (puint[i]>>g_keys[n]) + (puint[i]<<(32-g_keys[n]));
		puint[i] ^= k;
		k = puint[i];
		n = pchr[3+i*4]%32;
	}
	BYTE* pkey2 = (BYTE*)&key2;
	n%=8;
	for(j = i*4;j<len;j++)
	{
		pchr[j] = (pchr[j] >> n) + (pchr[j] << (8-n));
		pchr[j] ^= *pkey2++;
		n = pchr[j]%8;
	}

}

void Dec2(const BYTE* pSrc,BYTE* pDes,int len,unsigned int key2)
{
	int i,j,nn,n = 0;
	BYTE* pchr = pDes;
	memcpy(pchr,pSrc,len);
	UINT k,nk;
	k = key2;

	UINT* puint = (UINT*)pchr; 
	for(i = 0;i<len/4;i++)
	{
		nn = pchr[3+i*4]%32;
		nk = puint[i];
		puint[i] ^= k;
		puint[i] = (puint[i]<<g_keys[n]) + (puint[i]>>(32-g_keys[n]));
		n = nn;
		k = nk;
	}
	BYTE* pkey2 = (BYTE*)&key2;
	n%=8;
	for(j = i*4;j<len;j++)
	{
		nn = pchr[j]%8;
		pchr[j] ^= *pkey2++;
		pchr[j] = (pchr[j] << n) + (pchr[j] >> (8-n));
		n = nn;
	}

} 





//////////////////////////////////////////////////////////////////////////////////////////////
QvodFilterManager& QvodFilterManager::GetInstance()
{
	static QvodFilterManager _manager;
	return _manager;
}

QvodFilterManager::QvodFilterManager(void)
: mbLoaded(false)
{
	TCHAR current_path[512];
	if(GetQvodPlayerPath(current_path))
	{
		//LoadXMLs(L"");
		LoadXMLs(current_path);
	}
}


bool QvodFilterManager::LoadXMLs(CString path)
{
	if (!mbLoaded)
	{
		mbLoaded = true;
		if (!LoadDataFromXML(path + L"\\Filters.xml"))
		{
			mbLoaded = false;
			X_ASSERT(0);
		}

	}
	return true;
}

#define ENCRYPTION
bool QvodFilterManager::LoadDataFromXML(const CString& path)
{
	// 打开文件
	std::ifstream tfile(path,ios::in|ios::binary);
 	if (!tfile.is_open())
 	{
 		return false;
	}

	// 读取数据到buffer中
	tfile.seekg(0, ios_base::end);
	int nFileLen = tfile.tellg();
	BYTE* src = new BYTE[nFileLen+1];
	memset(src, 0 , nFileLen+1);
	tfile.seekg(0, ios_base::beg);
	tfile.read((char*)src, nFileLen);

	X_ASSERT(nFileLen == tfile.gcount());
	if (nFileLen != tfile.gcount())
	{
		delete[] src;
		return false;
	}

#ifdef ENCRYPTION
	// 解密Buffer
	BYTE* dst = new BYTE[nFileLen+1];
	if (!dst)
	{
		delete[] src;
		return false;
	}
	memset(dst, 0 , nFileLen+1);
	Dec2(src, dst, nFileLen, g_key2);

	delete[] src;
	src = dst;
#endif
	
	bool bOK = ParserXML(src);
 	
  	delete[] src;	
  	return bOK;
}



HMODULE   GetModuleFromAddr(PVOID   p) 
{ 
	MEMORY_BASIC_INFORMATION   m   =   {0}; 
	VirtualQuery(p,   &m,   sizeof(MEMORY_BASIC_INFORMATION)); 
	return   (HMODULE)   m.AllocationBase; 
} 

EXTERN_C BOOL WINAPI DllMain( HINSTANCE hInstance, DWORD dwReason, LPVOID);


bool QvodFilterManager::ParserXML(BYTE* src)
{
	//CoInitialize(NULL);
	CXMLDoc doc;
	bool bOK = doc.Init_();
	if(!bOK)
	{
		return false;
	}
	bOK = doc.LoadXml_((const char*)src);
	if (bOK)
	{
		CXMLElement xmlRoot;
		if(doc.GetDocumentElement_(xmlRoot))
		{
			bOK = true;
		}

		CXMLElement xmlFilters;
		if(xmlRoot.SelectSingleNode_(_T("QvodFilter"), xmlFilters))
		{
			bOK = LoadXML_Filters(&xmlFilters);
		}

		CXMLElement xmlBestGraph;
		if(xmlRoot.SelectSingleNode_(_T("BestGraph"), xmlBestGraph))
		{
			bOK = LoadXML_BestGraph(&xmlBestGraph);
		}

		CXMLElement xmlAccel;
		if(xmlRoot.SelectSingleNode_(_T("solution "), xmlAccel))
		{
			bOK = LoadXML_AccelSolution(&xmlAccel);
		}

		CXMLElement xmlMaps;
		if(xmlRoot.SelectSingleNode_(_T("FilterMaps"), xmlMaps))
		{
			bOK = LoadXML_FilterMap(&xmlMaps);
		}
	}
	else
	{
		OutputDebugString(L"PARSER XML ERROR!!!!!!!!!!");
		bOK = false;
	}
	return bOK;
}

// bool QvodFilterManager::ReadFromBin()
// {
// 	HMODULE hMod = GetModuleFromAddr(DllMain);
// 
// 	HRSRC hRsrc = FindResource(hMod, MAKEINTRESOURCE(IDR_XML1),L"XML");
// 	if (NULL == hRsrc)
// 		return false;
// 
// 	//获取资源的大小
// 	DWORD dwSize = SizeofResource(hMod, hRsrc);
// 	if (0 == dwSize)
// 		return false;
// 
// 	//加载资源
// 	HGLOBAL hGlobal = LoadResource(hMod, hRsrc);
// 	if (NULL == hGlobal)
// 		return false;
// 
// 	//锁定资源
// 	LPVOID pBuffer = LockResource(hGlobal);
// 	if (NULL == pBuffer)
// 		return false;
// 
// 	//读取XML
// 	BYTE* src = new BYTE[dwSize+1];
// 	memset(src, 0, dwSize +1);
// 	memcpy(src, pBuffer, dwSize);
// 
// 	bool bOK = ParserXML(src);
// 
// 	delete [] src;
// 
// 	return bOK;
// }

bool QvodFilterManager::LoadXML_Filters(CXMLElement* xmlViews)
{
	CXMLNodeList viewlist;
	xmlViews->GetChildNodes_(viewlist);
	long count = viewlist.GetItemCount_();
	for(long i=0; i<count; i++)
	{
		CXMLElement xmlView;
		if(!viewlist.GetItemDisp_(i, xmlView.m_Node))
		{
			continue;
		}

		if (xmlView.GetTagName_().IsEmpty())
		{
			continue;
		}

		QVOD_FILTER_INFO node;

		node.bEnable = xmlView.GetAttributeInt_(_T("enable"), 0);
		node.strName = xmlView.GetAttribute_(_T("name"));

		 //路径
		node.path = xmlView.GetAttribute_(L"path");
		 
		// Filter类型
		CString  strCapacity = xmlView.GetAttribute_(L"capacity");
		if (!strCapacity.IsEmpty())
		{
			//char output= (char*)strCapacity.GetBuffer(strCapacity.GetLength());
			node.capacity = _ttoi(strCapacity);
		}
		 
		// CLSID
		CString strCLSID = xmlView.GetAttribute_(L"clsid");
		node.clsid = CStringToClsid(strCLSID);
		 
		// Vista enbaled
		CString _system = xmlView.GetAttribute_(L"system");
		if (_system.IsEmpty())
			_system = "0";
		node.eSystem = (e_Filter_System_Apply)_ttoi(_system.GetBuffer(_system.GetLength()));

		// DMO
		CString strDmo = xmlView.GetAttribute_(L"dmo");
		if (strDmo == "true")
		{
			node.bDMO = true;
			// WRAP ID
			node.wrapid = CStringToClsid(xmlView.GetAttribute_(L"wrapperid"));

			// CATE ID
			node.catid = CStringToClsid(xmlView.GetAttribute_(L"catid"));

			// WRAP PATH
			node.wrapperpath = xmlView.GetAttribute_(L"wrapperpath");
		}


		// 
		CXMLNodeList viewlist2;
		xmlView.GetChildNodes_(viewlist2);
		for (int  j =0; j < viewlist2.GetItemCount_(); j++)
		{
			CXMLElement xmlView2;
			if(!viewlist2.GetItemDisp_(j, xmlView2.m_Node))
			{
				continue;
			}
			
			CString name = xmlView2.GetTagName_();
			if(name.IsEmpty())
			{
				continue;
			}

			if (name == "components")
			{
				_ComponentInfo info;
				info.path = xmlView2.GetAttribute_(L"path");

				CString strReg = xmlView2.GetAttribute_(L"reg");
				if (strReg == "true")
				{
					info.bReg = true;
				}

				info.clsid = CStringToClsid(xmlView2.GetAttribute_(L"clsid"));

				node.vComponents.push_back(info);
			}
			else if(name == "intype")
			{
				// 输入媒体类型
				_MediaType type;
				type.major = CStringToClsid(xmlView2.GetAttribute_(L"major"));
				type.sub = CStringToClsid(xmlView2.GetAttribute_(L"sub"));
				CString pri = xmlView2.GetAttribute_(L"pri");
				if (pri.IsEmpty())
					pri = "0";
				type.priority =  _ttoi(pri.GetBuffer(pri.GetLength()));
				node.vInTypes.push_back(type);
			}
// 			else if(name == "priority")
// 			{
// 				// 解码优先级
// 				QVOD_FILTER_DECODER_PRIORITY p;
// 				p.encode = VideoEncoderFromString(xmlView2.GetAttribute_(L"enc"));
// 				CString pri = xmlView2.GetAttribute_(L"pri");
// 				p.priority =  _ttoi(pri.GetBuffer(pri.GetLength()));
// 
// 				node.vDecoderPriority.push_back(p);
// 			}
		}
		mFilters.push_back(node);
	}
	return !mFilters.empty();
}


bool QvodFilterManager::LoadXML_AccelSolution(CXMLElement*xmlViews)
{
 	QVOD_ACCEL_SOLUTION solution;

	CXMLNodeList viewlist;
	xmlViews->GetChildNodes_(viewlist);
	long count = viewlist.GetItemCount_();
	for(long i=0; i<count; i++)
	{
		CXMLElement xmlView;
		if(!viewlist.GetItemDisp_(i, xmlView.m_Node))
		{
			continue;
		}

		CString name = xmlView.GetTagName_();
		if (name.IsEmpty())
		{
			continue;
		}

		if (name == "Container")
		{
			CXMLNodeList viewlist2;
			xmlView.GetChildNodes_(viewlist2);
			long count = viewlist2.GetItemCount_();
			for(long j=0; j<count; j++)
			{
				CXMLElement xmlView3;
				if(!viewlist2.GetItemDisp_(j, xmlView3.m_Node))
				{
					continue;
				}

				if (xmlView3.GetTagName_().IsEmpty())
				{
					continue;
				}
			
				QVOD_CONTAINER_TO_SRC c;
				CString container = xmlView3.GetTagName_();
				c.container = ContainerFromString(container);

				CXMLNodeList viewlist3;
				xmlView3.GetChildNodes_(viewlist3);
				long count = viewlist3.GetItemCount_();
				for(long k=0; k<count; k++)
				{

					CXMLElement xmlView4;
					if(!viewlist3.GetItemDisp_(k, xmlView4.m_Node))
					{
						continue;
					}

					if (xmlView4.GetTagName_().IsEmpty())
					{
						continue;
					}

					CLSID clsid = CStringToClsid(xmlView4.GetAttribute_(L"clsid"));
					CString strType = xmlView4.GetTagName_();
					if (strType == "Source")
						c.vSource.push_back(clsid);
					else
						c.vSplitters.push_back(clsid);
				}

				solution.vContainers.push_back(c);
			}

		}
		else if(name == "Video")
		{
			CXMLNodeList viewlist2;
			xmlView.GetChildNodes_(viewlist2);
			long count = viewlist2.GetItemCount_();
			for(long j=0; j<count; j++)
			{
				CXMLElement xmlView3;
				if(!viewlist2.GetItemDisp_(j, xmlView3.m_Node))
				{
					continue;
				}

				if (xmlView3.GetTagName_().IsEmpty())
				{
					continue;
				}

				QVOD_VIDEO_DECODER d;
 				CString cname = xmlView3.GetTagName_();
 				if (cname == "MPEG2")
 					d.encoder = _V_MPEG_2;
 				else if(cname == "H264")
 					d.encoder = _V_H264;
 				else if(cname == "VC-1")
 					d.encoder = _V_WVC1;

				CXMLElement decoder;
				if(xmlView3.SelectSingleNode_(L"Decoder", decoder))
				{
					CLSID clsid = CStringToClsid(decoder.GetAttribute_(L"clsid"));
					d.vDecoders.push_back(clsid);
				}

				solution.vVDecoders.push_back(d);
			}
		}
		else if(name == "Render")
		{
			CXMLNodeList viewlist2;
			xmlView.GetChildNodes_(viewlist2);
			long count = viewlist2.GetItemCount_();
			for(long j=0; j<count; j++)
			{
				CXMLElement xmlView3;
				if(!viewlist2.GetItemDisp_(j, xmlView3.m_Node))
				{
					continue;
				}

				if (xmlView3.GetTagName_().IsEmpty())
				{
					continue;
				}

				CLSID clsid = CStringToClsid(xmlView3.GetAttribute_(L"clsid"));
				solution.vRenders.push_back(clsid);		
			}
		}
	}
	mAccelSolutions.push_back(solution);
	return !mAccelSolutions[0].vContainers.empty() && !mAccelSolutions[0].vVDecoders.empty();

// 	solution.solution_name = root->Attribute("name");
// 	solution.solution_comment = root->Attribute("comment");
// 		
// 	
// 	TiXmlElement * pCategory = root->FirstChildElement();
// 	while(pCategory)
// 	{		
// 		CString catename = pCategory->Value();
// 		if (catename == "Container")
// 		{
// 			TiXmlElement * it = pCategory->FirstChildElement();
// 			while(it)
// 			{
// 				QVOD_CONTAINER_TO_SRC c;
// 				c.container = ContainerFromString(it->Value());
// 				TiXmlElement * it2 = it->FirstChildElement();
// 				while(it2)
// 				{
// 					CLSID clsid = CStringToClsid(it2->Attribute("clsid"));
// 					CString strType = it2->Value();
// 					if (strType == "Source")
// 						c.vSource.push_back(clsid);
// 					else
// 						c.vSplitters.push_back(clsid);
// 					
// 					it2 = it2->NextSiblingElement();
// 				}
// 
// 				solution.vContainers.push_back(c);
// 
// 				it = it->NextSiblingElement();
// 			}
// 		}
// 		else if(catename == "Video")
// 		{
// 			
// 			TiXmlElement * it = pCategory->FirstChildElement();
// 			while(it)
// 			{
// 				QVOD_VIDEO_DECODER d;
// 				CString cname = it->Value();
// 				if (cname == "MPEG2")
// 					d.encoder = _V_MPEG_2;
// 				else if(cname == "H264")
// 					d.encoder = _V_AVC;
// 				else if(cname == "VC-1")
// 					d.encoder = _V_VC1;
// 			
// 				TiXmlElement * it2 = it->FirstChildElement();
// 				while(it2)
// 				{
// 					CLSID clsid = CStringToClsid(it2->Attribute("clsid"));
// 					d.vDecoders.push_back(clsid);
// 				
// 					it2 = it2->NextSiblingElement();
// 				}
// 
// 				solution.vVDecoders.push_back(d);
// 
// 				it = it->NextSiblingElement();
// 			}
// 		}
// 		else if(catename == "Render")
// 		{
// 			TiXmlElement * it = pCategory->FirstChildElement();
// 			while(it)
// 			{
// 				CLSID clsid = CStringToClsid(it->Attribute("clsid"));
// 				solution.vRenders.push_back(clsid);		
// 				it = it->NextSiblingElement();
// 			}
// 		}
// 
// 		pCategory = pCategory->NextSiblingElement();
// 	}
// 
// 	mAccelSolutions.push_back(solution);
// 	


}

bool QvodFilterManager::LoadXML_FilterMap(CXMLElement*xmlViews)
{

	CXMLNodeList viewlist;
	xmlViews->GetChildNodes_(viewlist);
	long count = viewlist.GetItemCount_();
	for(long i=0; i<count; i++)
	{
		CXMLElement xmlView;
		if(!viewlist.GetItemDisp_(i, xmlView.m_Node))
		{
			continue;
		}

		CString name = xmlView.GetTagName_();
		if (name.IsEmpty())
		{
			continue;
		}
		
		QVOD_FILTER_MAP	node;
		CString value = xmlView.GetAttribute_(L"value");
		node.container = ContainerFromString(value);


		CXMLNodeList viewlist2;
		xmlView.GetChildNodes_(viewlist2);
		int count = viewlist2.GetItemCount_();
		for(int j=0; j<count; j++)
		{
			CXMLElement xmlView2;
			if(!viewlist2.GetItemDisp_(j, xmlView2.m_Node))
			{
				continue;
			}

			CString name = xmlView2.GetTagName_();
			if (name.IsEmpty())
			{
				continue;
			}

			name.MakeUpper();
 			if (name == "CB")
 			{
 				node.vCheckBytes.push_back(xmlView2.GetAttribute_(L"str"));
 			}
 			else if(name == "EXT")
 			{
 				node.vSuffix.push_back(xmlView2.GetAttribute_(L"str"));
 			}
 			else if(name == "FILTER")
 			{
 				node.vectFilters.push_back(CStringToClsid(xmlView2.GetAttribute_(L"clsid")));
 			}
		}
		mFilterMaps.push_back(node);
	}
	return !mFilterMaps.empty();

// 	TiXmlElement * pCategory = root->FirstChildElement();
// 	while(pCategory)
// 	{
// 		QVOD_FILTER_MAP	node;
// 		
// 		node.container = ContainerFromString(CString(pCategory->Attribute("value")));
// 
// 		TiXmlElement* pElement = pCategory->FirstChildElement();
// 		while(pElement)
// 		{
// 			CString name = pElement->Value();
// 			name.MakeUpper();
// 			if (name == "CB")
// 			{
// 				node.vCheckBytes.push_back(pElement->Attribute("str"));
// 			}
// 			else if(name == "EXT")
// 			{
// 				node.vSuffix.push_back(pElement->Attribute("str"));
// 			}
// 			else if(name == "FILTER")
// 			{
// 				node.vectFilters.push_back(CStringToClsid(pElement->Attribute("clsid")));
// 			}
// // 			else if(name == "SPLITTER")
// // 			{
// // 				node.vSplitter.push_back(CStringToClsid(pElement->Attribute("clsid")));
// // 			}
// // 			else if(name == "DECODER")
// // 			{
// // 				if (CString(pElement->Attribute("video")) == "true")
// // 				{
// // 					QVOD_VIDEO_DECODER n;
// // 					n.encoder = VideoEncoderFromString(pElement->Attribute("codec"));
// // 					n.vDecoders.push_back(CStringToClsid(pElement->Attribute("clsid")));
// // 					node.vVDecoders.push_back(n);
// // 				}
// // 				else
// // 				{
// // 					QVOD_AUDIO_DECODER n;
// // 					n.encoder = AudioEncoderFromString(pElement->Attribute("codec"));
// // 					n.vDecoders.push_back(CStringToClsid(pElement->Attribute("clsid")));
// // 					node.vADecoders.push_back(n);
// // 				}
// // 			}
// 			pElement = pElement->NextSiblingElement();
// 		}
// 
// 		mFilterMaps.push_back(node);
// 		pCategory = pCategory->NextSiblingElement();
// 	}

}	

bool QvodFilterManager::LoadXML_BestGraph(CXMLElement*xmlViews)
{
	CXMLNodeList viewlist;
	xmlViews->GetChildNodes_(viewlist);
	long count = viewlist.GetItemCount_();
	for(long i=0; i<count; i++)
	{
		CXMLElement xmlView;
		if(!viewlist.GetItemDisp_(i, xmlView.m_Node))
		{
			continue;
		}

		CString name = xmlView.GetTagName_();
		if (name.IsEmpty())
		{
			continue;
		}

		INNER_BEST_GRAPH node;
		node.container = ContainerFromString(xmlView.GetAttribute_(L"container"));
		node.vencoder = VideoEncoderFromString(xmlView.GetAttribute_(L"video"));
		node.aencoder = AudioEncoderFromString(xmlView.GetAttribute_(L"audio"));
		
		CXMLElement xmlView2;
		if(xmlView.SelectSingleNode_(L"Solution", xmlView2))
		{
			CXMLNodeList viewlist2;
			xmlView2.GetChildNodes_(viewlist2);
			int count = viewlist2.GetItemCount_();
			for(int j=0; j<count; j++)
			{
				CXMLElement xmlView2;
				if(!viewlist2.GetItemDisp_(j, xmlView2.m_Node))
				{
					continue;
				}

				CString name = xmlView2.GetTagName_();
				if (name.IsEmpty())
				{
					continue;
				}

// 				CXMLNodeList viewlist3;
// 				xmlView2.GetChildNodes_(viewlist3);
// 				long count = viewlist3.GetItemCount_();
// 				for(long k=0; k<count; k++)
// 				{
// 					CXMLElement xmlView3;
// 					if(!viewlist3.GetItemDisp_(i, xmlView3.m_Node))
// 					{
// 						continue;
// 					}
// 
// 					CString name = xmlView3.GetTagName_();
// 					if (name.IsEmpty())
// 					{
// 						continue;
// 					}

				CLSID clsid = CStringToClsid(xmlView2.GetAttribute_(L"clsid"));
				node.vectFilters.push_back(clsid);

				//}
				

			}
			mBestGraphs.push_back(node);
		}
		

	}
	return !mBestGraphs.empty();
	//TiXmlElement * pNode = root->FirstChildElement();
	//while(pNode)
	//{
	//	INNER_BEST_GRAPH node;
	//	node.container = ContainerFromString(pNode->Attribute("container"));
	//	node.vencoder = VideoEncoderFromString(pNode->Attribute("video"));
	//	node.aencoder = AudioEncoderFromString(pNode->Attribute("audio"));
	//	
	//	// 获取优先级最高方案（第一个方案）
	//	TiXmlElement* pSolution = pNode->FirstChildElement();
	//	if(pSolution)
	//	{
	//		TiXmlElement* pElement = pSolution->FirstChildElement();
	//		while(pElement)
	//		{
	//			CLSID clsid = CStringToClsid(pElement->Attribute("clsid"));
	//			node.vectFilters.push_back(clsid);

	//			pElement = pElement->NextSiblingElement();
	//		}	
	//		mBestGraphs.push_back(node);
	//	}
	//	pNode = pNode->NextSiblingElement();
	//}
	
}

// 获取默认加速方案
bool QvodFilterManager::GetAccelSolution_Default(QVOD_ACCEL_SOLUTION& solution) const
{
	if (mAccelSolutions.size())
	{
		solution = mAccelSolutions[0];
		return true;
	}
	return false;
}

// 获取可用的所有加速方案
bool QvodFilterManager::GetAccelSolutionList(std::vector<QVOD_ACCEL_SOLUTION>& solutions) const
{
	solutions = mAccelSolutions;
	return true;
}

// 获取指定加速方案
bool QvodFilterManager::GetAccelSolutionByName(const CString& name,QVOD_ACCEL_SOLUTION& solution) const
{
	for (int i = 0; i < mAccelSolutions.size(); i++)
	{
		if (mAccelSolutions[i].solution_name == name)
		{
			solution = mAccelSolutions[i];
			return true;
		}
	}
	return false;
}

// inline CLSID QvodFilterManager::GET_FILTER_FROM_VECT_BY_CAPACITY(const std::vector<CLSID>& filters, DWORD c) const
// {
// 	for(int i=0; i<filters.size(); i++)
// 	{
// 		if(GetFilterCapacity(filters[i]) == c) 
// 		{
// 			return filters[i];
// 		}
// 	}
// 	return CLSID_NULL;
// }
	
bool QvodFilterManager::GetBestGraph(E_CONTAINER c, E_V_CODEC ve, E_A_CODEC ae, std::vector<CLSID>& node) const
{
	for (int i = 0; i < mBestGraphs.size(); i++)
	{
		if (c == mBestGraphs[i].container)
		{
			if ((ve != _V_OTHER && ae != _A_OTHER && mBestGraphs[i].vencoder == ve && mBestGraphs[i].aencoder == ae) ||
				(ve != _V_OTHER && ae == _A_OTHER && mBestGraphs[i].vencoder == ve) ||
				(ve == _V_OTHER && ae != _A_OTHER && mBestGraphs[i].aencoder == ae))
			{
				node = mBestGraphs[i].vectFilters;

// 				node.source = GET_FILTER_FROM_VECT_BY_CAPACITY(mBestGraphs[i].vectFilters, E_FILTER_CAPACITY_READ | E_FILTER_CAPACITY_SPLIT);
// 				node.splitter = GET_FILTER_FROM_VECT_BY_CAPACITY(mBestGraphs[i].vectFilters, E_FILTER_CAPACITY_SPLIT);
// 				node.vdecoder = GET_FILTER_FROM_VECT_BY_CAPACITY(mBestGraphs[i].vectFilters, E_FILTER_CAPACITY_VIDEO_DEC);
// 				node.adecoder = GET_FILTER_FROM_VECT_BY_CAPACITY(mBestGraphs[i].vectFilters, E_FILTER_CAPACITY_AUDIO_DEC);
				return true;
			}
		}
	}
	return false;
}

// 获取Filter
bool QvodFilterManager::GetFilterByCLSID(const CLSID& clsid, QVOD_FILTER_INFO& node) const
{
	CString strClsid = CLSID2String(clsid);
	strClsid.MakeUpper();

	std::vector<QVOD_FILTER_INFO>::const_iterator pos = mFilters.begin();
	for (; pos != mFilters.end(); pos++)
	{
		if (clsid == pos->clsid)
		{
			node = *pos;
			return true;
		}
	}
	return false;
}

void QvodFilterManager::GetFilterByCapacity(DWORD capacity, std::vector<CLSID>& vectClsid) const
{
	vectClsid.clear();
	std::vector<QVOD_FILTER_INFO>::const_iterator it = mFilters.begin();
	for (; it != mFilters.end(); it++)
	{
		if (it->capacity & capacity)
		{
			vectClsid.push_back(it->clsid);
		}
	}
}

DWORD QvodFilterManager::GetFilterPriority(const CLSID& clsid, QVOD_MEDIA_TYPE& mt) const
{
	std::vector<QVOD_FILTER_INFO>::const_iterator pos = mFilters.begin();
	for (; pos != mFilters.end(); pos++)
	{
		if (clsid == pos->clsid)
		{
			for (int i  = 0; i < pos->vInTypes.size(); i++)
			{
				if (mt.sub == pos->vInTypes[i].sub)
				{
					return pos->vInTypes[i].priority;
				}
			}
		}
	}
	return 0;
}

void QvodFilterManager::GetFilterByMediaType(DWORD capacity, QVOD_MEDIA_TYPE& type, std::vector<CLSID>& vectClsid) const
{
	vectClsid.clear();
	std::vector<QVOD_FILTER_INFO>::const_iterator it = mFilters.begin();
	for (; it != mFilters.end(); it++)
	{
		if (it->capacity & capacity)
		{
			for (int i = 0; i < it->vInTypes.size(); i++)
			{
				if (it->vInTypes[i].major == type.major && it->vInTypes[i].sub == type.sub)
				{
					vectClsid.push_back(it->clsid);
					break;
				}
			}
		}
	}
}

// 获取Filter Capacity
DWORD QvodFilterManager::GetFilterCapacity(const CLSID& clsid) const
{
	LPOLESTR str;
	StringFromCLSID(clsid,&str);
	CString strClsid = str;
	CoTaskMemFree(str);

	std::vector<QVOD_FILTER_INFO>::const_iterator it = mFilters.begin();
	for (; it != mFilters.end(); it++)
	{
		if (clsid == it->clsid)
		{
			return it->capacity;
		}
	}	
	return 0;
}

const std::vector<QVOD_FILTER_MAP>& QvodFilterManager::GetFilterMaps() const
{ 
	return mFilterMaps;
}