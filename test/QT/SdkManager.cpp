#include "SdkManager.h"

#include <log/log.h>
#include <QDateTime>

#include "AppDef.h"
#include "Application.h"
#include "WidgetHelper.h"

#define SDK_CALL(api) \
{	int ret = api;\
	if (E_NO_ERR != ret) {\
		slog_error("sdk error code:%d, msg:%s", ret, PLVGetSdkErrorDescription(ret)); \
	}\
}

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
	SDK_CALL(PLVSetSdkLogFile(QT_TO_UTF8(logFile)));
	SDK_CALL(PLVSetSdkLogLevel(LOG_FILTER_DEBUG));
	SDK_CALL(PLVSetSdkHttpRequest(httpRequest));
	SDK_CALL(PLVSetSdkCacertFile(QT_TO_UTF8(App()->GetCacertFilePath())));
	
	SetHwdecEnable();
	SetKeepLastFrame();

	int type = App()->GlobalConfig().Get("Video", "VideoOutput").toInt();
	SetVideoOutputDevice(VIDEO_OUTPUT_DEVICE(type));

	auto & osd = Player::GetOSDConfig();
	osd.enable = App()->GlobalConfig().Get("Video", "EnableOSD", true).toBool();
	auto & logo = Player::GetLogoConfig();
	logo.enable = App()->GlobalConfig().Get("Video", "EnableLogo", true).toBool();
	auto & cache = Player::GetCacheConfig();
	cache.enable = App()->GlobalConfig().Get("Video", "EnableCache", false).toBool();
	cache.maxCacheBytes = App()->GlobalConfig().Get("Video", "MaxCacheBytes", -1).toInt();
	cache.maxCacheSeconds = App()->GlobalConfig().Get("Video", "MaxCacheSeconds", -1).toInt();
}
SdkManager::~SdkManager()
{
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
	manager->Release();
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

void SdkManager::Release()
{
	PLVReleaseSdkLibrary();
}

void SdkManager::SetViewer(const QString& viewerId, const QString& viewerName, const QString& viewerAvatar)
{
	account.viewerId = viewerId;
	account.viewerName = viewerName;
	account.viewerAvatar = viewerAvatar;

	SDK_CALL(PLVSetSdkViewerInfo(account.viewerId.toStdString().c_str(),
		account.viewerName.toStdString().c_str(),
		account.viewerAvatar.toStdString().c_str()));

	auto & osd = Player::GetOSDConfig();
	osd.text = viewerName;

	auto & logo = Player::GetLogoConfig();
	logo.text = "POLYV";
}

QString SdkManager::GetErrorDescription(int code)
{
	return PLVGetSdkErrorDescription(code);
}

void SdkManager::SetHwdecEnable(void)
{
	SDK_CALL(PLVSetSdkHwdecEnable(App()->GlobalConfig().Get("Video", "HwdecEnable").toBool()));
}
void SdkManager::SetKeepLastFrame(void)
{
	SDK_CALL(PLVSetSdkKeepLastFrame(App()->GlobalConfig().Get("Video", "KeepLastFrame").toBool()));
}

void SdkManager::SetVideoOutputDevice(VIDEO_OUTPUT_DEVICE type, const QString& context/* = QString()*/)
{
	SDK_CALL(PLVSetSdkVideoOutputDevice(type, context.isEmpty() ? NULL : context.toStdString().c_str()));
}
bool SdkManager::CheckFileComplete(const QString& vid, const QString& path, int rate)
{
	return PLVCheckFileComplete(vid.toStdString().c_str(), path.toStdString().c_str(), rate);
}

#ifdef _WIN32
bool SdkManager::GetSoftwareRecord(void* window)
{
	bool enable = false;
	SDK_CALL(PLVGetPreventRecord(window, &enable));
	return enable;
}
void SdkManager::SetSoftwareRecord(void* window, bool enable)
{
	SDK_CALL(PLVSetPreventRecord(window, enable));

	SDK_CALL(PLVSetPluginInjectHandler(enable, [](void* data) {
		SdkManager* obj = (SdkManager*)data;
		QMetaObject::invokeMethod(obj, "OnPluginInject", Qt::QueuedConnection);
	}, this));
}
void SdkManager::SetHdmiRecord(bool enable)
{
	SDK_CALL(PLVSetHDMIDeviceChangedHandler(enable, [](HDMI_DEVICE_TYPE type, const char* device, void* data) {
		SdkManager* obj = (SdkManager*)data;
		QMetaObject::invokeMethod(obj, "OnHDMIDeviceChanged", Qt::QueuedConnection,
			Q_ARG(int, type), Q_ARG(QString, device));
	}, this));
}
#endif

void SdkManager::OnPluginInject()
{
	emit SignalPluginInject();
}
void SdkManager::OnHDMIDeviceChanged(int type, QString device)
{
	emit SignalHDMIDeviceChanged(type, device);
}