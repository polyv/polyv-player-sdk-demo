#include "Downloader.h"

Downloader::Downloader(QObject* parent/* = nullptr*/)
	: QObject(parent)
{
	downloader = PLVDownloadCreate();
	PLVDownloadSetErrorHandler(downloader, [](const char* vid, int code, void* data) {
		(void)vid;
		Downloader* obj = (Downloader*)data;
		QMetaObject::invokeMethod(obj, "OnError", Qt::QueuedConnection,
			Q_ARG(QString, vid), Q_ARG(int, code));
	}, this);
	PLVDownloadSetProgressHandler(downloader, [](const char* vid, long long receivedBytes, long long totalBytes, void* data) {
		(void)vid;
		Downloader* obj = (Downloader*)data;
		QMetaObject::invokeMethod(obj, "OnProgress", Qt::QueuedConnection,
			Q_ARG(QString, vid), Q_ARG(qint64, receivedBytes), Q_ARG(qint64, totalBytes));
	}, this);
	PLVDownloadSetResultHandler(downloader, [](const char* vid, int rate, int code, void* data) {
		(void)vid;
		Downloader* obj = (Downloader*)data;
		QMetaObject::invokeMethod(obj, "OnResult", Qt::QueuedConnection,
			Q_ARG(QString, vid), Q_ARG(int, rate), Q_ARG(int, code));
	}, this);
}
Downloader::~Downloader(void)
{
	PLVDownloadDestroy(downloader);
	downloader = nullptr;
}

int Downloader::SetVideo(const QString& vid, const QString& path, int rate)
{
	return PLVDownloadSetVideo(downloader, vid.toStdString().c_str(), path.toStdString().c_str(), rate);
}
int Downloader::Reset()
{
	return PLVDownloadResetHandler(downloader);
}
int Downloader::Start(bool autoDownRate)
{
	return PLVDownloadStart(downloader, autoDownRate);
}
int Downloader::Pause()
{
	return PLVDownloadPause(downloader);
}
int Downloader::Stop()
{
	return PLVDownloadStop(downloader);
}
int Downloader::Delete()
{
	return PLVDownloadDelete(downloader);
}
bool Downloader::IsDownloading()
{
	return PLVDownloadIsDownloading(downloader);
}

void Downloader::OnError(QString vid, int code)
{
	(void)vid;
	emit SignalError(code);
}
void Downloader::OnResult(QString vid, int rate, int code)
{
	(void)vid;
	emit SignalResult(rate, code);
}
void Downloader::OnProgress(QString vid, qint64 receivedBytes, qint64 totalBytes)
{
	(void)vid;
	emit SignalProgress(receivedBytes, totalBytes);
}

