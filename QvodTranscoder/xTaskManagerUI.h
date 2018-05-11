#pragma  once
#include <boost/thread.hpp>
#include <boost/thread/condition.hpp>
#include <boost/multi_index_container.hpp>
#include <boost/multi_index/mem_fun.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/sequenced_index.hpp>
#include <boost/multi_index/identity.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/multi_index/composite_key.hpp>
#include <boost/multi_index/random_access_index.hpp>

using namespace boost;
using namespace boost::multi_index;

#include "xTaskItem.h"

#define MAX_TRANSCODING_COUNT	2

INDEX_TAG_DEF( TAG_BY_TASK_ID  )
INDEX_TAG_DEF( TAG_BY_URL )
INDEX_TAG_DEF( TAG_BY_STATE )

typedef  multi_index_container
<  
	xTaskItemPtr,
	indexed_by< 
	random_access<>,
	ordered_unique< tag<TAG_BY_TASK_ID>     ,BOOST_MULTI_INDEX_CONST_MEM_FUN(xTaskItem,int,get_task_id)>,                                                          
	ordered_unique< tag<TAG_BY_URL> ,BOOST_MULTI_INDEX_CONST_MEM_FUN(xTaskItem, FString, get_stdw_url) >,
	ordered_non_unique< tag<TAG_BY_STATE> ,BOOST_MULTI_INDEX_MEM_FUN(xTaskItem, E_ITME_STATES, get_states) >
	>
> _TaskItemSet;
typedef _TaskItemSet::nth_index<0>::type					TaskItemSet_ByIndex;
typedef _TaskItemSet::index<TAG_BY_TASK_ID>::type			TaskItemSet_ByTaskID;
typedef _TaskItemSet::index<TAG_BY_URL>::type				TaskItemSet_ByURL;
typedef _TaskItemSet::index<TAG_BY_STATE>::type				TaskItemSet_ByState;


class xTranscoder;
class xTaskManagerUI : public QObject, public IQvodCallBack
{
	Q_OBJECT
public:
	xTaskManagerUI(ITaskManager* manager);
	~xTaskManagerUI();

	void	set_test_model();

	/******* item operator function *****/
	E_RESULT add_item(QString);
	void	remove_item(int task_id);
	void	move_item(int task_id, int dst_index);
	void	clear_all();

	xTaskItemPtr	get_item_by_task_id(int task_id);
	xTaskItemPtr	get_item_by_index(int index);
	bool			is_audio_item();
	int				get_item_index(int task_id);


	int		item_count();
	int		get_count_by_status(E_ITME_STATES s);

	/******* item status control for all *****/
	bool	start(bool bTestMode = false);
	bool	stop();
	bool	pause();
	E_ENGINE_STATES	get_engine_states();
	void	reset_all_items(bool);

	/******* item status control for one *****/
	bool	stop_item(int task_id);
	bool	pause_item(int task_id);
	bool	continue_item(int task_id);
	E_ITME_STATES get_item_status(int task_id);

	/******* item config *****/
	bool	set_item_configure(const TRANSCODE_CONFIG& config);
	bool	get_item_configure(int task_id, TRANSCODE_CONFIG& config);
	void	set_directory(QString url) { mDirectory = url;}
	void	set_multi_task(bool);
	bool	is_multi_task() { return mbMultiTask;}
	bool	is_accel_trans(int task_id);
	//void	set_accelerateOpened(bool);//always open
	//void	set_output_format(QString format) { mOutputFormat = format;}
	boost::shared_ptr<Qvod_DetailMediaInfo>	get_src_media_info(int taskid);
	
	void	adjust_configure(int task_id, TRANSCODE_CONFIG& config);	//智能修正
private:
	enum E_NEXT_OPERATOR
	{
		E_NONE,
		E_START_ALL,	// 开始
		E_PAUSE_ALL,	// 暂停所有
		E_CONTINUE_ALL,	// 继续所有
		E_STOP_ALL,		// 停止所有
		E_PAUSE_ITEM,	// 暂停某个
		E_CONTINUE_ITEM,// 继续某个
		E_STOP_ITEM,	// 停止某个
		E_WAKE_WAIT_ITEM, // 开始下一个
	};

signals:
	void	sig_AsynFresh(int task_id);							//要求异步连接
	void	sig_AsynTaskError(int task_id,int error_msg);		//要求异步连接
	void    sig_AsynDiskFull();
	void	sig_AsynItemStatusChanged(int task_id, int status);	//要求异步连接

	void	sig_engine_states_changed(E_ENGINE_STATES);
	void	sig_item_count_changed(int);
	void	sig_moveTask(int taskId, int offset);

	void	sig_addTask(int taskId);
	void	sig_removeTask(int taskId);

protected slots:
	void	slot_AsynTaskError(int, int);
	
private:
	bool	set_next_operator(E_NEXT_OPERATOR);
	bool	start_a_transcoding(bool bPausedToo);
	
	bool	has_wait_for();

	void	process();

	long	EventNotify(UINT task_id, long param_type, long param);
	void	destory_completed_graph();

	void	pause_all();
	void	continue_all();
	void	stop_all();
	bool	change_states(xTaskItemPtr item, E_ITME_STATES status);

	bool	is_file_support(QString fileName);
	void	check_disk(xTaskItemPtr it, TRANSCODE_CONFIG& config);
	void	prepare_configure(xTaskItemPtr it, TRANSCODE_CONFIG& config);
private:
	ITaskManager*						mTaskMananger;
	TRANSCODE_CONFIG_Ptr				mConfigure;
	E_ENGINE_STATES						mStates;

	E_NEXT_OPERATOR						mNextOperator;

	QString								mDirectory;
	//QString								mOutputFormat;

	boost::condition					mCondition;
	boost::shared_ptr<boost::thread>	mThread;
	boost::recursive_mutex				mMutex;
	bool								mbTernimal;
	bool								mbMultiTask;
	bool								mbAccelerate;
	bool								mbTestMode;
	bool								mbUpdated;
	QTime								mWorkTime;		
	qint64								mPauseTime;
	qint64								mPauseCastTime;

	_TaskItemSet						mTaskSet;
	bool								mbFullDisk;//Disk full flag
};
typedef boost::shared_ptr<xTaskManagerUI>	xTaskManagerUIPtr;