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
	void RequestAll(int page = 1, int pageSize = 10);	
	void RequestVid(const QString& vid);

	QString GetToken(const QString& vid);

	void Stop();

	static QString GetTokenSafeBlock(MyVideoList* object, const QString& vid);

private:
	void Run(void);
	//void Request(int page, int count, const QString& vid);
	//void ParseResult(bool result, int page, int count, const QString& data);

private slots:
	void OnEndRequest();
private:
	//std::thread::id taskId = std::thread::id();
	bool isExit = false;
	std::mutex lock;

	volatile bool started = false;
	struct Item {
		int page;
		int count;
		QString vid;
	};
	QList<Item> taskList;
	std::thread* requestHttp = nullptr;
};

