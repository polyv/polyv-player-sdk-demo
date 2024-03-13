#include "mainwindow.h"
#include <QApplication>
#include <QTranslator>
#include <QSettings>
#include <QDir>
#include "util.h"

#ifdef _WIN32
#include <Dbghelp.h>
#pragma comment(lib, "dbghelp.lib")
LONG WINAPI ApplicationCrashHandler(struct _EXCEPTION_POINTERS* lpExceptionInfo)
{
    QString filepath = GetCrashPath();
    QDir crashDir(QFileInfo(filepath).path());
    if (!crashDir.exists()) {
        crashDir.mkpath(".");
    }
    HANDLE dump = CreateFileW(filepath.toStdWString().c_str(), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (dump != NULL && dump != INVALID_HANDLE_VALUE) {
        MINIDUMP_EXCEPTION_INFORMATION exInfo;
        exInfo.ThreadId = ::GetCurrentThreadId();
        exInfo.ExceptionPointers = lpExceptionInfo;
        exInfo.ClientPointers = false;
        MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(), dump, MiniDumpNormal, &exInfo, NULL, NULL);
        CloseHandle(dump);
    }
    return EXCEPTION_EXECUTE_HANDLER;
}
#endif

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
#ifdef _WIN32
    SetUnhandledExceptionFilter(ApplicationCrashHandler);
#endif
    SimpleTranslator translator(QT_UTF8(":/res/data/locale/zh-CN.ini"));
    a.installTranslator(&translator);

    QDir::setCurrent(a.applicationDirPath());
    curl_global_init(CURL_GLOBAL_ALL);
    MainWindow* w = new MainWindow();
    w->show();
    int ret = a.exec();
    delete w;
    curl_global_cleanup();

    return ret;
}
