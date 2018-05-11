#include "stdafx.h"
#include "TaskManager.h"
#include "tools.h"
#include "TranscodeTask.h"

//float g_dummy = 1.0f;//�����壬�������ñ�ģ�� load float point support

HINSTANCE	g_hInstance;
bool InitQvodTaskManager(void** ppTaskManager)
{
	CoInitialize(NULL);
	CTaskManager* ptm = new CTaskManager();
	*ppTaskManager = (void*)(ITaskManager*) ptm;

	return ptm ? true:false;
}

void DestroyQvodTaskManager(void* ppTaskManager)
{
	ITaskManager* pTaskMan = (ITaskManager*)ppTaskManager;
	delete pTaskMan;
	CoUninitialize();
}
extern "C" BOOL WINAPI DllEntryPoint(HINSTANCE, ULONG, LPVOID);

BOOL WINAPI DllMain( HINSTANCE hInstance, ULONG dwReason, LPVOID /*lpReserved*/)
{
	switch (dwReason)
	{
	case DLL_PROCESS_ATTACH:
		{
			g_hInstance = hInstance;
		}
		break;
	case DLL_THREAD_ATTACH:
		{
			break;
		}
	case DLL_THREAD_DETACH:
		{
			CoUninitialize();
			break;
		}
	case DLL_PROCESS_DETACH:
		{
			break;
		}
	}
	return TRUE;
}

CTaskManager::CTaskManager(void)
{
	m_pQvodCallBack = NULL;
	m_nThreadCount = 1;
}

CTaskManager::~CTaskManager(void)
{
	//OutputDebugString(L"destroy tsakmanager\n");
	m_vTaskList.clear();
// 	{
// 		boost::shared_ptr<ITask> pitask = it->second;
// 	}
}

//��������
HRESULT CTaskManager::CreateTranscodeTask(const wchar_t* src_url,DWORD& taskid)
{
	CAutoLock lock(&m_Lock);
	boost::shared_ptr<CTranscodeTask> p(new CTranscodeTask());
	taskid = CTaskIDManager::GetTaskID();
	TaskItem item = {taskid, p};
	m_vTaskList.push_back(item);
	p->SetCallBack(m_pQvodCallBack);
	return p->CreateTranscodeTask(src_url, taskid);
}

//����Ԥ������
HRESULT CTaskManager::CreatePreviewTask(DWORD taskid,  CComPtr<IPreview>& pPreview)
{
	CAutoLock lock(&m_Lock);
	for(int i = 0; i < m_vTaskList.size(); i++)
	{
		if(m_vTaskList[i].taskid == taskid && m_vTaskList[i].pITask)
		{
			return m_vTaskList[i].pITask->CreatePreviewTask(e_Preview, pPreview);
		}
	}

	return E_INVALIDARG;
}

//������������
HRESULT CTaskManager::SetTaskConfig(DWORD taskid, const TRANSCODE_CONFIG& pConfig)
{
	CAutoLock lock(&m_Lock);
	for(int i = 0; i < m_vTaskList.size(); i++)
	{
		if(m_vTaskList[i].taskid == taskid && m_vTaskList[i].pITask)
		{
			return m_vTaskList[i].pITask->SetTaskConfig(pConfig);
		}
	}

	return E_INVALIDARG;
}

//��ȡ������Ϣ
const TRANSCODE_CONFIG_Ptr CTaskManager::GetTaskConfig(DWORD taskid)
{
	CAutoLock lock(&m_Lock);
	for(int i = 0; i < m_vTaskList.size(); i++)
	{
		if(m_vTaskList[i].taskid == taskid && m_vTaskList[i].pITask)
		{
			return m_vTaskList[i].pITask->GetTaskConfig();
		}
	}
	return TRANSCODE_CONFIG_Ptr();
}

//ɾ������
HRESULT CTaskManager::RemoveTask(DWORD taskid)
{
	CAutoLock lock(&m_Lock);
	for(std::vector<TaskItem>::iterator it = m_vTaskList.begin(); it != m_vTaskList.end(); it++)
	{
		if(it->taskid == taskid && it->pITask)
		{
			it->pITask->StopTask();
			it->pITask->DestroyTask(false);
			m_vTaskList.erase(it);
			return S_OK;			
		}
	}
	return E_INVALIDARG;
}

//��ʼת��
HRESULT CTaskManager::RunTask(DWORD taskid)
{
	CAutoLock lock(&m_Lock);
	for(int i = 0; i < m_vTaskList.size(); i++)
	{  
		if(m_vTaskList[i].taskid == taskid && m_vTaskList[i].pITask)
		{
			m_vTaskList[i].pITask->SetVideoCodecThreadCount(m_nThreadCount);
			return m_vTaskList[i].pITask->RunTask();
		}
	}
	return E_INVALIDARG;
}

bool CTaskManager::IsAccelTrans(DWORD taskid)
{
	CAutoLock lock(&m_Lock);
	for(int i = 0; i < m_vTaskList.size(); i++)
	{  
		if(m_vTaskList[i].taskid == taskid && m_vTaskList[i].pITask)
		{
			return m_vTaskList[i].pITask->IsAccelTrans();
			
		}
	}
	return false;
}

//��ͣת��
HRESULT CTaskManager::PauseTask(DWORD taskid)
{
	CAutoLock lock(&m_Lock);
	for(int i = 0; i < m_vTaskList.size(); i++)
	{
		if(m_vTaskList[i].taskid == taskid && m_vTaskList[i].pITask)
		{
			return m_vTaskList[i].pITask->PauseTask();
		}
	}
	return E_INVALIDARG;
}

//ֹͣ����
HRESULT CTaskManager::StopTask(DWORD taskid)
{
	CAutoLock lock(&m_Lock);
	for(int i = 0; i < m_vTaskList.size(); i++)
	{
		if(m_vTaskList[i].taskid == taskid && m_vTaskList[i].pITask)
		{
			return m_vTaskList[i].pITask->StopTask();
		}
	}
	return E_INVALIDARG;
}

//��ȡ�ļ���Ϣ
const boost::shared_ptr<Qvod_DetailMediaInfo> CTaskManager::GetMediaInfo(DWORD	taskid)
{
	CAutoLock lock(&m_Lock);
	//get url 
	for(int i = 0; i < m_vTaskList.size(); i++)
	{
		if(m_vTaskList[i].taskid == taskid && m_vTaskList[i].pITask)
		{
			return m_vTaskList[i].pITask->GetMediaInfo();
		}
	}
	return boost::shared_ptr<Qvod_DetailMediaInfo>();
}

//��ȡ��ǰת����Ƶ���������߳���
DWORD CTaskManager::GetVideoCodecThreadCount()
{
	return m_nThreadCount;
}

//���õ�ǰת����Ƶ���������߳���
HRESULT CTaskManager::SetVideoCodecThreadCount(DWORD nthreadcount)
{
	CAutoLock lock(&m_Lock);
	if(nthreadcount > 0)
	{
		m_nThreadCount = nthreadcount;
		return S_OK;
	}
	return E_FAIL;
}


//��ȡ��ǰת��������
DWORD CTaskManager::GetTaskCount()
{
	CAutoLock lock(&m_Lock);
	return m_vTaskList.size();
}
//������Ϣ���մ��ھ��
HRESULT CTaskManager::SetCallBack(IQvodCallBack* pcallback)
{
	CAutoLock lock(&m_Lock);
	m_pQvodCallBack = pcallback;
	return S_OK;
}

//UI��Ϣ�ص�����
long CTaskManager::EventNotify(UINT task_id, long param_type, long param)
{
	if(m_pQvodCallBack != NULL)
	{
		return m_pQvodCallBack->EventNotify(task_id, param_type, param);
	}
	return E_FAIL;
}
