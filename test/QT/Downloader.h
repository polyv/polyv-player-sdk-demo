#pragma once

#include <QObject>
#include <plv-player-core.h>

//////////////////////////////////////////////////
class Downloader : public QObject {
	Q_OBJECT
public:
	explicit Downloader(QObject* parent = nullptr);
	~Downloader(void);

public:
	int SetVideo(const QString& vid, const QString& path, int rate);
	int Reset();
	int Start(bool autoDownRate);
	int Pause();
	int Stop();
	int Delete();
	bool IsDownloading();

signals:
	void SignalError(int code);
	void SignalResult(int rate, int code);
	void SignalProgress(qint64 receivedBytes, qint64 totalBytes);
private slots:
	void OnError(QString vid, int code);
	void OnResult(QString vid, int rate, int code);
	void OnProgress(QString vid, qint64 receivedBytes, qint64 totalBytes);
private:
	PLVDownloadPtr downloader = nullptr;

private:
	Downloader(const Downloader&) = delete;
	Downloader(Downloader&&) = delete;
	Downloader& operator=(const Downloader&) = delete;
	Downloader& operator=(Downloader&&) = delete;
};