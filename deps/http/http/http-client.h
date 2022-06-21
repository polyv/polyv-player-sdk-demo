#pragma once

#include<string>
#include<vector>
#include<atomic> 

enum HTTPTYPE {
	HTTP_GET,
	HTTP_DEL,
	HTTP_POST,
	HTTP_UPLOAD,
	HTTP_DOWNLOAD,
	HTTP_POSTFILEDS,
};
typedef void(*HttpDebugCB)(char* data, size_t size);
typedef void(*HttpProcessCB)(double dnow, double dtotal, void* user);

class  HttpClient {
public:
	HttpClient(const std::string& url, HTTPTYPE type, const bool* abort = nullptr);
	~HttpClient();

	static void SetDebugTrace(bool debug, HttpDebugCB cb);

	bool Request(std::string& result);
	long long RequestContentLength(void);

	HTTPTYPE GetHttpType() const { 
		return httpType; 
	}
	int GetErrorCode(void) const {
		return errorCode;
	}
	std::string GetErrorString(void) const {
		return errorString;
	}
	void SetSSL(bool ssl) {
		this->verifySSL = ssl;
	}
	void SetCaFile(const std::string &file) {
		this->caFile = file;
	}
	void SetCaPath(const std::string &path) {
		this->caPath = path;
	}
	void SetUserAgent(const std::string& value) {
		this->userAgent = value;
	}
	void SetContentType(const std::string& value) {
		this->contentType = value;
	}
	void SetPostData(const std::string& value) {
		this->postData = value;
	}
	void SetHttpHeaders(const std::vector<std::string> &params) {
		this->headerParams = params;
	}
	void SetHttpForms(const std::vector<std::pair<std::string, std::string>> &params) {
		this->formParams = params;
	}
	void SetTimeout(int second) {
		this->timeout = second;
	}
	void SetUploadTimeout(int second) {
		this->uploadTimeout = second;
	}
	void SetDownloadTimeout(int second) {
		this->downloadTimeout = second;
	}
	void SetConnectTimeout(int second) {
		this->connectTimeout = second;
	}
	void SetDownload(const std::string& fullName, HttpProcessCB cb, void* user) {
		this->fileName = fullName;
		this->processCB = cb;
		this->processContext = user;
	}
	void SetResumeDownload(bool resume, long long _contentLength) {
		resumeDownload = resume;
		this->contentLength = _contentLength;
	}
	void Process(double dnow, double dtotal);

    struct Proxy {
        enum Scheme { HTTP, HTTPS };
        Scheme scheme;
        unsigned int port;
        std::string host;
        std::string username;
        std::string password;
    };
    void SetProxy(const struct Proxy &proxy)
    {
        netProxy = proxy;
    }

	void Abort(const bool* abort) {
		pisAbort = abort;
	}

	bool IsAbort(void) const {
		return pisAbort ? *pisAbort : false;
	}
private:
	bool HttpGet();
	bool HttpDel();
	bool HttpPost();
	bool HttpPostFile();
	bool HttpDownload();

	long long GetFileSize(const char* fileName);
	
	struct HttpParam;
	bool InitCurl(const std::string& connectType, HttpParam* param);
	bool ReleaseCurl(HttpParam* param);

	const bool* pisAbort = nullptr;

	HttpParam* httpParam = nullptr;

	HTTPTYPE httpType;
	std::string httpUrl;
	std::string userAgent;
	std::string postData;
	std::string fileName;
	std::string contentType;
	std::string caFile;
	std::string caPath;
	std::vector<std::string> headerParams;
	std::vector<std::pair<std::string, std::string>> formParams;
    struct Proxy netProxy;

	long long contentLength = 0;

	bool verifySSL = true;
	bool resumeDownload = false;

	int timeout=30;// second
	int uploadTimeout = 2 * 60;// second
	int downloadTimeout = 2 * 60;// second
	int connectTimeout = 30;// second

	int errorCode = 0;
	std::string errorString;
	std::string resultString;

	HttpProcessCB processCB = nullptr;
	void* processContext = nullptr;

	static bool debugTrace;
	static HttpDebugCB debugCB;

private:  // emphasize the following members are private
	HttpClient(const HttpClient&) = delete;
	const HttpClient& operator=(const HttpClient&) = delete;


	static size_t HttpFileFuction(char* data, size_t size, size_t nmemb, void* user);
	static size_t HttpStringFuction(char* data, size_t size, size_t nmemb, void* user);
	static int HttpProcessFuction(void* user, double dtotal, double dnow, double, double);
};


