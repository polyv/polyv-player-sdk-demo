#pragma once

#include <QMainWindow>
#include <QMap>
#include <QPointer>
#include <QTableWidgetItem>

#include "VideoControl.h"

class MyVideoList;
class TipsWidget;
class MultiPlayerDialog;
class DetectRecordingDialog;


namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    bool Init(void);
	QString GetToken(const QString& vid);

	bool IsExistLocalFile(const QString& vid);
protected:
	virtual void closeEvent(QCloseEvent* e) override;
private slots:
	void on_paramButton_clicked(void);
	void on_settingButton_clicked(void);
	void on_refreshButton_clicked(void);
	void on_vidButton_clicked(void);
	void on_migrateButton_clicked(void);
	//void on_stackedWidget_currentChanged(int index);

	void OnShowListVideo(void);
	//void OnReturnListVideo(void);
	void OnShowTips(int level, const QString& msg);
	void OnShowToast(const QString& msg);

	void OnPlayVideo(bool local, const SharedVideoPtr& video, int rate);
	void OnChangeRatePlayVideo(int rate, int seekMillisecond, const SharedVideoPtr& video);
	//void OnOpenParamWindow(QString vid);

	void OnCompleteRequestVideo(void);
	void OnAppendMyVideo(const SharedVideoPtr& video, bool focus);
	void OnAppendDownloadVideo(int rate, const SharedVideoPtr& video);
	void OnRemoveDownloadVideo(int rate, const SharedVideoPtr& video);
	void OnAppendLocalVideo(int rate, const SharedVideoPtr& video);
	void OnRemoveLocalVideo(int rate, const SharedVideoPtr& video);
	void OnPlaylistLocalVideo(const SharedVideoPtr& video);

	void OnFullScreen(void);
	void OnExitFullScreen(void);

	void OnCloseLoadTip(void);

	void OnHighlightItem(void);

	void OnDetectHardwareRecording(int type, QString device);
	void OnDetectSoftwareRecording(int type, QString software);
	void OnEnableWindow(bool enable);
	
private:
	void ShowPlayer(void);
	void ShowListVideo(void);
	bool InsertMyVideoItem(const SharedVideoPtr& video, bool focus);
	bool InsertDownloadItem(int rate, const SharedVideoPtr& video);
	bool InsertLocalItem(int rate, const SharedVideoPtr& video);

	void StartItemHighlight(const std::string& vid);
	void StopItemHighlight(void);
	void SelectItem(int row, bool select);
	void CloseDialogs();

	QString CreateVid(int rate, const SharedVideoPtr& video);
	QString CreateVid(int rate, const std::string& vid);
private:
	void TestMyVideoItem(void);
private:
    Ui::MainWindow *ui;
	//QPushButton* returnVideo = nullptr;
	QPushButton* showVideo = nullptr;

	MyVideoList* myVideoList;

	TipsWidget* loadTipWidget = nullptr;

	QTimer* flickerTimer = nullptr;
	struct FlickerItem {
		int count;
		QTableWidgetItem* item;
	};
	using FlickerItemPtr = std::shared_ptr<FlickerItem>;
	QMap<std::string, FlickerItemPtr> mapFlicker;

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
	//void SelecteTableItem(QTableWidget* table, const SharedItemPtr& item, bool selecte);

	const int kMyVideoItemCount = 5;

	QPointer<DetectRecordingDialog> detectRecordingDialog;
};
