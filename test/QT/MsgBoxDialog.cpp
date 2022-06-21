#include "MsgBoxDialog.h"
#include "ui_MsgBoxDialog.h"
#include <QLabel>
#include <QTimer>
#include <QEvent>
#include <QStyle>
#include <QPushButton>
#include <QHBoxLayout>
#include <QApplication>
#include <QDesktopWidget>

MsgBoxDialog::MsgBoxDialog(QWidget *parent, const QString& text,
	const QString& okText, const QString& cancelText,
	const QString& title, ModifyControl modify, quint32 seconds):
	QDialog(parent)
	, ui(new Ui::MsgBoxDialog)
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
	ui->titleBar->SetTitleName(title);
	setWindowTitle(ui->titleBar->GetTitleName());
	
	ui->label->setText(text);
	QPushButton* ok = ui->buttonBox->button(QDialogButtonBox::Ok);
	QPushButton* cancel = ui->buttonBox->button(QDialogButtonBox::Cancel);
	ok->setCursor(Qt::PointingHandCursor);
	cancel->setCursor(Qt::PointingHandCursor);
	ui->buttonBox->layout()->setSpacing(16);
	ok->setMinimumSize(QSize(96, 32));
	cancel->setMinimumSize(QSize(96, 32));
	ok->setText(okText);
	cancel->setText(cancelText);
	ok->setDefault(false);
	cancel->setDefault(false);
	ok->setProperty("ButtonStyle", "normal");
	cancel->setProperty("ButtonStyle", "normal");

	if (modify) {
		modify(MSGBOX_OK, ok);
		modify(MSGBOX_CANCEL, cancel);
		modify(MSGBOX_CONTENT, ui->label);
		modify(MSGBOX_DIALOG, this);
	}
	ok->style()->polish(ok);
	cancel->style()->polish(cancel);

	//count down
	if (seconds > 0) {
		ui->label->setText(text + QString("(%1s)").arg(seconds));
		ui->label->setProperty("time", seconds);
		QTimer* countDown = new QTimer(this);
		connect(countDown, &QTimer::timeout, [=] {
			uint time = ui->label->property("time").toUInt();
			time--;
			if (time > 0) {
				ui->label->setText(text + QString("(%1s)").arg(time));
				ui->label->setProperty("time", time);
			}
			else {
				countDown->stop();
				if (ok->isDefault()) {
					ok->clicked();
				}
				else if (cancel->isDefault()) {
					cancel->clicked();
				}
			}
		});
		countDown->start(1000);
	}

	connect(ui->titleBar, &CustomTitle::SignalClose, [this] {
		setProperty("isClose", true);
	});
}

MsgBoxDialog::~MsgBoxDialog()
{
}

MsgBoxDialog::MsgBoxResult MsgBoxDialog::MsgBox(QWidget *parent, const QString& text, const QString& okText, const QString& cancelText,
	const QString& title, ModifyControl modify, quint32 seconds)
{
	MsgBoxDialog dialog(parent, text, okText, cancelText, title, modify, seconds);
	int result = dialog.exec();
	if (dialog.property("isClose").toBool()) {
		return MSGBOX_RESULT_CLOSE;
	}
	return (result == DialogCode::Accepted) ? MSGBOX_RESULT_OK : MSGBOX_RESULT_CANCEL;
}
MsgBoxDialog::MsgBoxResult MsgBoxDialog::ConfirmOK(QWidget *parent, const QString& text, const QString& okText, const QString& cancelText,
	const QString& title, ModifyControl modify)
{
	return MsgBox(parent, text, okText, cancelText, QString(), [modify](MsgBoxControl type, QWidget* control) {
		switch (type) {
		case MSGBOX_OK:
			((QPushButton*)control)->setDefault(true);
			((QPushButton*)control)->setFocus();
			((QPushButton*)control)->setProperty("ButtonStyle", "blue");
			control->style()->polish(control);
			break;

		}
		if (modify) {
			modify(type, control);
		}	
	});
}
MsgBoxDialog::MsgBoxResult MsgBoxDialog::ConfirmCancel(QWidget *parent, const QString& text, const QString& okText, const QString& cancelText,
	const QString& title, ModifyControl modify)
{
	return MsgBox(parent, text, okText, cancelText, QString(), [modify](MsgBoxControl type, QWidget* control) {
		switch (type) {
		case MSGBOX_CANCEL:
			((QPushButton*)control)->setDefault(true);
			((QPushButton*)control)->setFocus();
			((QPushButton*)control)->setProperty("ButtonStyle", "blue");
			control->style()->polish(control);
			break;
		}
		if (modify) {
			modify(type, control);
		}
	});
}
MsgBoxDialog::MsgBoxResult MsgBoxDialog::ConfirmNotice(QWidget *parent, const QString& text, const QString& okText, ModifyControl modify)
{
	return MsgBox(parent, text, okText, QString(), QString(), [modify](MsgBoxControl type, QWidget* control) {
		switch (type) {
		case MSGBOX_OK:
			((QPushButton*)control)->setDefault(true);
			((QPushButton*)control)->setFocus();
			((QPushButton*)control)->setProperty("ButtonStyle", "blue");
			control->style()->polish(control);
			break;
		case MSGBOX_CANCEL:
			control->setVisible(false);
			return;

		}
		if (modify) {
			modify(type, control);
		}	
	});
}