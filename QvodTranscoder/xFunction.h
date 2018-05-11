#pragma once

class ITaskManager;

ITaskManager* g_get_task_manager();

// 用快播播放器播放一个文件
void g_play_file(QString url);

// Explore 打开一个目录
void g_open_dir(QString dir);

// Explore 打开一个目录并选择此文件
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

//计算保持宽高比的boundingbox
QSize getFitSize(QSizeF destSize, QSizeF sizePicture);

//翻译List
void  TranslateQList(QStringList& ql);