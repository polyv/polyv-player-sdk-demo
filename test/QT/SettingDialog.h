#pragma once

#include <QDialog>

namespace Ui {
class SettingDialog;
}



class SettingDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SettingDialog(QWidget *parent = 0);
    ~SettingDialog();

protected:
	void closeEvent(QCloseEvent* e) override;

private slots:
	void on_logBtn_clicked(void);
	void on_videoBtn_clicked(void);
	void on_screenshotBtn_clicked(void);
	
	void on_permission_clicked();

	void on_osd_clicked(bool enable);
	void on_logo_clicked(bool enable);
	void on_cache_clicked(bool enable);
	
	
private:
	void InitTaskCount();
	void InitRetryCount();
	void InitVideoOutput();
	void InitLogLevel();
	void InitHttpRequest();
	void InitPlayRate();

	void InitOSDConfig(void);
	void InitLogoConfig(void);
	void InitCacheConfig(void);
private:
    Ui::SettingDialog *ui;
};

