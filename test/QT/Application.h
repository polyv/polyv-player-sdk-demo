#pragma once
#include <QDebug>
#include <QTranslator>
#include <QApplication>

#include <log/log.h>

#include "qt-helper.h"
#include "configure.h"
#include "translator.h"


class MainWindow;
class Application : public QApplication{
    Q_OBJECT
public:
    Application(int &argc, char **argv);
	~Application();


public:
	inline QString Translate(const char* lookup) const {
		return QTStr(lookup);
	}
	
    int AppRun();
	MainWindow* GetMainWindow(void) const;
	QString GetImagePath(void) const {
		return imagePath;
	}
	QString GetLocalPath(void) const {
		return localPath;
	}
	QString GetResourcesDir();
	QString GetCacertFilePath();

	Configure& GlobalConfig() {
		return globalConfig;
	}

private slots:
	void OnDestroyWindow();
private:
	bool InitTheme();
	bool InitConfig();
	bool InitLocale();	
	//QString appTheme;
	//QString appLocale;
	
	QString imagePath;
	QString localPath;

	Configure globalConfig;

	MainWindow* mainWindow = nullptr;
};

inline Application *App(){
    return static_cast<Application *>(qApp);
}



