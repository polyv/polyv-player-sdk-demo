#include "SettingDialog.h"
#include "ui_SettingDialog.h"

#include <QFileDialog>

#include "Application.h"
#include "WidgetHelper.h"
#include "SdkManager.h"
#include "MainWindow.h"

OSDConfig SettingDialog::osdConfig;
CacheConfig SettingDialog::cacheConfig;

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
	setAttribute(Qt::WA_DeleteOnClose, true);
	ui->titleBar->Init(this, TITLE_CLOSE_BTN);
	ui->titleBar->SetTitleable(true);
	ui->titleBar->SetTitleName(QTStr("Setting"));
	ui->titleBar->SetLogoable(false, QSize(188, 20));

	SetComboxListView(ui->comboBox);
	const int maxCount = 6;
	ui->comboBox->blockSignals(true);
	for (int i = 1; i <= maxCount; ++i) {
		ui->comboBox->addItem(QString("%1").arg(i), i);
	}
	int count = App()->GlobalConfig().Get("Download", "TaskCount").toInt();
	ui->comboBox->setCurrentIndex((count > 0 && count <= maxCount) ? count - 1 : 3 - 1);
	ui->comboBox->blockSignals(false);

	QString path = App()->GlobalConfig().Get("Download", "FilePath").toString();
	ui->pathEdit->setText(path);
	path = App()->GlobalConfig().Get("Download", "ScreenshotPath").toString();
	ui->screenshotPath->setText(path);
	ui->keepLastFrame->setChecked(App()->GlobalConfig().Get("Video", "KeepLastFrame").toBool());
	ui->hwdecEnable->setChecked(App()->GlobalConfig().Get("Video", "HwdecEnable").toBool());
#ifdef _WIN32
	ui->recordEnable->setChecked(App()->GlobalConfig().Get("Video", "SoftwareRecord").toBool());
	ui->hdmi->setChecked(App()->GlobalConfig().Get("Video", "HdmiCallback").toBool());
#else
	ui->recordEnable->setVisible(false);
	ui->hdmi->setVisible(false);
#endif // _WIN32

	ui->comboBox_2->blockSignals(true);
	int type = App()->GlobalConfig().Get("Video", "VideoOutput").toInt();
	ui->comboBox_2->setCurrentIndex(type);
	ui->comboBox_2->blockSignals(false);

	InitOSDConfig();

	InitCacheConfig();
}

SettingDialog::~SettingDialog()
{
    delete ui;
}

void SettingDialog::SetOSDText(const QString& text)
{
	osdConfig.text = text;
}

OSDConfig& SettingDialog::GetOSDConfig()
{
	if (osdConfig.text.isEmpty()) {
		osdConfig.text = SdkManager::GetManager()->GetAccount().viewerId;
	}
	return osdConfig;
}

CacheConfig& SettingDialog::GetCacheConfig()
{
	return cacheConfig;
}

void SettingDialog::on_ok_clicked(void)
{
	auto& config = App()->GlobalConfig();
	int oldType = config.Get("Video", "VideoOutput").toInt();
	auto oldKeepLastFrame = config.Get("Video", "KeepLastFrame").toBool();
	auto oldHwdecEnable = config.Get("Video", "HwdecEnable").toBool();
	int count = ui->comboBox->currentData().toInt();
	int type = ui->comboBox_2->currentIndex();
	
	config.Set("Download", "TaskCount", count);
	config.Set("Download", "FilePath", QT_TO_UTF8(ui->pathEdit->text()));
	config.Set("Download", "ScreenshotPath", QT_TO_UTF8(ui->screenshotPath->text()));
	
	config.Set("Video", "KeepLastFrame", ui->keepLastFrame->isChecked());
	config.Set("Video", "VideoOutput", type);
	config.Set("Video", "HwdecEnable", ui->hwdecEnable->isChecked());

	if (oldHwdecEnable != ui->hwdecEnable->isChecked()) {
		SdkManager::GetManager()->SetHwdecEnable();
	}
#ifdef _WIN32
	auto oldRecord = config.Get("Video", "SoftwareRecord").toBool();
	auto oldHdmi = config.Get("Video", "HdmiCallback").toBool();

	config.Set("Video", "SoftwareRecord", ui->recordEnable->isChecked());
	config.Set("Video", "HdmiCallback", ui->hdmi->isChecked());
	if (oldRecord != ui->recordEnable->isChecked()) {
		QMetaObject::invokeMethod(App()->GetMainWindow(), "OnUpdateSoftwareRecord", Qt::QueuedConnection,
			Q_ARG(bool, ui->recordEnable->isChecked()));
	}
	if (oldHdmi != ui->hdmi->isChecked()) {
		QMetaObject::invokeMethod(App()->GetMainWindow(), "OnUpdateHdmiRecord", Qt::QueuedConnection,
			Q_ARG(bool, ui->hdmi->isChecked()));
	}
#endif // _WIN32
	
	config.Save();
	if (oldKeepLastFrame != ui->keepLastFrame->isChecked()) {
		SdkManager::GetManager()->SetKeepLastFrame();
	}
	if (oldType != type) {
		PLVSetSdkVideoOutputDevice((VIDEO_OUTPUT_DEVICE)type);
	}
	bool change = false;
	if (cacheConfig.enable != ui->cache->isChecked()) {
		change = true;
		cacheConfig.enable = ui->cache->isChecked();
	}
	QString value = ui->cacheBytes->text();
	if (value.toInt() != cacheConfig.maxCacheBytes) {
		change = true;
		cacheConfig.maxCacheBytes = value.toInt();
	}
	value = ui->cacheSeconds->text();
	if (value.toInt() != cacheConfig.maxCacheSeconds) {
		change = true;
		cacheConfig.maxCacheSeconds = value.toInt();
	}
	cacheConfig.change = change;
	osdConfig.enable = ui->osd->isChecked();
	osdConfig.text = ui->osdText->text();
	osdConfig.textSize = ui->osdTextSize->value();
	osdConfig.textColor = ui->osdTextColor->text();
	osdConfig.textAlpha = ui->osdTextAlpha->value();
	osdConfig.border = ui->osdBorder->isChecked();
	osdConfig.borderColor = ui->osdBorderColor->text();
	osdConfig.borderAlpha = ui->osdBorderAlpha->value();
	osdConfig.borderWidth = ui->osdBorderWidth->value();
	osdConfig.displayDuration = ui->osdDisplayDuration->value();
	osdConfig.displayInterval = ui->osdDisplayInterval->value();
	osdConfig.fadeDuration = ui->osdFadeDuration->value();
	ui->osdRoll->isChecked() ? osdConfig.animationEffect = 0 : osdConfig.animationEffect = 1;
	accept();
}
void SettingDialog::on_cancel_clicked(void)
{
	close();
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
	//App()->GlobalConfig().SetString("Download", "FilePath", QT_TO_UTF8(dir));
	//App()->GlobalConfig().Save();
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
	//App()->GlobalConfig().SetString("Player", "ScreenshotPath", QT_TO_UTF8(dir));
	//App()->GlobalConfig().Save();
}

void SettingDialog::on_osd_clicked(bool enable)
{
	ui->osdWidget->setEnabled(enable);
}

void SettingDialog::on_cache_clicked(bool enable)
{
	ui->cacheWidget->setEnabled(enable);
}

void SettingDialog::InitOSDConfig(void)
{
	if (osdConfig.text.isEmpty()) {
		osdConfig.text = SdkManager::GetManager()->GetAccount().viewerId;
	}
	ui->osd->setChecked(osdConfig.enable);
	ui->osdText->setText(osdConfig.text);
	ui->osdTextSize->setValue(osdConfig.textSize);
	ui->osdTextColor->setText(osdConfig.textColor);
	ui->osdTextAlpha->setValue(osdConfig.textAlpha);
	ui->osdBorder->setChecked(osdConfig.border);
	ui->osdBorderColor->setText(osdConfig.borderColor);
	ui->osdBorderAlpha->setValue(osdConfig.borderAlpha);
	ui->osdBorderWidth->setValue(osdConfig.borderWidth);
	ui->osdDisplayDuration->setValue(osdConfig.displayDuration);
	ui->osdDisplayInterval->setValue(osdConfig.displayInterval);
	ui->osdFadeDuration->setValue(osdConfig.fadeDuration);
	0 == osdConfig.animationEffect ? ui->osdRoll->setChecked(true) : ui->osdBlink->setChecked(true);

	on_osd_clicked(osdConfig.enable);
}

void SettingDialog::InitCacheConfig(void)
{
	ui->cache->setChecked(cacheConfig.enable);
	cacheConfig.change = false;
	QString value;
	//if (0 != cacheConfig.maxCacheBytes) {
		value = QString("%1").arg(cacheConfig.maxCacheBytes);
	//}
	ui->cacheBytes->setText(value);
	value = QString();
	//if (0 != cacheConfig.maxCacheSeconds) {
		value = QString("%1").arg(cacheConfig.maxCacheSeconds);
	//}
	ui->cacheSeconds->setText(value);
	on_cache_clicked(cacheConfig.enable);
}
