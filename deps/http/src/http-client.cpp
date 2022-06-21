#include "http/http-client.h"
#include <sstream>
#include <thread>
#include <memory>
#include <stdio.h>
#include <stdlib.h>
#include <curl/curl.h>
#ifdef WIN32
#include <io.h>
#else
#include <sys/stat.h>
#endif // WIN32

#if defined(_MSC_VER) && _MSC_VER >= 1400 // VC++ 8.0
// Disable warning about strdup being deprecated.
#pragma warning(disable : 4996)
#endif


bool HttpClient::debugTrace = false;
HttpDebugCB HttpClient::debugCB = nullptr;

static int HttpDebugFuction(void *handle, curl_infotype type, char *data, size_t size, void* user)
{
	(void)handle;
	(void)type;
	HttpDebugCB cb = (HttpDebugCB)(user);
	cb(data, size);
	return 0;
}

struct HttpClient::HttpParam {
	CURL* curl = nullptr;
	struct curl_slist* header = nullptr;
	struct curl_httppost* formPost = nullptr;
	FILE* file = nullptr;
	bool isDelete = false;
	bool isDownload = false;
	char error[CURL_ERROR_SIZE] = { 0 };
	
	long long fileSize = 0;
};

HttpClient::HttpClient(const std::string& url, HTTPTYPE type, const bool* abort)
	: httpType(type)
	, pisAbort(abort)
	, httpUrl(url)
{
}

HttpClient::~HttpClient()
{
}

void HttpClient::SetDebugTrace(bool debug, HttpDebugCB cb)
{
	debugTrace = debug;
	debugCB = cb;
}

bool HttpClient::Request(std::string& result)
{
	bool ret = false;
	errorCode = 0;
	errorString.clear();
	resultString.clear();
	switch (httpType)
	{
	case HTTPTYPE::HTTP_GET:
	{
		ret = HttpGet();
		break;
	}
	case HTTPTYPE::HTTP_DEL:
	{
		ret = HttpDel();
		break;
	}
	case HTTPTYPE::HTTP_POST:
	{
		ret = HttpPost();
		break;
	}
	case HTTPTYPE::HTTP_DOWNLOAD:
	{
		ret = HttpDownload();
		break;
	}
	case HTTPTYPE::HTTP_POSTFILEDS:
	{
		ret = HttpPostFile();
		break;
	}		
	default:
		break;
	}
	result = ret ? resultString : (errorString.empty() ? resultString : errorString);
	if (!ret && errorString.empty()) {
		errorString = resultString;
	}
	return ret;
}

long long HttpClient::RequestContentLength(void)
{
	errorCode = 0;
	errorString.clear();
	resultString.clear();

	CURL* curl = curl_easy_init();
	if (!curl) {
		return 0;
	}
	curl_easy_setopt(curl, CURLOPT_URL, httpUrl.c_str());
	curl_easy_setopt(curl, CURLOPT_HEADER, 1L);
	curl_easy_setopt(curl, CURLOPT_NOBODY, 1L);

    if (!netProxy.host.empty()) {
        auto SchemeToString = [](Proxy::Scheme scheme) -> std::string {
            static const char *name[] = {"http", "https"};
            return name[scheme - Proxy::Scheme::HTTP];
        };
        std::stringstream ss;
        ss << SchemeToString(netProxy.scheme) << "://" << netProxy.host;
        curl_easy_setopt(curl, CURLOPT_PROXY, ss.str().c_str());
        curl_easy_setopt(curl, CURLOPT_PROXYPORT, (long)netProxy.port);
        curl_easy_setopt(curl, CURLOPT_PROXYUSERNAME, netProxy.username.c_str());
        curl_easy_setopt(curl, CURLOPT_PROXYPASSWORD, netProxy.password.c_str());
    }

    if (!caFile.empty()) {
        curl_easy_setopt(curl, CURLOPT_CAINFO, caFile.c_str());
    }
    if (!caPath.empty()) {
        curl_easy_setopt(curl, CURLOPT_CAPATH, caPath.c_str());
    }
    if (verifySSL) {
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2);
    } else {
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0);
    }

	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &HttpClient::HttpStringFuction);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, this);

	char error[CURL_ERROR_SIZE] = { 0 };
	curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, error);
	curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30);
	curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 30);
	if (debugTrace) {
		curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
	}
	if (debugCB) {
		curl_easy_setopt(curl, CURLOPT_DEBUGFUNCTION, HttpDebugFuction);
		curl_easy_setopt(curl, CURLOPT_DEBUGDATA, debugCB);
	}
#if LIBCURL_VERSION_NUM >= 0x072400
	// A lot of servers don't yet support ALPN
	curl_easy_setopt(curl, CURLOPT_SSL_ENABLE_ALPN, 0);
	curl_easy_setopt(curl, CURLOPT_SSL_OPTIONS, CURLSSLOPT_NO_REVOKE);
#endif
	CURLcode code = CURLE_OK;
	code = curl_easy_perform(curl);
	curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &errorCode);
	double length = 0;
	curl_easy_getinfo(curl, CURLINFO_CONTENT_LENGTH_DOWNLOAD, &length);
	curl_easy_cleanup(curl);
	curl = nullptr;

	errorString = error;
	bool result = (CURLE_OK == code && 200 == errorCode);
	if (!result) {
		length = 0;
	}
	return (long long)length;
}

void HttpClient::Process(double dnow, double dtotal)
{
	processCB(dnow, dtotal, processContext);
}

bool HttpClient::HttpGet()
{
	HttpParam param;
	if (!InitCurl("Content-Type:application/x-www-form-urlencoded;charset=UTF-8", &param)) {
		errorString = "create http failed";
		return false;
	}
	return ReleaseCurl(&param);
}

bool HttpClient::HttpDel()
{
	HttpParam param;
	param.isDelete = true;
	if (!InitCurl("Content-Type:application/x-www-form-urlencoded;charset=UTF-8", &param)) {
		errorString = "create http failed";
		return false;
	}
	return ReleaseCurl(&param);
}

bool HttpClient::HttpPost()
{
	HttpParam param;
	if (!InitCurl("Content-Type:application/json;charset=UTF-8", &param)) {
		errorString = "create http failed";
		return false;
	}
	return ReleaseCurl(&param);
}

bool HttpClient::HttpPostFile()
{
	HttpParam param;
	if (!InitCurl("Content-Type: multipart/form-data", &param)) {
		errorString = "create http failed";
		return false;
	}
	return ReleaseCurl(&param);
}

bool HttpClient::HttpDownload()
{
	HttpParam param;
	if (resumeDownload && contentLength > 0) {
		param.fileSize = GetFileSize(fileName.c_str());
		if (param.fileSize >= contentLength) {
			resultString = fileName;
			return true;
		}
		param.file = fopen(fileName.c_str(), "ab+");
		if (param.file){
			fseek(param.file, 0, SEEK_END);
		}	
	}
	else {
		param.file = fopen(fileName.c_str(), "wb");
	}
	if (!param.file){
		errorString = "file open failed";
		return false;
	}
	bool result = false;
	do
	{
		httpParam = &param;
		param.isDownload = true;
		if (!InitCurl("", &param)) {
			errorString = "create http failed";
			break;
		}
		result = ReleaseCurl(&param);
		fflush(param.file);
		fclose(param.file);
		param.file = nullptr;
		if (!result) {
			if (!resumeDownload) {
				remove(fileName.c_str());
			}
		}
		else {
			resultString = fileName;
		}
	} while (false);
	if (param.file) {
		fclose(param.file);
		param.file = nullptr;
	}
	httpParam = nullptr;
	return result;
}

long long HttpClient::GetFileSize(const char* _fileName)
{
#ifdef WIN32
	FILE* fp = fopen(_fileName, "rb");
	if (fp){
		long long localLen = _filelengthi64(_fileno(fp));
		fclose(fp);
		return localLen;
	}
#else
	struct stat statbuf;
	if (0 == stat(_fileName, &statbuf)) {
		return statbuf.st_size;
	}
#endif // WIN32
	return 0;
}



bool HttpClient::InitCurl(const std::string& connectType, HttpParam* param)
{
	param->curl = curl_easy_init();
	if (!param->curl) {
		return false;
	}
	CURL* curl = param->curl;
	if(!connectType.empty() || !this->contentType.empty()) {
		param->header = curl_slist_append(param->header,
			this->contentType.empty() ? connectType.c_str() : this->contentType.c_str());
	}
	if (!userAgent.empty()) {
		param->header = curl_slist_append(param->header, userAgent.c_str());
	}
	if (!postData.empty()) {
		curl_easy_setopt(curl, CURLOPT_POST, 1L);
		curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postData.c_str());
	}
	for (auto & it : headerParams) {
		param->header = curl_slist_append(param->header, it.c_str());
	}
	curl_httppost* lastElem = NULL;
	for (auto & it : formParams) {
		curl_formadd(&param->formPost, &lastElem, CURLFORM_COPYNAME, it.first.c_str(), CURLFORM_COPYCONTENTS,
			it.second.c_str(), CURLFORM_END);
	}
	curl_easy_setopt(curl, CURLOPT_URL, httpUrl.c_str());
    if (!netProxy.host.empty()) {
        auto SchemeToString = [](Proxy::Scheme scheme) -> std::string {
            static const char *name[] = {"http", "https"};
            return name[scheme - Proxy::Scheme::HTTP];
        };
        std::stringstream ss;
        ss << SchemeToString(netProxy.scheme) << "://" << netProxy.host;
        curl_easy_setopt(curl, CURLOPT_PROXY, ss.str().c_str());
        curl_easy_setopt(curl, CURLOPT_PROXYPORT, (long)netProxy.port);
        curl_easy_setopt(curl, CURLOPT_PROXYUSERNAME, netProxy.username.c_str());
        curl_easy_setopt(curl, CURLOPT_PROXYPASSWORD, netProxy.password.c_str());
    }

	if (!caFile.empty()) {
		curl_easy_setopt(curl, CURLOPT_CAINFO, caFile.c_str());
	}
	if (!caPath.empty()) {
		curl_easy_setopt(curl, CURLOPT_CAPATH, caPath.c_str());
	}
	if (verifySSL) {
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1);
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2);
	}
	else {
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0);
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0);
	}

	if (param->isDelete) {// delete request
		curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "DELETE");
	}
	if (param->isDownload) {// download request
		curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);		
	}
	if (processCB) {// for download process or upload process
		curl_easy_setopt(curl, CURLOPT_PROGRESSFUNCTION, &HttpClient::HttpProcessFuction);
		curl_easy_setopt(curl, CURLOPT_PROGRESSDATA, this);
	}
	if (param->file) {// for file download
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &HttpClient::HttpFileFuction);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, this);
	}
	else {// for string request
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &HttpClient::HttpStringFuction);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, this);
	}
	if (0 != param->fileSize) {
		curl_easy_setopt(curl, CURLOPT_RESUME_FROM_LARGE, param->fileSize);
	}
	if (param->header) {
		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, param->header);
	}
	curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, param->error);
	curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, connectTimeout);
	switch (httpType)
	{
	case HTTP_GET:
		curl_easy_setopt(curl, CURLOPT_HTTPGET, 1);
		break;
	case HTTP_POST:
		curl_easy_setopt(curl, CURLOPT_POST, 1L);
		break;
	case HTTP_UPLOAD:
		curl_easy_setopt(curl, CURLOPT_TIMEOUT, uploadTimeout);
		break;
	case HTTP_DOWNLOAD:
		curl_easy_setopt(curl, CURLOPT_TIMEOUT, downloadTimeout);
		break;
	default:
		curl_easy_setopt(curl, CURLOPT_TIMEOUT, timeout);
		break;
	}
	curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0);
	
#if LIBCURL_VERSION_NUM >= 0x072400
	// A lot of servers don't yet support ALPN
	//curl_easy_setopt(curl, CURLOPT_SSL_ENABLE_ALPN, 0);
	//curl_easy_setopt(curl, CURLOPT_SSL_OPTIONS, CURLSSLOPT_NO_REVOKE);
#endif

	if (debugTrace) {
		curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
	}
	if (debugCB) {
		curl_easy_setopt(curl, CURLOPT_DEBUGFUNCTION, HttpDebugFuction);
		curl_easy_setopt(curl, CURLOPT_DEBUGDATA, debugCB);
	}
	return true;
}

bool HttpClient::ReleaseCurl(HttpParam* param)
{
	if (!param) {
		return false;
	}
	CURLcode code = CURLE_OK;
	if (param->curl) {
		code = curl_easy_perform(param->curl);
		curl_easy_getinfo(param->curl, CURLINFO_RESPONSE_CODE, &errorCode);
		curl_easy_cleanup(param->curl);
		param->curl = nullptr;
	}
	if (param->header) {
		curl_slist_free_all(param->header);
		param->header = nullptr;
	}
	if (param->formPost) {
		curl_formfree(param->formPost);
		param->formPost = nullptr;
	}
	errorString = param->error;
	bool result = false;
	if (HTTP_DOWNLOAD == httpType && (416 == errorCode || 206 == errorCode)) {// Requested Range Not Satisfiable
		result = true;
	}
	else if (CURLE_OK == code && 200 == errorCode) {
		result = true;
	}
	return result;
}


size_t HttpClient::HttpFileFuction(char* data, size_t size, size_t nmemb, void* user)
{
	HttpClient* client = static_cast<HttpClient*>(user);
	if (client->IsAbort()) {
		return 0;
	}
	if (data && client->httpParam && client->httpParam->file) {
		return fwrite(data, size, nmemb, client->httpParam->file);
	}
	return 0;
}
size_t HttpClient::HttpStringFuction(char* data, size_t size, size_t nmemb, void* user)
{
	HttpClient* client = static_cast<HttpClient*>(user);
	if (client->IsAbort()) {
		return 0;
	}
	size_t total = size * nmemb;
	if (total) {
		client->resultString.append(data, total);
	}
	return total;
}

int HttpClient::HttpProcessFuction(void* user, double dtotal, double dnow, double, double)
{
	HttpClient* client = static_cast<HttpClient*>(user);
	if (client->IsAbort()) {
		return -1;
	}
	if (dtotal > 0) {
		double localLen = double(client->httpParam ? client->httpParam->fileSize : 0);
		client->Process(dnow + localLen, dtotal + localLen);
	}
	return 0;
}
//size_t HttpClient::HttpDebugFuction(void *handle, curl_infotype type, char *data, size_t size, void* user)
//{
//	(void)handle;
//	(void)type;
//	HttpClient* client = static_cast<HttpClient*>(user);
//	if (client->debugCB) {
//		client->debugCB(data, size);
//	}
//	return 0;
//}
