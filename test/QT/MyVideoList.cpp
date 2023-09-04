#include "MyVideoList.h"

#include <QString>
#include <QCryptographicHash>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QDateTime>

#include "AppDef.h"
#include "SdkManager.h"
#include "Application.h"
#include "VideoControl.h"
#include "TipsWidget.h"
#include "GlobalConfig.h"

#include <http/http-code.h>
#include <http/http-param.h>
#include <http/http-client.h>


////////////////////////////////////////////////////////////
MyVideoList::MyVideoList(QObject* parent)
	: QObject(parent)
{
	isExit = false;
	requestHttp = new std::thread(&MyVideoList::Run, this);

	VideoInfo::StartDownloadThread();
}

MyVideoList::~MyVideoList(void)
{
	//HttpManager::CloseManager();
	
}


class QuickThread : public QThread
{
public:
	explicit inline QuickThread(std::function<void()> _func)
		: func(_func)
	{
	}
private:
	virtual void run() override
	{
		func();
	}
	std::function<void()> func;
};

QThread* CreateQThread(std::function<void()> func)
{
	return new QuickThread(func);
}

QString MyVideoList::GetTokenSafeBlock(MyVideoList* object, const QString& vid)
{
	QString token;
	QEventLoop eventLoop;
	auto wait = [&]() {
		token = object->GetToken(vid);
		QMetaObject::invokeMethod(&eventLoop, "quit", Qt::QueuedConnection);
	};
	QScopedPointer<QThread> thread(new QuickThread(wait));
	thread->start();
	eventLoop.exec();
	thread->wait();
	return token;
}

static void RequestResult(int code, const PLVVideoRequestInfo infos[],
	int infosNum, const PLVVideoRequestPageInfo* pageInfo, void* data)
{
	do
	{
		MyVideoList* obj = (MyVideoList*)data;
		QMetaObject::invokeMethod(obj, "OnEndRequest", Qt::QueuedConnection);
		if (E_NO_ERR != code && E_ABORT_OPERATION != code) {
			slog_error("get video list error code:%d", code);
			QMetaObject::invokeMethod((QWidget*)App()->GetMainWindow(), "OnCloseLoadTip", Qt::QueuedConnection);
			QMetaObject::invokeMethod((QWidget*)App()->GetMainWindow(), "OnShowTips", Qt::QueuedConnection,
				Q_ARG(int, TipsWidget::TIP_WARN), Q_ARG(const QString&, QTStr("RequestListVideoError")));
			break;
		}
		std::list<SharedVideoPtr> items;
		for (int i = 0; i < infosNum; ++i) {
			if (61 != infos[i].status && 60 != infos[i].status) {
				// must release status
				continue;
			}
			SharedVideoPtr video = std::make_shared<VideoInfo>();
			video->vid = infos[i].vid ? infos[i].vid : std::string();
			video->imageUrl = infos[i].thumbnailURL ? infos[i].thumbnailURL : std::string();
			video->title = infos[i].title ? infos[i].title : std::string();
			video->duration = QString("%1").arg(infos[i].duration).toStdString();
			video->seed = infos[i].encrypt ? 1 : 0;
			video->size = infos[i].sourceFilesize;
			for (int j = 0; j < infos[i].ratesNum; ++j) {
				if (VIDEO_RATE_SOURCE != infos[i].rates[j].rate) {
					video->sizes.push_back(infos[i].rates[j].filesize);
				}
			}
			video->rateCount = (int)video->sizes.size();
			items.push_back(video);
		}
		if (items.empty()) {
			if (VIDEO_REQUEST_PAGE == pageInfo->type && pageInfo->totalPages > pageInfo->pageNumber) {
				obj->RequestAll(pageInfo->pageNumber + 1, pageInfo->pageSize);
				break;
			}
			slog_info("get video list empty");
			QMetaObject::invokeMethod((QWidget*)App()->GetMainWindow(), "OnCloseLoadTip", Qt::QueuedConnection);
			QMetaObject::invokeMethod((QWidget*)App()->GetMainWindow(), "OnShowTips", Qt::QueuedConnection,
				Q_ARG(int, TipsWidget::TIP_INFO), Q_ARG(const QString&, QTStr("RequestListVideoEmpty")));
			break;
		}
		for (auto& it : items) {
			QMetaObject::invokeMethod((QWidget*)App()->GetMainWindow(), "OnAppendMyVideo", Qt::QueuedConnection,
				Q_ARG(const SharedVideoPtr&, it), Q_ARG(bool, VIDEO_REQUEST_VID == pageInfo->type ? true : false));
		}

	} while (false);
	
	static bool firstComplete = false;
	if (!firstComplete) {
		firstComplete = true;
		QMetaObject::invokeMethod((QWidget*)App()->GetMainWindow(), "OnCompleteRequestVideo", Qt::QueuedConnection);
	}
}

void MyVideoList::RequestAll(int page/* = 1*/, int pageSize/* = 10*/)
{
	PLVVideoRequestParam param;
	param.type = VIDEO_REQUEST_PAGE;
	param.page = page;
	param.pageSize = pageSize;
	auto ret = PLVRequestVideoInfo(false, &param, [](
		int code, const PLVVideoRequestInfo infos[], 
		int infosNum, const PLVVideoRequestPageInfo* pageInfo, void* data) {
			RequestResult(code, infos, infosNum, pageInfo, data);
		}, this);
	if (E_NO_ERR == ret) {
		started = true;
	}
	else if (ret == E_STILL_REQUEST) {
		std::lock_guard<std::mutex> l(lock);
		taskList.push_back(Item{ page, pageSize, QString() });
	}
}

void MyVideoList::RequestVid(const QString& vid)
{
	PLVVideoRequestParam param;
	param.type = VIDEO_REQUEST_VID;
	auto vids = vid.toStdString();
	param.vids = vids.c_str();
	auto ret = PLVRequestVideoInfo(false, &param, [](
		int code, const PLVVideoRequestInfo infos[],
		int infosNum, const PLVVideoRequestPageInfo* pageInfo, void* data) {
			RequestResult(code, infos, infosNum, pageInfo, data);
		}, this);
	if (E_NO_ERR == ret) {
		started = true;
	}
	else if (ret == E_STILL_REQUEST) {
		std::lock_guard<std::mutex> l(lock);
		taskList.push_back(Item{ 1, 1, vid });
	}
}
void MyVideoList::Stop()
{

	{
		std::lock_guard<std::mutex> l(lock);
		taskList.clear();
	}
	isExit = true;

	requestHttp->join();
	delete requestHttp;

	VideoInfo::StopDownloadThread();
}

QString MyVideoList::GetToken(const QString& vid)
{
	QString url[2];
	switch (GlobalConfig::GetHttpRequest())
	{
	case FIRST_HTTP_REQUEST:
		url[0] = QString("http://%1").arg(APP_TOKEN_URL);
		url[1] = QString("https://%1").arg(APP_TOKEN_URL);
		break;
	case ONLY_HTTP_REQUEST:
		url[0] = QString("http://%1").arg(APP_TOKEN_URL);
		url[1] = QString("http://%1").arg(APP_TOKEN_URL);
		break;
	case FIRST_HTTPS_REQUEST:
		url[0] = QString("https://%1").arg(APP_TOKEN_URL);
		url[1] = QString("http://%1").arg(APP_TOKEN_URL);
		break;
	case ONLY_HTTPS_REQUEST:
		url[0] = QString("https://%1").arg(APP_TOKEN_URL);
		url[1] = QString("https://%1").arg(APP_TOKEN_URL);
		break;
	}
	for (int i = 0; i < 2; i++) {
		auto account = SdkManager::GetManager()->GetAccount();
		HttpParam param;
		param.Append("extraParams", "pc-sdk");
		param.Append("ts", std::to_string(HttpParam::GetTimestamp(false)));
		param.Append("userId", QT_TO_UTF8(account.userId));
		param.Append("videoId", QT_TO_UTF8(vid));
		param.Append("viewerId", QT_TO_UTF8(account.viewerId)); 
		//param.Append("viewerIp", "59.42.42.15");
		//param.Append("viewerName", HttpCode::EncodeUrlSalfBase64(QT_TO_UTF8(account.viewerName)));
		QString temp = account.secretKey + QT_UTF8(param.ToSign().c_str()) + account.secretKey;
		QString sign = QCryptographicHash::hash(QT_TO_UTF8(temp), QCryptographicHash::Md5).toHex().toUpper();
		param.Append("sign", QT_TO_UTF8(sign));

		HttpClient client(QT_TO_UTF8(url[i]), HTTP_POST);
		client.SetCaFile(QT_TO_UTF8(App()->GetCacertFilePath()));
		client.SetPostData(param.ToHttp());
		client.SetContentType("application/x-www-form-urlencoded"); 
		qDebug() << url[i] << "?" << param.ToHttp().c_str();
		std::string result;
		if (!client.Request(result)) {
			slog_error("get video token error msg:%s", client.GetErrorString().c_str());
			continue;
		}
		slog_debug("token:%s", result.c_str());
		if (std::string::npos == result.find("token")) {
			slog_error("get video token is null");
			continue;
		}
		QJsonParseError err;
		QJsonDocument jsonDocument = QJsonDocument::fromJson(result.c_str(), &err);
		if (err.error != QJsonParseError::NoError) {
			slog_error("get video token error:%s", result.c_str());
			continue;
		}
		if (!jsonDocument.isObject()) {
			slog_error("get video token error:%s", result.c_str());
			continue;
		}
		QJsonObject obj = jsonDocument.object();
		QString token = obj.value("data").toObject().value("token").toString();
		return token;
	}
	return QString();
}

void MyVideoList::Run(void)
{
	while (!isExit) {
		//QList<Item> temp;
		//{
		//	std::lock_guard<std::mutex> l(lock);
		//	temp.swap(taskList);
		//}
		if (taskList.isEmpty()) {
			std::this_thread::sleep_for(std::chrono::milliseconds(200));
			continue;
		}
		if (started) {
			std::this_thread::sleep_for(std::chrono::milliseconds(200));
			continue;
		}
		bool have = false;
		Item item{0, 0, QString()};
		{
			std::lock_guard<std::mutex> l(lock);
			auto it = taskList.begin();
			if (it != taskList.end()) {
				item = Item{ it->page, it->count, it->vid };
				have = true;
				taskList.erase(it);
			}
		}
		if (have) {
			item.vid.isEmpty() ? RequestAll(item.page, item.count) : RequestVid(item.vid);
		}
		//for (auto & it : temp) {
		//	Request(it.page, it.count, it.vid);
		//}
		//temp.clear();
	}
}

void MyVideoList::OnEndRequest()
{
	started = false;
}

//void MyVideoList::Request(int page, int count, const QString& vid)
//{
//	auto account = SdkManager::GetManager()->GetAccount();
//	QString url;
//	QString host = QString(APP_MY_VIDEOLIST_URL).arg(account.userId);
//	switch (GlobalConfig::GetHttpRequest())
//	{
//	case FIRST_HTTP_REQUEST:
//	case ONLY_HTTP_REQUEST:
//		url = QString("http://%1").arg(host);
//		break;
//	case FIRST_HTTPS_REQUEST:
//	case ONLY_HTTPS_REQUEST:
//		url = QString("https://%1").arg(host);
//		break;
//	}
//	HttpParam param;
//	param.Append("ptime", std::to_string(HttpParam::GetTimestamp(false)));
//	param.Append("userid", QT_TO_UTF8(account.userId));
//	param.Append("numPerPage", (long long)count);
//	param.Append("pageNum", (long long)page);
//	if (!vid.isEmpty()) {
//		param.Append("vids", QT_TO_UTF8(vid));
//	}
//	QString temp = QT_UTF8(param.ToHttp().c_str());
//	temp += account.secretKey;
//	QString sign = QCryptographicHash::hash(QT_TO_UTF8(temp), QCryptographicHash::Sha1).toHex().toUpper();
//	param.Append("sign", QT_TO_UTF8(sign));
//
//	qDebug() << url << "?" << param.ToHttp().c_str();
//	std::string result;
//	HttpClient client(QT_TO_UTF8(url) + std::string("?") + param.ToHttp(), HTTP_GET, &isExit);
//	client.SetCaFile(QT_TO_UTF8(App()->GetCacertFilePath()));
//	bool ret = client.Request(result);
//	if (!ret) {
//		qCritical() << "http request error:" << client.GetErrorString().c_str();
//		QMetaObject::invokeMethod((QWidget*)App()->GetMainWindow(), "OnShowTips", Qt::QueuedConnection,
//			Q_ARG(int, TipsWidget::TIP_WARN), Q_ARG(const QString&, QTStr("RequestListVideoError")));
//	}
//	else {
//		ParseResult(ret, page, count, QT_UTF8(result.c_str()));
//	}
//}
//
//void MyVideoList::ParseResult(bool result, int page, int count, const QString& data)
//{
//	/*
//	{
//	code: 200,
//		status : "success",
//		message : "success",
//		data : [
//	{
//	tag: "",
//		mp4 : "http://mpv.videocc.net/0d8b255141/d/0d8b2551413fca8bec91293c431eabcd_1.mp4",
//		title : "111111.1111111111111111111.111",
//		df : 1,
//		times : "0",
//		vid : "0d8b2551413fca8bec91293c431eabcd_0",
//		mp4_1 : "http://mpv.videocc.net/0d8b255141/d/0d8b2551413fca8bec91293c431eabcd_1.mp4",
//		cataid : "1",
//		swf_link : "http://player.polyv.net/videos/0d8b2551413fca8bec91293c431eabcd_0.swf",
//		status : "61",
//		seed : 0,
//		playerwidth : "600",
//		duration : "00:00:42",
//		first_image : "http://img.videocc.net/uimage/0/0d8b255141/d/0d8b2551413fca8bec91293c431eabcd_0_b.jpg",
//		original_definition : "768x432",
//		context : "",
//		playerheight : "337",
//		ptime : "2020-11-19 10:32:07",
//		source_filesize : 2159949,
//		filesize : [
//					   2192034
//				   ],
//		md5checksum : "d06ceb0cc8ae3b3649f65136da27187d",
//						   hls : [
//									 "http://hls.videocc.net/0d8b255141/d/0d8b2551413fca8bec91293c431eabcd_1.m3u8"
//								 ],
//						   uploader : {
//									 email: "test@polyv.net",
//										 name : "test",
//										 role : "主账号"
//									 },
//										 keepsource : "",
//										 cataname : "默认分类"
//	}
//			   ],
//		total: 1
//	}*/
//	int code = 0;
//	int pageSize = 0;
//	std::list<SharedVideoPtr> items;
//	std::string json = QT_TO_UTF8(data);
//	qDebug() << json.c_str();
//	do
//	{
//		if (!result) {
//			break;
//		}
//		result = false;
//		QJsonParseError err;
//		QJsonDocument jsonDocument = QJsonDocument::fromJson(json.c_str(), &err);
//		if (err.error != QJsonParseError::NoError) {
//			break;
//		}
//		if (!jsonDocument.isObject()) {
//			break;
//		}
//
//		QJsonObject obj = jsonDocument.object();
//		if (200 != obj["code"].toInt()) {
//			code = obj["code"].toInt();
//			json = QT_TO_UTF8(obj["message"].toString());
//			break;
//		}
//		QJsonArray arr = obj["data"].toArray();
//
//		pageSize = arr.size();
//		for (int i = 0; i < arr.size(); ++i) {
//			QJsonObject item = arr.at(i).toObject();
//
//			int status = item["status"].toString().toInt();
//			if (61 != status && 60 != status) {
//				// must release status
//				continue;
//			}
//
//			SharedVideoPtr video = std::make_shared<VideoInfo>();
//			video->vid = QT_TO_UTF8(item["vid"].toString());
//			video->videoUrl = QT_TO_UTF8(item["mp4"].toString());
//			video->imageUrl = QT_TO_UTF8(item["first_image"].toString());
//			video->title = QT_TO_UTF8(item["title"].toString());
//			video->duration = QT_TO_UTF8(item["duration"].toString());
//			video->seed = item["seed"].toInt();
//			video->rateCount = item["df"].toInt();
//			video->size = (long long)item["source_filesize"].toDouble();
//			auto arr = item["filesize"].toArray();
//			auto count = arr.size();
//			for (int j = 0; j < count; ++j) {
//				video->sizes.push_back((long long)arr.at(j).toDouble());
//			}
//			items.push_front(video);
//		}
//		result = true;
//	} while (false);
//	if (!result) {
//		slog_error("get video list error code:%d, msg:%s",
//			code, json.c_str());
//		QMetaObject::invokeMethod((QWidget*)App()->GetMainWindow(), "OnCloseLoadTip", Qt::QueuedConnection);
//		QMetaObject::invokeMethod((QWidget*)App()->GetMainWindow(), "OnShowTips", Qt::QueuedConnection,
//			Q_ARG(int, TipsWidget::TIP_WARN), Q_ARG(const QString&, QTStr("RequestListVideoError")));
//		return;
//	}
//	if (items.empty()) {
//		if (pageSize == count && 1 != count) {
//			std::lock_guard<std::mutex> l(lock);
//			taskList.push_back(Item{ page + 1, 10, QString() });
//			return;
//		}
//		slog_info("get video list empty");
//		QMetaObject::invokeMethod((QWidget*)App()->GetMainWindow(), "OnCloseLoadTip", Qt::QueuedConnection);
//		QMetaObject::invokeMethod((QWidget*)App()->GetMainWindow(), "OnShowTips", Qt::QueuedConnection,
//			Q_ARG(int, TipsWidget::TIP_INFO), Q_ARG(const QString&, QTStr("RequestListVideoEmpty")));
//		return;
//	}
//	//auto testtime = HttpParam::GetTimestamp(false);
//	//qDebug() << "the begin time:" << testtime;
//	for (auto & it : items) {
//		QMetaObject::invokeMethod((QWidget*)App()->GetMainWindow(), "OnAppendMyVideo", Qt::QueuedConnection,
//			Q_ARG(const SharedVideoPtr&, it), Q_ARG(bool, 1 == count ? true : false));
//	}
//	//qDebug() << "the end time:" << HttpParam::GetTimestamp(false) - testtime;
//	if (!items.empty()) {
//		static bool firstComplete = false;
//		if (!firstComplete) {
//			firstComplete = true;
//			QMetaObject::invokeMethod((QWidget*)App()->GetMainWindow(), "OnCompleteRequestVideo", Qt::QueuedConnection);
//		}
//	}
//}

