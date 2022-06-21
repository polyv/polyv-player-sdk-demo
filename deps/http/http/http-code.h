#pragma once
#include <string>



class  HttpCode {	
public:
	static std::string UrlEncode(const std::string& src);
	static std::string UrlDecode(const std::string& src);

	static std::string EncodeBase64(const std::string& src);
	static std::string DecodeBase64(const std::string& src);

	static std::string EncodeUrlSalfBase64(const std::string& src);
	static std::string DecodeUrlSalfBase64(const std::string& src);

	static std::string StrToHex(const std::string& hex);
	static std::string HexToStr(const std::string& str);

#ifdef USE_OPENSSL
	static std::string Md5(const std::string& data, bool upper);
	static std::string Md5(const char* data, size_t len, bool upper);
	static std::string Sha1(const std::string& data, bool upper);
	static std::string ToHexStr(const unsigned char *data, int len, bool upper);

	static void AES128CBCEncrypt(uint8_t* output, 
		uint8_t* input, uint32_t length, const uint8_t* key, const uint8_t* iv);
	static void AES128CBCDecrypt(uint8_t* output,
		uint8_t* input, uint32_t length, const uint8_t* key, const uint8_t* iv);

	//static std::string dbase64(const std::string &src);
	//static void AesCbcPkcs5Encrypt(unsigned char *src, int srcLen, unsigned char *key, 
	//	int keyLen, unsigned char *iv, unsigned char *out, int *outLen);
	//static void AesCbcPkcs5Decrypt(unsigned char *src, int srcLen, unsigned char *key,
	//	int keyLen, unsigned char *iv, unsigned char *out, int *outLen);
#endif// USE_OPENSSL
};


