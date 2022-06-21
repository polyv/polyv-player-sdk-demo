#include "http/http-param.h"

#include <sstream>
#include <ctime>
#include <chrono>

HttpParam::HttpParam()
{

}
HttpParam::HttpParam(const std::map<std::string, std::string>& params)
{
	mapParams = params;
}
HttpParam::HttpParam(const std::vector<std::pair<std::string, std::string>>& params)
{
	for (auto & it : params) {
		Append(it.first, it.second);
	}
}
HttpParam::~HttpParam()
{

}

int64_t HttpParam::GetTimestamp(bool second)
{
	std::time_t stamp = 0;
	if (second) {
		std::chrono::time_point<std::chrono::system_clock, std::chrono::seconds> tp =
			std::chrono::time_point_cast<std::chrono::seconds>(std::chrono::system_clock::now());
		auto tmp = std::chrono::duration_cast<std::chrono::seconds>(tp.time_since_epoch());
		stamp = tmp.count();
	}
	else {
		std::chrono::time_point<std::chrono::system_clock, std::chrono::milliseconds> tp =
			std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now());
		auto tmp = std::chrono::duration_cast<std::chrono::milliseconds>(tp.time_since_epoch());
		stamp = tmp.count();
	}
	return stamp;
}

std::string HttpParam::GetHostDomain(const std::string& url)
{
	if (0 == url.compare(0, 4, "http")) {
		size_t found = url.find_first_of("/\\");
		std::string str1 = url.substr(found + 2);
		found = str1.find_first_of("/\\");
		return (str1.substr(0, found));
	}
	else {
		size_t found = url.find_first_of("/\\");
		return (url.substr(0, found));
	}
}

std::string HttpParam::ToJsonString(const std::map<std::string, std::string>& params)
{
	return HttpParam(params).ToJson();
}
std::string HttpParam::ToHttpString(const std::map<std::string, std::string>& params)
{
	return HttpParam(params).ToHttp();
}
std::string HttpParam::ToSignString(const std::map<std::string, std::string>& params)
{
	return HttpParam(params).ToSign();
}
void HttpParam::Append(const std::string& key, double value)
{
	mapParams[key] = std::to_string((long double)value);
}
void HttpParam::Append(const std::string& key, int64_t value)
{
	mapParams[key] = std::to_string(value);
}
void HttpParam::Append(const std::string& key, uint64_t value)
{
	mapParams[key] = std::to_string(value);
}
void HttpParam::Append(const std::string& key, const std::string& value)
{
	mapParams[key] = value;
}
void HttpParam::Update(const std::string& key, double value)
{
	mapParams[key] = std::to_string((long double)value);
}
void HttpParam::Update(const std::string& key, int64_t value)
{
	mapParams[key] = std::to_string(value);
}
void HttpParam::Update(const std::string& key, uint64_t value)
{
	mapParams[key] = std::to_string(value);
}
void HttpParam::Update(const std::string& key, const std::string& value)
{
	mapParams[key] = value;
}
void HttpParam::Remove(const std::string& key)
{
	mapParams.erase(key);
}

std::string HttpParam::Value(const std::string& key) const
{
	auto it = mapParams.find(key);
	return it != mapParams.end() ? it->second : std::string();
}

std::string HttpParam::ToJson(void)
{
	std::string result;
	for (auto & it : mapParams) {
		if (it.second.empty()) {
			continue;
		}
		if (!result.empty()) {
			result += ",";
		}
		result += "\"";
		result += it.first;
		result += "\":\"";
		result += it.second;
		result += "\"";
	}
	return "{" + result + "}";
}
std::string HttpParam::ToHttp(void)
{
	std::string result;
	for (auto & it : mapParams) {
		if (it.second.empty()) {
			continue;
		}
		if (!result.empty()) {
			result += "&";
		}
		result += it.first;
		result += "=";
		result += it.second;
	}
	return result;
}
std::string HttpParam::ToSign(void)
{
	std::stringstream ss;
	for (auto & it : mapParams) {
		if (it.second.empty()) {
			continue;
		}
		ss << it.first << it.second;
	}
	return ss.str();
}