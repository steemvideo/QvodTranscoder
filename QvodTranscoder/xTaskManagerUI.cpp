#include "xTaskManagerUI.h"
#include "xTaskItem.h"
#include "MessageDefine.h"
#include "xIItemOperator.h"
#include "xTranscoder.h"
#include "commonfunction.h"
#include "xFunction.h"


#define FULL_PROGRESS	10000

struct _change_state
{
	_change_state(E_ITME_STATES s):new_states(s){}

	void operator()(xTaskItemPtr& e)
	{
		e->set_status(new_states);
	}

private:
	E_ITME_STATES new_states;
};

static void _dump(QString str, E_ENGINE_STATES s)
{
#ifdef _DEBUG
	switch(s)
	{
	case ENGINE_STATES_PAUSE:
		OutputDebugString(L"*** OPERATOR =");
		OutputDebugString(str.toStdWString().c_str());
		OutputDebugString(L"*** ** ENGINE STATUS = PAUSE\n");
		break;
	case ENGINE_STATES_TRANSCODING:
		OutputDebugString(L"*** OPERATOR =");
		OutputDebugString(str.toStdWString().c_str());
		OutputDebugString(L"*** ** ENGINE STATUS = TRANSCODING\n");
		break;
	case ENGINE_STATES_STOP:
		OutputDebugString(L"*** OPERATOR =");
		OutputDebugString(str.toStdWString().c_str());
		OutputDebugString(L"*** ** ENGINE STATUS = STOP\n");
		break;
	}
#endif
}

xTaskManagerUI::xTaskManagerUI(ITaskManager* manager)
: mTaskMananger(manager)
, mStates(ENGINE_STATES_STOP)
, mNextOperator(E_NONE)
, mbTernimal(false)
, mbMultiTask(false)
, mbUpdated(false)
, mPauseTime(0)
, mPauseCastTime(0)
, mbFullDisk(false)
, mbAccelerate(true)//always open
{
	mTaskMananger->SetCallBack(dynamic_cast<IQvodCallBack*>(this));

	mConfigure.reset(new TRANSCODE_CONFIG);

	mConfigure->ContainerType = _E_CON_MP4;
	mConfigure->VideoCodec.CodecType  = _V_H264;
	mConfigure->VideoCodec.width = 720;
	mConfigure->VideoCodec.height = 480;
	mConfigure->VideoCodec.framerate  = 15;
	mConfigure->VideoCodec.bitrate = 500 * 1000;
	mConfigure->AudioCodec.CodecType = _A_AAC;
	mConfigure->AudioCodec.channel = 2;
	mConfigure->AudioCodec.samplingrate  = 4800;

	mThread.reset(new boost::thread( boost::bind(&xTaskManagerUI::process, this)));
	connect(this, SIGNAL(sig_AsynTaskError(int,int)), this, SLOT(slot_AsynTaskError(int, int)), Qt::QueuedConnection);

}

xTaskManagerUI::~xTaskManagerUI()
{
	qDebug() << "xTaskManagerUI~begin";
	if (stop())
	{	
		mbTernimal = true;	
		{
			boost::recursive_mutex::scoped_lock lock(mMutex);
			mbUpdated = true;
		}
		mCondition.notify_all();
		mThread->join();
		mThread.reset();
	}
	qDebug() << "xTaskManagerUI~end";
}

E_RESULT xTaskManagerUI::add_item(QString fileName)
{
	if (fileName.isEmpty())
	{
		return E_ERROR_FILE_UNEXISTS;
	}
	boost::recursive_mutex::scoped_lock lock(mMutex);
	fileName = QDir::toNativeSeparators(fileName);
	TaskItemSet_ByURL& taskSet = mTaskSet.get< TAG_BY_URL >();
	TaskItemSet_ByURL::iterator pos = taskSet.find(fileName.toStdWString());
	if (pos != taskSet.end())
	{
		return E_ERROR_FILE_EXISTS;
	}

	if (!fileName.isEmpty())
	{
		boost::shared_ptr<Qvod_DetailMediaInfo> pMediaInfo(new Qvod_DetailMediaInfo);
		GetMediaInfo(fileName.toStdWString().c_str(), pMediaInfo);
		E_CONTAINER	container = ContainerFromString(QString::fromWCharArray(pMediaInfo->container).toStdWString().c_str());
		E_V_CODEC vc = VideoEncoderFromString(QString::fromWCharArray(pMediaInfo->videoInfo[0]->Codec).toStdWString().c_str());
		E_A_CODEC ac = AudioEncoderFromString(QString::fromWCharArray(pMediaInfo->audioInfo[0]->Codec).toStdWString().c_str());

		if (container == _E_CON_OTHER)
		{
			return E_ERROR_FILE_UNSUPPORTS;
		}

		if (vc == _V_OTHER && ac == _A_OTHER)
		{
			return E_ERROR_FILE_UNSUPPORTS;
		}

		if(!is_file_support(fileName))
		{
			return E_ERROR_FILE_UNSUPPORTS;
		}
	}

	// create a task
	DWORD	id;
	mTaskMananger->CreateTranscodeTask(fileName.toStdWString().c_str(), id);
	xTaskItemPtr item(new xTaskItem(id));
	item->set_status(ITEM_STATES_WAIT);
	item->set_url(fileName);

	TaskItemSet_ByIndex& taskSetIndex = mTaskSet.get< 0 >();
	taskSetIndex.push_back(item);

	emit sig_addTask(id);
	emit sig_item_count_changed(item_count());	
	return E_NO_ERROR;
}

void xTaskManagerUI::remove_item(int task_id)
{
	boost::recursive_mutex::scoped_lock lock(mMutex);
	TaskItemSet_ByTaskID& taskSet = mTaskSet.get< TAG_BY_TASK_ID >();
	TaskItemSet_ByTaskID::iterator pos = taskSet.find(task_id);
	if (pos != taskSet.end())
	{
		emit sig_removeTask(task_id);
		emit sig_item_count_changed(item_count());		
		
		if((*pos)->get_states() == ITEM_STATES_TRANSCODING 
			|| (*pos)->get_states() == ITEM_STATES_PAUSE)
		{
			change_states(*pos, ITEM_STATES_STOP);
		}
		
		mTaskMananger->RemoveTask(task_id);
		taskSet.erase(pos);
	}
	mNextOperator = E_WAKE_WAIT_ITEM;
	mCondition.notify_one();
}

int xTaskManagerUI::get_item_index(int task_id)
{
	boost::recursive_mutex::scoped_lock lock(mMutex);
	TaskItemSet_ByTaskID& taskSet = mTaskSet.get< TAG_BY_TASK_ID >();
	TaskItemSet_ByTaskID::iterator pos = taskSet.find(task_id);
	TaskItemSet_ByIndex::iterator pos_index = mTaskSet.project< 0 >(pos);
	return pos_index - mTaskSet.get<0>().begin();
}

void xTaskManagerUI::move_item(int task_id, int dst_index)
{
	if (dst_index < 0 || dst_index >= mTaskSet.size())
		return;

	boost::recursive_mutex::scoped_lock lock(mMutex);
	TaskItemSet_ByTaskID& taskSet_id = mTaskSet.get< TAG_BY_TASK_ID >();
	TaskItemSet_ByTaskID::iterator pos_id = taskSet_id.find(task_id);
	
	TaskItemSet_ByIndex::iterator pos_index = mTaskSet.project< 0 >(pos_id);

	xTaskItemPtr it = *pos_index;
	mTaskSet.get< 0 >().erase(pos_index);

	TaskItemSet_ByIndex::iterator pos_dst = mTaskSet.get< 0 >().begin() + dst_index;
	mTaskSet.get< 0 >().insert(pos_dst, it);

	emit sig_moveTask(task_id, dst_index);
}

void xTaskManagerUI::clear_all()
{
	boost::recursive_mutex::scoped_lock lock(mMutex);
	TaskItemSet_ByTaskID& taskSet = mTaskSet.get< TAG_BY_TASK_ID >();
	for (TaskItemSet_ByTaskID::iterator pos = taskSet.begin(); pos != taskSet.end(); pos++)
	{	
		if((*pos)->get_states() == ITEM_STATES_TRANSCODING 
			|| (*pos)->get_states() == ITEM_STATES_PAUSE)
		{
			change_states(*pos, ITEM_STATES_STOP);
		}
		mTaskMananger->RemoveTask((*pos)->get_task_id());
	}
	
	int cout = taskSet.size();
	taskSet.clear();
	if (cout)
	{
		emit sig_item_count_changed(0);
	}
	
	mStates = ENGINE_STATES_STOP;
	emit sig_engine_states_changed(mStates);
}

int xTaskManagerUI::item_count()
{
	boost::recursive_mutex::scoped_lock lock(mMutex);
	TaskItemSet_ByIndex& taskSet = mTaskSet.get< 0 >();
	return taskSet.size();
}

bool xTaskManagerUI::start(bool bTestMode)
{
	boost::recursive_mutex::scoped_lock lock(mMutex);
	E_ENGINE_STATES s = get_engine_states();
	if (s == ENGINE_STATES_TRANSCODING)
		return false;
	else if (s == ENGINE_STATES_PAUSE)
		mPauseCastTime += QDateTime::currentDateTime().toMSecsSinceEpoch() - mPauseTime;
	
	mbTestMode = bTestMode;
	mbUpdated = true;
	mbFullDisk = false;
	set_next_operator(s == ENGINE_STATES_PAUSE ? E_CONTINUE_ALL : E_START_ALL);
	
	mWorkTime = QTime::currentTime();
	mWorkTime.start();

	return true;
}

bool xTaskManagerUI::stop()
{
	set_next_operator(E_STOP_ALL);
	mPauseTime = 0;
	mbFullDisk = false;
	return true;
}

bool xTaskManagerUI::pause()
{
	set_next_operator(E_PAUSE_ALL);
	mPauseTime = QDateTime::currentDateTime().toMSecsSinceEpoch();
	return true;
}

E_ENGINE_STATES xTaskManagerUI::get_engine_states()
{
	boost::recursive_mutex::scoped_lock lock(mMutex);
	return mStates;
}

bool xTaskManagerUI::set_next_operator(E_NEXT_OPERATOR op)
{
	mNextOperator = op;
	mCondition.notify_one();
	return true;
}

void xTaskManagerUI::set_multi_task(bool b)
{
	boost::recursive_mutex::scoped_lock lock(mMutex);
	mbMultiTask = b;
}

// void xTaskManagerUI::set_accelerateOpened(bool b)
// {
// 	boost::recursive_mutex::scoped_lock lock(mMutex);
// 	mbAccelerate = b;
// }

// void xTaskManagerUI::process_get_image()
// {
// 	while(!mbTernimal)
// 	{
// 		int task_id = -1;
// 		while(task_id == -1)
// 		{
// 			boost::recursive_mutex::scoped_lock lock(mMutex);
// 			TaskItemSet_ByIndex& taskSet = mTaskSet.get< 0 >();
// 			for (TaskItemSet_ByIndex::iterator pos = taskSet.begin(); pos != taskSet.end(); pos++)
// 			{
// 				if (!(*pos)->has_image())
// 				{
// 					task_id = (*pos)->get_task_id();
// 					break;
// 				}
// 			}
// 			if (task_id!=-1)
// 				break;
// 
// 			mConGrabImage.wait(mMutex);	
// 			if (mbTernimal)
// 				break;
// 		}
// 
// 		CComPtr<IPreview> IPreviewCPtr;
// 		mTaskMananger->CreatePreviewTask(task_id, IPreviewCPtr);
// 		if (IPreviewCPtr)
// 		{
// 			IPreviewCPtr->Play();
// 
// 			DWORD lenth = 0;
// 			IPreviewCPtr->GetCurrentImage(NULL,lenth);
// 			if (lenth > 0)
// 			{
// 				boost::shared_array<BYTE> pBitmapPtr(new BYTE[lenth]);
// 				if(SUCCEEDED(IPreviewCPtr->GetCurrentImage(pBitmapPtr.get(), lenth)));
// 				{
// 					xTaskItemPtr it = get_item_by_task_id(task_id);
// 					if (it)
// 					{
// 						it->set_image(pBitmapPtr, lenth);
// 						emit sig_AsynFresh(task_id);
// 					}
// 				}
// 			}
// 
// 			IPreviewCPtr->Stop();
// 		}
// 	}	
// }

void xTaskManagerUI::process()
{
	CoInitialize(NULL);
	while(!mbTernimal)
	{
		E_NEXT_OPERATOR _operator;
		E_ENGINE_STATES _status;
		bool	 bMultiTask;
		{
			boost::recursive_mutex::scoped_lock lock(mMutex);
			if (!mbUpdated)
				mCondition.wait(lock);
			mbUpdated = false;
			_operator = mNextOperator;
			_status = mStates;
			mNextOperator = E_NONE;
			bMultiTask = mbMultiTask;
		}

		if(_operator == E_PAUSE_ALL)
		{
			_dump("## E_PAUSE_ALL BEGIN", mStates);

			pause_all();

			mStates = ENGINE_STATES_PAUSE;
			if (_status != mStates)
				emit sig_engine_states_changed(mStates);

			_dump("## E_PAUSE_ALL END", mStates);
		}
		else if(_operator == E_CONTINUE_ALL)
		{
			_dump("## E_CONTINUE_ALL BEGIN", mStates);

			continue_all();
			
			int tCount = get_count_by_status(ITEM_STATES_TRANSCODING);
			// 没有正在转码的文件
			if(!tCount)
			{	
				mStates = ENGINE_STATES_STOP;	
				int waitCount = get_count_by_status(ITEM_STATES_WAIT);
				if(waitCount)
				{		
					bool bOK = false;
					// check & begin a new transcoding
					while(get_count_by_status(ITEM_STATES_TRANSCODING) < (bMultiTask ? MAX_TRANSCODING_COUNT : 1))
					{
						if(start_a_transcoding(false))
							mStates = ENGINE_STATES_TRANSCODING;

						if (!has_wait_for())
							break;
					}
				}
			}
			else
			{
				mStates = ENGINE_STATES_TRANSCODING;
			}

			// send states report if all completed
			if (_status != mStates)
				emit sig_engine_states_changed(mStates);

			_dump("## E_CONTINUE_ALL END", mStates);
		}
		else if (_operator == E_STOP_ALL)
		{
			_dump("## E_STOP_ALL BEGIN", mStates);
			stop_all();

			//destory graph while complete
			destory_completed_graph();

			mStates = ENGINE_STATES_STOP;
			if (_status != mStates)
				emit sig_engine_states_changed(mStates);

			_dump("## E_STOP_ALL END", mStates);
		}
		else if(_operator == E_PAUSE_ITEM)
		{
			_dump("## E_PAUSE_ITEM BEGIN", mStates);
			int waitCount = get_count_by_status(ITEM_STATES_WAIT);
			if(!waitCount)
			{
				int sleepCount = get_count_by_status(ITEM_STATES_PAUSE);
				if(sleepCount)
					mStates = ENGINE_STATES_PAUSE;	
				else
					mStates = ENGINE_STATES_STOP;
			}
			else
			{
				mStates = ENGINE_STATES_TRANSCODING;
			}			
			if (_status != mStates)
				emit sig_engine_states_changed(mStates);

			// 继续下一个
			if (mStates == ENGINE_STATES_TRANSCODING)
			{
				// check & begin a new transcoding
				while(get_count_by_status(ITEM_STATES_TRANSCODING) < (bMultiTask ? MAX_TRANSCODING_COUNT : 1))
				{
					start_a_transcoding(false);
					if (!has_wait_for())
						break;
				}
			}
			_dump("## E_PAUSE_ITEM END", mStates);
		}
		else if(_operator == E_STOP_ITEM)
		{
			_dump("## E_STOP_ITEM BEGIN", mStates);

			int waitCount = get_count_by_status(ITEM_STATES_WAIT);
			int tCount = get_count_by_status(ITEM_STATES_TRANSCODING);
			if(!waitCount && !tCount)
			{
				int sleepCount = get_count_by_status(ITEM_STATES_PAUSE);
				if(sleepCount)
					mStates = ENGINE_STATES_PAUSE;	
				else
					mStates = ENGINE_STATES_STOP;
			}
			else
			{
				mStates = ENGINE_STATES_TRANSCODING;
			}			
			if (_status != mStates)
				emit sig_engine_states_changed(mStates);

			// 继续下一个
			if (mStates == ENGINE_STATES_TRANSCODING)
			{
				// check & begin a new transcoding
				while(get_count_by_status(ITEM_STATES_TRANSCODING) < (bMultiTask ? MAX_TRANSCODING_COUNT : 1))
				{
					start_a_transcoding(false);
					if (!has_wait_for())
						break;
				}
			}
			_dump("## E_STOP_ITEM END", mStates);
		}
		else if(_operator == E_CONTINUE_ITEM)
		{
			_dump("## E_CONTINUE_ITEM BEGIN", mStates);
			int tCount = get_count_by_status(ITEM_STATES_TRANSCODING);
			if(tCount)
			{
				mStates = ENGINE_STATES_TRANSCODING;

				if (_status != mStates)
					emit sig_engine_states_changed(mStates);
			}
			_dump("## E_CONTINUE_ITEM END", mStates);
		}
		else if(_operator == E_WAKE_WAIT_ITEM)
		{
			_dump("## E_WAKE_WAIT_ITEM BEGIN", mStates);

			//destory graph while complete
			destory_completed_graph();

			int waitCount = get_count_by_status(ITEM_STATES_WAIT);
			if(!waitCount)
			{		
				int pauseCount = get_count_by_status(ITEM_STATES_PAUSE);
				int tCount = get_count_by_status(ITEM_STATES_TRANSCODING);
				if(pauseCount > 0 || tCount > 0)
				{
					mStates = ENGINE_STATES_TRANSCODING;	
				}
				else
				{
					int sleepCount = get_count_by_status(ITEM_STATES_PAUSE);
					if(sleepCount)
						mStates = ENGINE_STATES_PAUSE;	
					else
						mStates = ENGINE_STATES_STOP;	
				}
			}
			// send states report if all completed
			if (_status != mStates)
				emit sig_engine_states_changed(mStates);
		
			if (mStates == ENGINE_STATES_TRANSCODING)
			{
				// check & begin a new transcoding
				while(get_count_by_status(ITEM_STATES_TRANSCODING) < (bMultiTask ? MAX_TRANSCODING_COUNT : 1))
				{
					start_a_transcoding(true);
					if (!has_wait_for())
						break;
				}
			}
			_dump("## E_WAKE_WAIT_ITEM END", mStates);
		}
		else if(_operator == E_START_ALL)
		{
			_dump("## E_START_ALL BEGIN", mStates);
			int waitCount = get_count_by_status(ITEM_STATES_WAIT);
			if(!waitCount)
				mStates = ENGINE_STATES_STOP;	
			else
				mStates = ENGINE_STATES_TRANSCODING;

			// send states report if all completed
			if (_status != mStates)
				emit sig_engine_states_changed(mStates);


			if (mStates == ENGINE_STATES_TRANSCODING)
			{
				// check & begin a new transcoding
				while(get_count_by_status(ITEM_STATES_TRANSCODING) < (bMultiTask ? MAX_TRANSCODING_COUNT : 1))
				{
					start_a_transcoding(true);
					if (!has_wait_for())
						break;
				}
			}
			_dump("## E_START_ALL END", mStates);
		}
	}
	CoUninitialize();
}


void xTaskManagerUI::destory_completed_graph()
{
	boost::recursive_mutex::scoped_lock lock(mMutex);
	TaskItemSet_ByIndex& taskSet = mTaskSet.get< 0 >();
	for (TaskItemSet_ByIndex::iterator pos = taskSet.begin(); pos != taskSet.end(); pos++)
	{
		if((*pos)->get_progress() >= FULL_PROGRESS  && (*pos)->get_states() == ITEM_STATES_TRANSCODING)
		{
			change_states(*pos, ITEM_STATES_COMPLETE);
		}
	}
}

void xTaskManagerUI::pause_all()
{
	boost::recursive_mutex::scoped_lock lock(mMutex);
	TaskItemSet_ByIndex& taskSet = mTaskSet.get< 0 >();
	for (TaskItemSet_ByIndex::iterator pos = taskSet.begin(); pos != taskSet.end(); pos++)
	{
		change_states(*pos, ITEM_STATES_PAUSE);
	}
}

void xTaskManagerUI::continue_all()
{
	boost::recursive_mutex::scoped_lock lock(mMutex);
	TaskItemSet_ByIndex& taskSet = mTaskSet.get< 0 >();
	int count = 0;
	for (TaskItemSet_ByIndex::iterator pos = taskSet.begin(); (pos != taskSet.end()) && (count < (mbMultiTask ? MAX_TRANSCODING_COUNT : 1)); pos++)
	{
		if((*pos)->get_states() == ITEM_STATES_PAUSE)
		{
			change_states(*pos, ITEM_STATES_TRANSCODING);
			count++;
		}
	}
}

void xTaskManagerUI::stop_all()
{
	boost::recursive_mutex::scoped_lock lock(mMutex);
	TaskItemSet_ByIndex& taskSet = mTaskSet.get< 0 >();
	for (TaskItemSet_ByIndex::iterator pos = taskSet.begin(); pos != taskSet.end(); pos++)
	{
		change_states(*pos,ITEM_STATES_STOP);
	}
}

bool xTaskManagerUI::change_states(xTaskItemPtr it, E_ITME_STATES status)
{
	E_ITME_STATES orgStates = it->get_states();
	if (orgStates == status)
	{
		return false;
	}

	bool bRet = false;
	if (status == ITEM_STATES_WAIT)
	{
		if(SUCCEEDED(mTaskMananger->StopTask(it->get_task_id())))
		{
			
		}
		
		it->set_start_progress(0);
		it->set_progress(0);
		bRet = true;
	}
	else if (status == ITEM_STATES_PAUSE)
	{
		if (orgStates == ITEM_STATES_TRANSCODING)
		{
			if(SUCCEEDED(mTaskMananger->PauseTask(it->get_task_id())))
			{
				bRet = true;
			}
		}
		else
		{
			status = orgStates;
		}
	}
	else if(status == ITEM_STATES_TRANSCODING)
	{
		_dump("## ITEM_STATES_TRANSCODING BEGIN", mStates);
		qDebug()<<it->get_display_text();
		// 挂起状态？
		if (it->get_states() == ITEM_STATES_WAIT)
		{
			TRANSCODE_CONFIG config;
			prepare_configure(it, config);

			// 磁盘空间检测
			check_disk(it, config);
			if (mbFullDisk)
			{
				emit sig_AsynDiskFull();
				return false;
			}

			// task configure
			if(FAILED(mTaskMananger->SetTaskConfig(it->get_task_id(), config)))
			{
				qDebug() << "!!!SetTaskConfig FAILED";
			}

			if(config.VideoCodec.CodecType == _V_H264)
			{
				SYSTEM_INFO info;
				GetSystemInfo(&info);
				mTaskMananger->SetVideoCodecThreadCount(info.dwNumberOfProcessors);
			}
		}
		_dump("## ITEM_STATES_TRANSCODING RUN TASK", mStates);
		if(!mbFullDisk && SUCCEEDED(mTaskMananger->RunTask(it->get_task_id())))
		{
			_dump("## ITEM_STATES_TRANSCODING RUN TASK SUCCESS", mStates);
			it->set_start_time(mWorkTime.elapsed());
			if (it->get_states() == ITEM_STATES_PAUSE)
				it->set_start_progress(it->get_progress());//记录上次转码进度
			else
				it->set_start_progress(0);
			bRet = true;
		}
		else
		{
			_dump("## ITEM_STATES_TRANSCODING RUN TASK ERROR!!!!!!!!!!", mStates);
			status = ITEM_STATES_ERROR;
		}
		_dump("## ITEM_STATES_TRANSCODING END", mStates);
	}
	else // ERROR || STOP || COMPLETE
	{
		if(status != ITEM_STATES_WAIT)	
		{
			_dump("## STOP TASK BEGIN", mStates);
			if(SUCCEEDED(mTaskMananger->StopTask(it->get_task_id())))
			{
				it->set_start_progress(0);
				if (status != ITEM_STATES_COMPLETE)
					it->set_progress(0);
			}
			_dump("## STOP TASK END", mStates);
		}

		// remove
		if( status == ITEM_STATES_STOP  || status == ITEM_STATES_ERROR)	
		{
			if (orgStates != ITEM_STATES_COMPLETE &&  it->get_progress() < FULL_PROGRESS)
			{
				QFile::remove(it->get_dest_url());
			}
		}

		if (it->get_states() == ITEM_STATES_WAIT)
			status = ITEM_STATES_WAIT;
		else if(it->get_states() == ITEM_STATES_COMPLETE)
			status = ITEM_STATES_COMPLETE;

		bRet = true;
	}

	if(orgStates != status)
	{
		TaskItemSet_ByTaskID& taskSet = mTaskSet.get< TAG_BY_TASK_ID >();
		TaskItemSet_ByTaskID::iterator pos = taskSet.find(it->get_task_id());
		if (pos != taskSet.end())
			taskSet.modify(pos, _change_state(status));

		emit sig_AsynFresh(it->get_task_id());

		emit sig_AsynItemStatusChanged(it->get_task_id(), status);
	}
	
	return bRet;
}

bool xTaskManagerUI::start_a_transcoding(bool bPausedToo)
{
	boost::recursive_mutex::scoped_lock lock(mMutex);
	TaskItemSet_ByIndex& taskSet = mTaskSet.get< 0 >();
	for(TaskItemSet_ByIndex::iterator pos = taskSet.begin(); pos != taskSet.end(); pos++)
	{
		xTaskItemPtr it = *pos;
		if(it->get_states() == ITEM_STATES_WAIT || (bPausedToo ? it->get_states() == ITEM_STATES_PAUSE : false))
		{
			if(change_states(it, ITEM_STATES_TRANSCODING))
			{
				return true;
			}
		}
	}
	return false;
}

void xTaskManagerUI::prepare_configure(xTaskItemPtr it, TRANSCODE_CONFIG& config)
{
	config = *mConfigure;

	CLIP_CONFIG	cf;
	cf.srcstartts = cf.srcstopts = cf.dststartts = cf.dststopts = 0;
	if (mbTestMode)
	{
		cf.srcstopts = 1 * 60 * 10000000;
	}

	cf.filepath = it->get_url().toStdWString();
	cf.bVideoenable = true;
	cf.bAudioenable = true;
	cf.bSubtitleenable = true;

	QString destURL = mDirectory +it->get_base_name() + QString("_1") + QString(".") + g_toFormatString(mConfigure->ContainerType);
	int i = 2;
	while(QFile::exists(destURL))
	{
		destURL = mDirectory +it->get_base_name() + QString("_") + QString::number(i) + QString(".") + g_toFormatString(mConfigure->ContainerType);
		i++;
	}

	config.DestURL = destURL.toStdWString();
	config.ClipConfig_List.push_back(cf);
	
	it->set_dest_url(destURL);
	if (!it->get_title_file_url().isEmpty())
	{
		cf.filepath = it->get_title_file_url().toStdWString();
		cf.bSubtitleenable = true;
		cf.bVideoenable = false;
		cf.bVideoenable = false;
		config.ClipConfig_List.push_back(cf);
	}

	config.Accel = mbAccelerate;

	//智能調整
	adjust_configure(it->get_task_id(), config);

}

/*	计算逻辑修改如下：
1.设目标SIZE为dst_size
2.设源SIZE为src_size
3.则dst_size = dst_size < src_size ? dst_size : src_size;//取小者,因为小像素转大像素无意义
4.最终的dst_size满足2个条件
4.1 dst_size <= src_size 
4.2 如果选择了锁定宽高比则 dst_size::width / dst_size::height == src_size::width / src_size::height
*/
void xTaskManagerUI::adjust_configure(int task_id, TRANSCODE_CONFIG& config)
{
	//智能設置轉碼參數
	Qvod_DetailMediaInfo_Ptr ptr = mTaskMananger->GetMediaInfo(task_id);
	if (ptr->nVideoCount > 0)
	{
		//修正視頻分辨率，視頻碼率
		int w = config.VideoCodec.width;
		int h = config.VideoCodec.height;
		int bit = config.VideoCodec.bitrate;

		// 都取小者
		int xNew = config.VideoCodec.width * config.VideoCodec.height;		//目标SIZE
		int xOrg = ptr->videoInfo[0]->nWidth * ptr->videoInfo[0]->nHeight;	//源SIZE

		if (xNew > xOrg && config.ContainerType != _E_CON_3GP/*非3GP*/)
		{
			config.VideoCodec.width = ptr->videoInfo[0]->nWidth;
			config.VideoCodec.height = ptr->videoInfo[0]->nHeight;
			config.VideoCodec.bitrate = ptr->videoInfo[0]->nBitrate;
		}

		//如果锁定宽高比则需要重新计算正确的宽高
		if (config.KeepAspectRatio)
		{
			if (config.ContainerType != _E_CON_3GP)
			{
				QSize size = getFitSize(QSizeF(config.VideoCodec.width, config.VideoCodec.height),QSizeF(ptr->videoInfo[0]->nWidth, ptr->videoInfo[0]->nHeight));

				config.VideoCodec.width = size.width();
				if(config.VideoCodec.width%2 != 0)
					config.VideoCodec.width++;

				config.VideoCodec.height = size.height();
				if(config.VideoCodec.height%2 != 0)
					config.VideoCodec.height++;
			}
		}
		
		if (config.VideoCodec.width == 0)
			config.VideoCodec.width = w;
		if (config.VideoCodec.height == 0)
			config.VideoCodec.height = h;

		if(config.VideoCodec.width%2 != 0)
			config.VideoCodec.width++;

		if(config.VideoCodec.height%2 != 0)
			config.VideoCodec.height++;

		if (config.VideoCodec.bitrate== 0)
			config.VideoCodec.bitrate = bit;

		config.VideoCodec.framerate = config.VideoCodec.framerate < ptr->videoInfo[0]->nFrameRate ? config.VideoCodec.framerate : ptr->videoInfo[0]->nFrameRate;
		if (config.VideoCodec.framerate == 0)
			config.VideoCodec.framerate = 25;

	}
	if (ptr->nAudioCount > 0)
	{
		int bit_audio = config.AudioCodec.bitrate;
		int sampling_audio = config.AudioCodec.samplingrate;

		//修正音頻碼率
		config.AudioCodec.bitrate = config.AudioCodec.bitrate < ptr->audioInfo[0]->nBitrate ? config.AudioCodec.bitrate : ptr->audioInfo[0]->nBitrate;
		//config.AudioCodec.samplingrate = config.AudioCodec.samplingrate < ptr->audioInfo[0]->nSamplingRate ? config.AudioCodec.samplingrate : ptr->audioInfo[0]->nSamplingRate;

		if (!config.AudioCodec.bitrate)
			config.AudioCodec.bitrate = bit_audio;
		if (!config.AudioCodec.samplingrate)
			config.AudioCodec.samplingrate = sampling_audio;
	}
}

int xTaskManagerUI::get_count_by_status(E_ITME_STATES s)
{
	boost::recursive_mutex::scoped_lock lock(mMutex);
	TaskItemSet_ByState& taskSet = mTaskSet.get< TAG_BY_STATE >();
	std::pair<TaskItemSet_ByState::iterator,TaskItemSet_ByState::iterator> _pair   = taskSet.equal_range(s);
	int  cout = std::distance(_pair.first, _pair.second);
	return cout;
}

bool xTaskManagerUI::has_wait_for()
{
	if(mbFullDisk)
		return false;

	boost::recursive_mutex::scoped_lock lock(mMutex);
	int count = get_count_by_status(ITEM_STATES_WAIT);
	return count > 0;
}

bool xTaskManagerUI::is_accel_trans(int task_id)
{
	return mTaskMananger->IsAccelTrans(task_id);
}

long xTaskManagerUI::EventNotify( UINT nMsg, long task_id, long param)
{
	//boost::recursive_mutex::scoped_lock lock(mMutex);
	xTaskItemPtr item=  get_item_by_task_id(task_id);
	if (nMsg == QTC_MSG_PROGRESS)
	{
		long oldProgress = item->get_progress();
		
		if (param != oldProgress)
		{
			
			// 设置进度
			item->set_progress(param);

			// 计算已耗时
			long elapsed_time = mWorkTime.elapsed() - item->get_start_time();

			long param_elasped = param - item->get_start_progress();
			//if (param_elasped > 0.01f)
			{
				// 计算剩余时间
				double tofp = elapsed_time / (double)param_elasped;
				long remain_time = (FULL_PROGRESS - param) * tofp;
				if (remain_time < 1000)
					remain_time = 1000;
				if (param >= FULL_PROGRESS)
					remain_time = 0;

				item->set_remain_time(remain_time);
			}

			// 转码完成
			if (param >= FULL_PROGRESS)
			{
				mNextOperator = E_WAKE_WAIT_ITEM;
				mCondition.notify_one();
			}

			// 刷新
			emit sig_AsynFresh(task_id);
			
		}
	}
	else
	{
		emit sig_AsynTaskError(task_id, nMsg);
	}
	return 0;
}

xTaskItemPtr xTaskManagerUI::get_item_by_task_id(int task_id)
{
	//boost::recursive_mutex::scoped_lock lock(mMutex);	
	TaskItemSet_ByTaskID& taskSet = mTaskSet.get< TAG_BY_TASK_ID >();
	TaskItemSet_ByTaskID::iterator pos = taskSet.find(task_id);
	if (pos != taskSet.end())
	{
		return *pos;
	}
	return xTaskItemPtr();
}

xTaskItemPtr xTaskManagerUI::get_item_by_index(int order)
{
	boost::recursive_mutex::scoped_lock lock(mMutex);	
	TaskItemSet_ByIndex& taskSet = mTaskSet.get< 0 >();
	if (order < 0 || order >= taskSet.size())
	{
		return xTaskItemPtr();
	}

	TaskItemSet_ByIndex::iterator pos = taskSet.begin();
	pos += order;
	return (*pos);
}

boost::shared_ptr<Qvod_DetailMediaInfo>	xTaskManagerUI::get_src_media_info(int taskid)
{
	xTaskItemPtr item = get_item_by_task_id(taskid);
	if (item)
	{
		return mTaskMananger->GetMediaInfo(item->get_task_id());
	}
	return boost::shared_ptr<Qvod_DetailMediaInfo>();
}

bool xTaskManagerUI::set_item_configure(const TRANSCODE_CONFIG& config)
{
	if (config.ContainerType == _E_CON_OTHER)
		return false;

	mConfigure->VideoCodec = config.VideoCodec;
	mConfigure->AudioCodec = config.AudioCodec;
	mConfigure->ContainerType = config.ContainerType;
	mConfigure->AudioVolume = 100;
	mConfigure->KeepAspectRatio = config.KeepAspectRatio;
	return true;
}

bool xTaskManagerUI::get_item_configure(int task_id, TRANSCODE_CONFIG& config)
{
	xTaskItemPtr it = get_item_by_task_id(task_id);
	if(it)
	{	// task configure
		TRANSCODE_CONFIG_Ptr p =  mTaskMananger->GetTaskConfig(it->get_task_id());
		if (p)
		{
			config = *p;
		}
	}
	return false;
}

void xTaskManagerUI::reset_all_items(bool bAll)
{
	boost::recursive_mutex::scoped_lock lock(mMutex);
	TaskItemSet_ByIndex& taskSet = mTaskSet.get< 0 >();
	for(TaskItemSet_ByIndex::iterator pos = taskSet.begin(); pos != taskSet.end(); pos++)
	{
		xTaskItemPtr it = *pos;
		if (bAll || it->get_states() != ITEM_STATES_COMPLETE)
		{
			change_states(it, ITEM_STATES_WAIT);
			emit sig_AsynFresh(it->get_task_id());
		}
	}
}

E_ITME_STATES xTaskManagerUI::get_item_status(int task_id)
{
	xTaskItemPtr ptr = get_item_by_task_id(task_id);
	if (ptr)
		return ptr->get_states();
	return ITEM_STATES_UNKNOWN;
}

bool xTaskManagerUI::stop_item(int task_id)
{
	boost::recursive_mutex::scoped_lock lock(mMutex);
	bool bOK = false;
	xTaskItemPtr it = get_item_by_task_id(task_id);
	if (it && !it->get_url().isEmpty() && (it->get_states() == ITEM_STATES_TRANSCODING || it->get_states() == ITEM_STATES_PAUSE))
	{
		bOK = change_states(it, ITEM_STATES_STOP);
	}

	mNextOperator = E_STOP_ITEM;
	mCondition.notify_one();
	return bOK;
}


bool xTaskManagerUI::pause_item(int task_id)
{
	boost::recursive_mutex::scoped_lock lock(mMutex);
 	bool bOK = false;
	xTaskItemPtr it = get_item_by_task_id(task_id);
	if (it && it->get_states() == ITEM_STATES_TRANSCODING)
	{
		bOK = change_states(it, ITEM_STATES_PAUSE);
	}

	mNextOperator = E_PAUSE_ITEM;
	mCondition.notify_one();
	
	return bOK;
}

bool xTaskManagerUI::continue_item(int task_id)
{
	boost::recursive_mutex::scoped_lock lock(mMutex);
	bool bOK = false;
	if(get_count_by_status(ITEM_STATES_TRANSCODING) >= (mbMultiTask ? MAX_TRANSCODING_COUNT : 1))
		return false;

	xTaskItemPtr it = get_item_by_task_id(task_id);
	if (it && it->get_states() == ITEM_STATES_PAUSE)
	{
		bOK = change_states(it, ITEM_STATES_TRANSCODING);
	}

	mNextOperator = E_CONTINUE_ITEM;
	mCondition.notify_one();
	return bOK;
}

void xTaskManagerUI::slot_AsynTaskError(int task_id, int error_msg)
{
	boost::recursive_mutex::scoped_lock lock(mMutex);
	xTaskItemPtr it = get_item_by_task_id(task_id);
	if (it && it->get_states() == ITEM_STATES_TRANSCODING)
	{
		change_states(it, ITEM_STATES_ERROR);
	}

	//磁盘空间满
	if (error_msg == QTC_MSG_FULLDISK_ERROR)
	{
		emit sig_AsynDiskFull();
		return;
	}

	//继续下一个
	mbUpdated = true;
	mNextOperator = E_WAKE_WAIT_ITEM;
	mCondition.notify_one();
}

bool xTaskManagerUI::is_file_support(QString fileName)
{
	int pos = fileName.lastIndexOf(QChar('.'),-1, Qt::CaseInsensitive);
	if (pos != -1)
	{
		QString suffix = fileName.mid(pos);
		if (suffix == QString(".!mv")
			|| suffix == QString(".swf")
			|| suffix == QString(".bik")
			|| suffix == QString(".csf"))
		{
			return false;
		}
	}
	return true;
}

void xTaskManagerUI::check_disk(xTaskItemPtr it, TRANSCODE_CONFIG& config)
{
	const Qvod_DetailMediaInfo_Ptr pMediaInfo = mTaskMananger->GetMediaInfo(it->get_task_id());
	if (pMediaInfo)
	{
		int vbit = config.VideoCodec.bitrate;
		int abit = config.AudioCodec.bitrate;
		int total = (vbit + abit) / 8;

		double size = total * pMediaInfo->nDuration / (float)1000;
		if (size>0)
		{
			size += 50 * 1024 * 1024;//

			ULARGE_INTEGER   lpFreeBytesAvailableToCaller; 
			ULARGE_INTEGER   lpTotalNumberOfBytes; 
			ULARGE_INTEGER   lpTotalNumberOfFreeBytes; 
			QString disk = mDirectory.left(3);
			if(GetDiskFreeSpaceEx(disk.toStdWString().c_str(),   &lpFreeBytesAvailableToCaller,   &lpTotalNumberOfBytes,   &lpTotalNumberOfFreeBytes))
			{
				if (size > lpFreeBytesAvailableToCaller.QuadPart)
				{
					mbFullDisk = true;
				}
			}
		}
	}
}

