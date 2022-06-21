#pragma once
#include <thread>
#include <vector>
#include <mutex>
#include <queue>
#include <future>
#include <string>
#include <map>

#include"http-client.h"
// if code -1 for cancel request
typedef void(*HttpResultCB)(bool result, int code, const std::string& data, void* user);
class HttpTask{
public:
	HttpTask(HttpClient* task, HttpResultCB result, void* user);
	~HttpTask(void);

	void SetRetryCount(int count);
	std::thread::id Start(void);
	
	void Cancel(void);
private:
	void Run(void);
	void Wait(void);
	bool IsRun(void) const {
		return threadRun;
	}

	volatile bool threadRun = false;
	//std::thread* worker = nullptr;
	std::thread::id taskId = std::thread::id();

	int retryCount = 1;
	bool isCancel = false;
	HttpClient* client = nullptr;
	HttpResultCB resultCB = nullptr;
	void* context = nullptr;

private:  // emphasize the following members are private
	HttpTask(const HttpTask&) = delete;
	const HttpTask& operator=(const HttpTask&) = delete;
};


// task object will dynamic allocation and delete inside
// client object will dynamic allocation and delete inside
class HttpManager{
public:
	static HttpManager *Instance();
	static void Destory();

	std::thread::id Request(HttpClient* client, HttpResultCB result, void* user);

	std::thread::id RequestTask(HttpTask* task);

	bool IsExist(const std::thread::id& taskId);

	void Cancel(const std::thread::id& taskId);
	void Clear(void);
protected:
	HttpManager();
   ~HttpManager();
private:
	void Remove(const std::thread::id& taskId);
	void RemoveTask(HttpTask* task);

	std::mutex lock;
	std::map<std::thread::id, HttpTask*> mapTask;

	static HttpManager* manager;

	friend class HttpTask;
};
