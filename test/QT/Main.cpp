#include "application.h"

#ifdef WIN32
#include "frameless-helper.h"
#endif // WIN32

int main(int argc, char *argv[])
{
#ifdef _WIN32
    SetDpiAwareness(1);
#endif
	QCoreApplication::addLibraryPath(".");
	
#ifdef __APPLE__
    QCoreApplication::setAttribute(Qt::AA_DontCreateNativeWidgetSiblings);
#else
    //use system scale,but change scale:100%-175% to 100%; 200%-275% to 200%, etc...
    //QGuiApplication::setHighDpiScaleFactorRoundingPolicy(Qt::HighDpiScaleFactorRoundingPolicy::Floor);
#endif
    QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

    Application a(argc, argv);
    return a.AppRun();
}
