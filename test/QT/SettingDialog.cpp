#include "SettingDialog.h"
#include "ui_SettingDialog.h"

#include <QFileDialog>
#include <QDesktopServices>

#include "GlobalConfig.h"
#include "Application.h"
#include "WidgetHelper.h"
#include "SdkManager.h"
#include "platform.h"

#ifdef __APPLE__
#include "mac/PermissionDialog.h"
#endif // __APPLE__

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
	ui->titleBar->Init(this, TITLE_CLOSE_BTN);
	ui->titleBar->SetTitleable(true);
	ui->titleBar->SetTitleName(QTStr("Setting"));
	ui->titleBar->SetLogoable(false, QSize(188, 20));

	InitTaskCount();
	InitRetryCount();
	InitVideoOutput();
	InitLogLevel();
	InitHttpRequest();
	InitPlayRate();

	ui->logPath->setText(GlobalConfig::GetLogPath());
	ui->logPath->setCursorPosition(0);
	ui->videoPath->setText(GlobalConfig::GetSaveVideoPath());
	ui->videoPath->setCursorPosition(0);
	ui->screenshotPath->setText(GlobalConfig::GetSaveScreenshotPath());
	ui->screenshotPath->setCursorPosition(0);
	
#ifdef _WIN32
	ui->permissionWidget->setVisible(false);
#endif// _WIN32
	ui->seek->setValue(GlobalConfig::GetVideoPlaySeek());
	ui->softwareRecording->setChecked(GlobalConfig::IsSoftwareRecording());
	ui->hardwareRecording->setChecked(GlobalConfig::IsHardwareRecording());
	ui->keepLastFrame->setChecked(GlobalConfig::IsKeepLastFrame());
	ui->hwdecEnable->setChecked(GlobalConfig::IsHwdecEnable());
	ui->autoDownRate->setChecked(GlobalConfig::IsAutoDownRate());
	ui->playWithToken->setChecked(GlobalConfig::IsPlayWithToken());

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
	GlobalConfig::SetAutoDownRate(ui->autoDownRate->isChecked());
	GlobalConfig::SetPlayWithToken(ui->playWithToken->isChecked());

	auto logPath = GlobalConfig::GetLogPath();
	if (logPath != ui->logPath->text()) {
		GlobalConfig::SetLogPath(ui->logPath->text());
		SdkManager::GetManager()->SetLogPath();
	}
	auto videoPath = GlobalConfig::GetSaveVideoPath();
	if (videoPath != ui->videoPath->text()) {
		GlobalConfig::SetSaveVideoPath(ui->videoPath->text());
	}
	auto screenshotPath = GlobalConfig::GetSaveScreenshotPath();
	if (screenshotPath != ui->screenshotPath->text()) {
		GlobalConfig::SetSaveScreenshotPath(ui->screenshotPath->text());
	}
	GlobalConfig::SetTaskCount(ui->taskCount->currentData().toInt());
	GlobalConfig::SetRetryCount(ui->retryCount->currentData().toInt());

	auto outputType = GlobalConfig::GetVideoOutput();
	if (outputType != ui->videoOutput->currentData().toInt()) {
		GlobalConfig::SetVideoOutput(ui->videoOutput->currentData().toInt());
		GlobalConfig::SetVideoOutputContext(ui->videoOutputContext->currentText());
		SdkManager::GetManager()->SetVideoOutputDevice();
	}
	auto playRate = GlobalConfig::GetPlayWithRate();
	if (playRate != ui->playRate->currentData().toInt()) {
		GlobalConfig::SetPlayWithRate(ui->playRate->currentData().toInt());
	}
	auto logLevel = GlobalConfig::GetLogLevel();
	if (logLevel != ui->logLevel->currentData().toInt()) {
		GlobalConfig::SetLogLevel(ui->logLevel->currentData().toInt());
		SdkManager::GetManager()->SetLogLevel();
	}
	auto logCallback = GlobalConfig::IsLogCallback();
	if (logCallback != ui->logCallback->isChecked()) {
		GlobalConfig::SetLogCallback(ui->logCallback->isChecked());
		SdkManager::GetManager()->SetLogCallback();
	}
	auto httpRequest = GlobalConfig::GetHttpRequest();
	if (httpRequest != ui->httpRequest->currentData().toInt()) {
		GlobalConfig::SetHttpRequest(ui->httpRequest->currentData().toInt());
		SdkManager::GetManager()->SetHttpRequest();
	}
	GlobalConfig::SetVideoPlaySeek(ui->seek->value());

	auto softwareRecording = GlobalConfig::IsSoftwareRecording();
	if (softwareRecording != ui->softwareRecording->isChecked()) {
		GlobalConfig::SetSoftwareRecording(ui->softwareRecording->isChecked());
		SdkManager::GetManager()->SetSoftwareRecording();
	}

	auto hardwareRecording = GlobalConfig::IsHardwareRecording();
	if (hardwareRecording != ui->hardwareRecording->isChecked()) {
		GlobalConfig::SetHardwareRecording(ui->hardwareRecording->isChecked());
		SdkManager::GetManager()->SetHardwareRecording();
	}

	auto hwdecEnable = GlobalConfig::IsHwdecEnable();
	if (hwdecEnable != ui->hwdecEnable->isChecked()) {
		GlobalConfig::SetHwdecEnable(ui->hwdecEnable->isChecked());
		SdkManager::GetManager()->SetHwdecEnable();
	}

	auto keepLastFrame = GlobalConfig::IsKeepLastFrame();
	if (keepLastFrame != ui->keepLastFrame->isChecked()) {
		GlobalConfig::SetKeepLastFrame(ui->keepLastFrame->isChecked());
		SdkManager::GetManager()->SetKeepLastFrame();
	}

	auto & cacheConfig = Player::GetCacheConfig();
	cacheConfig.enable = ui->cache->isChecked();
	cacheConfig.maxCacheBytes = ui->cacheBytes->text().toInt();
	cacheConfig.maxCacheSeconds = ui->cacheSeconds->text().toInt();
	GlobalConfig::SetVideoCache(cacheConfig.enable);
	GlobalConfig::SetMaxCacheBytes(cacheConfig.maxCacheBytes);
	GlobalConfig::SetMaxCacheSeconds(cacheConfig.maxCacheSeconds);
	
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
	GlobalConfig::SetVideoOsd(osdConfig.enable);

	auto & logoConfig = Player::GetLogoConfig();
	logoConfig.enable = ui->logo->isChecked();
	logoConfig.text = ui->logoText->text();
	logoConfig.textSize = ui->logoTextSize->value();
	logoConfig.textColor = ui->logoTextColor->text();
	logoConfig.borderSize = ui->logoBorderSize->value();
	logoConfig.borderColor = ui->logoBorderColor->text();
	logoConfig.alignX = ui->left == ui->alignX->checkedButton() ? -1 : (ui->right == ui->alignX->checkedButton() ? 1 : 0);
	logoConfig.alignY = ui->top == ui->alignY->checkedButton() ? -1 : (ui->bottom == ui->alignY->checkedButton() ? 1 : 0);
	GlobalConfig::SetVideoLogo(logoConfig.enable);

	GlobalConfig::Save();
}


static void OpenDir(QLineEdit* edit)
{
	QString path = edit->text();
	if (!path.isEmpty()) {
		QDesktopServices::openUrl(QUrl::fromLocalFile(path));
	}
}
void SettingDialog::on_logBtn_clicked()
{
	OpenDir(ui->logPath);
}
void SettingDialog::on_videoBtn_clicked()
{
	OpenDir(ui->videoPath);
}
void SettingDialog::on_screenshotBtn_clicked(void)
{
	OpenDir(ui->screenshotPath);
}
void SettingDialog::on_permission_clicked()
{
#ifdef __APPLE__
	MacPermissionStatus allfiles_permission =
		CheckPermission(kAllFilesAccess);
	MacPermissionStatus accessibility_permission =
		CheckPermission(kAccessibility);	
	PermissionDialog dlg(
		nullptr, allfiles_permission, accessibility_permission);
	dlg.exec();
#endif
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

static void InitComboxListView(QComboBox* list, int value, const QList<SdkManager::ValueItem>& values)
{
	SetComboxListView(list);
	list->blockSignals(true);
	for (auto& it : values) {
		it.defValue ? list->addItem(it.name + QTStr("DefaultValue"), it.value) :
			list->addItem(it.name, it.value);
	}
	if (-1 != value) {
		for (int i = 0; i < list->count(); ++i) {
			if (value == list->itemData(i).toInt()) {
				list->setCurrentIndex(i);
				break;
			}
		}
	}
	
	list->blockSignals(false);
}
static void InitComboxListView(QComboBox* list, int value, const QList<int>& values)
{
	SetComboxListView(list);
	list->blockSignals(true);
	for (auto& it : values) {
		list->addItem(QString("%1").arg(it), it);
	}
	if (-1 != value) {
		for (int i = 0; i < list->count(); ++i) {
			if (value == list->itemData(i).toInt()) {
				list->setCurrentIndex(i);
				break;
			}
		}
	}
	list->blockSignals(false);
}

void SettingDialog::InitTaskCount()
{
	InitComboxListView(ui->taskCount, GlobalConfig::GetTaskCount(), { 1,2,3,4,5,6 });
}
void SettingDialog::InitRetryCount()
{
	InitComboxListView(ui->retryCount, -1, { 0,1,2,3,4,5,6 });
	ui->retryCount->setItemText(0, tr("DownloadUnlimitRetry") + tr("DefaultValue"));
	int count = GlobalConfig::GetRetryCount();
	ui->retryCount->blockSignals(true);
	for (int i = 0; i < ui->retryCount->count(); ++i) {
		if (count == ui->retryCount->itemData(i).toInt()) {
			ui->retryCount->setCurrentIndex(i);
			break;
		}
	}
	ui->retryCount->blockSignals(false);
}
void SettingDialog::InitVideoOutput()
{
	InitComboxListView(ui->videoOutput, 
		GlobalConfig::GetVideoOutput(), SdkManager::GetManager()->GetOutputItems());
	auto values = SdkManager::GetManager()->GetOutputContexts();
	SetComboxListView(ui->videoOutputContext);
	for (auto& it : values) {
		ui->videoOutputContext->addItem(it);
	}
	ui->videoOutputContext->setItemText(0, ui->videoOutputContext->itemText(0) + tr("DefaultValue"));
	ui->videoOutputContext->setCurrentText(GlobalConfig::GetVideoOutputContext());
}
void SettingDialog::InitLogLevel()
{
	InitComboxListView(ui->logLevel,
		GlobalConfig::GetLogLevel(), SdkManager::GetManager()->GetLogItems());
	ui->logCallback->setChecked(GlobalConfig::IsLogCallback());
}
void SettingDialog::InitHttpRequest()
{
	InitComboxListView(ui->httpRequest, 
		GlobalConfig::GetHttpRequest(), SdkManager::GetManager()->GetHttpItems());
}
void SettingDialog::InitPlayRate()
{
	InitComboxListView(ui->playRate,
		GlobalConfig::GetPlayWithRate(), SdkManager::GetManager()->GetRateItems());
}

