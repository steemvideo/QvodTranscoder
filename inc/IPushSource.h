#pragma once
#include "IDataSource.h"
static GUID CLSID_PUSH_SOURCE = {0x16bcd081, 0x303d, 0x4d52, {0xb3, 0x16, 0xde, 0x89, 0x14, 0xdd, 0x6, 0x70}};

class IReceive;
static GUID IID_IPushSource = {'iqps', 0x9f0f, 0x4824, {0x8f, 0xc8, 0xe3, 0x2c, 0xa0, 0x49, 0x30, 0xc2}};

DECLARE_INTERFACE_(IPushSource, IUnknown)
{
	//��������id
	virtual void SetTaskId(DWORD taskid) = 0;

	//��ȡ����id
	virtual DWORD GetTaskId() = 0;

	//���ûص������ӿ�ָ��
	virtual HRESULT SetCallBack(interface IQvodCallBack *pCallBack) = 0;

	//���PushSource������
	virtual HRESULT AddStream(CMediaType *pmediatype, IReceive* pIReceive, DWORD& streamid) = 0;

	//�Ƴ�PushSource������
	virtual HRESULT RemoveStream(DWORD streamid) = 0;

	//����Editģ������Դ
	virtual void	SetIDataSource(IDataSource* edit_data_source) = 0;
};