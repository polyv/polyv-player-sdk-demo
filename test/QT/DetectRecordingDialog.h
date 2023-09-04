#pragma once
#include <QSet>
#include <QDialog>

namespace Ui {
class DetectRecordingDialog;
}

class DetectRecordingDialog : public QDialog
{
    Q_OBJECT

public:
    explicit DetectRecordingDialog(QWidget *parent = 0);
    ~DetectRecordingDialog();


public:
	void DetectHardwareRecording(int type, QString device);
	void DetectSoftwareRecording(int type, QString software);

private slots:
	void on_enable_clicked(void);
	void on_disable_clicked(void);
	void on_ignore_clicked(void);
	
private:
	void Refresh();
	
private:
    Ui::DetectRecordingDialog *ui;
	QSet<QString> setHardwares;
	QSet<QString> setSoftwares;
};

