#include "SettingDialog.h"
#include "ui_SettingDialog.h"

#include <QFileDialog>
#include <QDesktopServices>

#include "Application.h"
#include "WidgetHelper.h"
#include "SdkManager.h"


SettingDialog::SettingDialog(QWidget *parent) 
	: QDialog(parent)
	, ui(new Ui::SettingDialog)
{
    ui->setupUi(this);
	Qt::WindowFlags f;
	f |= Qt::Dialog;
	f |= Qt::WindowCloseButtonHint;
	f |= Qt::CustomizeWindowHint;
	f |= Qt::WindowSystemMenuHint;
#ifdef _WIN32
	f |= Qt::FramelessWindowHint;
#endif
	setWindowFlags(f);
	//setAttribute(Qt::WA_DeleteOnClose, true);
	ui->titleBar->Init(this, TITLE_CLOSE_BTN);
	ui->titleBar->SetTitleable(true);
	ui->titleBar->SetTitleName(QTStr("Setting"));
	ui->titleBar->SetLogoable(false, QSize(188, 20));

	SetComboxListView(ui->taskCount);
	const int maxCount = 6;
	ui->taskCount->blockSignals(true);
	for (int i = 1; i <= maxCount; ++i) {
		ui->taskCount->addItem(QString("%1").arg(i), i);
	}
	int count = App()->GlobalConfig().Get("Download", "TaskCount").toInt();
	ui->taskCount->setCurrentIndex((count > 0 && count <= maxCount) ? count - 1 : 3 - 1);
	ui->taskCount->blockSignals(false);

	SetComboxListView(ui->retry);
	ui->retry->blockSignals(true);
	for (int i = 0; i <= maxCount; ++i) {
		ui->retry->addItem(0 == i ? QTStr("UnlimitRetry") : QString("%1").arg(i), i);
	}
	count = App()->GlobalConfig().Get("Download", "RetryCount").toInt();
	ui->retry->setCurrentIndex(count <= maxCount ? count : 0);
	ui->retry->blockSignals(false);


	QString path = App()->GlobalConfig().Get("Download", "FilePath").toString();
	ui->pathEdit->setText(path);
	path = App()->GlobalConfig().Get("Download", "ScreenshotPath").toString();
	ui->screenshotPath->setText(path);
	ui->keepLastFrame->setChecked(App()->GlobalConfig().Get("Video", "KeepLastFrame").toBool());
	ui->hwdecEnable->setChecked(App()->GlobalConfig().Get("Video", "HwdecEnable").toBool());

	ui->debugLog->setChecked(App()->GlobalConfig().Get("App", "EnableDebugLog", true).toBool());

#ifdef _WIN32
	ui->recordEnable->setChecked(App()->GlobalConfig().Get("Video", "SoftwareRecord").toBool());
	ui->hdmi->setChecked(App()->GlobalConfig().Get("Video", "HdmiCallback").toBool());
#else
	ui->recordEnable->setVisible(false);
	ui->hdmi->setVisible(false);

	while (ui->videoOutput->count() != 2) {
		ui->videoOutput->removeItem(VIDEO_OUTPUT_GPU + 1);
	}
#endif // _WIN32

	ui->videoOutput->blockSignals(true);
	int type = App()->GlobalConfig().Get("Video", "VideoOutput").toInt();
	ui->videoOutput->setCurrentIndex(type);
	ui->videoOutput->blockSignals(false);


	int seek = App()->GlobalConfig().Get("Video", "VideoPlaySeek").toInt();
	ui->seek->setValue(seek);

	InitOSDConfig();
	InitLogoConfig();
	InitCacheConfig();
}

SettingDialog::~SettingDialog()
{
    delete ui;
}

void SettingDialog::closeEvent(QCloseEvent* e)
{
	auto& config = App()->GlobalConfig();
	bool enableDebugLog = config.Get("App", "EnableDebugLog").toBool();
	int oldType = config.Get("Video", "VideoOutput").toInt();
	auto oldKeepLastFrame = config.Get("Video", "KeepLastFrame").toBool();
	auto oldHwdecEnable = config.Get("Video", "HwdecEnable").toBool();
	int count = ui->taskCount->currentData().toInt();
	int retryCount = ui->retry->currentData().toInt();
	int type = ui->videoOutput->currentIndex();

	config.Set("App", "EnableDebugLog", ui->debugLog->isChecked());
	
	config.Set("Download", "TaskCount", count);
	config.Set("Download", "RetryCount", retryCount);
	config.Set("Download", "FilePath", QT_TO_UTF8(ui->pathEdit->text()));
	config.Set("Download", "ScreenshotPath", QT_TO_UTF8(ui->screenshotPath->text()));
	
	config.Set("Video", "KeepLastFrame", ui->keepLastFrame->isChecked());
	config.Set("Video", "VideoOutput", type);
	config.Set("Video", "HwdecEnable", ui->hwdecEnable->isChecked());

	config.Set("Video", "VideoPlaySeek", ui->seek->value());
	if (oldHwdecEnable != ui->hwdecEnable->isChecked()) {
		SdkManager::GetManager()->SetHwdecEnable();
	}
	if (enableDebugLog != ui->debugLog->isChecked()) {
		SdkManager::GetManager()->SetDebugLog();
	}
#ifdef _WIN32
	auto oldRecord = config.Get("Video", "SoftwareRecord").toBool();
	auto oldHdmi = config.Get("Video", "HdmiCallback").toBool();

	config.Set("Video", "SoftwareRecord", ui->recordEnable->isChecked());
	config.Set("Video", "HdmiCallback", ui->hdmi->isChecked());
	if (oldRecord != ui->recordEnable->isChecked()) {
		SdkManager::GetManager()->SetSoftwareRecord((void*)((QWidget*)App()->GetMainWindow())->winId(),
			ui->recordEnable->isChecked());
	}
	if (oldHdmi != ui->hdmi->isChecked()) {
		SdkManager::GetManager()->SetHdmiRecord(ui->hdmi->isChecked());
	}
#endif // _WIN32
	
	config.Save();
	if (oldKeepLastFrame != ui->keepLastFrame->isChecked()) {
		SdkManager::GetManager()->SetKeepLastFrame();
	}
	if (oldType != type) {
		SdkManager::GetManager()->SetVideoOutputDevice((VIDEO_OUTPUT_DEVICE)type);
	}
	SdkManager::GetManager()->SetRetryCount(retryCount);

	auto & cacheConfig = Player::GetCacheConfig();
	cacheConfig.enable = ui->cache->isChecked();
	cacheConfig.maxCacheBytes = ui->cacheBytes->text().toInt();
	cacheConfig.maxCacheSeconds = ui->cacheSeconds->text().toInt();
	config.Set("Video", "EnableCache", cacheConfig.enable);
	config.Set("Video", "MaxCacheBytes", cacheConfig.maxCacheBytes);
	config.Set("Video", "MaxCacheSeconds", cacheConfig.maxCacheSeconds);
	
	auto & osdConfig = Player::GetOSDConfig();
	osdConfig.enable = ui->osd->isChecked();
	osdConfig.text = ui->osdText->text();
	osdConfig.textSize = ui->osdTextSize->value();
	osdConfig.textColor = ui->osdTextColor->text();
	osdConfig.borderSize = ui->osdBorderSize->value();
	osdConfig.borderColor = ui->osdBorderColor->text();	
	osdConfig.displayDuration = ui->osdDisplayDuration->value();
	osdConfig.displayInterval = ui->osdDisplayInterval->value();
	osdConfig.fadeDuration = ui->osdFadeDuration->value();
	ui->osdRoll->isChecked() ? osdConfig.animationEffect = 0 : osdConfig.animationEffect = 1;
	config.Set("Video", "EnableOSD", osdConfig.enable);

	auto & logoConfig = Player::GetLogoConfig();
	logoConfig.enable = ui->logo->isChecked();
	logoConfig.text = ui->logoText->text();
	logoConfig.textFontName = ui->logoFontName->text();
	logoConfig.textSize = ui->logoTextSize->value();
	logoConfig.textColor = ui->logoTextColor->text();
	logoConfig.borderSize = ui->logoBorderSize->value();
	logoConfig.borderColor = ui->logoBorderColor->text();
	logoConfig.alignX = ui->left == ui->alignX->checkedButton() ? -1 : (ui->right == ui->alignX->checkedButton() ? 1 : 0);
	logoConfig.alignY = ui->top == ui->alignY->checkedButton() ? -1 : (ui->bottom == ui->alignY->checkedButton() ? 1 : 0);
	config.Set("Video", "EnableLogo", logoConfig.enable);
}

void SettingDialog::on_pathButton_clicked()
{
	QString dir = QFileDialog::getExistingDirectory(this,
		QTStr("SelectDirectory"),
		ui->pathEdit->text(),
		QFileDialog::ShowDirsOnly |
		QFileDialog::DontResolveSymlinks);
	if (dir.isEmpty()) {
		return;
	}
	ui->pathEdit->setText(dir);
}

void SettingDialog::on_screenshotButton_clicked(void)
{
	QString dir = QFileDialog::getExistingDirectory(this,
		QTStr("SelectDirectory"),
		ui->screenshotPath->text(),
		QFileDialog::ShowDirsOnly |
		QFileDialog::DontResolveSymlinks);
	if (dir.isEmpty()) {
		return;
	}
	ui->screenshotPath->setText(dir);
}

void SettingDialog::on_openShotDir_clicked()
{
	QString path = ui->screenshotPath->text();
	if (path.isEmpty()) {
		return;
	}
	QDesktopServices::openUrl(QUrl::fromLocalFile(path));
}

void SettingDialog::on_openDownloadDir_clicked(void)
{
	QString path = ui->pathEdit->text();
	if (path.isEmpty()) {
		return;
	}
	QDesktopServices::openUrl(QUrl::fromLocalFile(path));
}

void SettingDialog::on_osd_clicked(bool enable)
{
	ui->osdWidget->setEnabled(enable);
}

void SettingDialog::on_logo_clicked(bool enable)
{
	ui->logoWidget->setEnabled(enable);
}

void SettingDialog::on_cache_clicked(bool enable)
{
	ui->cacheWidget->setEnabled(enable);
}

void SettingDialog::InitOSDConfig(void)
{
	auto & osdConfig = Player::GetOSDConfig();
	ui->osd->setChecked(osdConfig.enable);
	ui->osdText->setText(osdConfig.text);
	ui->osdTextSize->setValue(osdConfig.textSize);
	ui->osdTextColor->setText(osdConfig.textColor);
	ui->osdBorderColor->setText(osdConfig.borderColor);
	ui->osdBorderSize->setValue(osdConfig.borderSize);
	ui->osdDisplayDuration->setValue(osdConfig.displayDuration);
	ui->osdDisplayInterval->setValue(osdConfig.displayInterval);
	ui->osdFadeDuration->setValue(osdConfig.fadeDuration);
	0 == osdConfig.animationEffect ? ui->osdRoll->setChecked(true) : ui->osdBlink->setChecked(true);

	on_osd_clicked(osdConfig.enable);
}

void SettingDialog::InitLogoConfig(void)
{
	auto & logo = Player::GetLogoConfig();
	ui->logo->setChecked(logo.enable);
	ui->logoText->setText(logo.text);
	ui->logoFontName->setText(logo.textFontName);
	ui->logoTextSize->setValue(logo.textSize);
	ui->logoTextColor->setText(logo.textColor);
	ui->logoBorderColor->setText(logo.borderColor);
	ui->logoBorderSize->setValue(logo.borderSize);

	1 == logo.alignX ? ui->right->setChecked(true) :
		(-1 == logo.alignX ? ui->left->setChecked(true) : ui->centerX->setChecked(true));
	1 == logo.alignY ? ui->bottom->setChecked(true) :
		(-1 == logo.alignY ? ui->top->setChecked(true) : ui->centerY->setChecked(true));

	on_logo_clicked(logo.enable);
}

void SettingDialog::InitCacheConfig(void)
{
	auto & config = Player::GetCacheConfig();
	ui->cache->setChecked(config.enable);
	ui->cacheBytes->setText(QString("%1").arg(config.maxCacheBytes));
	ui->cacheSeconds->setText(QString("%1").arg(config.maxCacheSeconds));

	on_cache_clicked(config.enable);
}
