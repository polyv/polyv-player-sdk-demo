#pragma once
#include <QString>
#include <QVariant>
#include <QTranslator>
#include <QSettings>
#include <QWidget>
#include <QMap>
#include <QTime>
#include <QLabel>
#include <atomic>
#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include "plv-player-core.h"
#include "plv-player-download.h"
#include "curl/curl.h"

class SimpleTranslator : public QTranslator {
    Q_OBJECT
public:
    SimpleTranslator(const QString& path);
    virtual bool isEmpty() const override;
    virtual QString translate(const char* context, const char* sourceText,
        const char* disambiguation, int n) const override;
private:
    QSettings settings;
};

class SimplePlayWidget : public QWidget {
    Q_OBJECT
public:
    SimplePlayWidget(QWidget* parent = nullptr);
    void SetPreventSoftwareRecording(bool enable);
    void EnterFullScreen();
    void LeaveFullScreen();
    void SwitchFullScreen();
    void keyPressEvent(QKeyEvent*) override;
    void closeEvent(QCloseEvent*) override;
private:
    bool preventSoftwareRecording{ false };
};
class SimplePlayProcess : public QWidget {
    Q_OBJECT
public:
    SimplePlayProcess(QWidget* parent = nullptr);
    void SetMaxValue(qint64);
    void SetCurrentValue(qint64);
    void SetCacheValue(qint64);
signals:
    void seek(qint64);
protected:
    void paintEvent(QPaintEvent*) override;
    void mouseMoveEvent(QMouseEvent*) override;
    void mousePressEvent(QMouseEvent*) override;
private:
    qint64 maxValue = 1;
    qint64 currentValue = 0;
    qint64 cacheValue = 0;
};

class SimpleDownloadDetails : public QWidget {
    Q_OBJECT
public:
    SimpleDownloadDetails(QVariantMap video, QVariantMap rateInfo, QWidget* parent = nullptr);
};
class SimpleDownloadProcess : public QWidget {
    Q_OBJECT
public:
    SimpleDownloadProcess(QWidget* parent = nullptr);
    void SetMaxValue(int64_t);
    void SetCurrentValue(int64_t);
protected:
    void paintEvent(QPaintEvent*) override;
private:
    int64_t maxValue = 1;
    int64_t currentValue = 0;
    QTime startTime = QTime::currentTime();
    QLabel* speedLabel = nullptr;
    QLabel* percentLabel = nullptr;
    QLabel* remainLabel = nullptr;
};
class SimpleDownloadOperator : public QWidget {
    Q_OBJECT
public:
    SimpleDownloadOperator(bool localFile = false, QWidget* parent = nullptr);
signals:
    void start();
    void stop();
    void pause();
    void resume();
    void openDir();
    void play();
    void deleted();
};

#define QTStr(name) QApplication::translate(nullptr, name)
#define QT_UTF8(str) QString::fromUtf8(str)
#define QT_TO_UTF8(str) str.toUtf8().constData()
#define SIMPLE_DEMO_NAME "plv-player-sdk-simple-demo"

QString Encrypt(const QString& src);
QString Decrypt(const QString& src);
QString GetConfigPath();
QString GetLogPath();
QString GetCrashPath();
QString GetCAPath();
QString GetVideoPath();
QString GetScreenshotPath();
QVariantList MakeVideoInfoList(const PLVVideoRequestInfo* infos, int num);
QVariantMap MakeVideoPageInfo(const PLVVideoRequestPageInfo* info);
QString GetFileSizeString(int64_t size);
QString GetTimeString(int64_t s);
QString GetRateName(int type);
QString GetRateListString(QVariantList rates);
QString GetStateName(int state);
QString GetPlayStateString(int rate, int state);

//note:This is just an example, please get the token on the server!
bool GetVideoToken(const QMap<QString, QString>& params, QString& token, std::atomic_bool* abort);
