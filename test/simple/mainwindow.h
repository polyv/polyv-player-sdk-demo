#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <thread>
#include "util.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

enum TabType {
    TabVideoList = 0,
    TabVideoPlay = 1,
    TabVideoDownload = 2,
    TabVideoCustomRender = 3,
};
//value same as LOG_FILTER_TYPE
enum MsgType {
    MsgOff = 0,
    MsgLogDebug = 1,
    MsgLogInfo = 2,
    MsgLogWarn = 3,
    MsgLogError = 4,
    MsgLogFatal = 5,
    MsgCallback = 100,
    MsgAPI = 101,
    MsgTips = 102,
};

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    static MainWindow* main;
private:
    Ui::MainWindow *ui;

private slots:
    void OnShowMsg(const QString& msg, int level = MsgCallback);
    void on_msgSearchLineEdit_returnPressed();
    void on_msgSearchPushButton_clicked();
    //init
    void on_initPushButton_clicked();
    void on_freePushButton_clicked();
    //detect recording
    void on_detectSoftwareRecordCheckBox_clicked();
    void on_detectHardwareRecordCheckBox_clicked();
    //video list
    void on_requestPushButton_clicked();
    void on_searchPushButton_clicked();
    void on_cancelPushButton_clicked();
    void OnVideoInfo(int code, QVariantList videoList, QVariantMap pageInfo);
    //play control
    void on_playModeComboBox_currentIndexChanged(int);
    void on_tokenCheckBox_stateChanged(int);
    void on_tokenRequestPushButton_clicked();
    void OnGetToken(bool, QString);
    void on_browsePushButton_clicked();
    void on_playPushButton_clicked();
    void on_stopPushButton_clicked();
    void on_pausePushButton_clicked();
    void on_resumePushButton_clicked();
    void on_shotScreenPushButton_clicked();
    void on_fullScreenPushButton_clicked();
    void on_speedComboBox_currentIndexChanged(int);
    void on_muteCheckBox_clicked();
    void on_volumeHorizontalSlider_valueChanged(int);
    void on_hardwareDecodeCheckBox_clicked();
    void on_keepLastFrameCheckBox_clicked();
    void on_videoOutputComboBox_currentIndexChanged(int);
    void on_logoEnableCheckBox_clicked();
    void on_osdEnableCheckBox_clicked();
    void on_playRateComboBox_activated(int);
    void OnMediaState(QString vid, int state);
    void OnMediaProperty(QString vid, int property, int format, QString value);
    void OnMediaRateChanged(QString vid, int inputRate, int realRate);
    void OnMediaProcess(QString vid, int ms);
    void OnSeek(qint64 ms);
    void on_shrinkOrExpandPushButton_clicked();
    //download
    void on_localVideoBrowserPushButton_clicked();
    void OnAddDownloader(QVariantMap video, QVariantMap rateInfo);
    void OnDownloaderProgress(qint64 downloader, QString vid, qint64 current, qint64 total);
    void OnDownloaderComplete(qint64 downloader, QString vid, int rate, int code);
    void on_localVideoPathLineEdit_textChanged(const QString& text);
    //custom render
    void on_customPlayPushButton_clicked();
    void on_customStopPushButton_clicked();
    void OnCustomRenderUpdate();
    void on_customSpeedComboBox_currentIndexChanged();
    void on_customPlayRateComboBox_currentIndexChanged();
private:
    bool eventFilter(QObject* obj, QEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
    void closeEvent(QCloseEvent* event) override;
private:
    void EnableControl(bool);
    void LoadConfig();
    void SaveConfig();
    void RequestVideoList(PLVVideoRequestParam* param);
    QMap<QString, QString> MakeTokenParams(QString vid, bool online);
    //play
    void InitPlayer();
    void FreePlayer();
    void PlayOnline(QString vid, bool encrypt, int rate);
    void PlayOffline(QString videoPath, QString vid, int rate);
    bool IsPlaying();
    //download
    int FindDownloader(int64_t downloader);
    void FreeDownloaders();
    void ScanFiles();
private:
    //token
    std::atomic_bool abortGetToken{ false };
    std::thread getTokenThread;
    bool gettingToken{ false };
    //player
    SimplePlayWidget* playWnd{ nullptr };
    int mediaState{ MEDIA_STATE_NONE };
    PLVPlayerPtr player{ nullptr };
    bool onlinePlay{ true };
    QString playerVid;
    int inputRate{ VIDEO_RATE_AUTO };
    int currentRate{ VIDEO_RATE_AUTO };
    QString duration;
    QString position;
    QString hwdec;
    QString vCodec;
    QString vBitrate;
    QString fps;
    QString width;
    QString height;
    QString aCodec;
    QString aBitrate;
    QString cacheSpeed;
    //custom render
#define MIN_RENDER_WIDTH 256
#define MIN_RENDER_HEIGHT 144
#define MAX_RENDER_WIDTH 1920
#define MAX_RENDER_HEIGHT 1088 //1088 align to 16! more fit!
    PLVPlayerPtr customPlayer{ nullptr };
    std::atomic_bool renderBufferFill{ false };
    PLVVideoFrame renderFrame{ nullptr };
    std::atomic_uint64_t displaySize{ (960ull << 32)| 540ull };
};
#endif // MAINWINDOW_H
