#include "util.h"
#include <QStandardPaths>
#include <QDateTime>
#include <QApplication>
#include <QTextCodec>
#include <QPainter>
#include <QPaintEvent>
#include <QJsonDocument>
#include <QToolTip>
#include <QHBoxLayout>
#include <QPushButton>
#include <chrono>
#include <QBackingStore>
#include <QPainter>
#include <map>
#include <sstream>
#include "openssl/md5.h"

SimpleTranslator::SimpleTranslator(const QString& path)
    :settings(path, QSettings::IniFormat) {
    settings.setIniCodec("utf-8");
}

bool SimpleTranslator::isEmpty() const
{
    return false;
}

QString SimpleTranslator::translate(const char* context, const char* sourceText, const char* disambiguation, int n) const
{
    return settings.value(QT_UTF8(sourceText)).toString();
}

SimplePlayProcess::SimplePlayProcess(QWidget* parent)
    :QWidget(parent)
{
}

SimplePlayWidget::SimplePlayWidget(QWidget* parent)
    : QWidget(parent)
{
    setAttribute(Qt::WA_NativeWindow);
    setAttribute(Qt::WA_DontCreateNativeAncestors);
}

void SimplePlayWidget::SetPreventSoftwareRecording(bool enable)
{
    preventSoftwareRecording = enable;
    if (isFullScreen()) {
        PLVSetPreventSoftwareRecording((void*)winId(), preventSoftwareRecording);
    }
}

void SimplePlayWidget::EnterFullScreen()
{
    if (!isFullScreen()) {
        setWindowFlags(windowFlags() | Qt::Window);
        auto parentWidget = qobject_cast<QWidget*>(parent());
        if (parentWidget) {
            move(parentWidget->mapToGlobal(QPoint(0,0)));
            show();
        }
        showFullScreen();
        PLVSetPreventSoftwareRecording((void*)winId(), preventSoftwareRecording);
    }
}

void SimplePlayWidget::LeaveFullScreen()
{
    if (isFullScreen()) {
        PLVSetPreventSoftwareRecording((void*)winId(), false);
        setWindowFlags(windowFlags() & ~Qt::Window);
        showNormal();
    }
}

void SimplePlayWidget::SwitchFullScreen()
{
    if (isFullScreen()) {
        LeaveFullScreen();
    }
    else {
        EnterFullScreen();
    }
}

void SimplePlayWidget::keyPressEvent(QKeyEvent* e)
{
    if (e->key() == Qt::Key_Escape) {
        LeaveFullScreen();
    }
}

void SimplePlayWidget::closeEvent(QCloseEvent* e)
{
    LeaveFullScreen();
    e->ignore();
}

void SimplePlayProcess::SetMaxValue(qint64 value)
{
    maxValue = value > 0 ? value : 1;
}

void SimplePlayProcess::SetCurrentValue(qint64 value)
{
    currentValue = value;
}

void SimplePlayProcess::SetCacheValue(qint64 value)
{
    cacheValue = value;
}

void SimplePlayProcess::paintEvent(QPaintEvent* event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.fillRect(event->rect(), Qt::lightGray);
    painter.fillRect(0, 0, cacheValue * width() / maxValue, height(), Qt::darkGray);
    painter.fillRect(0, 0, currentValue * width() / maxValue, height(), Qt::blue);
}

void SimplePlayProcess::mouseMoveEvent(QMouseEvent* event)
{
    int64_t value = event->pos().x() * maxValue / width();
    QToolTip::showText(event->globalPos(), GetTimeString(value / 1000), this);
}

void SimplePlayProcess::mousePressEvent(QMouseEvent* event)
{
    int64_t value = event->pos().x() * maxValue / width();
    emit seek(value);
}

SimpleDownloadDetails::SimpleDownloadDetails(QVariantMap video, QVariantMap rateInfo, QWidget* parent)
    : QWidget(parent)
{
    int rate = rateInfo.value("rate").toInt();
    int64_t fileSize = rateInfo.value("filesize").toLongLong();
    auto nameLabel = new QLabel(video.value("title").toString());
    auto rateSizeLabel = new QLabel(QString("%1,%2:%3").arg(GetRateName(rate)).arg(QTStr("Size")).arg(GetFileSizeString(fileSize)));
    auto vidLabel = new QLabel(video.value("vid").toString());
    auto layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(2);
    layout->addWidget(nameLabel);
    layout->addWidget(rateSizeLabel);
    layout->addWidget(vidLabel);
}

SimpleDownloadProcess::SimpleDownloadProcess(QWidget* parent)
    :QWidget(parent)
{
    speedLabel = new QLabel(QTStr("DownloadSpeed").arg("0KB"));
    percentLabel = new QLabel(QTStr("DownloadPercent").arg("0"));
    remainLabel = new QLabel(QTStr("DownloadRemain").arg("99:99:99"));
    auto layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(2);
    layout->addWidget(speedLabel);
    layout->addWidget(percentLabel);
    layout->addWidget(remainLabel);
}

void SimpleDownloadProcess::SetMaxValue(int64_t value)
{
    maxValue = value;
}

void SimpleDownloadProcess::SetCurrentValue(int64_t value)
{
    currentValue = value;
    int spanTime = startTime.secsTo(QTime::currentTime());
    spanTime = spanTime < 1 ? 1 : spanTime;
    int speed = currentValue / spanTime;
    speedLabel->setText(QTStr("DownloadSpeed").arg(GetFileSizeString(speed)));
    int percent = currentValue * 100 / maxValue;
    percentLabel->setText(QTStr("DownloadPercent").arg(percent));
    int remain = (maxValue - currentValue) / speed;
    remainLabel->setText(QTStr("DownloadRemain").arg(GetTimeString(remain)));
    update();
}

void SimpleDownloadProcess::paintEvent(QPaintEvent* event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.fillRect(event->rect(), Qt::lightGray);
    painter.fillRect(0, 0, currentValue * width() / maxValue, height(), Qt::darkGray);
}

SimpleDownloadOperator::SimpleDownloadOperator(bool localFile, QWidget* parent)
    :QWidget(parent)
{
    auto startButton = new QPushButton(QTStr("Start"));
    auto pauseButton = new QPushButton(QTStr("Pause"));
    auto resumeButton = new QPushButton(QTStr("Resume"));
    auto stopButton = new QPushButton(QTStr("Stop"));
    auto OpenDirButton = new QPushButton(QTStr("OpenDir"));
    auto playButton = new QPushButton(QTStr("Play"));
    auto deleteButton = new QPushButton(QTStr("Delete"));
    auto layout = new QGridLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(2);
    layout->addWidget(startButton, 0, 0);
    layout->addWidget(pauseButton, 0 ,1);
    layout->addWidget(resumeButton, 0, 2);
    layout->addWidget(stopButton, 0, 3);
    layout->addWidget(OpenDirButton, 1, 0, 1, 2);
    layout->addWidget(playButton, 1, 2);
    layout->addWidget(deleteButton, 1, 3);
    connect(startButton, SIGNAL(clicked()), this, SIGNAL(start()));
    connect(pauseButton, SIGNAL(clicked()), this, SIGNAL(pause()));
    connect(resumeButton, SIGNAL(clicked()), this, SIGNAL(resume()));
    connect(stopButton, SIGNAL(clicked()), this, SIGNAL(stop()));
    connect(OpenDirButton, SIGNAL(clicked()), this, SIGNAL(openDir()));
    connect(playButton, SIGNAL(clicked()), this, SIGNAL(play()));
    connect(deleteButton, SIGNAL(clicked()), this, SIGNAL(deleted()));
    startButton->setVisible(!localFile);
    pauseButton->setVisible(!localFile);
    resumeButton->setVisible(!localFile);
    stopButton->setVisible(!localFile);
}

static const char KStrKey[] = { 'K','P','l','v','P','l','a','y','e','r','S','t','r','K','e','y' };

QString Encrypt(const QString& src)
{
    if (src.isEmpty()) {
        return src;
    }
    QByteArray dst = src.toUtf8();
    int size = sizeof(KStrKey) - 1;
    for (int i = 0; i < dst.size(); ++i) {
        dst[i] = dst[i] ^ KStrKey[i % size];
    }
    return QString::fromUtf8(dst.toBase64());
}

QString Decrypt(const QString& src)
{
    if (src.isEmpty()) {
        return src;
    }
    QByteArray dst = src.toUtf8();
    dst = QByteArray::fromBase64(dst);
    int size = sizeof(KStrKey) - 1;
    for (int i = 0; i < dst.size(); ++i) {
        dst[i] = dst[i] ^ KStrKey[i % size];
    }
    return QString::fromUtf8(dst);
}

QString GetConfigPath()
{
    QStringList dataPaths = QStandardPaths::standardLocations(QStandardPaths::AppDataLocation);
    return QString("%1/config.ini").arg(dataPaths.at(0));
}

QString GetLogPath()
{
    QStringList dataPaths = QStandardPaths::standardLocations(QStandardPaths::AppDataLocation);
    return QString("%1/logs/sdk_%2.log").arg(dataPaths.at(0)).arg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh-mm-ss"));
}

QString GetCrashPath()
{
    QStringList dataPaths = QStandardPaths::standardLocations(QStandardPaths::AppDataLocation);
    return QString("%1/crashs/%2.dmp").arg(dataPaths.at(0)).arg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh-mm-ss"));
}

QString GetCAPath()
{
#ifdef _WIN32
    return QApplication::applicationDirPath() + "/cacert.pem";
#elif defined(__APPLE__)
    return QApplication::applicationDirPath() + "/../Resources/cacert.pem";
#endif
}

QString GetVideoPath()
{
    QStringList tempPaths = QStandardPaths::standardLocations(QStandardPaths::MoviesLocation);
    return tempPaths.at(0) + "/" + SIMPLE_DEMO_NAME;
}

QString GetScreenshotPath()
{
    QStringList tempPaths = QStandardPaths::standardLocations(QStandardPaths::MoviesLocation);
    return tempPaths.at(0) + "/" + SIMPLE_DEMO_NAME + "/screenshot";
}

QVariantList MakeVideoInfoList(const PLVVideoRequestInfo* infos, int num)
{
    QVariantList list;
    for (int n = 0; n < num; n++) {
        QVariantMap var;
        var.insert("encrypt", infos[n].encrypt);
        var.insert("status", infos[n].status);
        var.insert("cateId", infos[n].cateId);
        var.insert("sourceFilesize", infos[n].sourceFilesize);
        var.insert("duration", infos[n].duration);
        var.insert("vid", infos[n].vid);
        var.insert("title", infos[n].title);
        var.insert("thumbnailURL", infos[n].thumbnailURL);
        var.insert("coverURL", infos[n].coverURL);
        QVariantList rates;
        for (int i = 0; i < infos[n].ratesNum; i++) {
            QVariantMap rate;
            rate.insert("filesize", infos[n].rates[i].filesize);
            rate.insert("rate", infos[n].rates[i].rate);
            rate.insert("bitrate", infos[n].rates[i].bitrate);
            rate.insert("fps", infos[n].rates[i].fps);
            rate.insert("height", infos[n].rates[i].height);
            rate.insert("width", infos[n].rates[i].width);
            rates.append(rate);
        }
        var.insert("rates", rates);
        list.append(var);
    }
    return std::move(list);
}

QVariantMap MakeVideoPageInfo(const PLVVideoRequestPageInfo* info)
{
    QVariantMap var;
    var.insert("type", info->type);
    var.insert("totalPages", info->totalPages);
    var.insert("totalItems", info->totalItems);
    var.insert("pageNumber", info->pageNumber);
    var.insert("pageSize", info->pageSize);
    return std::move(var);
}

QString GetFileSizeString(int64_t size)
{
    const int64_t GB = 1024ll * 1024ll * 1024ll;
    const int64_t MB = 1024ll * 1024ll;
    const int64_t KB = 1024ll;
    if (size >= GB) {
        double s = (double)size / (double)GB;
        return QString("%1GB").arg(QString::number(s, 'f', 2));
    }
    else if (size >= MB) {
        double s = (double)size / (double)MB;
        return QString("%1MB").arg(QString::number(s, 'f', 2));
    }
    else if (size >= KB) {
        double s = (double)size / (double)KB;
        return QString("%1KB").arg(QString::number(s, 'f', 2));
    }
    else {
        return QString("%1B").arg(size);
    }
}

QString GetTimeString(int64_t s)
{
    auto t = QTime(0, 0, 0).addSecs((int)s);
    return t.toString("hh:mm:ss");
}

QString GetRateName(int type)
{
    switch (type) {
    case VIDEO_RATE_AUTO: return QTStr("Auto");
    case VIDEO_RATE_LD: return QTStr("LD");
    case VIDEO_RATE_SD: return QTStr("SD");
    case VIDEO_RATE_HD: return QTStr("HD");
    case VIDEO_RATE_SOURCE: return QTStr("Source");
    default: return QTStr("Unknown");
    }
}
QString GetRateListString(QVariantList rates)
{
    QString rateStr;
    for (int i = 0; i < rates.size(); i++) {
        auto rate = rates.at(i).toMap();
        int type = rate.value("rate").toInt();
        int width = rate.value("width").toInt();
        int height = rate.value("height").toInt();
        int fps = rate.value("fps").toInt();
        if (i != 0) {
            rateStr += ",";
        }
        rateStr += QString("%1").arg(GetRateName(type))/*.arg(width).arg(height)*/;
    }
    return rateStr;
}

QString GetStateName(int state)
{
    switch (state) {
    case MEDIA_STATE_NONE:return QTStr("None");
    case MEDIA_STATE_LOADING:return QTStr("PlayStateLoading");
    case MEDIA_STATE_LOADED:return QTStr("PlayStateLoaded");
    case MEDIA_STATE_PLAY:return QTStr("PlayStatePlay");
    case MEDIA_STATE_PAUSE:return QTStr("PlayStatePause");
    case MEDIA_STATE_BEGIN_CACHE:return QTStr("PlayStateBeginCache");
    case MEDIA_STATE_END_CACHE:return QTStr("PlayStateEndCache");
    case MEDIA_STATE_BEGIN_SEEKING:return QTStr("PlayStateBeginSeek");
    case MEDIA_STATE_END_SEEKING:return QTStr("PlayStateEndSeek");
    case MEDIA_STATE_FAIL:return QTStr("PlayStateFailed");
    case MEDIA_STATE_END:return QTStr("PlayStateEnd");
    default:return QTStr("Unknown");
    }
}

QString GetPlayStateString(int rate, int state)
{
    return QTStr("PlayStateTips").arg(GetRateName(rate)).arg(GetStateName(state));
}

static long long GetTimestampMS()
{
    std::chrono::time_point<std::chrono::system_clock, std::chrono::milliseconds> tp =
        std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now());
    auto tmp = std::chrono::duration_cast<std::chrono::milliseconds>(tp.time_since_epoch());
    return tmp.count();
}

static std::string GenerateMd5(std::string in, bool upper)
{
    unsigned char md5Result[16];
    MD5_CTX md5ctx;
    MD5_Init(&md5ctx);
    MD5_Update(&md5ctx, (void*)in.c_str(), in.size());
    MD5_Final(md5Result, &md5ctx);
    std::stringstream ss;
    char buf[4] = { 0 };
    for (auto c : md5Result) {
        snprintf(buf, 4, upper ? "%02X" : "%02x", c);
        ss << buf;
    }
    return ss.str();
}

static int HttpProcess(void* user, double dtotal, double dnow, double, double)
{
    auto abort = static_cast<std::atomic_bool*>(user);
    if (abort && abort->load()) {
        return -1;
    }
    return 0;
}

static size_t HttpWriteString(char* data, size_t size, size_t nmemb, void* user)
{
    auto& result = *static_cast<std::string*>(user);
    size_t total = size * nmemb;
    if (total) {
        result.append(data, total);
    }
    return total;
}

bool GetVideoToken(const QMap<QString, QString>& params, QString& token, std::atomic_bool* abort)
{
    //make params
    std::map<std::string, std::string> httpParams;
    httpParams["extraParams"] = "pc-sdk";
    httpParams["ts"] = std::to_string(GetTimestampMS());
    httpParams["videoId"] = QT_TO_UTF8(params.value("vid"));
    if (params.contains("viewerId")) {
        httpParams["viewerId"] = QT_TO_UTF8(params.value("viewerId"));
    }
    if (params.contains("viewerName")) {
        httpParams["viewerName"] = QT_TO_UTF8(params.value("viewerName"));
    }
    if (params.contains("viewerIp")) {
        httpParams["viewerIp"] = QT_TO_UTF8(params.value("viewerIp"));
    }
    bool subAccount = params.value("subAccount") == "1";
    std::string secret = subAccount ? QT_TO_UTF8(params.value("appSecret")) :
        QT_TO_UTF8(params.value("secretKey"));
    if (!subAccount) {
        httpParams["userId"] = QT_TO_UTF8(params.value("userId"));
    }
    else {
        httpParams["appId"] = QT_TO_UTF8(params.value("appId"));
    }
    //only for vrm12|vrm13 online play when use seed.
    if (!params.value("securitySeed").isEmpty()) {
        httpParams["securitySeed"] = QT_TO_UTF8(params.value("securitySeed"));
    }
    //todo: expires, disposable
    std::stringstream signss;
    signss << secret;
    for (auto& it : httpParams) {
        signss << it.first << it.second;
    }
    signss << secret;
    std::string sign = GenerateMd5(signss.str(), true);
    httpParams["sign"] = sign;
    std::stringstream postDatass;
    for (auto& it : httpParams) {
        if (!postDatass.str().empty()) {
            postDatass << "&";
        }
        postDatass << it.first << "=" << it.second;
    }
    std::string postData = postDatass.str();
    //for curl
    int errorCode = 200;
    char errorStr[CURL_ERROR_SIZE] = { 0 };
    std::string result;
    curl_slist* header = nullptr;
    header = curl_slist_append(header, "application/x-www-form-urlencoded");
    CURL* curl = curl_easy_init();
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, header);
    curl_easy_setopt(curl, CURLOPT_CAINFO, QT_TO_UTF8(GetCAPath()));
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2);
    curl_easy_setopt(curl, CURLOPT_URL, !subAccount ?
        "https://hls.videocc.net/service/v1/token" :
        "https://hls.videocc.net/service/v2/token/create-child");
    curl_easy_setopt(curl, CURLOPT_POST, 1L);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postData.c_str());
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 10);
    curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0);
    curl_easy_setopt(curl, CURLOPT_PROGRESSFUNCTION, HttpProcess);
    curl_easy_setopt(curl, CURLOPT_PROGRESSDATA, abort);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, HttpWriteString);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &result);
    curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, errorStr);
    CURLcode code = curl_easy_perform(curl);
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &errorCode);
    curl_easy_cleanup(curl);
    curl_slist_free_all(header);
    if (code == CURLE_OK && errorCode == 200) {
        QJsonParseError err;
        QJsonDocument jsonDocument = QJsonDocument::fromJson(result.c_str(), &err);
        QVariantMap var = jsonDocument.toVariant().toMap();
        token = var.value("data").toMap().value("token").toString();
        return true;
    }
    else {
        token = *errorStr ? QT_UTF8(errorStr) : QT_UTF8(result.c_str());
        return false;
    }
}
