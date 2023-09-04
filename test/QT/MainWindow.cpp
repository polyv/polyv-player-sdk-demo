#include "MainWindow.h"
#include "ui_MainWindow.h"

#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QDesktopWidget>
#include <QHeaderView>
#include <QKeyEvent>
#include <QScreen>
#include <http/http-param.h>

#include "SdkManager.h"
#include "Application.h"
#include "SettingDialog.h"
#include "InputDialog.h"
#include "VideoControl.h"
#include "TipsWidget.h"
#include "StatusButton.h"
#include "MyVideoList.h"
#include "DetectRecordingDialog.h"
#include "MsgBoxDialog.h"
#include "MultiPlayerDialog.h"
#include "MigrateDialog.h"

namespace {
	QString GetFileSizeStr(quint64 fileSize, int maxLevel = 4) // factor is 1024, maxLevel 4 is GB
	{
		double size = fileSize;
		QStringList sizeUnit =
			QStringList({ "B", "KB", "MB", "GB", "TB", "PB", "EB", "ZB", "YB" }).mid(0, maxLevel);
		double factor = 1024.0;
		QString fileSizeStr;
		for (auto& unit : sizeUnit.mid(0, maxLevel - 1)) {
			if (size < factor) {
				fileSizeStr = QString::asprintf("%3.2f %s", size, qUtf8Printable(unit));
				break;
			}
			size /= factor;
		}
		if (fileSizeStr.isEmpty())
			fileSizeStr = QString::asprintf("%.2f %s", size, qUtf8Printable(sizeUnit.last()));
		return fileSizeStr;
	}
}

MainWindow::MainWindow(QWidget *parent)
	: QMainWindow(parent)
	, ui(new Ui::MainWindow)
{
	ui->setupUi(this);
	Qt::WindowFlags f = windowFlags();
	f |= Qt::WindowCloseButtonHint;
	f |= Qt::WindowMinMaxButtonsHint;
	f |= Qt::WindowSystemMenuHint;
#ifdef _WIN32
	f |= Qt::FramelessWindowHint;
#endif
	setWindowFlags(f);
	setAttribute(Qt::WA_Mapped);
	
	myVideoList = new MyVideoList;
}

MainWindow::~MainWindow()
{
    delete ui;
}

bool MainWindow::Init(void)
{
	ui->titleBar->Init(this, TITLE_ALL_BTN);
	ui->titleBar->SetResizable(true);

	showVideo = new StatusButton(ui->titleBar, "showVideoButton", QTStr("ShowVideo"), QSize(24, 24));
	showVideo->setVisible(false);
	connect(showVideo, SIGNAL(clicked()), this, SLOT(OnShowListVideo()));
	QHBoxLayout* titleLayout = ui->titleBar->GetLeftLayout();
	titleLayout->insertWidget(0, showVideo);

	//returnVideo = new StatusButton(ui->titleBar, "returnVideoButton", QTStr("ReturnVideo"));
	//returnVideo->setVisible(false);
	//connect(returnVideo, SIGNAL(clicked()), this, SLOT(OnReturnListVideo()));
	//titleLayout->insertWidget(1, returnVideo);

	ui->myVideoButton->SetNormalIcon(QStringLiteral(":res/images/control/my_video_normal.svg"), 
		QStringLiteral(":res/images/control/my_video_active.svg"));
	ui->myVideoButton->SetCheckIcon(QStringLiteral(":res/images/control/my_video_checked.svg"),
		QStringLiteral(":res/images/control/my_video_active.svg"));

	ui->listVideoButton->SetNormalIcon(QStringLiteral(":res/images/control/list_video_normal.svg"),
		QStringLiteral(":res/images/control/list_video_active.svg"));
	ui->listVideoButton->SetCheckIcon(QStringLiteral(":res/images/control/list_video_checked.svg"),
		QStringLiteral(":res/images/control/list_video_active.svg"));

	ui->listVideoGroup->setId(ui->myVideoButton, 0);
	ui->listVideoGroup->setId(ui->listVideoButton, 1);
	connect(ui->listVideoGroup, static_cast<void(QButtonGroup::*)(int)>(&QButtonGroup::buttonClicked), this,
		[&](int id) {
		ui->listvideo->setCurrentIndex(id);
	});
	ui->myVideoButton->setChecked(true);

	ui->taskVideoGroup->setId(ui->downloadingButton, 0);
	ui->taskVideoGroup->setId(ui->localButton, 1);
	connect(ui->taskVideoGroup, static_cast<void(QButtonGroup::*)(int)>(&QButtonGroup::buttonClicked), this,
		[&](int id) {
		ui->taskvideo->setCurrentIndex(id);
	});
	ui->downloadingButton->setChecked(true);

	int col = 0;
	ui->myVideoTable->setColumnWidth(col++, 140);
	col++;
	ui->myVideoTable->setColumnWidth(col++, 100);
	ui->myVideoTable->setColumnWidth(col++, 100);
	ui->myVideoTable->setColumnWidth(col++, 120);
	col = 0;
	ui->myVideoTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
	ui->myVideoTable->horizontalHeader()->setSectionResizeMode(col++, QHeaderView::Fixed);
	ui->myVideoTable->horizontalHeader()->setSectionResizeMode(col++, QHeaderView::Stretch);
	ui->myVideoTable->horizontalHeader()->setSectionResizeMode(col++, QHeaderView::Fixed);
	ui->myVideoTable->horizontalHeader()->setSectionResizeMode(col++, QHeaderView::Fixed);
	ui->myVideoTable->horizontalHeader()->setSectionResizeMode(col++, QHeaderView::Fixed);
	ui->myvideo->InitTableWidget(ui->myVideoTable);

	col = 0;
	ui->downloadVideoTable->setColumnWidth(col++, 140);
	col++;
	ui->downloadVideoTable->setColumnWidth(col++, 100);
	ui->downloadVideoTable->setColumnWidth(col++, 250);
	ui->downloadVideoTable->setColumnWidth(col++, 120);
	col = 0;
	ui->downloadVideoTable->horizontalHeader()->setSectionResizeMode(col++, QHeaderView::Fixed);
	ui->downloadVideoTable->horizontalHeader()->setSectionResizeMode(col++, QHeaderView::Stretch);
	ui->downloadVideoTable->horizontalHeader()->setSectionResizeMode(col++, QHeaderView::Fixed);
	ui->downloadVideoTable->horizontalHeader()->setSectionResizeMode(col++, QHeaderView::Fixed);
	ui->downloadVideoTable->horizontalHeader()->setSectionResizeMode(col++, QHeaderView::Fixed);
	ui->downloadvideo->InitTableWidget(ui->downloadVideoTable);

	col = 0;
	ui->localVideoTable->setColumnWidth(col++, 140);
	col++;
	ui->localVideoTable->setColumnWidth(col++, 100);
	ui->localVideoTable->setColumnWidth(col++, 120);
	col = 0;
	ui->localVideoTable->horizontalHeader()->setSectionResizeMode(col++, QHeaderView::Fixed);
	ui->localVideoTable->horizontalHeader()->setSectionResizeMode(col++, QHeaderView::Stretch);
	ui->localVideoTable->horizontalHeader()->setSectionResizeMode(col++, QHeaderView::Fixed);
	ui->localVideoTable->horizontalHeader()->setSectionResizeMode(col++, QHeaderView::Fixed);
	ui->localvideo->InitTableWidget(ui->localVideoTable);

	QSize defSize(960, 540);
	ShowListVideo();
	ui->myVideoButton->click();
	ui->myVideoButton->setChecked(true);
	
	show();
	resize(defSize);
	QScreen* screen = QGuiApplication::screenAt(QCursor::pos());
	if (screen) {
		QRect rc = this->geometry();
		this->move(screen->geometry().left() + (screen->size().width() - rc.width()) / 2,
			screen->geometry().top() + (screen->size().height() - rc.height()) / 2);
	}	

	loadTipWidget = TipsWidget::ShowWithParam(ui->content, QTStr("LoadVideoInfo"), 
		TipsWidget::TIP_INFO, TipsWidget::TIP_POS_TOP, 0, false, false, true);
	myVideoList->RequestAll();

	SdkManager::GetManager()->SetSoftwareRecording();
	SdkManager::GetManager()->SetHardwareRecording();

	//resize(defSize);// multi screen, diff dpi
	return true;
}

QString MainWindow::GetToken(const QString& vid)
{
	return MyVideoList::GetTokenSafeBlock(myVideoList, vid);
	//return myVideoList->GetToken(vid);
}

bool MainWindow::IsExistLocalFile(const QString& vid)
{
	auto it = mapLocalVideos.begin();
	while (it != mapLocalVideos.end()) {
		QString videoId = QString::fromStdString(it.key());
		if (videoId.contains(vid)) {
			return true;
		}
		++it;
	}
	QDir dir(App()->GetLocalPath());
	QStringList filters;
	filters << "*.json";
	auto files = dir.entryList(filters, QDir::Files);
	for (auto& it : files) {
		if (it.contains(vid)) {
			return true;
		}
	}
	return false;
}

void MainWindow::closeEvent(QCloseEvent* e)
{
	if (MsgBoxDialog::MSGBOX_RESULT_OK != MsgBoxDialog::ConfirmOK(
		this, QTStr("AreYouSureToExit"), QTStr("OK"), QTStr("Cancel"), QString(), nullptr)) {
		e->ignore();
		return;
	}
	hide();
	CloseDialogs();
	MultiPlayerDialog::CloseAll();
	ui->player->Destroy();
	OnCloseLoadTip();
	if (myVideoList) {
		myVideoList->Stop();
		myVideoList->deleteLater();
	}
	QMainWindow::closeEvent(e);
}

void MainWindow::on_paramButton_clicked(void)
{
	ui->player->OpenParamWindow();
}

void MainWindow::on_settingButton_clicked(void)
{
	SettingDialog dlg(this);
	dlg.exec();
	ui->player->RefreshPlayer();
}

void MainWindow::on_refreshButton_clicked(void)
{
	myVideoList->RequestAll();
	OnShowTips(TipsWidget::TIP_INFO, tr("RefreshCurrentVideo"));
}

void MainWindow::on_vidButton_clicked(void)
{
	InputDialog dlg(InputDialog::INPUT_TYPE::VID, this);
	if (QDialog::Accepted == dlg.exec()) {
		auto vid = dlg.GetParam();
		auto item = mapMyVideos.value(QT_TO_UTF8(vid));
		if (item) {
			StartItemHighlight(item->video->vid);
		}
		else {
			myVideoList->RequestVid(vid);
		}
	}
}
void MainWindow::on_migrateButton_clicked(void)
{
	MigrateDialog dlg(this);
	dlg.exec();
}
//void MainWindow::on_stackedWidget_currentChanged(int index)
//{
//	ui->player->setVisible(1 == index ? true : false);
//}

void MainWindow::OnShowListVideo(void)
{
	ui->player->Stop();
	ShowListVideo();
}

//void MainWindow::OnReturnListVideo(void)
//{
//	ShowListVideo();
//}

void MainWindow::OnShowTips(int level, const QString& msg)
{
	TipsWidget::ShowLevel(ui->content,msg, (TipsWidget::TipType)level);
}

void MainWindow::OnShowToast(const QString& msg)
{
	TipsWidget::ShowToast(ui->content, msg);
}

void MainWindow::OnPlayVideo(bool local, const SharedVideoPtr& video, int rate)
{
	if (ui->player->IsLoading()) {
		OnShowTips(TipsWidget::TIP_WARN, QTStr("PlayerLoading"));
		return;
	}
	ShowPlayer();
	QString token;
	if (!local && 1 == video->seed) {
		token = GetToken(QT_UTF8(video->vid.c_str()));
	}
	int seek = App()->GlobalConfig().Get("Video", "VideoPlaySeek").toInt();
	if (!ui->player->Play(local, token, video, rate, seek * 1000)) {
		OnShowTips(TipsWidget::TIP_ERROR, QTStr("PlayerError"));
		ShowListVideo();
		return;
	}
	ui->titleBar->SetTitleName(QT_UTF8(video->title.c_str()));
}

void MainWindow::OnChangeRatePlayVideo(int rate, int seekMillisecond, const SharedVideoPtr& video)
{
	QString token;
	if (1 == video->seed) {
		token = GetToken(QT_UTF8(video->vid.c_str()));
		if (token.isEmpty()) {
			slog_error("request token error.");
			OnShowTips(TipsWidget::TIP_ERROR, QTStr("PlayerTokenError"));
			return;
		}
	}
//#ifdef _WIN32
	ui->player->OnlineRePlay(rate, seekMillisecond, token, video);
//#else
//	// in macos must wait player real stop media, (will stuck)
//	QTimer::singleShot(200, this, [this, rate, seekMillisecond, token, video] {
//		ui->player->OnlineRePlay(rate, seekMillisecond, token, video);
//	});
//#endif
	
}
//void MainWindow::OnOpenParamWindow(QString vid)
//{
//	(void)vid;
//	on_paramButton_clicked();
//}
void MainWindow::OnCloseLoadTip(void)
{
	if (loadTipWidget) {
		loadTipWidget->close();
		loadTipWidget = nullptr;
	}
}

void MainWindow::OnHighlightItem(void)
{
	auto it = mapFlicker.begin();
	while (it != mapFlicker.end()) {
		auto item = it.value();
		if (item->count > 10) {
			SelectItem(item->item->row(), false);
			it = mapFlicker.erase(it); 
		}
		else {
			auto select = item->item->isSelected();
			SelectItem(item->item->row(), !item->item->isSelected());
			item->count++;
			++it;
		}
	}
	if (mapFlicker.isEmpty()) {
		StopItemHighlight();
	}
}

void MainWindow::OnDetectSoftwareRecording(int type, QString software)
{
	if (!detectRecordingDialog) {
		detectRecordingDialog = new DetectRecordingDialog(this);
		detectRecordingDialog->hide();
	}
	detectRecordingDialog->DetectSoftwareRecording(type, software);	
}

void MainWindow::OnDetectHardwareRecording(int type, QString device)
{
	if (!detectRecordingDialog) {
		detectRecordingDialog = new DetectRecordingDialog(this);
		detectRecordingDialog->hide();
	}
	detectRecordingDialog->DetectHardwareRecording(type, device);
}

void MainWindow::OnEnableWindow(bool enable)
{
	if (!enable) {
		OnShowListVideo();
	}
	ui->content->setEnabled(enable);
}

void MainWindow::OnCompleteRequestVideo(void)
{
	OnCloseLoadTip();
	QDir dir(App()->GetLocalPath());
	QStringList filters;
	filters << "*.json";
	auto files = dir.entryList(filters, QDir::Files);
	qDebug() << files;
	for (auto & it : files) {
		bool removeFile = false;
		QString fileName = QString("%1/%2").arg(App()->GetLocalPath()).arg(it);
		do
		{
			QFile file(fileName);
			if (!file.open(QFile::ReadOnly)) {
				break;
			}
			QJsonParseError err;
			QJsonDocument jsonDocument = QJsonDocument::fromJson(
				file.readAll(), &err);
			file.close();
			if (err.error != QJsonParseError::NoError) {
				removeFile = true;				
				continue;
			}
			if (!jsonDocument.isObject()) {
				removeFile = true;
				break;
			}
			QJsonObject item = jsonDocument.object();
			QString vid = item["vid"].toString();
			QString path = item["path"].toString();
			int rate = item["rate"].toInt();
			if (!SdkManager::GetManager()->CheckFileComplete(vid, path, rate)) {
				break;
			}
			SharedVideoPtr videoInfo;
			auto video = mapMyVideos.value(vid.toStdString());
			if (!video) {
				QString videoUrl = item["mp4"].toString();
				videoInfo = std::make_shared<VideoInfo>();
				videoInfo->vid = vid.toStdString();
				videoInfo->filePath = path.toStdString();
				videoInfo->videoUrl = videoUrl.toStdString();
				videoInfo->imageUrl = QT_TO_UTF8(item["first_image"].toString());
				videoInfo->title = QT_TO_UTF8(item["title"].toString());
				videoInfo->duration = QT_TO_UTF8(item["duration"].toString());
				videoInfo->seed = item["seed"].toInt();
				videoInfo->rateCount = item["df"].toInt();
				videoInfo->size = (long long)item["source_filesize"].toDouble();
			}
			else {
				videoInfo = video->video;
				videoInfo->filePath = path.toStdString();
			}
			InsertLocalItem(rate, videoInfo);
		} while (false);
		if (removeFile) {
			QFile::remove(fileName);
		}
	}
}

void MainWindow::OnAppendMyVideo(const SharedVideoPtr& video, bool focus)
{
	//auto testtime = HttpParam::GetTimestamp(false);
	//qDebug() << "the begin time:" << testtime;
	if (InsertMyVideoItem(video, focus)) {
		ui->myVideoButton->click();
		ui->myVideoButton->setChecked(true);
	}
	//qDebug() << "the end time:" << HttpParam::GetTimestamp(false) - testtime;
}
void MainWindow::OnAppendDownloadVideo(int rate, const SharedVideoPtr& video)
{
	if (InsertDownloadItem(rate, video)) {
		ui->listVideoButton->click();
		ui->listVideoButton->setChecked(true);
		ui->downloadingButton->click();
		ui->downloadingButton->setChecked(true);
	}
}
void MainWindow::OnRemoveDownloadVideo(int rate, const SharedVideoPtr& video)
{
	QString vid = CreateVid(rate, video);
	auto item = mapDownloadVideos.take(vid.toStdString());
	if (!item) {
		return;
	}
	ui->downloadVideoTable->removeRow(item->item->row());
	ui->downloadvideo->SetTableEmpty(mapDownloadVideos.isEmpty());
}
void MainWindow::OnAppendLocalVideo(int rate, const SharedVideoPtr& video)
{
	QString vid = CreateVid(rate, video);
	auto item = mapDownloadVideos.take(vid.toStdString());
	if (item) {
		ui->downloadVideoTable->removeRow(item->item->row());
		ui->downloadvideo->SetTableEmpty(mapDownloadVideos.isEmpty());
	}
	if (InsertLocalItem(rate, video)) {
		ui->listVideoButton->click();
		ui->listVideoButton->setChecked(true);
		ui->localButton->click();
		ui->localButton->setChecked(true);
	}
}

void MainWindow::OnRemoveLocalVideo(int rate, const SharedVideoPtr& video)
{
	QString vid = CreateVid(rate, video);
	auto item = mapLocalVideos.take(vid.toStdString());
	if (!item) {
		return;
	}
	ui->localVideoTable->removeRow(item->item->row());
	ui->localvideo->SetTableEmpty(mapLocalVideos.isEmpty());

	if (VIDEO_RATE_AUTO == rate) {
		for (int i = VIDEO_RATE_LD; i <= VIDEO_RATE_HD; ++i) {
			SdkManager::GetManager()->DeleteLocalVideoFile(QString::fromStdString(video->vid), QString::fromStdString(video->filePath), i);
		}
	}
	else {
		SdkManager::GetManager()->DeleteLocalVideoFile(QString::fromStdString(video->vid), QString::fromStdString(video->filePath), rate);
	}	
	QString localPath = App()->GetLocalPath();
	QString fileName = QString("%1/%2_%3.json").arg(localPath).arg(video->vid.c_str()).arg(rate);
	QFile::remove(fileName);

	//localPath = QString("%1/%2").arg(QT_UTF8(video->filePath.c_str())).
	//	arg(QT_UTF8(video->vid.c_str()));
	//QDir dir(localPath);
	//if (!dir.exists()) {
	//	return;
	//}
	//QStringList filters;
	//filters << "*.mp4" << "*.ts" << "*.key" << "*.m3u8";
	//auto files = dir.entryList(filters, QDir::Files);
	//qDebug() << files;
	//std::string videoPoolId = video->vid.substr(0, video->vid.length() - 2);
	//fileName = QT_UTF8((videoPoolId + "_" + std::to_string((long long)video->rate)).c_str());
	//int fileCount = 0;
	//for (auto & it : files) {
	//	QString filePath = QString("%1/%2").arg(localPath).arg(it);
	//	if (filePath.contains(fileName)) {
	//		QFile::remove(filePath);
	//		fileCount++;
	//	}
	//}
	//if (fileCount == files.size()) {
	//	dir.rmdir(localPath);
	//}
}

void MainWindow::OnPlaylistLocalVideo(const SharedVideoPtr& video)
{
	MultiPlayerDialog::Open(video);
}

void MainWindow::OnFullScreen(void)
{
	ui->titleBar->ShowFullScreen(true);
}

void MainWindow::OnExitFullScreen(void)
{
	ui->titleBar->ShowFullScreen(false);
}

void MainWindow::ShowPlayer(void)
{
#ifdef _WIN32
	ui->titleBar->SetResizeFrameColor(QColor(12, 12, 12));
#endif
	ui->player->setVisible(true);
	ui->list->setVisible(false);

	ui->titleBar->SetLogoable(false, QSize(188, 20));
	ui->titleBar->SetTitleable(true);
	showVideo->setVisible(true);
	//returnVideo->setVisible(true);
}
void MainWindow::ShowListVideo(void)
{
#ifdef _WIN32
	ui->titleBar->SetResizeFrameColor(QColor(237, 241, 247));
#endif
	ui->player->setVisible(false);
	ui->list->setVisible(true);

	ui->titleBar->SetLogoable(true, QSize(188, 20));
	ui->titleBar->SetTitleable(false);
	showVideo->setVisible(false);
	//returnVideo->setVisible(false);
}

bool MainWindow::InsertMyVideoItem(const SharedVideoPtr& video, bool focus)
{
	bool result = false;
	do
	{
		if (!video) {
			break;
		}
		result = true;
		auto item = mapMyVideos.value(video->vid);
		if (item) {
			ui->myVideoTable->scrollToItem(item->item);
			if (item->video->SetTitle(video->title, video->rate, true)) {
				item->video->SetImage(video->imageUrl, video->rate);
				//QString text = QString("%1\nVID:%2").arg(QT_UTF8(video->title.c_str())).arg(QT_UTF8(video->vid.c_str()));
				//QTableWidgetItem* tableItem = ui->myVideoTable->item(item->item->row(), 1);
				//tableItem->setText(text);
				UpdateItemTitle(ui->downloadVideoTable, mapDownloadVideos, video->vid, video->title);
				UpdateItemTitle(ui->localVideoTable, mapLocalVideos, video->vid, video->title);
			}
			if (focus) { 
				StartItemHighlight(video->vid);
			}	
			break;
		}
		item = std::make_shared<Item>();
		item->video = video;
		int row = (focus ? 0 : ui->myVideoTable->rowCount());
		ui->myVideoTable->insertRow(row);
		int col = 0;
		item->item = new QTableWidgetItem;
		ui->myVideoTable->setItem(row, col, item->item);
		auto cover = new VideoCoverWidget(ui->myVideoTable, false, video, video->rate);
		ui->myVideoTable->setCellWidget(row, col++, cover);
		auto title = new VideoTitleWidget(ui->myVideoTable, true, video);
		//QString text = QString("%1\nVID:%2").arg(QT_UTF8(video->title.c_str())).arg(QT_UTF8(video->vid.c_str()));
		//QString text = QString("<span style='font-size:14px;text-align:left;color:#668196'>%1</span><br>\
		//	<span style='font-size:12px;color:#668196;text-align:left'>VID:%2</span>").arg(QT_UTF8(video->title.c_str())).arg(QT_UTF8(video->vid.c_str()));
		//QTableWidgetItem* tableItem = new QTableWidgetItem;
		//ui->myVideoTable->setItem(row, col++, tableItem);
		ui->myVideoTable->setCellWidget(row, col++, title);

		auto tableItem = new QTableWidgetItem(QT_UTF8(video->duration.c_str()));
		tableItem->setTextAlignment(Qt::AlignCenter);
		ui->myVideoTable->setItem(row, col++, tableItem);

		QString sizeText = GetFileSizeStr(video->size);
		//if (video->sizes.empty()) {
		//	double size = video->size / (1024 * 1024);
		//	sizeText = QString("%1MB").arg(QString::number(size, 'f', 2));
		//}
		//else {
		//	for (size_t i = 0; i < video->sizes.size(); ++i) {
		//		double size = video->sizes[i] / (1024 * 1024);
		//		sizeText += (sizeText.isEmpty() ?
		//			QString("%1MB").arg(QString::number(size, 'f', 2)) :
		//			QString(",%1MB").arg(QString::number(size, 'f', 2)));
		//	}
		//}	
		tableItem = new QTableWidgetItem(sizeText);
		//if (!video->sizes.empty()) {
		//	tableItem->setToolTip(sizeText);
		//}
		tableItem->setTextAlignment(Qt::AlignCenter);
		ui->myVideoTable->setItem(row, col++, tableItem);
		auto action = new VideoActionWidget(ui->myVideoTable,
			VideoActionWidget::ACTION_DOWNLOAD, video, 0);
		//connect(action, SIGNAL(SignalDownload(int, const SharedVideoPtr&)),
		//	this, SLOT(OnDownloadVideo(int, const SharedVideoPtr&)));
		ui->myVideoTable->setCellWidget(row, col++, action);

		mapMyVideos.insert(video->vid, item);

		video->DownloadImage(video->rate);

		if (focus) {
			StartItemHighlight(video->vid);
		}
	} while (false);
	
	ui->myvideo->SetTableEmpty(mapMyVideos.isEmpty());

	return true;
}

bool MainWindow::InsertDownloadItem(int rate, const SharedVideoPtr& video)
{
	QString vid = CreateVid(rate, video);
	bool result = false;
	do
	{
		if (!video) {
			break;
		}
		auto item = mapLocalVideos.value(vid.toStdString());
		if (item) {
			if (item->setVideoRate.contains(rate)) {
				TipsWidget::ShowToast(ui->myvideo, QTStr("AlreadyDownloaded"));
				break;
			}
		}	
		item = mapDownloadVideos.value(vid.toStdString());
		if (item) {
			result = true;
			TipsWidget::ShowToast(ui->myvideo, QTStr("AlreadyAppendDownloading"));
			break;
		}
		int count = App()->GlobalConfig().Get("Download", "TaskCount").toInt();
		if (mapDownloadVideos.count() >= count) {
			TipsWidget::ShowLevel(ui->content, QTStr("DownloadedUpperLimit"), TipsWidget::TIP_WARN);
			break;
		}
		result = true;
		item = std::make_shared<Item>();
		item->video = video;
		item->setVideoRate.insert(rate);
		int row = 0;// ui->docTable->rowCount();
		ui->downloadVideoTable->insertRow(row);
		int col = 0;
		item->item = new QTableWidgetItem;
		ui->downloadVideoTable->setItem(row, col, item->item);
		ui->downloadVideoTable->setCellWidget(row, col++, new VideoCoverWidget(ui->downloadVideoTable, false, video, rate));
		QTableWidgetItem* tableItem = new QTableWidgetItem(QT_UTF8(video->title.c_str()));
		//tableItem->setTextAlignment(Qt::AlignCenter);
		ui->downloadVideoTable->setItem(row, col++, tableItem);
		double size = video->size;
		if (0 != rate && rate <= video->sizes.size()) {
			size = video->sizes.at(rate - 1);
		}
		tableItem = new QTableWidgetItem(GetFileSizeStr(size));
		tableItem->setTextAlignment(Qt::AlignCenter);
		ui->downloadVideoTable->setItem(row, col++, tableItem);

		auto status = new VideoDownloadWidget(ui->downloadVideoTable, video, rate);
		ui->downloadVideoTable->setCellWidget(row, col++, status);

		auto action = new VideoActionWidget(ui->downloadVideoTable,
			VideoActionWidget::ACTION_DOWNLOADING, video, rate);
		ui->downloadVideoTable->setCellWidget(row, col++, action);

		mapDownloadVideos.insert(vid.toStdString(), item);

	} while (false);

	ui->downloadvideo->SetTableEmpty(mapDownloadVideos.isEmpty());
	return result;
}

bool MainWindow::InsertLocalItem(int rate, const SharedVideoPtr& video)
{
	bool result = false;
	QString vid = CreateVid(rate, video);
	do
	{
		if (!video) {
			break;
		}
		result = true;
		auto item = mapLocalVideos.value(vid.toStdString());
		if (item) {
			ui->localVideoTable->scrollToItem(item->item);
			item->setVideoRate.insert(rate);
			break;
		}
		do
		{
			QJsonObject obj;
			QString localPath = App()->GetLocalPath();
			QString fileName = QString("%1/%2_%3.json").arg(localPath).arg(video->vid.c_str()).arg(rate);
			if (QFile::exists(fileName)) {
				QFile file(fileName);
				if (!file.open(QFile::ReadOnly)) {
					break;
				}
				QJsonParseError err;
				QJsonDocument jsonDocument = QJsonDocument::fromJson(
					file.readAll(), &err);
				file.close();
				if (err.error != QJsonParseError::NoError) {
					break;
				}
				if (!jsonDocument.isObject()) {
					break;
				}
				obj = jsonDocument.object();
			}
			else {
				obj["vid"] = QT_UTF8(video->vid.c_str());		
			}
			obj["rate"] = rate;
			if (!video->filePath.empty()) {
				obj["path"] = QT_UTF8(video->filePath.c_str());
			}
			obj["mp4"] = QString::fromStdString(video->videoUrl);
			obj["first_image"] = QString::fromStdString(video->imageUrl);
			obj["title"] = QString::fromStdString(video->title);
			obj["duration"] = QString::fromStdString(video->duration);
			obj["seed"] = video->seed;
			obj["df"] = video->rateCount;
			obj["source_filesize"] = (double)video->size;
			QFile file(fileName);
			if (file.open(QFile::WriteOnly | QFile::Truncate)) {
				QJsonDocument json(obj);
				file.write(json.toJson());
				file.close();
			}
		} while (false);
		item = std::make_shared<Item>();
		item->video = video;
		item->video->rate = rate;
		item->setVideoRate.insert(rate);
		int row = 0;// ui->docTable->rowCount();
		ui->localVideoTable->insertRow(row);
		int col = 0;
		item->item = new QTableWidgetItem;
		ui->localVideoTable->setItem(row, col, item->item);
		ui->localVideoTable->setCellWidget(row, col++, new VideoCoverWidget(ui->localVideoTable, true, video, rate));
		QTableWidgetItem* tableItem = new QTableWidgetItem(QT_UTF8(video->title.c_str()));
		//tableItem->setTextAlignment(Qt::AlignCenter);
		ui->localVideoTable->setItem(row, col++, tableItem);

		double size = video->size;
		if (0 != rate && rate <= video->sizes.size()) {
			size = video->sizes.at(rate - 1);
		}
		tableItem = new QTableWidgetItem(GetFileSizeStr(size));
		tableItem->setTextAlignment(Qt::AlignCenter);
		ui->localVideoTable->setItem(row, col++, tableItem);

		auto action = new VideoActionWidget(ui->localVideoTable,
			VideoActionWidget::ACTION_LOCAL, video, rate);
		ui->localVideoTable->setCellWidget(row, col++, action);
		mapLocalVideos.insert(vid.toStdString(), item);
	} while (false);

	ui->localvideo->SetTableEmpty(mapLocalVideos.isEmpty());

	return result;
}

void MainWindow::UpdateItemTitle(QTableWidget* table, const QMap<std::string, SharedItemPtr>& map,
	const std::string& vid, const std::string& title)
{
	for (int i = VIDEO_RATE_LD; i <= VIDEO_RATE_HD; ++i) {
		auto videoId = CreateVid(i, vid);
		auto item = map.value(vid);
		if (!item) {
			continue;
		}
		QTableWidgetItem* tableItem = table->item(item->item->row(), 1);
		if (tableItem) {
			tableItem->setText(QT_UTF8(title.c_str()));
		}
	}
}

void MainWindow::StartItemHighlight(const std::string& vid)
{
	if (mapFlicker.contains(vid)) {
		return;
	}
	auto item = mapMyVideos.value(vid);
	if (item && item->item) {
		auto flickerItem = std::make_shared<FlickerItem>();
		flickerItem->count = 0;
		flickerItem->item = item->item;
		mapFlicker.insert(vid, flickerItem);
		ui->myVideoTable->scrollTo(ui->myVideoTable->model()->index(item->item->row(), 0));
		if (!flickerTimer) {
			flickerTimer = new QTimer(this);
			connect(flickerTimer, SIGNAL(timeout()), this, SLOT(OnHighlightItem()));
		}
		flickerTimer->start(200);
	}
}

void MainWindow::StopItemHighlight(void)
{
	if (flickerTimer) {
		flickerTimer->stop();
	}
	for (auto& it : mapFlicker) {
		SelectItem(it->item->row(), false);
	}
	mapFlicker.clear();
}
void MainWindow::SelectItem(int row, bool select)
{
	auto selectionModel = ui->myVideoTable->selectionModel();
	auto mode = ui->myVideoTable->model();
	auto index = mode->index(row, 0);
	auto& topLeft = index;
	auto bottomRight = mode->index(row, mode->columnCount() - 1);
	QItemSelection selection(topLeft, bottomRight);
	selectionModel->select(selection, select ? QItemSelectionModel::Select : QItemSelectionModel::Deselect);
}

void MainWindow::CloseDialogs()
{
	QList<QDialog*> childDialogs = this->findChildren<QDialog*>();
	if (!childDialogs.isEmpty()) {
		for (int i = 0; i < childDialogs.size(); ++i) {
			childDialogs.at(i)->done(0);
		}
	}
}
QString MainWindow::CreateVid(int rate, const SharedVideoPtr& video)
{
	return CreateVid(rate, video->vid);
}
QString MainWindow::CreateVid(int rate, const std::string& vid)
{
	return QString("%1:%2").arg(vid.c_str()).arg(rate);
}

void MainWindow::TestMyVideoItem(void)
{
	auto item = std::make_shared<VideoInfo>();

	auto timestamp = HttpParam::GetTimestamp(false);

	item->vid = QT_TO_UTF8(QString("%1").arg(timestamp));
	item->title = "test video";
	item->size = 323;
	item->duration = "12:00:00";

	InsertMyVideoItem(item, false);
}
