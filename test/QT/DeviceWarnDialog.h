#pragma once

#include <QDialog>

namespace Ui {
class DeviceWarnDialog;
}

class DeviceWarnDialog : public QDialog
{
    Q_OBJECT

public:
    explicit DeviceWarnDialog(const QString& id, QWidget *parent = 0);
    ~DeviceWarnDialog();


	void SetInfo(const QString& info);
//signals:
//	void SignalClose();
//
//protected:
//	void closeEvent(QCloseEvent* e) override;
private slots:
	void on_enable_clicked(void);
	void on_disable_clicked(void);
	void on_ignore_clicked(void);
	
private:
    Ui::DeviceWarnDialog *ui;

	QString devId;
};

