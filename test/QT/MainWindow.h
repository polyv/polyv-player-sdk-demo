#pragma once

#include <QWidget>
#include <QPointer>
#include <QTableWidgetItem>

#include "VideoControl.h"

class MyVideoList;
class ParamDialog;
class TipsWidget;
class DeviceWarnDialog;


namespace Ui {
class MainWindow;
}

class MainWindow : public QWidget
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    bool Init(void);
protected:
	virtual void keyPressEvent(QKeyEvent* e) override;
	virtual void resizeEvent(QResizeEvent *event) override;
	virtual void closeEvent(QCloseEvent* e) override;
private slots:
	void on_paramButton_clicked(void);
	void on_settingButton_clicked(void);
	void on_refreshButton_clicked(void);
	void on_vidButton_clicked(void);
	//void on_stackedWidget_currentChanged(int index);

	void OnShowListVideo(void);
	void OnReturnListVideo(void);
	void OnShowTips(int level, const QString& msg);
	void OnShowToast(const QString& msg);

	void OnPlayVideo(bool local, const SharedVideoPtr& video);
	void OnChangeRatePlayVideo(int rate, int seekMillisecond, const SharedVideoPtr& video);

	void OnCompleteRequestVideo(void);
	void OnAppendMyVideo(const SharedVideoPtr& video, bool focus);
	void OnAppendDownloadVideo(int rate, const SharedVideoPtr& video);
	void OnRemoveDownloadVideo(const SharedVideoPtr& video);
	void OnAppendLocalVideo(int rate, const SharedVideoPtr& video);
	void OnRemoveLocalVideo(const SharedVideoPtr& video);
	void OnPlaylistLocalVideo(int opt, const SharedVideoPtr& video);

	void OnFullScreen(void);
	void OnExitFullScreen(void);

	void OnCloseLoadTip(void);

	void OnHighlightItem(void);

#ifdef _WIN32
	void OnHDMIDevice(QString id, int type);
	void OnPluginInject(void);
	void OnUpdateSoftwareRecord(bool enable);
	void OnUpdateHdmiRecord(bool enable);
#endif // _WIN32

	void OnEnableWindow(const QString& id, bool enable);
	void OnEnableWindows();

	
private:
	void ShowPlayer(void);
	void ShowListVideo(void);
	bool InsertMyVideoItem(const SharedVideoPtr& video, bool focus);
	bool InsertDownloadItem(int rate, const SharedVideoPtr& video);
	bool InsertLocalItem(int rate, const SharedVideoPtr& video);

	void StartItemHighlight(const std::string& vid);
	void StopItemHighlight(void);
private:
	void TestMyVideoItem(void);
private:
    Ui::MainWindow *ui;
	QPushButton* returnVideo = nullptr;
	QPushButton* showVideo = nullptr;
	QPointer<ParamDialog> paramDialog;

	MyVideoList* myVideoList;

	TipsWidget* loadTipWidget = nullptr;

	QTimer* itemHighlightTimer = nullptr;
	int curHighlight = 0;
	std::string curHighlightVid;

	struct Item {
		QTableWidgetItem* item;
		SharedVideoPtr video;
		QSet<int> setVideoRate;
	};
	typedef std::shared_ptr<Item> SharedItemPtr;
	QMap<std::string, SharedItemPtr> mapMyVideos;
	QMap<std::string, SharedItemPtr> mapLocalVideos;
	QMap<std::string, SharedItemPtr> mapDownloadVideos;

	void UpdateItemTitle(QTableWidget* table, const QMap<std::string, SharedItemPtr>& map, 
		const std::string& vid, const std::string& title);
	
	bool isFullscreen = false;
	bool isFullscreenMax = false;
	QRect fullscreenNormalRect;

	const int kMyVideoItemCount = 5;


	enum {
		HDMI_ENABLE_WIN = 0,
		HDMI_DISABLE_WIN,
		HDMI_IGNORE
	};
	QMap<QString, int> mapDisables;
	bool isEnableWindow = true;
	void EnableWindow(bool enable);
};
