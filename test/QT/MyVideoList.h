#pragma once
#include <thread>
#include <atomic>
#include <mutex>
#include <QSet>
#include <QObject>

class MyVideoList : public QObject {
	Q_OBJECT
public:
	MyVideoList(QObject* parent = nullptr);
	~MyVideoList(void);

public:
	//void Request(const QString& vid);
	void RequestAll(void);	
	void RequestVid(const QString& vid);

	QString GetToken(const QString& vid);

	void Stop();

	static QString GetTokenSafeBlock(MyVideoList* object, const QString& vid);

private:
	void Run(void);
	void Request(int page, int count, const QString& vid);
	void ParseResult(bool result, int page, int count, const QString& data);
private:
	//std::thread::id taskId = std::thread::id();
	bool isExit = false;
	std::mutex lock;

	struct Item {
		int page;
		int count;
		QString vid;
	};
	QList<Item> vids;
	std::thread* requestHttp = nullptr;
};

