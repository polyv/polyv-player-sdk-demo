#include "GlobalConfig.h"

#include <QDir>

#include "AppDef.h"
#include "SdkManager.h"
#include "WidgetHelper.h"

void GlobalConfig::InitDefault()
{
	QString path = GetVideoPath();
	path += "/";
	path += APP_PROJECT_NAME;
	QDir().mkpath(path);
	SetDefaultSaveVideoPath(path);

	path += "/";
	path += "screenshot";
	QDir().mkpath(path);
	SetDefaultSaveScreenshotPath(path);

	path = GetConfigPath();
	path += "/";
	path += APP_PROJECT_NAME;
	path += "/logs";
	SetDefaultLogPath(path);

	SetDefaultTaskCount(3);
	SetDefaultRetryCount(0);
	SetDefaultHwdecEnable(false);
	SetDefaultKeepLastFrame(true);
	SetDefaultSoftwareRecording(false);
	SetDefaultHardwareRecording(true);
	SetDefaultLogLevel(LOG_FILTER_INFO);
	SetDefaultLogCallback(false);
	SetDefaultVideoOutput(VIDEO_OUTPUT_NONE);
	SetDefaultHttpRequest(ONLY_HTTPS_REQUEST);
	SetDefaultVideoCache(false);
	SetDefaultVideoLogo(true);
	SetDefaultVideoOsd(true);
	SetDefaultVideoPlaySeek(0);
}

void GlobalConfig::Save()
{
	App()->GlobalConfig().Save();
}