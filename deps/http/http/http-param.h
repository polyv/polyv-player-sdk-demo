#pragma once

#include <map>
#include <string>
#include <vector>


class  HttpParam {
public:
	HttpParam();
	HttpParam(const std::map<std::string, std::string>& params);
	HttpParam(const std::vector<std::pair<std::string, std::string>>& params);
	~HttpParam();

	static int64_t GetTimestamp(bool second);

	static std::string GetHostDomain(const std::string& url);

	static std::string ToJsonString(const std::map<std::string, std::string>& params);
	static std::string ToHttpString(const std::map<std::string, std::string>& params);
	static std::string ToSignString(const std::map<std::string, std::string>& params);
public:
	void Append(const std::string& key, double value);
	void Append(const std::string& key, int64_t value);
	void Append(const std::string& key, uint64_t value);
	void Append(const std::string& key, const std::string& value);
	void Update(const std::string& key, double value);
	void Update(const std::string& key, int64_t value);
	void Update(const std::string& key, uint64_t value);
	void Update(const std::string& key, const std::string& value);
	void Remove(const std::string& key);

	std::string Value(const std::string& key) const;

	std::string ToJson(void);
	std::string ToHttp(void);
	std::string ToSign(void);
private:
	std::map<std::string, std::string> mapParams;
};


