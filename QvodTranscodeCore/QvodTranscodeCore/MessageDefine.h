#pragma once


#define QTC_MSG_BASE  0x100 + WM_USER

#define QTC_MSG_PROGRESS  QTC_MSG_BASE + 0x1            //ת��ʱ����� wparamΪtaskid��lparamΪ����
#define QTC_MSG_PREVIEW_PROGRESS  QTC_MSG_BASE + 0x2    //Ԥ��ʱ����� wparamΪtaskid��lparamΪ����ʱ�䣨ms��
#define QTC_MSG_TRANSCODE_COMPLETE  QTC_MSG_BASE + 0x3  //ת�����