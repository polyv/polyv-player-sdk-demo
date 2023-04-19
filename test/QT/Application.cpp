#include "application.h"

#include <string>
#include <vector>
#include <sstream>
#include <QDir>
#include <QFile>
#include <QMessageBox>
#include <QStyleFactory>

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <shellapi.h>
#include <shlobj.h>
#include <Dwmapi.h>
#include "frameless-helper.h"
#endif // WIN32
#include <http/http-manager.h>

#include "AppDef.h"
#include "SdkManager.h"
#include "WidgetHelper.h"
#include "LoginDialog.h"
#include "MainWindow.h"
#include "platform.h"


// test
#include "MultiPlayerDialog.h"

extern void LogMessage(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
	QString logMessage = qFormatLogMessage(type, context, msg);
	std::string log = logMessage.toStdString();
	switch (type)
	{
	case QtInfoMsg:
		slog(SLOG_INFO, "[qt]:%s", log.c_str());
		break;
	case QtWarningMsg:
		if (!msg.contains("was loaded over HTTPS", Qt::CaseInsensitive)) {
			slog(SLOG_WARN, "[qt]:%s", log.c_str());
		}
		break;
	case QtCriticalMsg:
		slog(SLOG_ERROR, "[qt]:%s", log.c_str());
		break;
	case QtFatalMsg:
		slog(SLOG_FATAL, "[qt]:%s", log.c_str());
		break;	
	default:
		slog_debug(log.c_str());
#ifdef Q_OS_WIN
		OutputDebugString(reinterpret_cast<const wchar_t*>(logMessage.utf16()));
#endif
		break;
	}
    //qt_message_output(type, context, msg);
}

Application::Application(int &argc, char **argv) 
	: QApplication(argc, argv)
{
	HttpManager::Instance();
	QString configPath = GetConfigPath();
//#ifdef _WIN32
//	QString crashesPath = configPath;
//	crashesPath += "/";
//	crashesPath += APP_PROJECT_NAME;
//	crashesPath += "/crashes";
//	QDir().mkpath(crashesPath);
//	CreateExceptionDumpForRemote(crashesPath.toLocal8Bit(), NULL);
//#endif
	QString logPath = configPath;
	logPath += "/";
	logPath += APP_PROJECT_NAME;
	logPath += "/logs";
	QDir().mkpath(logPath);
	slog_init(logPath.toUtf8(), nullptr);

#ifdef _WIN32
	qInstallMessageHandler(LogMessage);
	setFont(QFont("Microsoft YaHei"));
#endif
#ifdef __APPLE__
	setFont(QFont("PingFang SC"));
#ifndef _DEBUG
	qInstallMessageHandler(LogMessage);
#endif
#endif

	QApplication::setStyle(QStyleFactory::create("Fusion"));
	QDir::setCurrent(applicationDirPath());
	setQuitOnLastWindowClosed(false);

	slog_info("app version:%s", APP_VERSION);
}

Application::~Application()
{
	if (globalConfig.IsOpen()) {
		globalConfig.Save();
	}
	globalConfig.Close();
	SdkManager::CloseManager();
	HttpManager::Destory();
	slog_info("app exit");
	slog_release();
}

QString Application::GetResourcesDir()
{
#ifdef _WIN32
	return App()->applicationDirPath();
#elif defined(__APPLE__)
	return App()->applicationDirPath() + "/../Resources";
#endif
}
QString Application::GetCacertFilePath()
{
	return GetResourcesDir() + "/cacert.pem";
}

int Application::AppRun()
{
    do {
        if (!InitConfig()) {
			slog_error("init config failed");
            break;
        }

        if (!InitTheme()) {
			slog_error("init theme failed");
            break;
        }

		if (!InitLocale()) {
			slog_error("init locale failed");
			break;
		}
		slog_info("app run ok");
		Translator translator;
		installTranslator(&translator);
		{// test
			//MultiPlayerDialog dlg;
			//dlg.exec();
		}
		{
			LoginDialog dlg;
			dlg.setWindowTitle(QTStr("MainTitle"));
			if (LoginDialog::Accepted != dlg.exec()) {
				break;
			}
			auto account = SdkManager::GetManager()->GetAccount();

			imagePath = GetConfigPath();
			imagePath += "/";
			imagePath += APP_PROJECT_NAME;
			imagePath += "/images/";
			imagePath += account.userId;
			QDir().mkpath(imagePath);

			localPath = GetConfigPath();
			localPath += "/";
			localPath += APP_PROJECT_NAME;
			localPath += "/locals/";
			localPath += account.userId;
			QDir().mkpath(localPath);
		}
		mainWindow = new MainWindow(nullptr);
		mainWindow->setWindowTitle(QTStr("MainTitle"));
		mainWindow->setAttribute(Qt::WA_DeleteOnClose, true);
		connect(mainWindow, SIGNAL(destroyed()), this, SLOT(quit()));
		//connect(mainWindow, SIGNAL(destroyed()), this, SLOT(OnDestroyWindow()), Qt::QueuedConnection);

		if (!mainWindow->Init()) {
			QMessageBox::critical(nullptr, QTStr("SystemError"), QTStr("CannotInitWindows"));
			break;
		}
		return exec();
    }while (false);
	slog_error("app run error");
    return -1;
}

MainWindow* Application::GetMainWindow(void) const
{
	return mainWindow;
}

bool Application::InitConfig()
{
	QString configPath = GetConfigPath();
	QString configFile = configPath;
	configFile += "/";
	configFile += APP_PROJECT_NAME;
	configFile += "/global.ini";
	if (!globalConfig.Open(configFile)) {
		slog_error("failed to open global.ini");
		return false;
	}
	//globalConfig.SetDefault("General", "Theme", APP_THEME);
	//globalConfig.SetDefault("General", "Language", APP_DEFAULT_LANG);

	globalConfig.SetDefault("Download", "TaskCount", 3);
	globalConfig.SetDefault("Download", "RetryCount", 0);

	QString videoPath = GetVideoPath();
	videoPath += "/";
	videoPath += APP_PROJECT_NAME;
	QDir().mkpath(videoPath);
	globalConfig.SetDefault("Download", "FilePath", videoPath);

	videoPath += "/";
	videoPath += "screenshot";
	QDir().mkpath(videoPath);
	globalConfig.SetDefault("Download", "ScreenshotPath", videoPath);

	globalConfig.SetDefault("Video", "HwdecEnable", false);
	globalConfig.SetDefault("Video", "KeepLastFrame", true);
#ifdef _WIN32
	globalConfig.SetDefault("Video", "SoftwareRecord", false);
	globalConfig.SetDefault("Video", "HdmiCallback", true);
#endif

	return globalConfig.IsOpen();
}
void Application::OnDestroyWindow()
{
	SdkManager::CloseManager();
	quit();
}

bool Application::InitTheme()
{
	QString fileName = QString(":/res/data/themes/Default.qss");
	QFile file(fileName);
	file.open(QFile::ReadOnly);
	setStyleSheet(file.readAll());
	file.close();

	return true;
	//appTheme = APP_THEME;
	//QString theme = globalConfig.Get("General", "Theme").toString();
	//if (theme.isEmpty()) {
	//	appTheme = theme;
	//}
	//QString themeFile = APP_DATA_PATH "/app/themes/";
	//themeFile += appTheme;
	//themeFile += ".qss";

	////if (!File::IsExist(themeFile.c_str())) {
	////	blog_error("[app]:failed to find %s", themeFile.c_str());
	////	return false;
	////}
	//QFile css(themeFile);
	//if (!css.open(QFile::ReadOnly | QFile::Text)) {
	//	slog_error("failed to open %s", QT_TO_UTF8(themeFile));
	//	return false;
	//}
	//setStyleSheet(css.readAll());
	////QString path = QString("file:///") + themeFile.c_str();
	////setStyleSheet(path);
	//return true;
}

bool Application::InitLocale()
{
	auto Set = [](const QString& lang) {
		QString file = QString(":/res/data/locale/%1.ini").arg(lang);
		if (!Translator::GetTranslator()->Append(file)) {
			slog_error("Failed to create locale from file %s",
				QT_TO_UTF8(file));
			return false;
		}
		return true;
	};
	QString def = APP_DEFAULT_LANG;
	if (!Set(def)) {
	}
	//QString lang = globalConfig.Get("General", "Language").toString(); 
	//if (lang == def) {
	//	return true;	
	//}
	//if (!lang.isEmpty()) {
	//	Set(lang);
	//}
	//auto locales = GetPreferredLocales();
	//for (auto & it : locales) {
	//	QString locale = QString::fromStdWString(it);
	//	if (locale == lang) {
	//		break;
	//	}
	//	Set(locale);
	//	slog_info("using preferred locale '%s'", QT_TO_UTF8(locale));
	//	break;
	//}
	return true;
}

