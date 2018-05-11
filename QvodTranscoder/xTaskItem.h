#ifndef XTABLEITEM_H
#define XTABLEITEM_H

class xTaskItem
{
public:
	xTaskItem(int task_id);
	~xTaskItem();

public:
	int			get_task_id() const { return mTaskID;}

	E_ITME_STATES	get_states()		 {return mStates;}
	void		set_status(E_ITME_STATES s);

	long		get_progress()		{return mProgress;}
	void		set_progress(long p) { mProgress = p; }

// 	void		set_real_complete() { mbComplete = true;}
// 	bool		is_real_complete() { return mbComplete;}

	long		get_start_progress()		{return mSProgress;}
	void		set_start_progress(long p) { mSProgress = p;}

	void		set_start_time(long t) { mStartTime = t;}
	long		get_start_time() const { return mStartTime;}

	void		set_remain_time(long t) { mRemainTime = t;}
	long		get_remain_time() const { return mRemainTime;}

	QString		get_url() const { return mURL;}
	void		set_url(QString u);

	FString		get_stdw_url() const 
	{ 
		return mURL.toStdWString();
	}

	void		set_dest_url(QString u) { mDestURL = u;}
	QString		get_dest_url() { return mDestURL;}

	QString		get_title_file_url() const 
	{ 
		return mTitleUrl;
	}
	void		set_title_file_url(QString url) 
	{ 
		mTitleUrl = url;
	}

	QString		get_display_text();
	QString		get_base_name();

	bool		has_image() { return mbHasImage;}
	void		set_image(boost::shared_array<BYTE> bitmapPtr, int length);
	QImage		get_image();

private:
	E_ITME_STATES	mStates;
	QString		mURL;		// 源文件
	QString		mDestURL;	// 目标文件
	QString		mTitleUrl;	// 字幕文件

	long		mProgress;
	long		mStartTime;
	long		mRemainTime;
	long		mSProgress;
	QImage		mImage;
	bool		mbHasImage;
	int			mTaskID;

	bool		mbComplete;
};

typedef boost::shared_ptr<xTaskItem>	xTaskItemPtr;

#endif // XTABLEITEM_H
