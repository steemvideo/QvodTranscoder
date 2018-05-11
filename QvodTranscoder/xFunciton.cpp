#include "xFunction.h"
#include "commonfunction.h"

extern ITaskManager* g_Taskmanager;
ITaskManager* g_get_task_manager()
{
	return g_Taskmanager;
}

void g_play_file(QString url)
{
	QString cmd = "-localfile " + QString("\"") + url + QString("\"");
#ifdef QVOD_TRANSCODER_ONLY
	//ShellExecute(NULL,L"open",L"d:\\program files\\qvodplayer\\qvodplayer.exe", cmd.toStdWString().c_str(), NULL,SW_SHOW);
	ShellExecute(NULL,L"open",L"qvodplayer.exe", cmd.toStdWString().c_str(), NULL,SW_SHOW);
#else
	ShellExecute(NULL,L"open",L"qvodplayer.exe", cmd.toStdWString().c_str(), NULL,SW_SHOW);
#endif
}


void g_open_dir(QString dir)
{
	FString str = dir.toStdWString();
	ShellExecute(NULL,L"explore", str.c_str() ,NULL,NULL,SW_SHOW);
}


void g_open_url(QString url)
{
	QString cmd = QString(" /select,\"%1\"").arg(url);
	ShellExecute(NULL, _T("open"), _T("Explorer.exe"), cmd.toStdWString().c_str(), NULL, SW_SHOWDEFAULT);
}

QString g_container_strings[] = 
{
	"Unknown",
	"RMVB",
	"AVI",
	"MKV",
	"MP4",		//MPEG-4 Part 14",3GP系
	"3GP",		//3GP
	"MOV",		//Apple系 包含MOV，QT

	"MPEG",	//MPEG1/2/4 标准容器格式
	"DAT",
	"MPEG4ES",	//MPEG4基本流
	"MPEGTS",	//TS
	"MPEGPS",	//VOB EVO HD-DVD
	"HKMPG",	//Hikvision mp4
	"DIVX",	//Divx Media Format(DMF)
	"WMV",		//ASF/WMV/WMA/WMP
	"WMA",
	"TS",		//TS
	"M2TS",	//M2TS/MTS",支持DBAV 与 AVCHD
	"DVD",		//DVD盘，特殊处理（DVD导航）
	"QMV",
	"VP5",
	"VP6",
	"VP7",
	"WEBM",//VP8
	"FLV",
	"PMP",
	"BIK",
	"CSF",
	"DV",	
	"XLMV",//迅雷格式
	"M2V",
	"SWF",
	"AMV", //Anime Music Video File
	"IVF",//Indeo Video Format file
	"IVM",
	"SCM",
	"AOB",
	"AMR",
	"DC",
	"DSM",
	"MP2",
	"MP3",
	"OGG",
	"DTS",
	"MIDI",
	"WVP",
	"AAC",
	"AC3",
	"AIFF",
	"ALAW",
	"APE",
	"AU",
	"AWB",//AMR-WB
	"CDA",
	"FLAC",
	"MOD",
	"MP3",//MPEG-2 Audio File
	"MPC",//Musepack Audio
	"RA",
	"TTA",
	"WAV",
	"CDXA",
	"SKM",
	"MTV",
	"MXF",
	"OTHER"
};

QString g_video_codec_strings[] =
{

	"MPEG1",
	"MPEG2",

	"MPEG4",
	"MPEG4",

	"MPEG4",//XVID
	"MPEG4",//DIVX

	/*ITU 系列*/
	"H261",
	"H263",
	"H263+",
	"H264",

	/*Real Media系列*/
	"RV10",
	"RV20",
	"RV30",
	"RV40",			

	/*Windows 系列*/
	"MPEG4",//Microsoft ISO MPEG-4 Video Codec
	"MPEG4",//Microsoft MPEG-4 Video Codec v2	MP42
	"MPEG4",//Microsoft MPEG-4 Video Codec v3	MP43

	"WMV",//Windows Media Video v7	WMV1
	"WMV",//Windows Media Video v8	WMV2
	"WMV",//Windows Media Video v9	WMV3 
	"WVC1",//Windows Media Video v9 Advanced Profile	WVC1
	"MSVIDEO1",//MS VIDEO 1
	"VP3",
	"VP4",
	"VP5",
	"VP6",
	"VP60",  
	"VP61",
	"VP62",
	"VP7",   
	"VP8",	

	/*其他*/
	"HFYU",		//Huffyuv
	"YUV",			//YUV
	"YV12",		//YV12
	"UYVY",		//UYVY
	"JPEG",        //JPEG11
	"PNG",

	"IV31",		//IndeoV31
	"IV32",

	//"FLV,			//FLV
	"MJPG",		//MOTION JPEG
	"LJPG",		//LossLess JPEG

	"ZLIB",		//AVIZlib
	"TSCC",		//tscc
	"CVID",		//Cinepak
	"RAW",			//uncompressed Video
	"SMC",			//Graphics
	"RLE",			//Animation
	"APPV",		//Apple Video
	"KPCD",		//Kodak Photo CD
	"SORV",		//Sorenson video
	"SCRN",		//Screen video
	"THEORA",		//Ogg Theora         
	"ZMP5",
	"CSF",
	"Unknown"//未知
};

QString g_audio_codec_strings[] =
{
	"AAC",		//LC-AAC/HE-AAC
	"AC3",				//AC3
	"AC3",			//E-AC-3
	"MP2",				//MPEG1 Layer 2
	"MP3",				//MPEG1 Layer 3
	"MP4",             //MPEG4 Audio
	"AMR_NB",			//AMR
	"AMR_WB",
	"DTS",
	"MIDI",
	"PCM",				//脉冲编码调制
	"DPCM",			//差分脉冲编码调制
	"APCM",			//自适应脉冲编码调制
	"ADPCM",			//自适应差分脉冲编码调制
	"PCM_LE",			//PCM Little Endian
	"PCM_BE",			//PCM Big	 Endian
	"COOK",			//rmvb AUDIO
	"SIPR",			//rmvb AUDIO
	"TSCC",			//tscc
	"QCELP",			//Qualcomm公司于1993年提出了可变速率的CELP，常称为QCELP",QCELP 语音压缩编码算法包括8K和13K两种
	"WMA",
	"WMA",
	"WMA",

	"WMA",
	"WMA",
	"WMA",

	"WMA",
	"WMA",
	"WMA",            //windows media audio 9.0

	"RAW",             //uncompressed audio
	"MAC",             //kMACE3Compression
	"IMA",             //kIMACompression", Samples have been compressed using IMA 4:1.
	"LAW",             //kULawCompression",kALawCompression", uLaw2:1
	"ACM",             //kMicrosoftADPCMFormat: Microsoft ADPCM-ACM code 2;kDVIIntelIMAFormat:DVI/Intel IMAADPCM-ACM code 17
	"DVA",             //kDVAudioFormat:DV Audio
	"QDM",             //kQDesignCompression:QDesign music
	"QCA",             //kQUALCOMMCompression:QUALCOMM PureVoice
	"G711",            //G.711 A-law logarithmic PCM and G.711 mu-law logarithmic PCM
	"NYM",             //Nellymoser
	"SPX",             //Speex
	"VORBIS",          //Ogg vorbis
	"APE",				//Monkey's audio
	"FLAC",
	"ALAC",
	"MLP",				//DVD-Audio MLP
	"CSF",
	"AIFF",			//AIFF
	"TTA",				//True Type Audio
	"VP7",
	"MPC",				//Musepack audio
	"TRUEHD",			//Dolby True HD
	"WVP",				//Wavpack
	"Unknown"//未知
};

QString g_toFormatString(QString formatName)
{
	E_CONTAINER con = ContainerFromString(formatName.toStdWString().c_str());
	if (con != _E_CON_OTHER && (int)con < sizeof(g_container_strings) )
		return g_container_strings[con];
	else
		return "Unknown";
}

QString g_toFormatString(E_CONTAINER con)
{
	if (con != _E_CON_OTHER && (int)con < sizeof(g_container_strings) )
		return g_container_strings[con];
	else
		return "Unknown";
}

QString g_toVideoFormatString(QString video)
{
	E_V_CODEC	vCodec = VideoEncoderFromString(video.toStdWString().c_str());
	if (vCodec != _V_OTHER && (int)vCodec < sizeof(g_video_codec_strings) )
		return g_video_codec_strings[vCodec];
	else
		return "Unknown";
}

QString g_toVideoFormatString(E_V_CODEC vCodec)
{
	if (vCodec != _V_OTHER &&  (int)vCodec < sizeof(g_video_codec_strings) )
		return g_video_codec_strings[vCodec];
	else
		return "Unknown";
}

QString g_toAudioFormatString(QString audio)
{
	E_A_CODEC	aCodec = AudioEncoderFromString(audio.toStdWString().c_str());
	if (aCodec != _A_OTHER && (int)aCodec < sizeof(g_audio_codec_strings) )
		return g_audio_codec_strings[aCodec];
	else
		return "Unknown";
}

QString g_toAudioFormatString(E_A_CODEC aCodec)
{
	if (aCodec != _A_OTHER &&  (int)aCodec < sizeof(g_audio_codec_strings) )
		return g_audio_codec_strings[aCodec];
	else
		return "Unknown";
}

bool g_isVideoFormat(QString str)
{
	E_V_CODEC	vCodec = VideoEncoderFromString(str.toStdWString().c_str());
	return vCodec != _V_OTHER;
}

bool g_isAudioFormat(QString str)
{
	E_A_CODEC	aCodec = AudioEncoderFromString(str.toStdWString().c_str());
	return aCodec != _A_OTHER;
}

QSize getFitSize(QSizeF destSize, QSizeF sizePicture)
{
	qreal asp_image = sizePicture.width() / sizePicture.height();
	qreal asp_dest = destSize.width() / destSize.height();

	qreal height = destSize.height();
	qreal width = destSize.width();
	if (asp_image < asp_dest)
	{
		height = destSize.height();
		width = height * asp_image;
	}
	else
	{
		width = destSize.width();
		height = width / asp_image;
	}
	return QSize(width, height);
}

void g_parse_cmdline(QString cmd, QStringList& urls, int* planguage)
{
	/*int pos = cmd.indexOf("-language:");

	if(pos >= 0 && planguage)
	{
		pos = pos + QString("-language:").length();
		QString lan = cmd.mid(pos, pos);
		int tmp = lan.at(0).toAscii() - '0';
		if(tmp >= 0 && tmp <= 3)
		{
			*planguage = tmp;
		}
	}
	pos = cmd.indexOf("-localfile ");
	if (pos != -1)
	{
		
	}*/

	//解析 -localfile "XXX1.xxx|XXX2.xxx"
	int pos = cmd.indexOf("localfile");
	if (pos != -1)
	{
		QString _tmp = cmd.mid(pos+QString("localfile").length());
		int pos_b = _tmp.indexOf('"');
		int pos_e = _tmp.lastIndexOf('"');
		QString _urls = _tmp.mid(pos_b+1, pos_e - pos_b-1);

		urls = _urls.split("|");

		int pos_end_localfile = cmd.lastIndexOf('"');
		cmd.remove(pos-1, pos_end_localfile - pos + 2);
	}

	QStringList parameters = cmd.split("-",QString::SkipEmptyParts);
	for (int i = 0; i < parameters.size(); i++)
	{
		QString tempParam = parameters[i];
		
		// Parse language
		if (tempParam == QString("cn"))
		{
			// 简体中文
			*planguage = 0;
			continue;
		}

		if (tempParam == QString("tw"))
		{
			// 繁体中文
			*planguage = 1;
			continue;
		}
	
		if (tempParam == QString("en"))
		{
			// 英文
			*planguage = 2;
			continue;
		}
	}

// 	int pos = 0;
// 	while(pos < cmd.length() && cmd.at(pos) == '-')
// 	{
// 		++pos;
// 		if(cmd.mid(pos, QString("language:").length()) == QString("language:"))
// 		{
// 			pos += QString("language:").length();
// 			if(planguage)
// 			{
// 				*planguage = cmd.at(pos).toAscii() - '0';
// 			}
// 			
// 		}
// 		else if(cmd.mid(pos, QString("localfile ").length()) == QString("localfile "))
// 		{
// 			//pos += QString("localfile ").length();
// 			QString _tmp = cmd.mid(pos+QString("localfile ").length());
// 			int pos_b = _tmp.indexOf('"');
// 			int pos_e = _tmp.lastIndexOf('"');
// 			QString _urls = _tmp.mid(pos_b+1, pos_e - pos_b-1);
// 
// 			urls = _urls.split("|");
// 			pos = pos_e;
// 		}
// 		pos = cmd.indexOf('-', pos);
// 
// 		if(pos < 0)
// 			break;
// 	}
}

void TranslateQList(QStringList& ql)
{
	QStringList::iterator i;
	for(i = ql.begin(); i != ql.end(); i++)
	{
		*i = QObject::tr(i->toAscii().constData());
	}
}