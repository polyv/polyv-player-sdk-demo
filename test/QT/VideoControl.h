#pragma once
#include <string>
#include <memory>
#include <atomic>
#include <thread>
#include <queue>
#include <QWidget>
#include <QLabel>
#include <QTimer>
#include <QThread>
#include <QPointer>
#include <QMetaType>
#include <QPushButton>
#include <QProgressBar>

#include "SdkManager.h"


class VideoInfo;
typedef std::shared_ptr<VideoInfo> SharedVideoPtr;
Q_DECLARE_METATYPE(SharedVideoPtr)



////////////////////////////////////////////////
class DownloadManager {
	//Q_OBJECT
public:
	DownloadManager();

	
public:
	void Start(void);
	void Stop(void);

	int AppendTask(const SharedVideoPtr& video, const std::string& url, const std::string& filename);

private:
	void Run(void);

private:
	//std::string fileName;
	//std::string downloadUrl;
	bool isExit = false;
	int connectTimeout = 30;
	int downloadTimeout = 60 * 3;
	//FILE* file = NULL;
	struct Item {
		int taskId = 0;
		std::string url;
		std::string filename;
		SharedVideoPtr video;
		Item() {}
		Item(int id, const std::string& _url, const std::string& _file, const SharedVideoPtr& _video) {
			taskId = id;
			url = _url;
			filename = _file;
			video = _video;
		}
	};
	std::mutex mutex;
	std::thread* download = nullptr;
	std::queue<Item> queueTask;
};

////////////////////////////////////////////////
enum {
	ATTR_VIDEO_TITLE = 0,
	ATTR_VIDEO_IMAGE,
	ATTR_VIDEO_DOWNLOAD_STATUS,
};
enum {
	DOWNLOAD_OK = 0,
	DOWNLOAD_RUN,
	DOWNLOAD_PAUSE,
	DOWNLOAD_ERROR,
	DOWNLOAD_CANCEL
};


class VideoInfo : public QObject,  public std::enable_shared_from_this<VideoInfo> {
	Q_OBJECT
public:
	VideoInfo();
	~VideoInfo(void);

	static void StartDownloadThread(void);
	static void StopDownloadThread(void);

	std::string vid;
	std::string videoUrl;
	std::string imageUrl;
	std::string imageFile;
	std::string title;
	std::string duration;

	std::string filePath;
	int seed = 0;
	int rate = VIDEO_RATE_AUTO;
	int rateCount = 0;
	long size = 0;

	int downloadStatus = DOWNLOAD_OK;

	int taskId = 0;
	
	bool SetTitle(const std::string& title, bool signal = true);
	bool SetImage(const std::string& url);
	bool SetDownloadStatus(int status, bool signal = true);

	void DownloadImage(void);
	void DownloadImageResult(int taskId, bool result);
signals:
	void SignalAttributeChanged(int attribute, const SharedVideoPtr& video);

private slots:
	void OnDownloadImage(std::string fileName);
private:
	std::string filename;

	static DownloadManager* downloadImage;
};



////////////////////////////////////////////////
class VideoCoverWidget : public QWidget {
	Q_OBJECT
public:
	VideoCoverWidget(QWidget* parent, bool local, const SharedVideoPtr& video);

private slots:
	void OnAttributeChanged(int attribute, const SharedVideoPtr& video);
private:
	bool localPlay;
	QLabel* imageLabel = nullptr;
	SharedVideoPtr videoInfo;
};

////////////////////////////////////////////////
class VideoTitleWidget : public QWidget {
	Q_OBJECT
public:
	VideoTitleWidget(QWidget* parent, bool showVID, const SharedVideoPtr& video);

protected:
	virtual void resizeEvent(QResizeEvent *event) override;

private slots:
	void OnAttributeChanged(int attribute, const SharedVideoPtr& video);
private:
	QLabel* vidLabel = nullptr;
	QLabel* textLabel = nullptr;
	SharedVideoPtr videoInfo;
};

////////////////////////////////////////////////
class VideoDownloadWidget : public QWidget {
	Q_OBJECT
public:
	VideoDownloadWidget(QWidget* parent, const SharedVideoPtr& video, int rate);
	~VideoDownloadWidget();

private slots:
	void OnDownloadErrorHandler(int code);
	void OnDownloadProgressHandler(long long receivedBytes, long long totalBytes);
	void OnDownloadResultHandler(int rate, int code);

	void OnAttributeChanged(int attribute, const SharedVideoPtr& video);

	void OnSpeedTimer(void);
private:
	int curRate;
	SharedVideoPtr videoInfo;
	PLVDownloadPtr downloadObj = nullptr;
	QProgressBar* progressBar;
	QLabel* tipsLabel;
	QLabel* speedLabel;

	std::atomic<long long> totalSize;
	std::atomic<long long> currentSize;
	std::atomic<long long> receivedSize;
	QTimer* speedTimer = nullptr;
};

////////////////////////////////////////////////
class VideoActionWidget : public QWidget{
	Q_OBJECT
public:
	enum ActionType {
		ACTION_LOCAL = 0,
		ACTION_DOWNLOAD,
		ACTION_DOWNLOADING,
	};
	VideoActionWidget(QWidget* parent, ActionType action, const SharedVideoPtr& video);

private slots:
	void OnAttributeChanged(int attribute, const SharedVideoPtr& video);
private:
	ActionType actionType;
	SharedVideoPtr videoInfo;
	QPushButton* downloadBtn;
	QPushButton* appendBtn;
	QPushButton* openBtn;
	QPushButton* deleteBtn;

	QPushButton* pauseBtn;
	QPushButton* retryBtn;
};
