#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QSettings>
#include <QDateTime>
#include <QFileDialog>
#include <thread>
#include <atomic>
#include <QContextMenuEvent>
#include <QKeyEvent>
#include <QMenu>
#include <QMessageBox>
#include <QDesktopServices>
#include <QShortcut>
#include <QScrollBar>
#include "playimage.h"

#ifndef ALIGN_SIZE
#define ALIGN_SIZE(size, align) size = (((size) + (align - 1)) & (~(align - 1)))
#endif
static inline int GetAlignment() {
    return 64;//mpv require!
}
static uint64_t GetRGBXSize(uint32_t width, uint32_t height)
{
    int alignment = GetAlignment();
    size_t stride = width * 4;
    ALIGN_SIZE(stride, alignment);
    size_t size = stride * height;
    ALIGN_SIZE(size, alignment);
    return size;
}
static void FillRGBXPlanes(PLVVideoFrame* frame, uint8_t* data) {
    uint32_t width = frame->width, height = frame->height;
    int alignment = GetAlignment();
    frame->data[0] = data;
    frame->linesize[0] = width * 4;
    ALIGN_SIZE(frame->linesize[0], alignment);
}

MainWindow* MainWindow::main = nullptr;
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    main = this;
    ui->setupUi(this);
    ui->videoTableWidget->installEventFilter(this);
    connect(ui->subAccountCheckBox, &QCheckBox::stateChanged, this,
        [this]() {
            bool subAccount = ui->subAccountCheckBox->isChecked();
            ui->userIdLineEdit->setEnabled(!subAccount);
            ui->secretKeyLineEdit->setEnabled(!subAccount);
            ui->appIdLineEdit->setEnabled(subAccount);
            ui->appSecretLineEdit->setEnabled(subAccount);
        });
    EnableControl(false);
    LoadConfig();
    //tips
    OnShowMsg(QTStr("DemoUseTips1"), MsgTips);
    OnShowMsg(QTStr("DemoUseTips2"), MsgTips);
}

MainWindow::~MainWindow()
{
    on_freePushButton_clicked();
    SaveConfig();
    delete ui;
}

void MainWindow::OnShowMsg(const QString& msg, int level)
{
    QTextCursor cursor = ui->msgPlainTextEdit->textCursor();
    cursor.movePosition(QTextCursor::End);
    QTextCharFormat fmt;
    if (level == MsgOff) {
        fmt.setForeground(Qt::black);
        cursor.setCharFormat(fmt);
    }
    else if (level == MsgLogDebug) {
        fmt.setForeground(Qt::lightGray);
        cursor.setCharFormat(fmt);
        cursor.insertText("[Debug]");
    }
    else if (level == MsgLogInfo) {
        fmt.setForeground(Qt::blue);
        cursor.setCharFormat(fmt);
        cursor.insertText("[Info]");
    }
    else if (level == MsgLogWarn) {
        fmt.setForeground(Qt::darkMagenta);
        cursor.setCharFormat(fmt);
        cursor.insertText("[Warn]");
    }
    else if (level == MsgLogError) {
        fmt.setForeground(Qt::magenta);
        cursor.setCharFormat(fmt);
        cursor.insertText("[Error]");
    }
    else if (level == MsgLogFatal) {
        fmt.setForeground(Qt::magenta);
        cursor.setCharFormat(fmt);
        cursor.insertText("[Fatal]");
    }
    else if (level == MsgCallback) {
        fmt.setForeground(Qt::red);
        cursor.setCharFormat(fmt);
        cursor.insertText("[Callback]");
    }
    else if (level == MsgAPI) {
        fmt.setForeground(Qt::red);
        cursor.setCharFormat(fmt);
        cursor.insertText("[API]");
    }
    else if (level == MsgTips) {
        fmt.setForeground(Qt::red);
        cursor.setCharFormat(fmt);
    }
    cursor.insertText(msg);
    cursor.insertText("\n");
    if (ui->autoScrollCheckBox->isChecked()) {
        ui->msgPlainTextEdit->verticalScrollBar()->setValue(ui->msgPlainTextEdit->verticalScrollBar()->maximum());
    }
}

void MainWindow::on_msgSearchLineEdit_returnPressed()
{
    on_msgSearchPushButton_clicked();
}

void MainWindow::on_msgSearchPushButton_clicked()
{
    QString text = ui->msgSearchLineEdit->text();
    if (!text.isEmpty()) {
        bool ret = ui->msgPlainTextEdit->find(text, QTextDocument::FindBackward);
        if (!ret) {
            ui->msgPlainTextEdit->moveCursor(QTextCursor::End);
            ret = ui->msgPlainTextEdit->find(text, QTextDocument::FindBackward);
        }
    }
}

void MainWindow::on_initPushButton_clicked()
{
    if ((!ui->subAccountCheckBox->isChecked() && ui->userIdLineEdit->text().isEmpty()) ||
        (ui->subAccountCheckBox->isChecked() && ui->appIdLineEdit->text().isEmpty())) {
        OnShowMsg(QT_UTF8("set account!"), MsgOff);
        return;
    }
    int errCode = E_NO_ERR;
    OnShowMsg(QString("sdk version:%1").arg(PLVGetSdkVersion()), MsgOff);
    bool logCallback = ui->logCallbackCheckBox->isChecked();
    PLVSetSdkLogMessageCallback(logCallback,
        [](LOG_FILTER_TYPE level, const char* message, void* data) {
            auto main = static_cast<MainWindow*>(data);
            QMetaObject::invokeMethod(main, "OnShowMsg", Q_ARG(const QString&, message), Q_ARG(int, level));
        }, this);
    PLVSetSdkLogLevel((LOG_FILTER_TYPE)ui->logLevelComboBox->currentIndex());
    PLVSetSdkLogFile(QT_TO_UTF8(ui->logPathLineEdit->text()));
    int retryAttempts = ui->retryAttemptsLineEdit->text().toInt();
    int retryMinSpace = ui->retryMinSpaceLineEdit->text().toInt();
    int retryMaxSpace = ui->retryMaxSpaceLineEdit->text().toInt();
    PLVSetSdkRetryAttempts(retryAttempts, retryMinSpace, retryMaxSpace);
    QString playSeed = ui->playSeedLineEdit->text();
    PLVSetSdkSeed(QT_TO_UTF8(playSeed));
    PLVSetSdkLocalRememberPlay(ui->localRememberFrameCheckBox->isChecked());
    QString viewerId = ui->viewerIdLineEdit->text();
    QString viewerName = ui->viewerNameLineEdit->text();
    PLVSetSdkViewerInfo(QT_TO_UTF8(viewerId), QT_TO_UTF8(viewerName), nullptr);
    std::string userId = ui->userIdLineEdit->text().toStdString();
    std::string secretKey = ui->secretKeyLineEdit->text().toStdString();
    std::string appId = ui->appIdLineEdit->text().toStdString();
    std::string appSecret = ui->appSecretLineEdit->text().toStdString();
    bool useSubAccount = ui->subAccountCheckBox->isChecked();
    PLVAccountInfo accountInfo;
    accountInfo.userId = !useSubAccount ? userId.c_str() : nullptr;
    accountInfo.secretKey = !useSubAccount ? secretKey.c_str() : nullptr;
    accountInfo.appId = useSubAccount ? appId.c_str() : nullptr;
    accountInfo.appSecret = useSubAccount ? appSecret.c_str() : nullptr;
    accountInfo.subAccount = useSubAccount;
    errCode = PLVInitSdkLibrary(&accountInfo);
    if (errCode != E_NO_ERR) {
        QString msg = QString("code:%1,msg:%2").arg(errCode).arg(QT_UTF8(PLVGetSdkErrorDescription(errCode)));
        OnShowMsg(msg, MsgAPI);
        return;
    }
    on_requestPushButton_clicked();
    InitPlayer();
    EnableControl(true);
}

void MainWindow::on_freePushButton_clicked()
{
    on_customStopPushButton_clicked();
    FreeDownloaders();
    FreePlayer();
    EnableControl(false);
    PLVReleaseSdkLibrary();
}

void MainWindow::on_detectSoftwareRecordCheckBox_clicked()
{
    bool enable = ui->detectSoftwareRecordCheckBox->isChecked();
    PLVSetDetectSoftwareRecordingHandler(enable,
        [](SOFTWARE_RECORDING_NOTIFY_TYPE type, const char* softwares, void* data) {
            auto obj = static_cast<MainWindow*>(data);
            QString msg = QString("software recording, type:%1, softwares:%2").arg(type).arg(QT_UTF8(softwares));
            QMetaObject::invokeMethod(obj, "OnShowMsg", Q_ARG(const QString&, msg), Q_ARG(int, MsgCallback));
        }, this);
    PLVSetPreventSoftwareRecording((void*)winId(), enable);
    //If the full screen playback window is an independent top-level window, you needs prevent recording too.
    playWnd->SetPreventSoftwareRecording(enable);
}

void MainWindow::on_detectHardwareRecordCheckBox_clicked()
{
    bool enable = ui->detectHardwareRecordCheckBox->isChecked();
    PLVSetDetectHardwareRecordingHandler(enable,
        [](DEVICE_CHANGED_TYPE type, const char* device, void* data) {
            auto obj = static_cast<MainWindow*>(data);
            QString msg = QString("hardware recording, type:%1, device:%2").arg(type).arg(QT_UTF8(device));
            QMetaObject::invokeMethod(obj, "OnShowMsg", Q_ARG(const QString&, msg), Q_ARG(int, MsgCallback));
        }, this);
}

void MainWindow::on_requestPushButton_clicked()
{
    PLVVideoRequestParam param;
    param.type = VIDEO_REQUEST_PAGE;
    param.vids = nullptr;
    param.page = ui->pageSpinBox->value();
    param.pageSize = 20;
    RequestVideoList(&param);
}

void MainWindow::on_searchPushButton_clicked()
{
    QString text = ui->vidsearchLineEdit->text();
    std::string vids = text.trimmed().toStdString();
    PLVVideoRequestParam param;
    param.type = VIDEO_REQUEST_VID;
    param.vids = vids.c_str();
    param.page = 1;
    param.pageSize = 20;
    RequestVideoList(&param);
}

void MainWindow::on_cancelPushButton_clicked()
{
    PLVCancelRequestVideoInfo();
}

void MainWindow::OnVideoInfo(int code, QVariantList videoList, QVariantMap pageInfo)
{
    if (code != E_NO_ERR) {
        QString msg = QString("code:%1,msg:%2").arg(code).arg(QT_UTF8(PLVGetSdkErrorDescription(code)));
        OnShowMsg(msg, MsgAPI);
        return;
    }
    ui->videoTableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    ui->videoTableWidget->setColumnWidth(0, 180);
    ui->videoTableWidget->setColumnWidth(1, 64);
    ui->videoTableWidget->setColumnWidth(2, 320);
    ui->videoTableWidget->setColumnWidth(3, 40);
    ui->videoTableWidget->setColumnWidth(4, 100);
    ui->videoTableWidget->setColumnWidth(5, 80);
    ui->videoTableWidget->setColumnWidth(6, 100);
    ui->videoTableWidget->clearContents();
    ui->videoTableWidget->setRowCount(0);
    for (auto& item : videoList) {
        auto video = item.toMap();
        int row = ui->videoTableWidget->rowCount();
        ui->videoTableWidget->insertRow(row);
        auto item0 = new QTableWidgetItem(video.value("title").toString());
        item0->setData(Qt::UserRole, video);
        auto item1 = new QTableWidgetItem(video.value("encrypt").toBool() ? "encrypt" : "normal");
        auto item2 = new QTableWidgetItem(video.value("vid").toString());
        auto item3 = new QTableWidgetItem(QString::number(video.value("status").toInt()));
        auto item4 = new QTableWidgetItem(GetFileSizeString(video.value("sourceFilesize").toLongLong()));
        auto item5 = new QTableWidgetItem(GetTimeString((int64_t)video.value("duration").toReal()));
        auto item6 = new QTableWidgetItem(GetRateListString(video.value("rates").toList()));
        ui->videoTableWidget->setItem(row, 0, item0);
        ui->videoTableWidget->setItem(row, 1, item1);
        ui->videoTableWidget->setItem(row, 2, item2);
        ui->videoTableWidget->setItem(row, 3, item3);
        ui->videoTableWidget->setItem(row, 4, item4);
        ui->videoTableWidget->setItem(row, 5, item5);
        ui->videoTableWidget->setItem(row, 6, item6);
    }
    ui->pageLabel->setText(QString("%1/%2")
        .arg(pageInfo.value("pageNumber").toInt())
        .arg(pageInfo.value("totalPages").toInt()));
}

void MainWindow::on_playModeComboBox_currentIndexChanged(int index)
{
    ui->tokenCheckBox->setEnabled(index == 0);
    ui->playTokenLineEdit->setEnabled(index == 0 && ui->tokenCheckBox->isChecked());
    ui->tokenRequestPushButton->setEnabled(index == 0 && ui->tokenCheckBox->isChecked());
    ui->videoPathLineEdit->setEnabled(index == 1);
    ui->browsePushButton->setEnabled(index == 1);
}

void MainWindow::on_tokenCheckBox_stateChanged(int)
{
    ui->playTokenLineEdit->setEnabled(ui->tokenCheckBox->isChecked());
    ui->tokenRequestPushButton->setEnabled(ui->tokenCheckBox->isChecked());
}

void MainWindow::on_tokenRequestPushButton_clicked()
{
    QString vid = ui->playVidLineEdit->text();
    if (vid.isEmpty() || gettingToken) {
        return;
    }
    gettingToken = true;
    abortGetToken.exchange(false);
    getTokenThread = std::thread(
        [this, vid]() {
            QString resultStr;
            bool result = GetVideoToken(MakeTokenParams(vid, true), resultStr, &abortGetToken);
            QMetaObject::invokeMethod(this, "OnGetToken", Q_ARG(bool, result), Q_ARG(QString, resultStr));
        });
}

void MainWindow::OnGetToken(bool result, QString resultStr)
{
    gettingToken = false;
    if (getTokenThread.joinable()) {
        getTokenThread.join();
    }
    if (result) {
        ui->playTokenLineEdit->setText(resultStr);
    }
    else {
        ui->playTokenLineEdit->clear();
        OnShowMsg(resultStr, MsgAPI);
    }
}

void MainWindow::on_browsePushButton_clicked()
{
    QString path = QFileDialog::getExistingDirectory(this, QTStr("ChooseVideoPath"), ui->videoPathLineEdit->text());
    if (!path.isEmpty()) {
        ui->videoPathLineEdit->setText(path);
    }
}

void MainWindow::on_playPushButton_clicked()
{
    bool onlinePlay_ = ui->playModeComboBox->currentIndex() == 0;
    QString vid = ui->playVidLineEdit->text();
    bool useToken = ui->tokenCheckBox->isChecked();//only encrypt need!
    QString token = ui->playTokenLineEdit->text();
    bool autoDownRate = ui->autoDownRateCheckBox->isChecked();
    inputRate = ui->playRateComboBox->currentIndex();
    currentRate = inputRate;
    QString videoPath = ui->videoPathLineEdit->text();
    //check
    if (IsPlaying()) {
        QMessageBox::information(this, "Tips", QTStr("PlayPlaying"));
        return;
    }
    if (vid.isEmpty()) {
        QMessageBox::information(this, "Tips", QTStr("PlayVidEmpty"));
        return;
    }
    if (!onlinePlay_ && videoPath.isEmpty()) {
        QMessageBox::information(this, "Tips", QTStr("PlayPathEmpty"));
        return;
    }

    //get token note:only encrypt video need use token!
    if (useToken && token.isEmpty()) {
        on_tokenRequestPushButton_clicked();
        while (gettingToken) {
            QApplication::processEvents();
        }
        token = ui->playTokenLineEdit->text();
        if (token.isEmpty()) { return; }
    }
    //play
    onlinePlay = onlinePlay_;
    playerVid = vid;
    PLVPlayerSetInfo(player, QT_TO_UTF8(vid), QT_TO_UTF8(videoPath), inputRate);
    if (onlinePlay) {
        PLVPlayerPlay(player, useToken ? QT_TO_UTF8(token) : nullptr, 0, autoDownRate, false, false);
    }
    else {
        PLVPlayerPlayLocal(player, 0, autoDownRate);
    }
}

void MainWindow::on_stopPushButton_clicked()
{
    abortGetToken.exchange(true);
    PLVPlayerStop(player);
    while (gettingToken || (mediaState != MEDIA_STATE_NONE && mediaState != MEDIA_STATE_FAIL && mediaState != MEDIA_STATE_END)) {
        QApplication::processEvents();
    }
}

void MainWindow::on_pausePushButton_clicked()
{
    if (IsPlaying()) {
        PLVPlayerPause(player, true);
    }
}

void MainWindow::on_resumePushButton_clicked()
{
    if (IsPlaying()) {
        PLVPlayerPause(player, false);
    }
}

void MainWindow::on_shotScreenPushButton_clicked()
{
    if (IsPlaying()) {
        QString filepath = GetScreenshotPath() + "/" + QDateTime::currentDateTime().toString("yyyy-MM-dd-hh-mm-ss.jpg");
        PLVPlayerScreenshot(player, QT_TO_UTF8(filepath));
        int ret = QMessageBox::question(this, QTStr("PlayScreenshot"), QString("%1,%2?").arg(filepath).arg(QTStr("OpenDir")));
        if (ret == QMessageBox::Yes) {
            QDesktopServices::openUrl(QUrl::fromLocalFile(GetScreenshotPath()));
        }
    }
}

void MainWindow::on_fullScreenPushButton_clicked()
{
    playWnd->SwitchFullScreen();
}

void MainWindow::on_speedComboBox_currentIndexChanged(int index)
{
    if (!player) { return; }
    QString speed = ui->speedComboBox->currentText().replace("x", "");
    PLVPlayerSetSpeed(player, speed.toDouble());
}

void MainWindow::on_muteCheckBox_clicked()
{
    if (!player) { return; }
    PLVPlayerSetMute(player, ui->muteCheckBox->isChecked());
}

void MainWindow::on_volumeHorizontalSlider_valueChanged(int value)
{
    if (!player) { return; }
    PLVPlayerSetVolume(player, value);
    ui->volumeLabel->setText(QString::number(value));
}

void MainWindow::on_hardwareDecodeCheckBox_clicked()
{
    PLVSetSdkHwdecEnable(ui->hardwareDecodeCheckBox->isChecked());
}

void MainWindow::on_keepLastFrameCheckBox_clicked()
{
    PLVSetSdkKeepLastFrame(ui->keepLastFrameCheckBox->isChecked());
}

void MainWindow::on_videoOutputComboBox_currentIndexChanged(int)
{
#ifdef _WIN32
    int vo = ui->videoOutputComboBox->currentIndex();
    PLVSetSdkVideoOutputDevice(VIDEO_OUTPUT_DEVICE(vo), nullptr);
#endif
}

void MainWindow::on_logoEnableCheckBox_clicked()
{
    if (!player) { return; }
    bool logoEnable = ui->logoEnableCheckBox->isChecked();
    std::string logoText = ui->logoTextLineEdit->text().toStdString();
    int logoTextSize = ui->logoTextSizeLineEdit->text().toInt();
    std::string logoTextColor = ui->logoTextColorLineEdit->text().toStdString();
    int logoBorderSize = ui->logoBorderSizeLineEdit->text().toInt();
    std::string logoBorderColor = ui->logoBorderColorLineEdit->text().toStdString();
    int logoHPos = ui->logoHPosComboBox->currentIndex() - 1;
    int logoVPos = ui->logoVPosComboBox->currentIndex() - 1;
    logoEnable = logoEnable && !logoText.empty();
    PLVLogoTextInfo info;
    info.text = logoText.c_str();
    info.textSize = logoTextSize;
    info.textColor = logoTextColor.c_str();
    info.borderSize = logoBorderSize;
    info.borderColor = logoBorderColor.c_str();
    info.alignX = logoHPos;
    info.alignY = logoVPos;
    PLVPlayerSetLogoText(player, logoEnable, &info);
}

void MainWindow::on_osdEnableCheckBox_clicked()
{
    if (!player) { return; }
    bool osdEnable = ui->osdEnableCheckBox->isChecked();
    std::string osdText = ui->osdTextLineEdit->text().toStdString();
    int osdTextSize = ui->osdTextSizeLineEdit->text().toInt();
    std::string osdTextColor = ui->osdTextColorLineEdit->text().toStdString();
    int osdBorderSize = ui->osdBorderSizeLineEdit->text().toInt();
    std::string osdBorderColor = ui->osdBorderColorLineEdit->text().toStdString();
    int osdEffect = ui->osdEffectComboBox->currentIndex();
    int osdDuration = ui->osdDurationSpinBox->value();
    int osdSpace = ui->osdSpaceSpinBox->value();
    int osdFade = ui->osdFadeSpinBox->value();
    osdEnable = osdEnable && !osdText.empty();
    PLVOsdConfigInfo info;
    info.text = osdText.c_str();
    info.textSize = osdTextSize;
    info.textColor = osdTextColor.c_str();
    info.borderSize = osdBorderSize;
    info.borderColor = osdBorderColor.c_str();
    info.animationEffect = OSD_DISPLAY_TYPE(osdEffect);
    info.displayDuration = osdDuration;
    info.displayInterval = osdSpace;
    info.fadeDuration = osdFade;
    PLVPlayerSetOSDConfig(player, osdEnable, &info);
}

void MainWindow::on_playRateComboBox_activated(int index)
{
    int rate = (VIDEO_RATE_TYPE)index;
    if (IsPlaying() && inputRate != rate) {
        on_stopPushButton_clicked();
        on_playPushButton_clicked();
    }
}

void MainWindow::OnMediaState(QString vid, int state)
{
    mediaState = state;
    ui->playStatusLabel->setText(GetPlayStateString(currentRate, mediaState));
    OnShowMsg(QString("vid:%1,state:%2").arg(vid).arg(GetStateName(state)), MsgCallback);
}

void MainWindow::OnMediaProperty(QString vid, int property, int format, QString value)
{
    switch (property) {
    case MEDIA_PROPERTY_DURATION: duration = value; break;
    case MEDIA_PROPERTY_HWDEC: hwdec = value; break;
    case MEDIA_PROPERTY_VIDEO_CODEC: vCodec = value; break;
    case MEDIA_PROPERTY_VIDEO_BITRATE: vBitrate = value; break;
    case MEDIA_PROPERTY_VIDEO_FPS: fps = value; break;
    case MEDIA_PROPERTY_VIDEO_WIDTH: width = value; break;
    case MEDIA_PROPERTY_VIDEO_HEIGHT: height = value; break;
    case MEDIA_PROPERTY_AUDIO_CODEC: aCodec = value; break;
    case MEDIA_PROPERTY_AUDIO_BITRATE: aBitrate = value; break;
    case MEDIA_PROPERTY_CACHE_SPEED: cacheSpeed = value; break;
    default: break;
    }
    QString playInfo = QString("Video:%1%2,%3x%4@%5-%6kbps,Audio:%7-%8kbps,Cache:%9kbps")
        .arg(vCodec.left(vCodec.indexOf(" (")))
        .arg(hwdec != "" && hwdec != "no" ? QString("-%1").arg(hwdec) : "")
        .arg(width).arg(height).arg(QString::number(fps.toFloat(), 'f', 2)).arg(vBitrate.toLongLong() / 1024)
        .arg(aCodec.left(aCodec.indexOf(" (")))
        .arg(aBitrate.toLongLong() / 1024)
        .arg(cacheSpeed.toLongLong() / 1024);
    ui->playInfoLabel->setText(playInfo);
    //play slider
    if (property == MEDIA_PROPERTY_DURATION) {
        ui->progressWidget->SetMaxValue(duration.toLongLong());
    }
    else if (property == MEDIA_PROPERTY_CACHE_TIME) {
        ui->progressWidget->SetCacheValue(value.toLongLong());
    }
}

void MainWindow::OnMediaRateChanged(QString vid, int inputRate_, int realRate)
{
    inputRate = inputRate_;
    currentRate = realRate;
    ui->playStatusLabel->setText(GetPlayStateString(currentRate, mediaState));
    OnShowMsg(QString("vid:%1,input rate:%2, real rate:%3")
        .arg(vid).arg(GetRateName(inputRate)).arg(GetRateName(currentRate)), MsgCallback);
}

void MainWindow::OnMediaProcess(QString vid, int ms)
{
    position = QString::number(ms);
    ui->progressWidget->SetCurrentValue(ms);
    QString playTime = QString("%1/%2")
        .arg(GetTimeString(position.toLongLong()/1000)).arg(GetTimeString(duration.toLongLong()/1000));
    ui->playTimeLabel->setText(playTime);
}

void MainWindow::OnSeek(qint64 ms)
{
    if (IsPlaying()) {
        PLVPlayerSetSeek(player, ms);
    }
}

void MainWindow::on_shrinkOrExpandPushButton_clicked()
{
    bool shrink = ui->shrinkOrExpandPushButton->property("status").toBool();
    ui->shrinkOrExpandPushButton->setProperty("status", !shrink);
    ui->shrinkOrExpandPushButton->setText(shrink ? QTStr("Shrink") : QTStr("Expand"));
    ui->playControlGroupBox->setVisible(shrink);
    ui->logoGroupBox->setVisible(shrink);
    ui->osdGroupBox->setVisible(shrink);
}

void MainWindow::on_localVideoBrowserPushButton_clicked()
{
    QString path = QFileDialog::getExistingDirectory(this, QTStr("ChooseVideoPath"), ui->localVideoPathLineEdit->text());
    if (!path.isEmpty()) {
        ui->localVideoPathLineEdit->setText(path);
    }
}

void MainWindow::OnAddDownloader(QVariantMap video, QVariantMap rateInfo)
{
    //check path
    QString path = ui->localVideoPathLineEdit->text();
    QDir dir(path);
    if (path.isEmpty() || (!dir.exists() && !dir.mkdir("."))) {
        QMessageBox::information(this, "Tips", QTStr("VideoSavePathError"));
        return;
    }
    //check has download!
    ui->tabWidget->setCurrentIndex(TabVideoDownload);
    QString vid = video.value("vid").toString();
    if (rateInfo.value("rate").toInt() == VIDEO_RATE_SOURCE) {
        rateInfo["rate"] = VIDEO_RATE_LD;
    }
    int rate = rateInfo.value("rate").toInt();
    int row = ui->downloadVideoTableWidget->rowCount();
    for (int i = 0; i < row; i++) {
        auto detailsWidget = qobject_cast<SimpleDownloadDetails*>(ui->downloadVideoTableWidget->cellWidget(i, 0));
        QString vid_ = detailsWidget->property("video").toMap().value("vid").toString();
        int rate_ = detailsWidget->property("rate").toMap().value("rate").toInt();
        if (vid == vid_ && rate == rate_) {
            QMessageBox::information(this, "Tips", QTStr("DownloadAlreadyIn"));
            return;
        }
    }
    //token
    bool encrypt = video.value("encrypt").toBool();
    QString token;
    std::atomic_bool abort{ false };
    if (encrypt && !GetVideoToken(MakeTokenParams(vid, false), token, &abort)) {
        OnShowMsg(token, MsgAPI);
        return;
    }
    //downloader
    auto downloader = PLVDownloadCreate();
    PLVDownloadSetErrorHandler(downloader,
        [](const char* vid, int code, void* data) {
            QString msg = QString("download error! ptr:%1, vid:%2, code:%3")
                .arg((int64_t)data).arg(QT_UTF8(vid)).arg(code);
            QMetaObject::invokeMethod(MainWindow::main, "OnShowMsg", Q_ARG(const QString&, msg), Q_ARG(int, MsgCallback));
        }, downloader);
    PLVDownloadSetProgressHandler(downloader,
        [](const char* vid, long long receivedBytes, long long totalBytes, void* data) {
            QMetaObject::invokeMethod(MainWindow::main, "OnDownloaderProgress",
                Q_ARG(qint64, (qint64)data),
                Q_ARG(QString, QT_UTF8(vid)),
                Q_ARG(qint64, receivedBytes),
                Q_ARG(qint64, totalBytes));
        }, downloader);
    PLVDownloadSetResultHandler(downloader,
        [](const char* vid, int rate, int code, void* data) {
            QMetaObject::invokeMethod(MainWindow::main, "OnDownloaderComplete",
                Q_ARG(qint64, (qint64)data),
                Q_ARG(QString, QT_UTF8(vid)),
                Q_ARG(int, rate),
                Q_ARG(int, code));
        }, downloader);
    PLVDownloadSetInfo(downloader, QT_TO_UTF8(vid), QT_TO_UTF8(path), rate);
    PLVDownloadStart(downloader, QT_TO_UTF8(token), false);
    //add item
    ui->downloadVideoTableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    ui->downloadVideoTableWidget->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    ui->downloadVideoTableWidget->verticalHeader()->setDefaultSectionSize(60);
    ui->downloadVideoTableWidget->setColumnWidth(0, 310);
    ui->downloadVideoTableWidget->setColumnWidth(1, 300);
    ui->downloadVideoTableWidget->setColumnWidth(2, 200);
    row = ui->downloadVideoTableWidget->rowCount();
    ui->downloadVideoTableWidget->insertRow(row);
    auto detailWidget = new SimpleDownloadDetails(video, rateInfo);
    detailWidget->setProperty("videoPath", path);
    detailWidget->setProperty("video", video);
    detailWidget->setProperty("rate", rateInfo);
    detailWidget->setProperty("downloader", (int64_t)downloader);
    auto processWidget = new SimpleDownloadProcess();
    processWidget->setProperty("downloader", (int64_t)downloader);
    auto operatorWidget = new SimpleDownloadOperator();
    operatorWidget->setProperty("downloader", (int64_t)downloader);
    connect(operatorWidget, &SimpleDownloadOperator::start, this,
        [this]() {
            auto downloader = sender()->property("downloader").toLongLong();
            //token
            int index = FindDownloader(downloader);
            auto detailsWidget = qobject_cast<SimpleDownloadDetails*>(ui->downloadVideoTableWidget->cellWidget(index, 0));
            QString vid = detailsWidget->property("video").toMap().value("vid").toString();
            bool encrypt = detailsWidget->property("video").toMap().value("encrypt").toBool();
            QString token;
            std::atomic_bool abort{ false };
            if (encrypt && !GetVideoToken(MakeTokenParams(vid, false), token, &abort)) {
                OnShowMsg(token, MsgAPI);
                return;
            }
            PLVDownloadStart((PLVDownloadPtr)downloader, QT_TO_UTF8(token), false);
        });
    connect(operatorWidget, &SimpleDownloadOperator::stop, this,
        [this]() {
            auto downloader = (PLVDownloadPtr)(sender()->property("downloader").toLongLong());
            PLVDownloadStop(downloader);
        });
    connect(operatorWidget, &SimpleDownloadOperator::pause, this,
        [this]() {
            auto downloader = (PLVDownloadPtr)(sender()->property("downloader").toLongLong());
            PLVDownloadPause(downloader, true);
        });
    connect(operatorWidget, &SimpleDownloadOperator::resume, this,
        [this]() {
            auto downloader = (PLVDownloadPtr)(sender()->property("downloader").toLongLong());
            PLVDownloadPause(downloader, false);
        });
    connect(operatorWidget, &SimpleDownloadOperator::openDir, this,
        [this]() {
            auto downloader = (sender()->property("downloader").toLongLong());
            int index = FindDownloader(downloader);
            auto detailsWidget = qobject_cast<SimpleDownloadDetails*>(ui->downloadVideoTableWidget->cellWidget(index, 0));
            QString vid = detailsWidget->property("video").toMap().value("vid").toString();
            QString filepah = ui->localVideoPathLineEdit->text() + "/" + vid;
            QDesktopServices::openUrl(QUrl::fromLocalFile(filepah));
        });
    connect(operatorWidget, &SimpleDownloadOperator::play, this,
        [this]() {
            auto downloader = (sender()->property("downloader").toLongLong());
            int index = FindDownloader(downloader);
            auto detailsWidget = qobject_cast<SimpleDownloadDetails*>(ui->downloadVideoTableWidget->cellWidget(index, 0));
            QString videoPath = detailsWidget->property("videoPath").toString();
            QString vid = detailsWidget->property("video").toMap().value("vid").toString();
            int rate = detailsWidget->property("rate").toMap().value("rate").toInt();
            PlayOffline(videoPath, vid, rate);
        });
    connect(operatorWidget, &SimpleDownloadOperator::deleted, this,
        [this]() {
            auto downloader = (PLVDownloadPtr)(sender()->property("downloader").toLongLong());
            int index = FindDownloader((int64_t)downloader);
            auto detailsWidget = qobject_cast<SimpleDownloadDetails*>(ui->downloadVideoTableWidget->cellWidget(index, 0));
            QString videoPath = detailsWidget->property("videoPath").toString();
            QString vid = detailsWidget->property("video").toMap().value("vid").toString();
            if (IsPlaying() && !onlinePlay && vid == playerVid) {
                QMessageBox::information(this, "Tips", QTStr("DeletePlayingTips"));
                return;
            }
            PLVDownloadDelete(downloader);
            PLVDownloadDestroy(downloader);
            ui->downloadVideoTableWidget->removeRow(index);
        });
    ui->downloadVideoTableWidget->setCellWidget(row, 0, detailWidget);
    ui->downloadVideoTableWidget->setCellWidget(row, 1, processWidget);
    ui->downloadVideoTableWidget->setCellWidget(row, 2, operatorWidget);
}

void MainWindow::OnDownloaderProgress(qint64 downloader, QString vid, qint64 current, qint64 total)
{
    int row = ui->downloadVideoTableWidget->rowCount();
    for (int i = 0; i < row; i++) {
        auto processWidget = qobject_cast<SimpleDownloadProcess*>(ui->downloadVideoTableWidget->cellWidget(i, 1));
        if (downloader == processWidget->property("downloader").toLongLong()) {
            processWidget->SetMaxValue(total);
            processWidget->SetCurrentValue(current);
            break;
        }
    }
}

void MainWindow::OnDownloaderComplete(qint64 downloader, QString vid, int rate, int code)
{
    QString msg = QString("download complete! ptr:%1, vid:%2, rate:%3, code:%4")
        .arg(downloader).arg(vid).arg(rate).arg(code);
    OnShowMsg(msg, MsgCallback);
    ScanFiles();
}

void MainWindow::on_localVideoPathLineEdit_textChanged(const QString& text)
{
    if (QDir(text).exists()) {
        ScanFiles();
    }
    else {
        ui->localVideoTableWidget->clearContents();
        ui->localVideoTableWidget->setRowCount(0);
    }
}

void MainWindow::on_customPlayPushButton_clicked()
{
    QString vid = ui->customPlayVidLineEdit->text();
    bool useToken = ui->customTokenCheckBox->isChecked();
    int rate = ui->customPlayRateComboBox->currentIndex();
    //check
    if (customPlayer) {
        QMessageBox::information(this, "Tips", QTStr("PlayPlaying"));
        return;
    }
    if (vid.isEmpty()) {
        QMessageBox::information(this, "Tips", QTStr("PlayVidEmpty"));
        return;
    }
    //token
    QString token;
    std::atomic_bool abort{ false };
    if (useToken && !GetVideoToken(MakeTokenParams(vid, true), token, &abort)) {
        OnShowMsg(token, MsgAPI);
        return;
    }
    //play
    memset(&renderFrame, 0, sizeof(renderFrame));
    renderFrame.data[0] = new uint8_t[GetRGBXSize(MAX_RENDER_WIDTH, MAX_RENDER_HEIGHT)];
    customPlayer = PLVPlayerCreate(nullptr);
    PLVPlayerSetVideoFrameHandler(customPlayer,
        [](const char* vid, PLVVideoFrame* frame, void* data) {
            auto obj = static_cast<MainWindow*>(data);
            if (!obj->renderBufferFill.load()) {
                bool isVertical = frame->width <= frame->height;
                uint32_t maxRenderWidth = !isVertical ? MAX_RENDER_WIDTH : MAX_RENDER_HEIGHT;
                uint32_t maxRenderHeight = !isVertical ? MAX_RENDER_HEIGHT : MAX_RENDER_WIDTH;
                uint32_t minRenderWidth = !isVertical ? MIN_RENDER_WIDTH : MIN_RENDER_HEIGHT;
                uint32_t minRenderHeight = !isVertical ? MIN_RENDER_HEIGHT : MIN_RENDER_WIDTH;
                //get display size here!
                uint64_t size = obj->displaySize.load();
                uint32_t displayWidth = (size >> 32) & 0xFFFFFFFF;
                uint32_t displayHeight = size & 0xFFFFFFFF;
                //calc render size! don't exceed video size!
                uint32_t renderWidth = 0, renderHeight = 0;
                renderWidth = std::clamp(displayWidth, minRenderWidth, maxRenderWidth);
                renderHeight = std::clamp(displayHeight, minRenderHeight, maxRenderHeight);
                renderWidth = std::min(renderWidth, frame->width);
                renderHeight = std::min(renderHeight, frame->height);
                //keep radio
                float videoRadio = (float)frame->width / (float)frame->height;
                float displayRadio = (float)renderWidth / (float)renderHeight;
                if (videoRadio > displayRadio) {
                    ALIGN_SIZE(renderWidth, 16);
                    renderWidth = std::min(renderWidth, frame->width);
                    renderHeight = renderWidth * frame->height / frame->width;
                    ALIGN_SIZE(renderHeight, 2);
                    renderHeight = std::min(renderHeight, frame->height);
                }
                else {
                    ALIGN_SIZE(renderHeight, 2);
                    renderHeight = std::min(renderHeight, frame->height);
                    renderWidth = renderHeight * frame->width / frame->height;
                    ALIGN_SIZE(renderWidth, 16);
                    renderWidth = std::min(renderWidth, frame->width);
                }
                //set frame!
                frame->width = renderWidth;
                frame->height = renderHeight;
                FillRGBXPlanes(frame, obj->renderFrame.data[0]);
                return true;
            }
            else {
                //notify paint
                QMetaObject::invokeMethod(obj, "OnCustomRenderUpdate");
                return false;
            }
        },
        [](const char* vid, PLVVideoFrame* frame, void* data) {
            auto obj = static_cast<MainWindow*>(data);
            //fill frame
            obj->renderFrame.width = frame->width;
            obj->renderFrame.height = frame->height;
            obj->renderFrame.format = frame->format;
            obj->renderFrame.timestamp = frame->timestamp;
            FillRGBXPlanes(&obj->renderFrame, obj->renderFrame.data[0]);
            obj->renderBufferFill.exchange(true);
            //notify paint
            QMetaObject::invokeMethod(obj, "OnCustomRenderUpdate");
            return true;
        }, this);
    PLVPlayerSetInfo(customPlayer, QT_TO_UTF8(vid), nullptr, rate);
    PLVPlayerPlay(customPlayer, useToken ? QT_TO_UTF8(token) : nullptr, 0, true, false, false);
}

void MainWindow::on_customStopPushButton_clicked()
{
    renderBufferFill.exchange(false);
    if (customPlayer) {
        PLVPlayerStop(customPlayer);
        PLVPlayerDestroy(customPlayer);
        customPlayer = nullptr;
        delete[] renderFrame.data[0];
        memset(&renderFrame, 0, sizeof(renderFrame));
    }
}

void MainWindow::OnCustomRenderUpdate()
{
    if (renderBufferFill.load()) {
        ui->playOpenGLWidget->updateImage(QImage(renderFrame.data[0], renderFrame.width, renderFrame.height, renderFrame.linesize[0], QImage::Format_RGBX8888));
        renderBufferFill.exchange(false);
    }
}

void MainWindow::on_customSpeedComboBox_currentIndexChanged()
{
    if (!customPlayer) { return; }
    QString speed = ui->customSpeedComboBox->currentText().replace("x", "");
    PLVPlayerSetSpeed(customPlayer, speed.toDouble());
}

void MainWindow::on_customPlayRateComboBox_currentIndexChanged()
{
    if (!customPlayer) { return; }
    on_customStopPushButton_clicked();
    on_customPlayPushButton_clicked();
}

bool MainWindow::eventFilter(QObject* obj, QEvent* event)
{
    if (obj == ui->videoTableWidget && event->type() == QEvent::ContextMenu) {
        auto item = ui->videoTableWidget->currentItem();
        if (ui->videoTableWidget->isEnabled() && item) {
            auto video = ui->videoTableWidget->item(item->row(), 0)->data(Qt::UserRole).toMap();
            auto contextMenuEvt = static_cast<QContextMenuEvent*>(event);
            QMenu menu(this);
            auto playAction = menu.addAction(QTStr("Play"));
            connect(playAction, &QAction::triggered, this,
                [this, video]() {
                    PlayOnline(video.value("vid").toString(), video.value("encrypt").toBool(), 0);
                });
            auto playWithRateMenu = menu.addMenu(QTStr("PlayWithRate"));
            for (auto& item : video.value("rates").toList()) {
                int rate = item.toMap().value("rate").toInt();
                auto rateAction = playWithRateMenu->addAction(GetRateName(rate));
                connect(rateAction, &QAction::triggered, this,
                    [this, video, rate]() {
                        PlayOnline(video.value("vid").toString(), video.value("encrypt").toBool(), rate);
                    });
            }
#ifdef _WIN32
            auto playCustomAction = menu.addAction(QTStr("CustomRenderPlay"));
            connect(playCustomAction, &QAction::triggered, this,
                [this, video]() {
                    QString vid = video.value("vid").toString();
                    bool encrypt = video.value("encrypt").toBool();
                    ui->customPlayVidLineEdit->setText(vid);
                    ui->customTokenCheckBox->setChecked(encrypt);
                    ui->customPlayRateComboBox->setCurrentIndex(0);
                    ui->tabWidget->setCurrentIndex(TabVideoCustomRender);
                    on_customPlayPushButton_clicked();
                });
#endif
            auto downloadRatesMenu = menu.addMenu(QTStr("Download"));
            for (auto& item : video.value("rates").toList()) {
                int rate = item.toMap().value("rate").toInt();
                auto rateAction = downloadRatesMenu->addAction(GetRateName(rate));
                QVariantMap downVideo = video;
                QVariantMap rateInfo = item.toMap();
                connect(rateAction, &QAction::triggered, this,
                    [this, downVideo, rateInfo]() {
                        OnAddDownloader(downVideo, rateInfo);
                    });
            }
            menu.exec(contextMenuEvt->globalPos());
        }
    }
    return QMainWindow::eventFilter(obj, event);
}

void MainWindow::resizeEvent(QResizeEvent* event)
{
    uint64_t size = static_cast<uint64_t>(ui->playOpenGLWidget->width()) << 32;
    size |= ui->playOpenGLWidget->height();
    displaySize.exchange(size);
    QMainWindow::resizeEvent(event);
}

void MainWindow::closeEvent(QCloseEvent* e)
{
    abortGetToken.exchange(true);
    QMainWindow::closeEvent(e);
}

void MainWindow::EnableControl(bool initialized)
{
    ui->logLevelComboBox->setEnabled(!initialized);
    ui->logPathLineEdit->setEnabled(!initialized);
    ui->logCallbackCheckBox->setEnabled(!initialized);
    ui->retryAttemptsLineEdit->setEnabled(!initialized);
    ui->retryMinSpaceLineEdit->setEnabled(!initialized);
    ui->retryMaxSpaceLineEdit->setEnabled(!initialized);
    ui->playSeedLineEdit->setEnabled(!initialized);
    ui->localRememberFrameCheckBox->setEnabled(!initialized);
    ui->viewerIdLineEdit->setEnabled(!initialized);
    ui->viewerNameLineEdit->setEnabled(!initialized);
    ui->userIdLineEdit->setEnabled(!initialized && !ui->subAccountCheckBox->isChecked());
    ui->secretKeyLineEdit->setEnabled(!initialized && !ui->subAccountCheckBox->isChecked());
    ui->appIdLineEdit->setEnabled(!initialized && ui->subAccountCheckBox->isChecked());
    ui->appSecretLineEdit->setEnabled(!initialized && ui->subAccountCheckBox->isChecked());
    ui->subAccountCheckBox->setEnabled(!initialized);
    ui->initPushButton->setEnabled(!initialized);
    ui->freePushButton->setEnabled(initialized);
    ui->detectSoftwareRecordCheckBox->setEnabled(initialized);
    ui->detectHardwareRecordCheckBox->setEnabled(initialized);
    ui->videoTableWidget->setEnabled(initialized);
    !initialized ? ui->tabWidget->setCurrentIndex(TabVideoList) : void(0);
    ui->tabWidget->setTabEnabled(TabVideoPlay, initialized);
    ui->tabWidget->setTabEnabled(TabVideoDownload, initialized);
#ifdef _WIN32
    ui->tabWidget->setTabEnabled(TabVideoCustomRender, initialized);
#else
    ui->tabWidget->setTabEnabled(TabVideoCustomRender, false);
#endif
}

void MainWindow::LoadConfig()
{
    QSettings settings(GetConfigPath(), QSettings::IniFormat);
    settings.setIniCodec("utf-8");
    //global config
    int logLevel = settings.value("logLevel", (int)LOG_FILTER_INFO).toInt();
    ui->logLevelComboBox->setCurrentIndex(logLevel);
    ui->logPathLineEdit->setText(GetLogPath());
    bool logCallback = settings.value("logCallback", true).toBool();
    ui->logCallbackCheckBox->setChecked(logCallback);
    int retryAttempts = settings.value("retryAttempts", -1).toInt();
    int retryMinSpace = settings.value("retryMinSpace", 500).toInt();
    int retryMaxSpace = settings.value("retryMaxSpace", 25000).toInt();
    ui->retryAttemptsLineEdit->setText(QString::number(retryAttempts));
    ui->retryMinSpaceLineEdit->setText(QString::number(retryMinSpace));
    ui->retryMaxSpaceLineEdit->setText(QString::number(retryMaxSpace));
    QString playSeed = Decrypt(settings.value("playSeed").toString());
    ui->playSeedLineEdit->setText(playSeed);
    bool localRememberFrame = settings.value("localRememberFrame", true).toBool();
    ui->localRememberFrameCheckBox->setChecked(localRememberFrame);
    QString viewerId = settings.value("viewerId", "polyv-user-10").toString();
    QString viewerName = settings.value("viewerName", "polyv-user-10-name").toString();
    ui->viewerIdLineEdit->setText(viewerId);
    ui->viewerNameLineEdit->setText(viewerName);
    QString userId = Decrypt(settings.value("userId").toString());
    QString secretKey = Decrypt(settings.value("secretKey").toString());
    QString appId = Decrypt(settings.value("appId").toString());
    QString appSecret = Decrypt(settings.value("appSecret").toString());
    bool subAccount = settings.value("subAccount", false).toBool();
    ui->userIdLineEdit->setText(userId);
    ui->secretKeyLineEdit->setText(secretKey);
    ui->appIdLineEdit->setText(appId);
    ui->appSecretLineEdit->setText(appSecret);
    ui->subAccountCheckBox->setChecked(subAccount);
    //play config
    bool autoDownRate = settings.value("autoDownRate", true).toBool();
    ui->autoDownRateCheckBox->setChecked(autoDownRate);
    QString videoPath = settings.value("videoPath", GetVideoPath()).toString();
    ui->videoPathLineEdit->setText(videoPath);
    bool hardwareDecode = settings.value("hardwareDecode", false).toBool();
    ui->hardwareDecodeCheckBox->setChecked(hardwareDecode);
    bool keepLastFrame = settings.value("keepLastFrame", true).toBool();
    ui->keepLastFrameCheckBox->setChecked(keepLastFrame);
    int videoOutput = settings.value("videoOutput", 0).toInt();
    ui->videoOutputComboBox->setCurrentIndex(videoOutput);
    //logo
    bool logoEnable = settings.value("logoEnable", false).toBool();
    ui->logoEnableCheckBox->setChecked(logoEnable);
    QString logoText = settings.value("logoText").toString();
    ui->logoTextLineEdit->setText(logoText);
    int logoTextSize = settings.value("logoTextSize", 55).toInt();
    ui->logoTextSizeLineEdit->setText(QString::number(logoTextSize));
    QString logoTextColor = settings.value("logoTextColor", "#FF000000").toString();
    ui->logoTextColorLineEdit->setText(logoTextColor);
    int logoBorderSize = settings.value("logoBorderSize", 1).toInt();
    ui->logoBorderSizeLineEdit->setText(QString::number(logoBorderSize));
    QString logoBorderColor = settings.value("logoBorderColor", "#FFFFFFFF").toString();
    ui->logoBorderColorLineEdit->setText(logoBorderColor);
    int logoHPos = settings.value("logoHPos", 2).toInt();
    ui->logoHPosComboBox->setCurrentIndex(logoHPos);
    int logoVPos = settings.value("logoVPos", 0).toInt();
    ui->logoVPosComboBox->setCurrentIndex(logoVPos);
    //osd
    bool osdEnable = settings.value("osdEnable", false).toBool();
    ui->osdEnableCheckBox->setChecked(osdEnable);
    QString osdText = settings.value("osdText").toString();
    ui->osdTextLineEdit->setText(osdText);
    int osdTextSize = settings.value("osdTextSize", 55).toInt();
    ui->osdTextSizeLineEdit->setText(QString::number(osdTextSize));
    QString osdTextColor = settings.value("osdTextColor", "#FF000000").toString();
    ui->osdTextColorLineEdit->setText(osdTextColor);
    int osdBorderSize = settings.value("osdBorderSize", 0).toInt();
    ui->osdBorderSizeLineEdit->setText(QString::number(osdBorderSize));
    QString osdBorderColor = settings.value("osdBorderColor", "#FFFFFFFF").toString();
    ui->osdBorderColorLineEdit->setText(osdBorderColor);
    int osdEffect = settings.value("osdEffect", 0).toInt();
    ui->osdEffectComboBox->setCurrentIndex(osdEffect);
    int osdDuration = settings.value("osdDuration", 5).toInt();
    ui->osdDurationSpinBox->setValue(osdDuration);
    int osdSpace = settings.value("osdSpace", 1).toInt();
    ui->osdSpaceSpinBox->setValue(osdSpace);
    int osdFade = settings.value("osdFade", 3).toInt();
    ui->osdFadeSpinBox->setValue(osdFade);
    //download
    QString localVideoPath = settings.value("localVideoPath", GetVideoPath()).toString();
    ui->localVideoPathLineEdit->setText(localVideoPath);
}

void MainWindow::SaveConfig()
{
    QSettings settings(GetConfigPath(), QSettings::IniFormat);
    settings.setIniCodec("utf-8");
    //global
    int logLevel = ui->logLevelComboBox->currentIndex();
    settings.setValue("logLevel", logLevel);
    bool logCallback = ui->logCallbackCheckBox->isChecked();
    settings.setValue("logCallback", logCallback);
    int retryAttempts = ui->retryAttemptsLineEdit->text().toInt();
    int retryMinSpace = ui->retryMinSpaceLineEdit->text().toInt();
    int retryMaxSpace = ui->retryMaxSpaceLineEdit->text().toInt();
    settings.setValue("retryAttempts", retryAttempts);
    settings.setValue("retryMinSpace", retryMinSpace);
    settings.setValue("retryMaxSpace", retryMaxSpace);
    QString playSeed = ui->playSeedLineEdit->text();
    settings.setValue("playSeed", Encrypt(playSeed));
    bool localRememberFrame = ui->localRememberFrameCheckBox->isChecked();
    settings.setValue("localRememberFrame", localRememberFrame);
    QString viewerId = ui->viewerIdLineEdit->text();
    QString viewerName = ui->viewerNameLineEdit->text();
    settings.setValue("viewerId", viewerId);
    settings.setValue("viewerName", viewerName);
    QString userId = ui->userIdLineEdit->text();
    QString secretKey = ui->secretKeyLineEdit->text();
    QString appId = ui->appIdLineEdit->text();
    QString appSecret = ui->appSecretLineEdit->text();
    bool subAccount = ui->subAccountCheckBox->isChecked();
    settings.setValue("userId", Encrypt(userId));
    settings.setValue("secretKey", Encrypt(secretKey));
    settings.setValue("appId", Encrypt(appId));
    settings.setValue("appSecret", Encrypt(appSecret));
    settings.setValue("subAccount", subAccount);
    //play
    bool autoDownRate = ui->autoDownRateCheckBox->isChecked();
    settings.setValue("autoDownRate", autoDownRate);
    QString videoPath = ui->videoPathLineEdit->text();
    settings.setValue("videoPath", videoPath);
    bool hardwareDecode = ui->hardwareDecodeCheckBox->isChecked();
    settings.setValue("hardwareDecode", hardwareDecode);
    bool keepLastFrame = ui->keepLastFrameCheckBox->isChecked();
    settings.setValue("keepLastFrame", keepLastFrame);
    int videoOutput = ui->videoOutputComboBox->currentIndex();
    settings.setValue("videoOutput", videoOutput);
    //logo
    bool logoEnable = ui->logoEnableCheckBox->isChecked();
    settings.setValue("logoEnable", logoEnable);
    QString logoText = ui->logoTextLineEdit->text();
    settings.setValue("logoText", logoText);
    int logoTextSize = ui->logoTextSizeLineEdit->text().toInt();
    settings.setValue("logoTextSize", logoTextSize);
    QString logoTextColor = ui->logoTextColorLineEdit->text();
    settings.setValue("logoTextColor", logoTextColor);
    int logoBorderSize = ui->logoBorderSizeLineEdit->text().toInt();
    settings.setValue("logoBorderSize", logoBorderSize);
    QString logoBorderColor = ui->logoBorderColorLineEdit->text();
    settings.setValue("logoBorderColor", logoBorderColor);
    int logoHPos = ui->logoHPosComboBox->currentIndex();
    settings.setValue("logoHPos", logoHPos);
    int logoVPos = ui->logoVPosComboBox->currentIndex();
    settings.setValue("logoVPos", logoVPos);
    //osd
    bool osdEnable = ui->osdEnableCheckBox->isChecked();
    settings.setValue("osdEnable", osdEnable);
    QString osdText = ui->osdTextLineEdit->text();
    settings.setValue("osdText", osdText);
    int osdTextSize = ui->osdTextSizeLineEdit->text().toInt();
    settings.setValue("osdTextSize", osdTextSize);
    QString osdTextColor = ui->osdTextColorLineEdit->text();
    settings.setValue("osdTextColor", osdTextColor);
    int osdBorderSize = ui->osdBorderSizeLineEdit->text().toInt();
    settings.setValue("osdBorderSize", osdBorderSize);
    QString osdBorderColor = ui->osdBorderColorLineEdit->text();
    settings.setValue("osdBorderColor", osdBorderColor);
    int osdEffect = ui->osdEffectComboBox->currentIndex();
    settings.setValue("osdEffect", osdEffect);
    int osdDuration = ui->osdDurationSpinBox->value();
    settings.setValue("osdDuration", osdDuration);
    int osdSpace = ui->osdSpaceSpinBox->value();
    settings.setValue("osdSpace", osdSpace);
    int osdFade = ui->osdFadeSpinBox->value();
    settings.setValue("osdFade", osdFade);
    //download
    QString localVideoPath = ui->localVideoPathLineEdit->text();
    settings.setValue("localVideoPath", localVideoPath);
    settings.sync();
}

void MainWindow::RequestVideoList(PLVVideoRequestParam* param)
{
    PLVRequestVideoInfo(true, param,
        [](int code, const PLVVideoRequestInfo infos[], int infosNum, const PLVVideoRequestPageInfo* pageInfo, void* data) {
            auto main = static_cast<MainWindow*>(data);
            QMetaObject::invokeMethod(main, "OnVideoInfo",
                Q_ARG(int, code),
                Q_ARG(QVariantList, code == E_NO_ERR ? MakeVideoInfoList(infos, infosNum) : QVariantList()),
                Q_ARG(QVariantMap, code == E_NO_ERR ? MakeVideoPageInfo(pageInfo) : QVariantMap()));
        }, this);
}

QMap<QString, QString> MainWindow::MakeTokenParams(QString vid, bool online)
{
    QMap<QString, QString> params{
                {"vid", vid},
                {"userId", ui->userIdLineEdit->text()},
                {"secretKey", ui->secretKeyLineEdit->text()},
                {"appId", ui->appIdLineEdit->text()},
                {"appSecret", ui->appSecretLineEdit->text()},
                {"subAccount", ui->subAccountCheckBox->isChecked() ? "1" : "0"}};
    if (!ui->viewerIdLineEdit->text().isEmpty()) {
        params["viewerId"] = ui->viewerIdLineEdit->text();
    }
    if (online) {
        params["securitySeed"] = ui->playSeedLineEdit->text();
    }
    return params;
}

void MainWindow::InitPlayer()
{
    if (!playWnd) {
        playWnd = new SimplePlayWidget();
        auto layout = new QGridLayout(ui->playLabel);
        layout->setContentsMargins(0, 0, 0, 0);
        layout->addWidget(playWnd);
    }
    player = PLVPlayerCreate((void*)playWnd->winId());
    PLVPlayerSetStateHandler(player,
        [](const char* vid, int state, void* data) {
            auto obj = static_cast<MainWindow*>(data);
            QMetaObject::invokeMethod(obj, "OnMediaState", Q_ARG(QString, QT_UTF8(vid)), Q_ARG(int, state));
        }, this);
    PLVPlayerSetPropertyHandler(player,
        [](const char* vid, int property, int format, const char* value, void* data) {
            auto obj = static_cast<MainWindow*>(data);
            QMetaObject::invokeMethod(obj, "OnMediaProperty",
                Q_ARG(QString, QT_UTF8(vid)), Q_ARG(int, property), Q_ARG(int, format), Q_ARG(QString, QT_UTF8(value)));
        }, this);
    PLVPlayerSetRateChangeHandler(player,
        [](const char* vid, int inputRate, int realRate, void* data) {
            auto obj = static_cast<MainWindow*>(data);
            QMetaObject::invokeMethod(obj, "OnMediaRateChanged", Q_ARG(QString, QT_UTF8(vid)), Q_ARG(int, inputRate), Q_ARG(int, realRate));
        }, this);
    PLVPlayerSetProgressHandler(player,
        [](const char* vid, int millisecond, void* data) {
            auto obj = static_cast<MainWindow*>(data);
            QMetaObject::invokeMethod(obj, "OnMediaProcess", Q_ARG(QString, QT_UTF8(vid)), Q_ARG(int, millisecond));
        }, this);
    PLVPlayerSetAudioPlayErrorHandler(player,
        [](const char* vid, void* data) {
            auto obj = static_cast<MainWindow*>(data);
            QString msg = QString("audio play error!");
            QMetaObject::invokeMethod(obj, "OnShowMsg", Q_ARG(const QString&, msg), Q_ARG(int, MsgCallback));
        }, this);
    PLVPlayerSetAudioDeviceHandler(player,
        [](int audioDeviceCount, void* data) {
            auto obj = static_cast<MainWindow*>(data);
            QString msg = QString("audio device count:%1").arg(audioDeviceCount);
            QMetaObject::invokeMethod(obj, "OnShowMsg", Q_ARG(const QString&, msg), Q_ARG(int, MsgCallback));
        }, this);
    connect(ui->progressWidget, SIGNAL(seek(qint64)), this, SLOT(OnSeek(qint64)));
    //config
    on_hardwareDecodeCheckBox_clicked();
    on_keepLastFrameCheckBox_clicked();
    on_videoOutputComboBox_currentIndexChanged(ui->videoOutputComboBox->currentIndex());
    on_logoEnableCheckBox_clicked();
    on_osdEnableCheckBox_clicked();
}

void MainWindow::FreePlayer()
{
    if (player) {
        PLVPlayerStop(player);
        PLVPlayerDestroy(player);
        player = nullptr;
        onlinePlay = true;
        playerVid.clear();
        inputRate = VIDEO_RATE_AUTO;
        currentRate = VIDEO_RATE_AUTO;
        ui->playVidLineEdit->clear();
        ui->playTokenLineEdit->clear();
    }
}

void MainWindow::PlayOnline(QString vid, bool encrypt, int rate)
{
    ui->playModeComboBox->setCurrentIndex(0);
    //encrypt need request token!
    ui->tokenCheckBox->setChecked(encrypt);
    if (encrypt && playerVid != vid) {
        ui->playTokenLineEdit->clear();
    }
    ui->tabWidget->setCurrentIndex(TabVideoPlay);
    ui->playVidLineEdit->setText(vid);
    ui->playRateComboBox->setCurrentIndex(rate);
    if (!IsPlaying() || !onlinePlay || playerVid != vid || inputRate != rate) {
        on_stopPushButton_clicked();
        on_playPushButton_clicked();
    }
}

void MainWindow::PlayOffline(QString videoPath, QString vid, int rate)
{
    ui->playModeComboBox->setCurrentIndex(1);
    ui->videoPathLineEdit->setText(videoPath);
    ui->tabWidget->setCurrentIndex(TabVideoPlay);
    ui->playVidLineEdit->setText(vid);
    ui->playRateComboBox->setCurrentIndex(rate);
    if (!IsPlaying() || onlinePlay || playerVid != vid || inputRate != rate) {
        on_stopPushButton_clicked();
        on_playPushButton_clicked();
    }
}

bool MainWindow::IsPlaying()
{
    return gettingToken ||
        (mediaState != MEDIA_STATE_NONE && mediaState != MEDIA_STATE_FAIL && mediaState != MEDIA_STATE_END);
}

int MainWindow::FindDownloader(int64_t downloader)
{
    int row = ui->downloadVideoTableWidget->rowCount();
    for (int i = 0; i < row; i++) {
        auto detailsWidget = qobject_cast<SimpleDownloadDetails*>(ui->downloadVideoTableWidget->cellWidget(i, 0));
        if (downloader == detailsWidget->property("downloader").toLongLong()) {
            return i;
        }
    }
    return -1;
}

void MainWindow::FreeDownloaders()
{
    int row = ui->downloadVideoTableWidget->rowCount();
    for (int i = 0; i < row; i++) {
        auto detailsWidget = qobject_cast<SimpleDownloadDetails*>(ui->downloadVideoTableWidget->cellWidget(i, 0));
        auto downloader = (PLVDownloadPtr)(detailsWidget->property("downloader").toLongLong());
        PLVDownloadDestroy(downloader);
    }
    ui->downloadVideoTableWidget->clearContents();
    ui->downloadVideoTableWidget->setRowCount(0);
}

void MainWindow::ScanFiles()
{
    auto FindMp4OrM3u8 = [](QString filepath) {
        QFileInfoList filelist = QDir(filepath).entryInfoList(QDir::Files | QDir::NoDotAndDotDot);
        for (auto& file : filelist) {
            if (file.suffix() == "m3u8" || file.suffix() == "mp4") {
                return true;
            }
        }
        return false;
    };
    //you need manager download files by yourself!
    ui->localVideoTableWidget->clearContents();
    ui->localVideoTableWidget->setRowCount(0);
    ui->localVideoTableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    ui->localVideoTableWidget->setColumnWidth(0, 310);
    ui->localVideoTableWidget->setColumnWidth(1, 200);
    int row = 0;
    QString path = ui->localVideoPathLineEdit->text();
    QDir dir(path);
    if (dir.exists()) {
        QStringList videodirs = dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
        for (auto& video : videodirs) {
            if (FindMp4OrM3u8(path + "/" + video)) {
                auto item0 = new QTableWidgetItem(video);
                auto operatorWidget = new SimpleDownloadOperator(true);
                operatorWidget->setProperty("vid", video);
                operatorWidget->setProperty("videoPath", path);
                connect(operatorWidget, &SimpleDownloadOperator::openDir, this,
                    [this]() {
                        QString videoPath = sender()->property("videoPath").toString();
                        QString vid = sender()->property("vid").toString();
                        QDesktopServices::openUrl(QUrl::fromLocalFile(videoPath + "/" + vid));
                    });
                connect(operatorWidget, &SimpleDownloadOperator::play, this,
                    [this]() {
                        QString videoPath = sender()->property("videoPath").toString();
                        QString vid = sender()->property("vid").toString();
                        PlayOffline(videoPath, vid, 0);
                    });
                connect(operatorWidget, &SimpleDownloadOperator::deleted, this,
                    [this]() {
                        QString videoPath = sender()->property("videoPath").toString();
                        QString vid = sender()->property("vid").toString();
                        if (IsPlaying() && !onlinePlay && vid == playerVid) {
                            QMessageBox::information(this, "Tips", QTStr("DeletePlayingTips"));
                            return;
                        }
                        PLVDeleteLocalVideoFile(QT_TO_UTF8(vid), QT_TO_UTF8(videoPath), 0);
                        ScanFiles();
                    });
                ui->localVideoTableWidget->insertRow(row);
                ui->localVideoTableWidget->setItem(row, 0, item0);
                ui->localVideoTableWidget->setCellWidget(row, 1, operatorWidget);
                row++;
            }
        }
    }
}
