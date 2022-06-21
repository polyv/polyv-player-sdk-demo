#include "SdkManager.h"

#include <log/log.h>
#include <QDateTime>

#include "AppDef.h"
#include "Application.h"
#include "WidgetHelper.h"


SdkManager* SdkManager::manager = nullptr;
SdkManager::SdkManager(void)
	: QObject(nullptr)
{
	slog_info("sdk version:%s", PLVGetSdkVersion());
	QString configPath = GetConfigPath();
	QString logFile = configPath;
	logFile += "/";
	logFile += APP_PROJECT_NAME;
	logFile += "/logs/";
	logFile += QDateTime::currentDateTime().toString("yyyy-MM-dd hh-mm-ss");
	logFile += "_sdk.log";
	PLVSetSdkLogFile(QT_TO_UTF8(logFile));
	PLVSetSdkLogLevel(LOG_FILTER_DEBUG);
	PLVSetSdkHttpRequest(httpRequest);
	PLVSetSdkCacertFile(QT_TO_UTF8(App()->GetCacertFilePath()));
	
	SetHwdecEnable();
	SetKeepLastFrame();
}
SdkManager::~SdkManager()
{
	PLVReleaseSdkLibrary();
}

SdkManager* SdkManager::GetManager()
{
	if (nullptr == manager) {
		manager = new SdkManager();
	}
	return manager;

}
void SdkManager::CloseManager()
{
	if (!manager) {
		return;
	}
	delete manager;
	manager = nullptr;
}

void SdkManager::Init(const QString& userId, const QString secretKey, const QString& readToken)
{
	int ret = PLVInitSdkLibrary(QT_TO_UTF8(userId), QT_TO_UTF8(secretKey), QT_TO_UTF8(readToken));
	if (E_NO_ERR != ret) {
		slog_error("init sdk error:%s", PLVGetSdkErrorDescription(ret));
		emit SignalInitResult(false, PLVGetSdkErrorDescription(ret));
		return;
	}
	account.userId = userId;
	account.secretKey = secretKey;
	account.readToken = readToken;
	emit SignalInitResult(true, QString());
}

void SdkManager::SetViewer(const QString& viewerId, const QString& viewerName, const QString& viewerAvatar)
{
	account.viewerId = viewerId;
	account.viewerName = viewerName;
	account.viewerAvatar = viewerAvatar;

	PLVSetSdkViewerInfo(account.viewerId.toStdString().c_str(), 
		account.viewerName.toStdString().c_str(),
		account.viewerAvatar.toStdString().c_str());
}

void SdkManager::SetHwdecEnable(void)
{
	PLVSetSdkHwdecEnable(App()->GlobalConfig().Get("Video", "HwdecEnable").toBool());
}
void SdkManager::SetKeepLastFrame(void)
{
	PLVSetSdkKeepLastFrame(App()->GlobalConfig().Get("Video", "KeepLastFrame").toBool());
}