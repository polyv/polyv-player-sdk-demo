#include "VideoControl.h"

#include <QFile>
#include <QMenu>
#include <QPointer>
#include <QStackedLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFileInfo>
#include <QUrl>
#include <QDesktopServices>

#include <http/http-client.h>
#include <http/http-param.h>
#include <http/http-manager.h>

#include "AppDef.h"
#include "Application.h"
#include "MainWindow.h"
#include "SdkManager.h"
#include "StatusButton.h"
#include "Application.h"
#include "WidgetHelper.h"

DownloadManager::DownloadManager(void)
{
}

void DownloadManager::Start(void)
{
	if (download) {
		return;
	}
	isExit = false;
	download = new std::thread(&DownloadManager::Run, this);
	download->joinable();
}
void DownloadManager::Stop(void)
{
	{
		std::lock_guard<std::mutex> l(mutex);
		while (!queueTask.empty()) {
			queueTask.pop();
		}
		isExit = true;
	}
	if (download) {
		download->join();
		delete download;
		download = nullptr;
	}
}

int DownloadManager::AppendTask(const SharedVideoPtr& video, 
	const std::string& url, const std::string& filename)
{
	if (isExit) {
		return 0;
	}
	static int s_taskId = 0;
	std::lock_guard<std::mutex> l(mutex);
	int taskId = ++s_taskId;
	queueTask.push(Item(taskId, url, filename, video));
	return taskId;
}


void DownloadManager::Run()
{
	struct Item item;
	while (!isExit) {	
		if (queueTask.empty()) {
			std::this_thread::sleep_for(std::chrono::milliseconds(200));
			continue;
		}
		{
			std::lock_guard<std::mutex> l(mutex);
			if (!queueTask.empty()) {
				item = queueTask.front();
				queueTask.pop();
			}	
		}
		HttpClient client(item.url.c_str(), HTTP_DOWNLOAD);
		client.SetCaFile(QT_TO_UTF8(App()->GetCacertFilePath()));
#ifdef _WIN32
		std::string filename = QT_UTF8(item.filename.c_str()).toLocal8Bit();
#else
		std::string filename = item.filename.c_str();
#endif // _WIN32
		client.SetDownload(filename.c_str(), nullptr, nullptr);
		std::string result;
		if (!client.Request(result)) {
			slog_error("file open error,file:%s,url:%s",
				item.filename.c_str(), item.url.c_str());
			item.video->DownloadImageResult(item.taskId, false);
		}
		else {
			item.video->DownloadImageResult(item.taskId, true);
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(200));
	}
}

////////////////////////////////////////////////
DownloadManager* VideoInfo::downloadImage = nullptr;
VideoInfo::VideoInfo()
	: QObject(nullptr)
{
	qRegisterMetaType<std::string>("std::string");
	qRegisterMetaType<SharedVideoPtr>("SharedVideoPtr");
}

VideoInfo::~VideoInfo()
{
	//ExitDownload();
}

void VideoInfo::StartDownloadThread(void)
{
	if (downloadImage) {
		return;
	}
	downloadImage = new DownloadManager;
	downloadImage->Start();
}

void VideoInfo::StopDownloadThread(void)
{
	if (!downloadImage) {
		return;
	}
	downloadImage->Stop();
	delete downloadImage;
	downloadImage = nullptr;
}

bool VideoInfo::SetTitle(const std::string& title, bool signal/* = true*/) 
{
	if (this->title == title) {
		return false;
	}
	this->title = title;
	if (!signal) {
		return true;
	}
	emit SignalAttributeChanged(ATTR_VIDEO_TITLE, shared_from_this());
	return true;
}
bool VideoInfo::SetImage(const std::string& url)
{
	if (this->imageUrl == url && !this->imageFile.empty()) {
		return false;
	}
	if (std::string::npos != url.find("http")) {
		this->imageUrl = url;
		DownloadImage();
	}
	return true;
}
bool VideoInfo::SetDownloadStatus(int status, bool signal/* = true*/)
{
	//if (this->downloadStatus == status) {
	//	return false;
	//}
	this->downloadStatus = status;
	if (!signal) {
		return true;
	}
	emit SignalAttributeChanged(ATTR_VIDEO_DOWNLOAD_STATUS, shared_from_this());
	return true;
}

void VideoInfo::DownloadImage(void)
{
	//if (std::string::npos == imageUrl.find("http")) {
	//	return; 
	//}
	QFileInfo fileInfo(QT_UTF8(imageUrl.c_str()));
	QString fileName = QString("%1/%2").arg(App()->GetImagePath()).arg(fileInfo.fileName());
	if (QFile::exists(fileName)) {
		this->imageFile = QT_TO_UTF8(fileName);
		emit SignalAttributeChanged(ATTR_VIDEO_IMAGE, shared_from_this());
		return;
	}
	if (0 != taskId) {
		return;
	}
	filename = QT_TO_UTF8(fileName);
	if (downloadImage) {
		taskId = downloadImage->AppendTask(shared_from_this(), imageUrl, filename);
	}
	//if (isExitDownload) {
	//	return;
	//}
	//if (!downloadImage) {
	//	downloadImage = new DownloadManager();
	//	downloadImage->start();
	//}

	//


	//auto thread = App()->GetMainWindow()->GetDownloadThread();
	//if (!thread) {
	//	return;
	//}
	//connect(downloadImage, SIGNAL(SignalResult(int, bool)), this,
	//	SLOT(OnDownloadImageResult(int, bool)), Qt::QueuedConnection);
	

	//download = new DownloadManager(imageUrl, filename);
	//connect(download, &DownloadManager::SignalResult, this, [_this, filename](bool result) {
	//	_this->ExitDownload();
	//	if (!result) {
	//		blog_error("[http]:download image error:%s", filename.c_str());
	//		return;
	//	}
	//	QMetaObject::invokeMethod(_this.get(), "OnDownloadImage", Qt::QueuedConnection,
	//		Q_ARG(std::string, filename));
	//}, Qt::QueuedConnection);

	//HttpClient* client = new HttpClient(imageUrl, HTTP_DOWNLOAD);
	//client->SetDownload(QT_TO_UTF8(fileName), nullptr, nullptr);
	//imageTaskId = HttpManager::GetManager()->Request(client, [](bool result, int code, const std::string& data, void* user) {
	//	VideoInfo* obj = (VideoInfo*)user;
	//	if (!result) {
	//		blog_error("[http]:download image code:%d,msg:%s", code, data.c_str());
	//		return;
	//	}
	//	QMetaObject::invokeMethod(obj, "OnDownloadImage", Qt::QueuedConnection,
	//		Q_ARG(std::string, data));
	//}, this);
}

void VideoInfo::DownloadImageResult(int taskId, bool result)
{
	if (this->taskId != taskId) {
		return;
	}
	this->taskId = 0;
	if (!result) {
		slog_error("download image error:%s", this->filename.c_str());
		return;
	}
	this->imageFile = filename;
	emit SignalAttributeChanged(ATTR_VIDEO_IMAGE, shared_from_this());
}

//void VideoInfo::ExitDownload(void)
//{
//	if (!download) {
//		return;
//	}
//	download->wait();
//	download->deleteLater();
//	download = nullptr;
//}

void VideoInfo::OnDownloadImage(std::string fileName)
{
	//imageTaskId = std::thread::id();
	this->imageFile = fileName;
	emit SignalAttributeChanged(ATTR_VIDEO_IMAGE, shared_from_this());
}

////////////////////////////////////////////////
VideoCoverWidget::VideoCoverWidget(QWidget* parent, bool local, const SharedVideoPtr& video)
	: QWidget(parent)
	, localPlay(local)
	, videoInfo(video)
{
	QStackedLayout* layout = new QStackedLayout();
	layout->setSpacing(0);
	layout->setContentsMargins(0, 0, 0, 0);
	layout->setStackingMode(QStackedLayout::StackAll);
	setLayout(layout);

	auto CreateWidget = [](QWidget* parent) {
		QWidget* page = new QWidget(parent);
		QHBoxLayout* layout = new QHBoxLayout();
		layout->setSpacing(0);
		layout->setContentsMargins(0, 0, 0, 0);
		page->setLayout(layout);
		return page;
	};

	QWidget* page2 = CreateWidget(this);
	layout->addWidget(page2);
	QPushButton* playButton = new StatusButton(page2, "playButton",
		localPlay ? QTStr("LocalPlay") : QTStr("OnlinePlay"), QSize(24, 24));
	connect(playButton, &QPushButton::clicked, [&] {
		QMetaObject::invokeMethod((QWidget*)App()->GetMainWindow(), 
			"OnPlayVideo", Q_ARG(bool, localPlay), Q_ARG(const SharedVideoPtr&, videoInfo));
	});
	((QHBoxLayout*)page2->layout())->addWidget(playButton, 0, Qt::AlignCenter);

	QWidget* page1 = CreateWidget(this);
	layout->addWidget(page1);
	imageLabel = new QLabel(page1);
	imageLabel->setObjectName("imageLabel");
	imageLabel->setFixedSize(QSize(80, 45));
	((QHBoxLayout*)page1->layout())->addWidget(imageLabel, 0, Qt::AlignCenter);

	connect(video.get(), SIGNAL(SignalAttributeChanged(int, const SharedVideoPtr&)),
		this, SLOT(OnAttributeChanged(int, const SharedVideoPtr&)), Qt::QueuedConnection);

	if (QFile::exists(QT_UTF8(video->imageFile.c_str()))) {
		OnAttributeChanged(ATTR_VIDEO_IMAGE, video);
	}
}

void VideoCoverWidget::OnAttributeChanged(int attribute, const SharedVideoPtr& video)
{
	if (ATTR_VIDEO_IMAGE != attribute) {
		return;
	}
	imageLabel->setPixmap(QPixmap::fromImage(QImage(QT_UTF8(video->imageFile.c_str()))).
		scaled(80, 45, Qt::KeepAspectRatio));
}

////////////////////////////////////////////////
VideoTitleWidget::VideoTitleWidget(QWidget* parent, bool showVID, const SharedVideoPtr& video)
	: QWidget(parent)
	, videoInfo(video)
{
	QVBoxLayout* layout = new QVBoxLayout();
	layout->setSpacing(2);
	layout->setContentsMargins(6, 0, 0, 0);
	setLayout(layout);
	textLabel = new QLabel(this);
	textLabel->setObjectName("textLabel");
	textLabel->setText(QT_UTF8(video->title.c_str()));
	
	if (showVID) {
		textLabel->setAlignment(Qt::AlignLeft | Qt::AlignBottom);
		layout->addWidget(textLabel, 1, Qt::AlignLeft | Qt::AlignBottom);

		vidLabel = new QLabel(this);
		vidLabel->setObjectName("vidLabel");
		vidLabel->setAlignment(Qt::AlignLeft | Qt::AlignTop);
		layout->addWidget(vidLabel, 1, Qt::AlignLeft | Qt::AlignTop);
		vidLabel->setText(QT_UTF8(video->vid.c_str()));
	}
	else {
		textLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
		layout->addWidget(textLabel, 1, Qt::AlignLeft | Qt::AlignVCenter);
	}

	connect(video.get(), SIGNAL(SignalAttributeChanged(int, const SharedVideoPtr&)),
		this, SLOT(OnAttributeChanged(int, const SharedVideoPtr&)), Qt::QueuedConnection);
}

void VideoTitleWidget::resizeEvent(QResizeEvent *event)
{
	QWidget::resizeEvent(event);
	OnAttributeChanged(ATTR_VIDEO_TITLE, videoInfo);
}

void VideoTitleWidget::OnAttributeChanged(int attribute, const SharedVideoPtr& video)
{
	if (ATTR_VIDEO_TITLE != attribute) {
		return;
	}
	QFontMetrics fontWidth(textLabel->font());

	int textWidth = fontWidth.horizontalAdvance(QT_UTF8(videoInfo->title.c_str()));
	int widgetWidth = this->width() - 8;
	if (textWidth < widgetWidth) {
		textLabel->setText(QT_UTF8(videoInfo->title.c_str()));
		textLabel->setToolTip(QString());
		return;
	}
	QString showName = fontWidth.elidedText(QT_UTF8(videoInfo->title.c_str()), Qt::ElideRight, widgetWidth);
	textLabel->setText(showName);
	textLabel->setToolTip(QT_UTF8(videoInfo->title.c_str()));
}



////////////////////////////////////////////////
VideoDownloadWidget::VideoDownloadWidget(QWidget* parent, const SharedVideoPtr& video, int rate)
	: QWidget(parent)
	, videoInfo(video)
	, curRate(rate)
	, totalSize(0)
	, currentSize(0)
	, receivedSize(0)
{
	progressBar = new QProgressBar(this);
	progressBar->setMinimum(0);
	progressBar->setMaximum(100);
	progressBar->setValue(0);
	progressBar->setOrientation(Qt::Horizontal);
	progressBar->setFixedHeight(4);
	progressBar->setTextVisible(false);

	tipsLabel = new QLabel(this);
	tipsLabel->setObjectName("progressLabel");
	speedLabel = new QLabel(this);
	speedLabel->setObjectName("speedLabel");

	QHBoxLayout* hlayout = new QHBoxLayout();
	hlayout->addWidget(tipsLabel);
	hlayout->addStretch(1);
	hlayout->addWidget(speedLabel);

	QVBoxLayout* layout = new QVBoxLayout();
	layout->setSpacing(8);
	layout->setContentsMargins(10, 0, 10, 0);
	layout->addStretch(1);
	layout->addWidget(progressBar);
	layout->addLayout(hlayout);
	layout->addStretch(1);
	setLayout(layout);
	
	downloadObj = PLVDownloadCreate();
	PLVDownloadSetErrorHandler(downloadObj, [](const char* vid, int code, void* data) {
		(void)vid;
		VideoDownloadWidget* obj = (VideoDownloadWidget*)data;
		QMetaObject::invokeMethod(obj, "OnDownloadErrorHandler", Qt::QueuedConnection, 
			Q_ARG(int, code));
	}, this);
	//downloadObj->SetCurrentBitRateHandler([](const char* vid, int inputBitRate, int realBitRate, void* data) {
	//	(void)vid;
	//	VideoDownloadWidget* obj = (VideoDownloadWidget*)data;
	//	QMetaObject::invokeMethod(obj, "OnCurrentBitRateHandler", Qt::QueuedConnection,
	//		Q_ARG(int, inputBitRate), Q_ARG(int, realBitRate));
	//}, this);
	PLVDownloadSetProgressHandler(downloadObj, [](const char* vid, long long receivedBytes, long long totalBytes, void* data) {
		(void)vid;
		VideoDownloadWidget* obj = (VideoDownloadWidget*)data;
		QMetaObject::invokeMethod(obj, "OnDownloadProgressHandler", Qt::QueuedConnection, 
			Q_ARG(long long, receivedBytes), Q_ARG(long long, totalBytes));
	}, this);
	PLVDownloadSetResultHandler(downloadObj, [](const char* vid, int rate, int code, void* data) {
		(void)vid;
		VideoDownloadWidget* obj = (VideoDownloadWidget*)data;
		QMetaObject::invokeMethod(obj, "OnDownloadResultHandler", Qt::QueuedConnection,
			Q_ARG(int, rate), Q_ARG(int, code));
	}, this);

	QString path = App()->GlobalConfig().Get("Download", "FilePath").toString();
	video->filePath = QT_TO_UTF8(path);
	video->rate = rate;
	PLVDownloadSetVideo(downloadObj, video->vid.c_str(), video->filePath.c_str(), rate);
	int code = PLVDownloadStart(downloadObj, true);

	if (E_NO_ERR != code) {
		tipsLabel->setText(QTStr("DownloadError"));
		SetControlSheet(tipsLabel, "LabelStyle", "error");
	}
	else {
		//tipsLabel->setText(QTStr("DownloadWait"));
	}

	connect(video.get(), SIGNAL(SignalAttributeChanged(int, const SharedVideoPtr&)),
		this, SLOT(OnAttributeChanged(int, const SharedVideoPtr&)), Qt::QueuedConnection);

	speedTimer = new QTimer(this);
	speedTimer->setInterval(1000);
	connect(speedTimer, SIGNAL(timeout()), this, SLOT(OnSpeedTimer()));
	speedTimer->start();
}

VideoDownloadWidget::~VideoDownloadWidget()
{
	PLVDownloadDestroy(downloadObj);
	downloadObj = nullptr;
}

void VideoDownloadWidget::OnDownloadErrorHandler(int code)
{
	slog_error("error code:%d, msg:%s", code, PLVGetSdkErrorDescription(code));
}
//void VideoDownloadWidget::OnCurrentBitRateHandler(int inputBitRate, int realBitRate)
//{
//	videoInfo->rate = realBitRate;
//	QString msg = QString(QTStr("BitRateChannge")).arg(inputBitRate).arg(realBitRate);
//	QMetaObject::invokeMethod((QWidget*)App()->GetMainWindow(), "OnShowToast",
//		Q_ARG(const QString&, msg));
//}
void VideoDownloadWidget::OnDownloadProgressHandler(long long receivedBytes, long long totalBytes)
{
	long value = 0;
	if (totalBytes) {
		value = receivedBytes * 1.0 / totalBytes * 100.0;
		progressBar->setValue(value);

		this->totalSize = totalBytes;
		this->currentSize += (receivedBytes - this->receivedSize);
		this->receivedSize = receivedBytes;
	}
	//tipsLabel->setText(QString("%1%").arg(value));
}

void VideoDownloadWidget::OnDownloadResultHandler(int rate, int code)
{
	if (E_NO_ERR != code) {
		OnDownloadErrorHandler(code);
		switch (code) 
		{
		case E_ABORT_DOWNLOAD:
		case E_DELETE_VIDEO:
			videoInfo->SetDownloadStatus(DOWNLOAD_OK);
			break;
		default:
			videoInfo->SetDownloadStatus(DOWNLOAD_ERROR);
			break;
		}
	}
	else {
		progressBar->setValue(100);
		tipsLabel->setText(QString("100%"));

		SetControlSheet(tipsLabel, "LabelStyle", "");
		QMetaObject::invokeMethod((QWidget*)App()->GetMainWindow(), "OnAppendLocalVideo", Qt::QueuedConnection,
			Q_ARG(int, rate), Q_ARG(const SharedVideoPtr&, videoInfo));
	}
}

void VideoDownloadWidget::OnAttributeChanged(int attribute, const SharedVideoPtr& video)
{
	switch (attribute)
	{
	case ATTR_VIDEO_DOWNLOAD_STATUS:
		switch (video->downloadStatus)
		{
		case DOWNLOAD_OK:
			break;
		case DOWNLOAD_RUN:
			PLVDownloadStart(downloadObj, true);
			speedTimer->start();
			break;
		case DOWNLOAD_PAUSE:
			PLVDownloadStop(downloadObj);
			speedTimer->stop();
			tipsLabel->setText(QTStr("Pause"));
			speedLabel->setText(QString());
			break;
		case DOWNLOAD_ERROR:
			tipsLabel->setText(QTStr("DownloadError"));
			SetControlSheet(tipsLabel, "LabelStyle", "error");
			break;
		case DOWNLOAD_CANCEL:
			PLVDownloadDelete(downloadObj);
			QMetaObject::invokeMethod((QWidget*)App()->GetMainWindow(), "OnRemoveDownloadVideo",
				Qt::QueuedConnection, Q_ARG(const SharedVideoPtr&, videoInfo));
			break;
		}
		break;
	}
}

void VideoDownloadWidget::OnSpeedTimer(void)
{
	static const int c_size = 1024;
	long speed = currentSize / c_size;
	currentSize = 0;
	if (0 >= speed) {
		speed = 0;
	}
	QString speedStr;
	if (speed < 1024) {
		speedStr = QString("%1%2").arg(speed).arg("KB/S");
	}
	else {
		speedStr = QString("%1%2").arg(QString::number((double)speed / c_size, 'f', 2)).arg("MB/S");
	}
	QString timerStr = QTStr("OverOneDay");
	if (0 != speed) {
		QString second = "00", minute = "00", hour = "00";
		qint64 leftSize = 0;
		leftSize = this->totalSize - this->receivedSize;
		qint64 leftTime;
		leftTime = (leftSize / c_size) / speed;
		if (leftTime >= 0 && leftTime < 60) {
			second = QString::number(leftTime);
			if (second.length() < 2) {
				second.insert(0, "0");
			}
			timerStr = QString("%1:%2:%3").arg(hour).arg(minute).arg(second);
		}
		else if (leftTime >= 60 && leftTime < 3600){
			minute = QString::number(leftTime / 60);
			if (minute.length() < 2) {
				minute.insert(0, "0");
			}	
			second = QString::number(leftTime % 60);
			if (second.length() < 2) {
				second.insert(0, "0");
			}
			timerStr = QString("%1:%2:%3").arg(hour).arg(minute).arg(second);
		}
		else if (leftTime >= 3600 && leftTime < 3600 * 24){
			hour = QString::number(leftTime / 3600);
			if (hour.length() < 2) {
				hour.insert(0, "0");
			}
			minute = QString::number((leftTime % 3600) / 60);
			if (minute.length() < 2) {
				minute.insert(0, "0");
			}
			second = QString::number((leftTime % 3600) % 60);
			if (second.length() < 2) {
				second.insert(0, "0");
			}				
			timerStr = QString("%1:%2:%3").arg(hour).arg(minute).arg(second);
		}
	}
	tipsLabel->setText(timerStr);
	speedLabel->setText(speedStr);
}

////////////////////////////////////////////////
VideoActionWidget::VideoActionWidget(QWidget* parent, ActionType action, const SharedVideoPtr& video)
	: QWidget(parent)
	, actionType(action)
	, videoInfo(video)
{
	QHBoxLayout* layout = new QHBoxLayout();
	layout->setSpacing(16);
	layout->setContentsMargins(0, 0, 0, 0);
	setLayout(layout);

	switch (action)
	{
	case ACTION_DOWNLOAD:
		downloadBtn = new StatusButton(this, "downloadButton", QTStr("Download"), QSize(24, 24));
		connect(downloadBtn, &QPushButton::clicked, [&] {
			QPointer<QMenu> popup = new QMenu(this);
			auto AddAction = [this](QPointer<QMenu>& popup, const QString& name, int rate, const SharedVideoPtr& video) {
				QAction *item = new QAction(name, this);
				popup->addAction(item);
				connect(item, &QAction::triggered, [this, rate](bool pause) {
					QMetaObject::invokeMethod((QWidget*)App()->GetMainWindow(),
						"OnAppendDownloadVideo", Q_ARG(int, rate), Q_ARG(const SharedVideoPtr&, videoInfo));
				});
			};
			AddAction(popup, QTStr("SD"), VIDEO_RATE_SD, videoInfo);
			AddAction(popup, QTStr("HD"), VIDEO_RATE_HD, videoInfo);
			AddAction(popup, QTStr("BD"), VIDEO_RATE_BD, videoInfo);
			auto pos = downloadBtn->mapToGlobal(downloadBtn->pos());
			popup->exec(QPoint(pos.x() - 12, pos.y() - 12));
		});
		layout->addWidget(downloadBtn, 0, Qt::AlignCenter);
		break;
	case ACTION_DOWNLOADING:
		layout->addStretch(1);
		pauseBtn = new StatusButton(this, "pauseButton", QTStr("PauseDownload"), QTStr("StartDownload"), QSize(24, 24));
		connect(pauseBtn, &QPushButton::clicked, [this](bool pause) {
			videoInfo->SetDownloadStatus(pause ? DOWNLOAD_PAUSE : DOWNLOAD_RUN);
		});
		layout->addWidget(pauseBtn);
		retryBtn = new StatusButton(this, "retryButton", QTStr("RetryDownload"), QSize(24, 24));
		connect(retryBtn, &QPushButton::clicked, [this] {
			videoInfo->SetDownloadStatus(DOWNLOAD_RUN);
		});
		retryBtn->setVisible(false);
		layout->addWidget(retryBtn);
		deleteBtn = new StatusButton(this, "deleteButton", QTStr("DeleteDownload"), QSize(24, 24));
		connect(deleteBtn, &QPushButton::clicked, [this] {
			videoInfo->SetDownloadStatus(DOWNLOAD_CANCEL);
		});
		layout->addWidget(deleteBtn);
		layout->addStretch(1);
		break;
	case ACTION_LOCAL:
		layout->addStretch(1);
		appendBtn = new StatusButton(this, "appendButton", QTStr("LocalLoad"), QSize(24, 24));
		connect(appendBtn, &QPushButton::clicked, [this]() {
			QPointer<QMenu> popup = new QMenu(this);
			auto AddAction = [this](QPointer<QMenu>& popup, const QString& name, int opt, const SharedVideoPtr& video) {
				QAction *item = new QAction(name, this);
				popup->addAction(item);
				connect(item, &QAction::triggered, [this, opt](bool pause) {
					QMetaObject::invokeMethod((QWidget*)App()->GetMainWindow(),
						"OnPlaylistLocalVideo", Qt::QueuedConnection, Q_ARG(int, opt), Q_ARG(const SharedVideoPtr&, videoInfo));
				});
			};
			AddAction(popup, QTStr("LocalLoad"), 0, videoInfo);
			AddAction(popup, QTStr("LocalPlay"), 1, videoInfo);
			auto pos = appendBtn->mapToGlobal(appendBtn->pos());
			popup->exec(QPoint(pos.x() - 12, pos.y() - 12));
		});
		layout->addWidget(appendBtn);
		openBtn = new StatusButton(this, "openButton", QTStr("OpenDirectory"), QSize(24, 24));
		connect(openBtn, &QPushButton::clicked, [this]() {
			std::string path = videoInfo->filePath + "/" + videoInfo->vid;
			QDesktopServices::openUrl(QUrl::fromLocalFile(QT_UTF8(path.c_str())));
		});
		//connect(openBtn, &QPushButton::clicked,
		//	std::bind(&VideoActionWidget::SignalOpen, this, videoInfo));
		layout->addWidget(openBtn);
		deleteBtn = new StatusButton(this, "deleteButton", QTStr("DeleteFile"), QSize(24, 24));
		connect(deleteBtn, &QPushButton::clicked, [this]() {
			QMetaObject::invokeMethod((QWidget*)App()->GetMainWindow(),
				"OnRemoveLocalVideo", Qt::QueuedConnection, Q_ARG(const SharedVideoPtr&, videoInfo));
		});
		layout->addWidget(deleteBtn);
		layout->addStretch(1);
		break;
	}

	connect(video.get(), SIGNAL(SignalAttributeChanged(int, const SharedVideoPtr&)),
		this, SLOT(OnAttributeChanged(int, const SharedVideoPtr&)), Qt::QueuedConnection);
}

void VideoActionWidget::OnAttributeChanged(int attribute, const SharedVideoPtr& video)
{
	switch (attribute)
	{
	case ATTR_VIDEO_DOWNLOAD_STATUS:
		if (ACTION_DOWNLOADING != actionType) {
			break;
		}
		switch (video->downloadStatus)
		{
		case DOWNLOAD_OK:
			break;
		case DOWNLOAD_RUN:
			pauseBtn->setVisible(true);
			pauseBtn->setChecked(false);
			retryBtn->setVisible(false);
			break;
		case DOWNLOAD_PAUSE:
			pauseBtn->setVisible(true);
			pauseBtn->setChecked(true);
			retryBtn->setVisible(false);
			break;
		case DOWNLOAD_ERROR:
			pauseBtn->setVisible(false);
			pauseBtn->setChecked(false);
			retryBtn->setVisible(true);
			break;
		}
		break;
	}
}