#include "http/http-manager.h"

#include <memory>
#include <curl/curl.h>

HttpTask::HttpTask(HttpClient* task, HttpResultCB result, void* user)
	: client(task)
	, resultCB(result)
	, context(user)
{
}

HttpTask::~HttpTask()
{
	Cancel();
	delete client;
	client = nullptr;
}

void HttpTask::SetRetryCount(int count)
{
	retryCount = (count > 0 ? count : 1);
}

void HttpTask::Cancel(void)
{
	client->Abort(&isCancel);
	isCancel = true;
	Wait();
}

void HttpTask::Wait(void)
{
	//if (!worker) {
	//	return;
	//}
	while (threadRun) {
		std::this_thread::yield();
	}
	//worker->join();
	//delete worker;
	//worker = nullptr;
}

std::thread::id HttpTask::Start(void)
{
	if (IsRun()) {
		return taskId;
	}
	threadRun = true;
	std::thread th(&HttpTask::Run, this);
	taskId = th.get_id();
	th.detach();
	return taskId;
	//worker = new std::thread(, this);
	//return worker->get_id();
}

void HttpTask::Run(void)
{
	bool ret = false;
	std::string result;
	for (int i = 0; i < retryCount; ++i) {
		if (isCancel) {
			break;
		}
		result.clear();
		ret = client->Request(result);
		if (ret) {
			break;
		}
	}
	if (!isCancel) {
		resultCB(ret, client->GetErrorCode(), result, context);
	}
	else {
		resultCB(false, -1, std::string(), context);
	}
	threadRun = false;
	HttpManager::Instance()->Remove(taskId);
}


HttpManager* HttpManager::manager = nullptr;
HttpManager* HttpManager::Instance()
{
	if (nullptr == manager) {
		manager = new HttpManager();
	}
	return manager;

}
void HttpManager::Destory()
{
	if (!manager) {
		return;
	}
	manager->Clear();
	delete manager;
	manager = nullptr;
}

HttpManager::HttpManager()
{
	curl_global_init(CURL_GLOBAL_ALL);
}
HttpManager::~HttpManager()
{
	Clear();
	curl_global_cleanup();
}

std::thread::id HttpManager::Request(HttpClient* client, HttpResultCB result, void* user)
{
	return RequestTask(new HttpTask(client, result, user));
}

std::thread::id HttpManager::RequestTask(HttpTask* task)
{
	std::thread::id taskId = task->Start();
	std::lock_guard<std::mutex> l(lock);
	mapTask.insert(std::pair<std::thread::id, HttpTask*>(taskId, task));
	return taskId;
}

bool HttpManager::IsExist(const std::thread::id& taskId)
{
	std::lock_guard<std::mutex> l(lock);
	auto it = mapTask.find(taskId);
	return (it == mapTask.end()) ? false : true;
}

void HttpManager::Cancel(const std::thread::id& taskId)
{
	std::lock_guard<std::mutex> l(lock);
	auto it = mapTask.find(taskId);
	if (it == mapTask.end()) {
		return;
	}
	auto task = it->second;
	mapTask.erase(it);
	task->Cancel();
	RemoveTask(task);
}

void HttpManager::Clear(void)
{
	std::map<std::thread::id, HttpTask*> temp;
	{
		std::lock_guard<std::mutex> l(lock);
		temp.swap(mapTask);
		mapTask.clear();
	}
	for (auto & it : temp) {
		it.second->Cancel();
		RemoveTask(it.second);
	}
	temp.clear();
}

void HttpManager::Remove(const std::thread::id& taskId)
{
	std::lock_guard<std::mutex> l(lock);
	auto it = mapTask.find(taskId);
	if (it == mapTask.end()) {
		return;
	}
	auto task = it->second;
	mapTask.erase(it);
	RemoveTask(task);
}

void HttpManager::RemoveTask(HttpTask* task)
{
	if (!task) {
		return;
	}
	delete task;
	task = nullptr;
}
