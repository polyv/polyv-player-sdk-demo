#include "DetectRecordingDialog.h"
#include "ui_DetectRecordingDialog.h"

#include <QFileDialog>

#include "Application.h"
#include "WidgetHelper.h"
#include "TipsWidget.h"
#include "SdkManager.h"
#include "MainWindow.h"


DetectRecordingDialog::DetectRecordingDialog(QWidget *parent)
	: QDialog(parent)
	, ui(new Ui::DetectRecordingDialog)
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
	ui->titleBar->SetTitleName(QTStr("DetectRecording"));
	ui->titleBar->SetLogoable(false, QSize(0, 0));
}

DetectRecordingDialog::~DetectRecordingDialog()
{
    delete ui;
}

void DetectRecordingDialog::DetectHardwareRecording(int type, QString device)
{
	switch (type) {
	case DEVICE_CHANGED_NONE:
		setHardwares.clear();
		break;
	case HDMI_DEVICE_USE:
		setHardwares.insert(device);
		break;
	case HDMI_DEVICE_UNUSE:
		setHardwares.remove(device);
		break;
	}
	Refresh();
}
void DetectRecordingDialog::DetectSoftwareRecording(int type, QString software)
{
	switch (type) {
	case SOFTWARE_RECORDING_NONE:
		setSoftwares.clear();
		break;
	case SOFTWARE_RECORDING_START:
		setSoftwares.insert(software);
		break;
	case SOFTWARE_RECORDING_STOP:
		setSoftwares.remove(software);
		break;
	}
	Refresh();
}



void DetectRecordingDialog::on_enable_clicked(void)
{
	QMetaObject::invokeMethod(App()->GetMainWindow(), "OnEnableWindow", Q_ARG(bool, true));
	hide();
}
void DetectRecordingDialog::on_disable_clicked(void)
{
	QMetaObject::invokeMethod(App()->GetMainWindow(), "OnEnableWindow", Q_ARG(bool, false));
	hide();
}
void DetectRecordingDialog::on_ignore_clicked(void)
{
	hide();
}

void DetectRecordingDialog::Refresh()
{
	if (setSoftwares.isEmpty() && setHardwares.isEmpty()) {	
		on_enable_clicked();
		return;
	}
	QString softwareStr, hardwareStr;
	if (!setSoftwares.isEmpty()) {
		auto softwareList = setSoftwares.toList();
		softwareStr = QString("%1").arg(tr("DetectRecordingSoftware")).arg(softwareList.join(","));
	}
	if (!setHardwares.isEmpty()) {
		auto hardwareList = setHardwares.toList();
		hardwareStr = QString("%1").arg(tr("DetectRecordingHardware")).arg(hardwareList.join(","));
	}
	QString tips = QString("%1").arg(tr("DetectRecordingTip")).arg(softwareStr + hardwareStr);

	ui->tips->setText(tips); 
	show();
}

