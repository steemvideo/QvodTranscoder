#pragma once

class ITaskManager;

ITaskManager* g_get_task_manager();

// �ÿ첥����������һ���ļ�
void g_play_file(QString url);

// Explore ��һ��Ŀ¼
void g_open_dir(QString dir);

// Explore ��һ��Ŀ¼��ѡ����ļ�
void g_open_url(QString url);

QString g_toFormatString(QString formatName);

QString g_toFormatString(E_CONTAINER c);

QString g_toVideoFormatString(QString video);
QString g_toVideoFormatString(E_V_CODEC c);

QString g_toAudioFormatString(QString audio);
QString g_toAudioFormatString(E_A_CODEC c);

bool g_isVideoFormat(QString str);
bool g_isAudioFormat(QString str);

void g_parse_cmdline(QString cmd, QStringList& urls, int* planguage = NULL);

//���㱣�ֿ�߱ȵ�boundingbox
QSize getFitSize(QSizeF destSize, QSizeF sizePicture);

//����List
void  TranslateQList(QStringList& ql);