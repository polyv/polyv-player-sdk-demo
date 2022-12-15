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
	void on_pathButton_clicked(void);
	void on_screenshotButton_clicked(void);
	void on_openShotDir_clicked(void);
	void on_openDownloadDir_clicked(void);

	void on_osd_clicked(bool enable);
	void on_logo_clicked(bool enable);
	void on_cache_clicked(bool enable);
	
private:
	void InitOSDConfig(void);
	void InitLogoConfig(void);
	void InitCacheConfig(void);
private:
    Ui::SettingDialog *ui;
};

