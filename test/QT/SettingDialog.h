#pragma once

#include <QDialog>

namespace Ui {
class SettingDialog;
}


struct OSDConfig {
	QString text;
	int textSize = 55;
	QString textColor = "#0ff000";
	int textAlpha = 255;
	bool border = true;
	QString borderColor = "#ff0000";
	int borderAlpha = 255;
	int borderWidth = 1;
	int animationEffect = 0;
	int displayDuration = 10;
	int displayInterval = 1;
	int fadeDuration = 1;

	bool enable = true;
};

struct CacheConfig {
	bool enable = true;
	bool change = false;
	int maxCacheBytes = -1;
	int maxCacheSeconds = -1;
};

class SettingDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SettingDialog(QWidget *parent = 0);
    ~SettingDialog();


	static void SetOSDText(const QString& text);
	static OSDConfig& GetOSDConfig();

	static CacheConfig& GetCacheConfig();

private slots:
	void on_ok_clicked(void);
	void on_cancel_clicked(void);
	void on_pathButton_clicked(void);
	void on_screenshotButton_clicked(void);

	void on_osd_clicked(bool enable);
	void on_cache_clicked(bool enable);
	
private:
	void InitOSDConfig(void);
	void InitCacheConfig(void);
private:
    Ui::SettingDialog *ui;

	static OSDConfig osdConfig;

	static CacheConfig cacheConfig;
};

