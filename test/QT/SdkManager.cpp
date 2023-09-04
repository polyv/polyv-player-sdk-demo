#include "SdkManager.h"

#include <log/log.h>
#include <QDateTime>

#include "AppDef.h"
#include "GlobalConfig.h"
#include "MainWindow.h"
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

	SetLogPath();
	SDK_CALL(PLVSetSdkCacertFile(QT_TO_UTF8(App()->GetCacertFilePath())));
	SetLogLevel();
	SetLogCallback();
	SetHwdecEnable();
	SetKeepLastFrame();
	SetVideoOutputDevice();
	SetHttpRequest();
	SetRetryCount();

	auto & osd = Player::GetOSDConfig();
	osd.enable = GlobalConfig::IsVideoOsd();
	auto & logo = Player::GetLogoConfig();
	logo.enable = GlobalConfig::IsVideoLogo();
	auto & cache = Player::GetCacheConfig();
	cache.enable = GlobalConfig::IsVideoCache();
	cache.maxCacheBytes = GlobalConfig::GetMaxCacheBytes();
	cache.maxCacheSeconds = GlobalConfig::GetMaxCacheSeconds();
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

void SdkManager::Init(const QString& userId, const QString secretKey)
{
	int ret = PLVInitSdkLibrary(QT_TO_UTF8(userId), QT_TO_UTF8(secretKey));
	if (E_NO_ERR != ret) {
		slog_error("init sdk error:%s", PLVGetSdkErrorDescription(ret));
		emit SignalInitResult(false, PLVGetSdkErrorDescription(ret));
		return;
	}
	account.userId = userId;
	account.secretKey = secretKey;
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
	logo.text = viewerName;
}

QString SdkManager::GetErrorDescription(int code)
{
	return PLVGetSdkErrorDescription(code);
}
bool SdkManager::CheckFileComplete(const QString& vid, const QString& path, int rate)
{
	return PLVCheckFileComplete(vid.toStdString().c_str(), path.toStdString().c_str(), rate);
}
bool SdkManager::DeleteLocalVideoFile(const QString& vid, const QString& path, int rate)
{
	return E_NO_ERR == PLVDeleteLocalVideoFile(vid.toStdString().c_str(), path.toStdString().c_str(), rate);
}
bool SdkManager::MigrateLocalVideoKeyFile(const QString& keyFile, const QString& secretKey)
{
	return E_NO_ERR == PLVMigrateLocalVideoKeyFile(keyFile.toStdString().c_str(), secretKey.toStdString().c_str());
}

void SdkManager::SetLogPath()
{
	auto path = GlobalConfig::GetLogPath();
	if (path.isEmpty()) {
		path = GetConfigPath();
		path += "/";
		path += APP_PROJECT_NAME;
		path += "/logs";
	}
	QString logFile = path;
	logFile += "/";
	logFile += QDateTime::currentDateTime().toString("yyyy-MM-dd hh-mm-ss");
	logFile += "_sdk.log";
	SDK_CALL(PLVSetSdkLogFile(QT_TO_UTF8(logFile)));
}
void SdkManager::SetLogLevel()
{
	SDK_CALL(PLVSetSdkLogLevel((LOG_FILTER_TYPE)GlobalConfig::GetLogLevel()));
}

static int FromSdkLogLevel(LOG_FILTER_TYPE level)
{
	int newLevel = -1;
	switch (level)
	{
	case LOG_FILTER_OFF:
		newLevel = SLOG_OFF;
		break;
	case LOG_FILTER_DEBUG:
		newLevel = SLOG_DEBUG;
		break;
	case LOG_FILTER_INFO:
		newLevel = SLOG_INFO;
		break;
	case LOG_FILTER_WARN:
		newLevel = SLOG_WARN;
		break;
	case LOG_FILTER_ERROR:
		newLevel = SLOG_ERROR;
		break;
	case LOG_FILTER_FATAL:
		newLevel = SLOG_FATAL;
		break;
	}
	return newLevel;
}
void SdkManager::SetLogCallback()
{
	SDK_CALL(PLVSetSdkLogMessageCallback(GlobalConfig::IsLogCallback(), 
		[](LOG_FILTER_TYPE level, const char* message, void* data) {
		(void)data;
		slog(FromSdkLogLevel(level), "[sdk]:%s", message);
		}, nullptr));
}

void SdkManager::SetHwdecEnable(void)
{
	SDK_CALL(PLVSetSdkHwdecEnable(GlobalConfig::IsHwdecEnable()));
}
void SdkManager::SetKeepLastFrame(void)
{
	SDK_CALL(PLVSetSdkKeepLastFrame(GlobalConfig::IsKeepLastFrame()));
}

void SdkManager::SetVideoOutputDevice()
{
	auto context = GlobalConfig::GetVideoOutputContext();
	SDK_CALL(PLVSetSdkVideoOutputDevice(
		(VIDEO_OUTPUT_DEVICE)GlobalConfig::GetVideoOutput(), context.isEmpty() ? NULL : context.toStdString().c_str()));
}

void SdkManager::SetHttpRequest()
{
	SDK_CALL(PLVSetSdkHttpRequest((SDK_HTTP_REQUEST)GlobalConfig::GetHttpRequest()));
}

void SdkManager::SetRetryCount()
{
	auto count = GlobalConfig::GetRetryCount();
	SDK_CALL(PLVSetSdkRetryAttempts(0 == count ? 0xFFFFFFFF : count, 500, 25000));
}

void SdkManager::SetSoftwareRecording()
{
	auto enable = GlobalConfig::IsSoftwareRecording();
	SDK_CALL(PLVSetPreventSoftwareRecording((void*)App()->GetMainWindow()->winId(), enable));

	SDK_CALL(PLVSetDetectSoftwareRecordingHandler(enable, 
		[](SOFTWARE_RECORDING_NOTIFY_TYPE type, const char* softwares, void* data) {
		(void)data;
		QMetaObject::invokeMethod(App()->GetMainWindow(), "OnDetectSoftwareRecording", Qt::QueuedConnection,
			Q_ARG(int, type), Q_ARG(QString, softwares));
	}, nullptr));
}

void SdkManager::SetHardwareRecording()
{
	SDK_CALL(PLVSetDetectHardwareRecordingHandler(GlobalConfig::IsHardwareRecording(),
		[](DEVICE_CHANGED_TYPE type, const char* device, void* data) {
		(void)data;
		QMetaObject::invokeMethod(App()->GetMainWindow(), "OnDetectHardwareRecording", Qt::QueuedConnection, 
			Q_ARG(int, type), Q_ARG(QString, device));
	}, nullptr));
}

QList<SdkManager::ValueItem> SdkManager::GetLogItems() const
{
	static QList<SdkManager::ValueItem> s_items = {
		{LOG_FILTER_OFF, "Off", false},
		{LOG_FILTER_DEBUG, "Debug", false},
		{LOG_FILTER_INFO, "Info", true},
		{LOG_FILTER_WARN, "Warn", false},
		{LOG_FILTER_ERROR, "Error", false},
		{LOG_FILTER_FATAL, "Fatal", false}
	};
	return s_items;
}
QList<SdkManager::ValueItem> SdkManager::GetHttpItems() const
{
	static QList<SdkManager::ValueItem> s_items = {
			{FIRST_HTTP_REQUEST, tr("RequestProtocolFirstHttp"), false},
			{FIRST_HTTPS_REQUEST, tr("RequestProtocolFirstHttps"), false},
			{ONLY_HTTP_REQUEST, tr("RequestProtocolOnlyHttp"), false},
			{ONLY_HTTPS_REQUEST, tr("RequestProtocolOnlyHttps"), true}
	};
	return s_items;
}
QList<SdkManager::ValueItem> SdkManager::GetOutputItems() const
{
	static QList<SdkManager::ValueItem> s_items = {
			{VIDEO_OUTPUT_NONE, "None", true},
			{VIDEO_OUTPUT_GPU, "GPU", false},
#ifdef _WIN32
			{VIDEO_OUTPUT_DIRECT3D, "D3D", false},
			{VIDEO_OUTPUT_GDI, "GDI", false}
#endif// end _WIN32
	};
	return s_items;
}
QList<SdkManager::ValueItem> SdkManager::GetRateItems() const
{
	static QList<SdkManager::ValueItem> s_items = {
			{-1, "None", false},
			{VIDEO_RATE_AUTO, tr("Auto"), true},
			{VIDEO_RATE_LD, tr("LD"), false},
			{VIDEO_RATE_SD, tr("SD"), false},
			{VIDEO_RATE_HD, tr("HD"), false},
			{VIDEO_RATE_SOURCE, tr("Source"), false}
	};
	return s_items;
}
QStringList SdkManager::GetOutputContexts() const
{
	static QStringList s_items = { "auto", "angle",
#ifdef _WIN32
		"d3d11", "win", "dxinterop"
#endif// end _WIN32
	};
	return s_items;
}
