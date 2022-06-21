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
#include <http/http-param.h>

#include "SdkManager.h"
#include "Application.h"
#include "ParamDialog.h"
#include "SettingDialog.h"
#include "VidDialog.h"
#include "VideoControl.h"
#include "TipsWidget.h"
#include "StatusButton.h"
#include "MyVideoList.h"
#include "DeviceWarnDialog.h"
#include "MsgBoxDialog.h"


MainWindow::MainWindow(QWidget *parent)
	: QWidget(parent)
	, ui(new Ui::MainWindow)
{
	ui->setupUi(this);
	Qt::WindowFlags f;
	f |= Qt::Dialog;
	f |= Qt::WindowCloseButtonHint;
	f |= Qt::WindowMinMaxButtonsHint;
	f |= Qt::CustomizeWindowHint;
	f |= Qt::WindowSystemMenuHint;
#ifdef _WIN32
	f |= Qt::FramelessWindowHint;
#endif
	setWindowFlags(f);
	setAttribute(Qt::WA_Mapped);
	setAttribute(Qt::WA_DeleteOnClose);
	
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

	ui->player->SetMainWindow(this);

	showVideo = new StatusButton(ui->titleBar, "showVideoButton", QTStr("ShowVideo"), QSize(24, 24));
	showVideo->setVisible(false);
	connect(showVideo, SIGNAL(clicked()), this, SLOT(OnShowListVideo()));
	QHBoxLayout* titleLayout = ui->titleBar->GetLeftLayout();
	titleLayout->insertWidget(0, showVideo);

	returnVideo = new StatusButton(ui->titleBar, "returnVideoButton", QTStr("ReturnVideo"));
	returnVideo->setVisible(false);
	connect(returnVideo, SIGNAL(clicked()), this, SLOT(OnReturnListVideo()));
	titleLayout->insertWidget(1, returnVideo);


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
	connect(ui->listVideoGroup, static_cast<void(QButtonGroup::*)(int)>(&QButtonGroup::buttonClicked),
		[&](int id) {
		ui->listvideo->setCurrentIndex(id);
	});
	ui->myVideoButton->setChecked(true);

	ui->taskVideoGroup->setId(ui->downloadingButton, 0);
	ui->taskVideoGroup->setId(ui->localButton, 1);
	connect(ui->taskVideoGroup, static_cast<void(QButtonGroup::*)(int)>(&QButtonGroup::buttonClicked),
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

	ShowListVideo();
	show();

	loadTipWidget = TipsWidget::ShowWithParam(ui->content, QTStr("LoadVideoInfo"), 
		TipsWidget::TIP_INFO, TipsWidget::TIP_POS_TOP, 0, false, false, true);
	myVideoList->RequestAll();

#ifdef _WIN32
	OnUpdateSoftwareRecord(App()->GlobalConfig().Get("Video", "SoftwareRecord").toBool());
	OnUpdateHdmiRecord(App()->GlobalConfig().Get("Video", "HdmiCallback").toBool());
#endif// _WIN32
	return true;
}

void MainWindow::keyPressEvent(QKeyEvent* e)
{
	if (Qt::Key_Escape == e->key() && isFullscreen) {
		OnExitFullScreen();
	}
	QWidget::keyPressEvent(e);	
}

void MainWindow::resizeEvent(QResizeEvent *e)
{
	QWidget::resizeEvent(e);
}

void MainWindow::closeEvent(QCloseEvent* e)
{
	hide();
	OnCloseLoadTip();
	ui->player->Destroy();
	QWidget::closeEvent(e);
}

void MainWindow::on_paramButton_clicked(void)
{
	if (paramDialog) {
		paramDialog->show();
		return;
	}
	paramDialog = new ParamDialog(this);
	connect(ui->player, SIGNAL(SignalPropChange(int, const QString&)), 
		paramDialog, SLOT(OnPropChange(int, const QString&)));
	connect(ui->player, SIGNAL(SignalPropReset()), 
		paramDialog, SLOT(OnPropReset()));
	connect(paramDialog, &ParamDialog::finished, [&](int) {
		paramDialog = nullptr;
	});
	auto props = ui->player->GetProps();
	for (auto &it : props) {
		paramDialog->SetPropValue(it.property, it.value);
	}
	paramDialog->show();
}

void MainWindow::on_settingButton_clicked(void)
{
	SettingDialog* dlg = new SettingDialog(this);
	if (QDialog::Accepted == dlg->exec()) {
		ui->player->UpdateCache();
	}
}

void MainWindow::on_refreshButton_clicked(void)
{
	myVideoList->RequestAll();
}

void MainWindow::on_vidButton_clicked(void)
{
	VidDialog* dlg = new VidDialog(this);
	if (QDialog::Accepted == dlg->exec()) {
		auto vid = dlg->GetVID();
		auto item = mapMyVideos.value(QT_TO_UTF8(vid));
		if (item) {
			StartItemHighlight(item->video->vid);
		}
		else {
			myVideoList->RequestVid(vid);
		}
	}
	dlg->deleteLater();
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

void MainWindow::OnReturnListVideo(void)
{
	ShowListVideo();
}

void MainWindow::OnShowTips(int level, const QString& msg)
{
	TipsWidget::ShowLevel(ui->content,msg, (TipsWidget::TipType)level);
}

void MainWindow::OnShowToast(const QString& msg)
{
	TipsWidget::ShowToast(ui->content, msg);
}

void MainWindow::OnPlayVideo(bool local, const SharedVideoPtr& video)
{
	QString token;
	if (!local && 1 == video->seed) {
		token = myVideoList->GetToken(QT_UTF8(video->vid.c_str()));
	}
	if (!ui->player->Play(local, token, video, 0)) {
		OnShowTips(TipsWidget::TIP_ERROR, QTStr("PlayerError"));
		return;
	}
	ui->titleBar->SetTitleName(QT_UTF8(video->title.c_str()));
	ShowPlayer();
}

void MainWindow::OnChangeRatePlayVideo(int rate, int seekMillisecond, const SharedVideoPtr& video)
{
	QString token;
	if (1 == video->seed) {
		token = myVideoList->GetToken(QT_UTF8(video->vid.c_str()));
	}
	ui->player->Stop();
	if (ui->player->RePlay(rate, seekMillisecond, token, video)) {
		return;
	}
	ui->player->Play(false, token, video, seekMillisecond);
}

void MainWindow::OnCloseLoadTip(void)
{
	if (loadTipWidget) {
		loadTipWidget->close();
		loadTipWidget->deleteLater();
		loadTipWidget = nullptr;
	}
}

void MainWindow::OnHighlightItem(void)
{
	auto item = mapMyVideos.value(curHighlightVid);
	if (item) {
		auto select = ui->myVideoTable->item(item->item->row(), 1)->isSelected();//  ui->myVideoTable->isItemSelected(ui->myVideoTable->item(item->item->row(), 1));
		ui->myVideoTable->item(item->item->row(), 1)->setSelected(!select);// ui->myVideoTable->setItemSelected(ui->myVideoTable->item(item->item->row(), 1), !select);

		curHighlight++;
	}
	if (curHighlight >= 10) {
		StopItemHighlight();
	}
}

#ifdef _WIN32
void MainWindow::OnHDMIDevice(QString id, int type)
{
	static DeviceWarnDialog* warnDialog = nullptr;
	switch (type)
	{
	case HDMI_DEVICE_NONE:
		mapDisables.clear();

		break;
	case HDMI_DEVICE_USE: {
		if (!mapDisables.contains(id)) {
			if (warnDialog) {
				warnDialog->activateWindow();
				warnDialog->show();
				return;
			}
			warnDialog = new DeviceWarnDialog(id, this);
			connect(warnDialog, &DeviceWarnDialog::finished, [&](int) {
				warnDialog = nullptr;
			});
			warnDialog->show();
			return;
		}
	}
						break;
	case HDMI_DEVICE_UNUSE:
		mapDisables.remove(id);
		break;
	}
	if (mapDisables.isEmpty()) {
		EnableWindow(true);

		if (warnDialog) {
			warnDialog->close();
			warnDialog = nullptr;
		}
	}
}

void MainWindow::OnPluginInject(void)
{
	if (MsgBoxDialog::MSGBOX_RESULT_OK == MsgBoxDialog::ConfirmOK(
		this, QTStr("HasSoftwareInjectRecording"), QTStr("Disable"), QTStr("Ignore"), QString(), nullptr)) {
		EnableWindow(false);
	}
}

void MainWindow::OnUpdateSoftwareRecord(bool enable)
{
	if (E_NO_ERR != PLVSetPreventRecord((void*)this->winId(), enable)) {
		slog_error("set prevent record error");
	}

	PLVSetPluginInjectHandler(enable, [](void* data) {
		MainWindow* obj = (MainWindow*)data;
		QMetaObject::invokeMethod(obj, "OnPluginInject", Qt::QueuedConnection);
	}, this);	
}
void MainWindow::OnUpdateHdmiRecord(bool enable)
{
	PLVSetHDMIDeviceChangedHandler(enable, [](HDMI_DEVICE_TYPE type, const char* device, void* data) {
		MainWindow* obj = (MainWindow*)data;
		QMetaObject::invokeMethod(obj, "OnHDMIDevice", Qt::QueuedConnection,
			Q_ARG(QString, device), Q_ARG(int, type));
	}, this);
}
#endif // _WIN32



void MainWindow::OnEnableWindow(const QString& id, bool enable)
{
	if (!enable) {
		mapDisables[id] = enable;
		EnableWindow(enable);
		return;
	}
	mapDisables.remove(id);
	if (enable && mapDisables.isEmpty()) {
		EnableWindow(enable);
	}
}

void MainWindow::OnEnableWindows()
{
	mapDisables.clear();
	EnableWindow(true);
}



void MainWindow::EnableWindow(bool enable)
{
	if (isEnableWindow == enable) {
		return;
	}
	if (!enable) {
		OnShowListVideo();
	}
	isEnableWindow = enable;
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
		bool result = false;
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
				continue;
			}
			if (!jsonDocument.isObject()) {
				break;
			}
			QJsonObject item = jsonDocument.object();
			std::string vid = QT_TO_UTF8(item["vid"].toString());
			std::string path = QT_TO_UTF8(item["path"].toString());
			int rate = item["rate"].toInt();
			auto video = mapMyVideos.value(vid);
			if (!video) {
				break;
			}
			if (!PLVCheckFileComplete(vid.c_str(), path.c_str(), rate)) {
				break;
			}
			video->video->filePath = path;
			InsertLocalItem(rate, video->video);
			result = true;
		} while (false);
		if (!result) {
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
void MainWindow::OnRemoveDownloadVideo(const SharedVideoPtr& video)
{
	auto item = mapDownloadVideos.take(video->vid);
	if (!item) {
		return;
	}
	ui->downloadVideoTable->removeRow(item->item->row());
	ui->downloadvideo->SetTableEmpty(mapDownloadVideos.isEmpty());
}
void MainWindow::OnAppendLocalVideo(int rate, const SharedVideoPtr& video)
{
	auto item = mapDownloadVideos.take(video->vid);
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

void MainWindow::OnRemoveLocalVideo(const SharedVideoPtr& video)
{
	auto item = mapLocalVideos.take(video->vid);
	if (!item) {
		return;
	}
	ui->localVideoTable->removeRow(item->item->row());
	ui->localvideo->SetTableEmpty(mapLocalVideos.isEmpty());

	auto downloadObj = PLVDownloadCreate();
	for (int i = VIDEO_RATE_SD; i <= VIDEO_RATE_BD; ++i) {
		PLVDownloadSetVideo(downloadObj, video->vid.c_str(), video->filePath.c_str(), i);
		PLVDownloadDelete(downloadObj);
	}
	PLVDownloadDestroy(downloadObj);

	QString localPath = App()->GetLocalPath();
	QString fileName = QString("%1/%2.json").arg(localPath).arg(video->vid.c_str());
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

void MainWindow::OnPlaylistLocalVideo(int opt, const SharedVideoPtr& video)
{
	if (ui->player->LoadLocal(opt, video)) {
		ShowPlayer();
		ui->titleBar->SetTitleName(QT_UTF8(video->title.c_str()));
	}
}

void MainWindow::OnFullScreen(void)
{
	isFullscreen = true;
	fullscreenNormalRect = geometry();
	if (!ui->titleBar->IsMaximize()) {
		isFullscreenMax = true;
		ui->titleBar->SetMaximize(true);
	}
	QDesktopWidget *deskWidget = qApp->desktop();
	setGeometry(deskWidget->screenGeometry(this));
	ui->titleBar->hide();
	ui->titleBar->SetFullScreen(true);
}

void MainWindow::OnExitFullScreen(void)
{
	isFullscreen = false;
	if (ui->titleBar->IsMaximize() && !isFullscreenMax) {
		ui->titleBar->ShowMaximize();
	}
	else {
		ui->titleBar->ShowNormal();
	}
	isFullscreenMax = false;
	setGeometry(fullscreenNormalRect);
	ui->titleBar->show();
	ui->titleBar->SetFullScreen(false);
}

void MainWindow::ShowPlayer(void)
{
	ui->player->setVisible(true);
	ui->list->setVisible(false);

	ui->player->SetShowPlayer(true);

	ui->titleBar->SetLogoable(false, QSize(188, 20));
	ui->titleBar->SetTitleable(true);
	showVideo->setVisible(true);
	returnVideo->setVisible(true);
}
void MainWindow::ShowListVideo(void)
{
	ui->player->setVisible(false);
	ui->list->setVisible(true);

	ui->player->SetShowPlayer(false);

	ui->titleBar->SetLogoable(true, QSize(188, 20));
	ui->titleBar->SetTitleable(false);
	showVideo->setVisible(false);
	returnVideo->setVisible(false);
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
			if (item->video->SetTitle(video->title, false)) {
				QString text = QString("%1\nVID:%2").arg(QT_UTF8(video->title.c_str())).arg(QT_UTF8(video->vid.c_str()));
				QTableWidgetItem* tableItem = ui->myVideoTable->item(item->item->row(), 1);
				tableItem->setText(text);
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
		int row = 0;// ui->docTable->rowCount();
		ui->myVideoTable->insertRow(row);
		int col = 0;
		item->item = new QTableWidgetItem;
		ui->myVideoTable->setItem(row, col, item->item);
		auto cover = new VideoCoverWidget(ui->myVideoTable, false, video);
		ui->myVideoTable->setCellWidget(row, col++, cover);
		QString text = QString("%1\nVID:%2").arg(QT_UTF8(video->title.c_str())).arg(QT_UTF8(video->vid.c_str()));
		//QString text = QString("<span style='font-size:14px;text-align:left;color:#668196'>%1</span><br>\
		//	<span style='font-size:12px;color:#668196;text-align:left'>VID:%2</span>").arg(QT_UTF8(video->title.c_str())).arg(QT_UTF8(video->vid.c_str()));
		QTableWidgetItem* tableItem = new QTableWidgetItem(text);
		ui->myVideoTable->setItem(row, col++, tableItem);
		tableItem = new QTableWidgetItem(QT_UTF8(video->duration.c_str()));
		tableItem->setTextAlignment(Qt::AlignCenter);
		ui->myVideoTable->setItem(row, col++, tableItem);
		double size = video->size / (1024 * 1024);
		tableItem = new QTableWidgetItem(QString("%1MB").arg(QString::number(size, 'f', 2)));
		tableItem->setTextAlignment(Qt::AlignCenter);
		ui->myVideoTable->setItem(row, col++, tableItem);
		auto action = new VideoActionWidget(ui->myVideoTable,
			VideoActionWidget::ACTION_DOWNLOAD, video);
		//connect(action, SIGNAL(SignalDownload(int, const SharedVideoPtr&)),
		//	this, SLOT(OnDownloadVideo(int, const SharedVideoPtr&)));
		ui->myVideoTable->setCellWidget(row, col++, action);

		mapMyVideos.insert(video->vid, item);

		video->DownloadImage();

		if (focus) {
			StartItemHighlight(video->vid);
		}
	} while (false);
	
	ui->myvideo->SetTableEmpty(mapMyVideos.isEmpty());

	return true;
}

bool MainWindow::InsertDownloadItem(int rate, const SharedVideoPtr& video)
{
	bool result = false;
	do
	{
		if (!video) {
			break;
		}
		auto item = mapLocalVideos.value(video->vid);
		if (item) {
			if (item->setVideoRate.contains(rate)) {
				TipsWidget::ShowToast(ui->myvideo, QTStr("AlreadyDownloaded"));
				break;
			}
		}	
		item = mapDownloadVideos.value(video->vid);
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
		ui->downloadVideoTable->setCellWidget(row, col++, new VideoCoverWidget(ui->downloadVideoTable, false, video));
		QTableWidgetItem* tableItem = new QTableWidgetItem(QT_UTF8(video->title.c_str()));
		ui->downloadVideoTable->setItem(row, col++, tableItem);
		double size = video->size / (1024 * 1024);
		tableItem = new QTableWidgetItem(QString("%1MB").arg(QString::number(size, 'f', 2)));
		tableItem->setTextAlignment(Qt::AlignCenter);
		ui->downloadVideoTable->setItem(row, col++, tableItem);

		auto status = new VideoDownloadWidget(ui->downloadVideoTable, video, rate);
		ui->downloadVideoTable->setCellWidget(row, col++, status);

		auto action = new VideoActionWidget(ui->downloadVideoTable,
			VideoActionWidget::ACTION_DOWNLOADING, video);
		ui->downloadVideoTable->setCellWidget(row, col++, action);

		mapDownloadVideos.insert(video->vid, item);

	} while (false);

	ui->downloadvideo->SetTableEmpty(mapDownloadVideos.isEmpty());
	return result;
}

bool MainWindow::InsertLocalItem(int rate, const SharedVideoPtr& video)
{
	bool result = false;
	do
	{
		if (!video) {
			break;
		}
		result = true;
		auto item = mapLocalVideos.value(video->vid);
		if (item) {
			ui->localVideoTable->scrollToItem(item->item);
			item->setVideoRate.insert(rate);
			break;
		}
		do
		{
			QJsonObject obj;
			QString localPath = App()->GetLocalPath();
			QString fileName = QString("%1/%2.json").arg(localPath).arg(video->vid.c_str());
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
		ui->localVideoTable->setCellWidget(row, col++, new VideoCoverWidget(ui->localVideoTable, true, video));
		QTableWidgetItem* tableItem = new QTableWidgetItem(QT_UTF8(video->title.c_str()));
		ui->localVideoTable->setItem(row, col++, tableItem);
		double size = video->size / (1024 * 1024);
		tableItem = new QTableWidgetItem(QString("%1MB").arg(QString::number(size, 'f', 2)));
		tableItem->setTextAlignment(Qt::AlignCenter);
		ui->localVideoTable->setItem(row, col++, tableItem);

		auto action = new VideoActionWidget(ui->localVideoTable,
			VideoActionWidget::ACTION_LOCAL, video);
		ui->localVideoTable->setCellWidget(row, col++, action);
		mapLocalVideos.insert(video->vid, item);
	} while (false);

	ui->localvideo->SetTableEmpty(mapLocalVideos.isEmpty());

	return result;
}

void MainWindow::UpdateItemTitle(QTableWidget* table, const QMap<std::string, SharedItemPtr>& map,
	const std::string& vid, const std::string& title)
{
	auto item = map.value(vid);
	if (!item) {
		return;
	}
	QTableWidgetItem* tableItem = table->item(item->item->row(), 1);
	tableItem->setText(QT_UTF8(title.c_str()));
}

void MainWindow::StartItemHighlight(const std::string& vid)
{
	if (vid != curHighlightVid) {
		StopItemHighlight();		
		curHighlightVid = vid;
	}
	auto item = mapMyVideos.value(curHighlightVid);
	if (item) {
		ui->myVideoTable->item(item->item->row(), 1)->setSelected(true);// ui->myVideoTable->setItemSelected(ui->myVideoTable->item(item->item->row(), 1), true);
		if (!itemHighlightTimer) {
			itemHighlightTimer = new QTimer(this);
			connect(itemHighlightTimer, SIGNAL(timeout()), this, SLOT(OnHighlightItem()));
		}
		curHighlight = 0;
		itemHighlightTimer->start(200);
	}
}

void MainWindow::StopItemHighlight(void)
{
	if (itemHighlightTimer) {
		itemHighlightTimer->stop();
	}
	auto item = mapMyVideos.value(curHighlightVid);
	if (item) {
		ui->myVideoTable->item(item->item->row(), 1)->setSelected(false);// ui->myVideoTable->setItemSelected(ui->myVideoTable->item(item->item->row(), 1), false);
	}
	curHighlight = 0;
	curHighlightVid = std::string();
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
