#include "application.h"

int main(int argc, char *argv[])
{
	QCoreApplication::addLibraryPath(".");
	QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
	QCoreApplication::setAttribute(Qt::AA_DontCreateNativeWidgetSiblings);
    Application a(argc, argv);
    return a.AppRun();
}
