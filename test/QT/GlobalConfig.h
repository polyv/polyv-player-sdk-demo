#pragma once
#include "Application.h"

#ifndef DEF_FUN_INT
#define DEF_FUN_INT(name, tag, key)\
	static int Get##name(void){\
		return App()->GlobalConfig().Get(#tag, #key).toInt();\
	}\
	static void Set##name(int val){\
		App()->GlobalConfig().Set(#tag, #key, val);\
	}\
	static void SetDefault##name(int val) {\
		App()->GlobalConfig().SetDefault(#tag, #key, val); \
	}
#endif// DEF_FUN_INT

#ifndef DEF_FUN_BOOL
#define DEF_FUN_BOOL(name, tag, key)\
	static bool Is##name(void){\
		return App()->GlobalConfig().Get(#tag, #key).toBool();\
	}\
	static void Set##name(bool val){\
		App()->GlobalConfig().Set(#tag, #key, val);\
	}\
	static void SetDefault##name(bool val) {\
		App()->GlobalConfig().SetDefault(#tag, #key, val); \
	}
#endif// DEF_FUN_BOOL

#ifndef DEF_FUN_STRING
#define DEF_FUN_STRING(name, tag, key)\
	static QString Get##name(void){\
		return App()->GlobalConfig().Get(#tag, #key).toString();\
	}\
	static void Set##name(const QString& val){\
		App()->GlobalConfig().Set(#tag, #key, val);\
	}\
	static void SetDefault##name(const QString& val) {\
		App()->GlobalConfig().SetDefault(#tag, #key, val); \
	}
#endif// DEF_FUN_INT

class GlobalConfig{
public:
	static void InitDefault();

	DEF_FUN_BOOL(Remember, App, Remember);
	DEF_FUN_STRING(UserId, App, UserId);
	DEF_FUN_STRING(SecretKey, App, SecretKey);
	DEF_FUN_STRING(ReadToken, App, ReadToken);

	DEF_FUN_INT(HttpRequest, App, HttpRequest);
	DEF_FUN_STRING(LogPath, App, LogPath);
	DEF_FUN_INT(LogLevel, App, LogLevel);
	DEF_FUN_BOOL(LogCallback, App, LogCallback);
	DEF_FUN_BOOL(SoftwareRecording, App, SoftwareRecording);
	DEF_FUN_BOOL(HardwareRecording, App, HardwareRecording);

	DEF_FUN_BOOL(AutoDownRate, App, AutoDownRate);
	DEF_FUN_BOOL(PlayWithToken, App, PlayWithToken);
	DEF_FUN_INT(PlayWithRate, App, PlayWithRate);

	DEF_FUN_INT(TaskCount, Download, TaskCount);
	DEF_FUN_INT(RetryCount, Download, RetryCount);
	DEF_FUN_STRING(SaveVideoPath, Download, SaveVideoPath);
	DEF_FUN_STRING(SaveScreenshotPath, Download, SaveScreenshotPath);

	DEF_FUN_BOOL(HwdecEnable, Video, HwdecEnable);
	DEF_FUN_BOOL(KeepLastFrame, Video, KeepLastFrame);
	DEF_FUN_INT(VideoPlaySeek, Video, VideoPlaySeek);
	DEF_FUN_INT(VideoOutput, Video, VideoOutput);
	DEF_FUN_STRING(VideoOutputContext, Video, VideoOutputContext);
	DEF_FUN_BOOL(VideoOsd, Video, EnableVideoOsd);
	DEF_FUN_BOOL(VideoLogo, Video, EnableVideoLogo);
	DEF_FUN_BOOL(VideoCache, Video, EnableVideoCache);
	DEF_FUN_INT(MaxCacheBytes, Video, MaxCacheBytes);
	DEF_FUN_INT(MaxCacheSeconds, Video, MaxCacheSeconds);


	static void Save();


private:  // emphasize the following members are private
	GlobalConfig(const GlobalConfig&) = delete;
	const GlobalConfig& operator=(const GlobalConfig&) = delete;
};
