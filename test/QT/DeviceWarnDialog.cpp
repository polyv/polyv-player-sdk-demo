#include "DeviceWarnDialog.h"
#include "ui_DeviceWarnDialog.h"

#include <QFileDialog>

#include "Application.h"
#include "WidgetHelper.h"
#include "TipsWidget.h"
#include "SdkManager.h"
#include "MainWindow.h"


DeviceWarnDialog::DeviceWarnDialog(const QString& id, QWidget *parent)
	: QDialog(parent)
	, ui(new Ui::DeviceWarnDialog)
	, devId(id)
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
	ui->titleBar->SetTitleName(QTStr("HDMIWarnTip"));
	ui->titleBar->SetLogoable(false, QSize(0, 0));

	QString text = QString(QTStr("HDMIWarn")).arg(id);
	ui->warnLabel->setText(text);
}

DeviceWarnDialog::~DeviceWarnDialog()
{
    delete ui;
}

void DeviceWarnDialog::SetInfo(const QString& info)
{
	ui->warnLabel->setText(info);
}

//void DeviceWarnDialog::closeEvent(QCloseEvent* e)
//{
//	emit SignalClose();
//	QDialog::closeEvent(e);
//}

void DeviceWarnDialog::on_enable_clicked(void)
{
	QMetaObject::invokeMethod(App()->GetMainWindow(), "OnEnableWindows");
	close();
}
void DeviceWarnDialog::on_disable_clicked(void)
{
	QMetaObject::invokeMethod(App()->GetMainWindow(), "OnEnableWindow",
		Q_ARG(const QString&, devId), Q_ARG(bool, false));
	close();
}
void DeviceWarnDialog::on_ignore_clicked(void)
{
	close();
}

