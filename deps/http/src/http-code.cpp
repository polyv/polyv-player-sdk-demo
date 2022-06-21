#include "http/http-code.h"

#include <vector>
#include <memory>
#include <sstream>


//#ifndef WIN32
//#define UINT_MAX      0xffffffff    // maximum unsigned int value
//#endif // !WIN32


namespace {
	unsigned char ToHex(unsigned char x)
	{
		return  x > 9 ? x + 55 : x + 48;
	}

	unsigned char FromHex(unsigned char x)
	{
		unsigned char y = 0;
		if (x >= 'A' && x <= 'Z') y = x - 'A' + 10;
		else if (x >= 'a' && x <= 'z') y = x - 'a' + 10;
		else if (x >= '0' && x <= '9') y = x - '0';
		return y;
	}
}
std::string HttpCode::UrlEncode(const std::string &src)
{
    std::stringstream dest;
    static const char *hex = "0123456789ABCDEF";
    unsigned char c;

    for (size_t i = 0; i < src.size(); i++) {
        c = (unsigned char)src[i];
        if (isalnum(c) || (c == '-') || (c == '_') || (c == '.') || (c == '~')) {
            dest << c;
        } else if (c == ' ') {
            dest << "%20";
        } else {
            dest << '%' << hex[c >> 4] << hex[c & 15];
        }
    }

    return dest.str();
}
std::string HttpCode::UrlDecode(const std::string &src)
{
    std::stringstream unescaped;
    unescaped.fill('0');
    unescaped << std::hex;

    size_t safeLength = src.size();
    const char *safe = src.c_str();
    for (auto i = safe, n = safe + safeLength; i != n; ++i) {
        char c = *i;
        if (c == '%') {
            char hex[3];
            hex[0] = *(i + 1);
            hex[1] = *(i + 2);
            hex[2] = 0;
            i += 2;
            auto hexAsInteger = strtol(hex, nullptr, 16);
            unescaped << (char)hexAsInteger;
        } else {
            unescaped << *i;
        }
    }

    return unescaped.str();
}


namespace {

std::string Base64Encode(const char *src, int len)
{
    if (!src || len == 0) {
        return "";
    }

    static const char *ENC = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    auto in = reinterpret_cast<const unsigned char *>(src);
    auto inLen = len;
    std::stringstream ss;
    while (inLen) {
        // first 6 bits of char 1
        ss << ENC[*in >> 2];
        if (!--inLen) {
            // last 2 bits of char 1, 4 bits of 0
            ss << ENC[(*in & 0x3) << 4];
            ss << '=';
            ss << '=';
            break;
        }
        // last 2 bits of char 1, first 4 bits of char 2
        ss << ENC[((*in & 0x3) << 4) | (*(in + 1) >> 4)];
        in++;
        if (!--inLen) {
            // last 4 bits of char 2, 2 bits of 0
            ss << ENC[(*in & 0xF) << 2];
            ss << '=';
            break;
        }
        // last 4 bits of char 2, first 2 bits of char 3
        ss << ENC[((*in & 0xF) << 2) | (*(in + 1) >> 6)];
        in++;
        // last 6 bits of char 3
        ss << ENC[*in & 0x3F];
        in++, inLen--;
    }
    return ss.str();
}


std::string Base64Decode(const char *data, int len)
{
    int in_len = len;
    int i = 0;
    int in_ = 0;
    unsigned char part4[4];

    const int max_len = (len * 3 / 4);
    std::vector<unsigned char> ret(max_len);
    int idx = 0;

    while (in_len-- && (data[in_] != '=')) {
        unsigned char ch = data[in_++];
        if ('A' <= ch && ch <= 'Z')
            ch = ch - 'A'; // A - Z
        else if ('a' <= ch && ch <= 'z')
            ch = ch - 'a' + 26; // a - z
        else if ('0' <= ch && ch <= '9')
            ch = ch - '0' + 52; // 0 - 9
        else if ('+' == ch)
            ch = 62; // +
        else if ('/' == ch)
            ch = 63; // /
        else if ('=' == ch)
            ch = 64; // =
        else
            ch = 0xff; // something wrong
        part4[i++] = ch;
        if (i == 4) {
            ret[idx++] = (part4[0] << 2) + ((part4[1] & 0x30) >> 4);
            ret[idx++] = ((part4[1] & 0xf) << 4) + ((part4[2] & 0x3c) >> 2);
            ret[idx++] = ((part4[2] & 0x3) << 6) + part4[3];
            i = 0;
        }
    }

    if (i) {
        for (int j = i; j < 4; j++)
            part4[j] = 0xFF;
        ret[idx++] = (part4[0] << 2) + ((part4[1] & 0x30) >> 4);
        if (part4[2] != 0xFF) {
            ret[idx++] = ((part4[1] & 0xf) << 4) + ((part4[2] & 0x3c) >> 2);
            if (part4[3] != 0xFF) {
                ret[idx++] = ((part4[2] & 0x3) << 6) + part4[3];
            }
        }
    }

    ret.resize(idx);
    return std::string((char *)ret.data(), ret.size());
}

} // namespace
std::string HttpCode::EncodeBase64(const std::string &src)
{
    return Base64Encode(src.c_str(), (int)src.size());
}
std::string HttpCode::DecodeBase64(const std::string &src)
{
    return Base64Decode(src.c_str(), (int)src.size());
}


std::string HttpCode::EncodeUrlSalfBase64(const std::string& src)
{
	if (src.empty()) {
		return std::string();
	}
	std::string dest = EncodeBase64(src);
	for (size_t i = 0; i < dest.size(); ++i){
		if (dest[i] == '+'){
			dest[i] = '-';
			continue;
		}
		if (dest[i] == '/'){
			dest[i] = '_';
			continue;
		}
		if (dest[i] == '='){
			dest.pop_back();
			continue;
		}
	}
	return dest;
}
std::string HttpCode::DecodeUrlSalfBase64(const std::string& src)
{
	if (src.empty()) {
		return std::string();
	}
	std::string dest = src;
	for (size_t i = 0; i < dest.size(); ++i){
		if (dest[i] == '-'){
			dest[i] = '+';
			continue;
		}
		if (dest[i] == '_'){
			dest[i] = '/';
			continue;
		}
	}
	int remainder = dest.length() % 4;
	while (remainder--){
		dest.append("=");
	}
	return DecodeBase64(dest);
}


std::string HttpCode::StrToHex(const std::string& data)
{
	const std::string hex = "0123456789ABCDEF";
	std::stringstream ss;
	for (std::string::size_type i = 0; i < data.size(); ++i) {
		ss << hex[(unsigned char)data[i] >> 4] << hex[(unsigned char)data[i] & 0xf];
	}
	//std::cout << ss.str() << std::endl;
	return ss.str();
}
std::string HttpCode::HexToStr(const std::string& data)
{
	std::string result;
	for (size_t i = 0; i < data.length(); i += 2){
		std::string byte = data.substr(i, 2);
		char chr = (char)(int)strtol(byte.c_str(), NULL, 16);
		result.push_back(chr);
	}
	return result;
}

#ifdef USE_OPENSSL
extern "C" {
#include <openssl/md5.h>
#include <openssl/sha.h>
//#include <openssl/aes.h>

#include <openssl/aes.h>
#include <openssl/evp.h>
#include <openssl/err.h>
#include <openssl/bio.h>  
#include <openssl/buffer.h>
}

//std::string HttpCode::dbase64(const std::string &src)
//{
//	BIO *b64 = NULL;
//	BIO *bmem = NULL;
//	//int size = (int)src.size() * 3 / 4 + 10;
//	std::shared_ptr<char> txt_dec(new char[src.length()], [](char *ptr) {delete[] ptr; });
//
//	//char *buffer = (char *)malloc(length);
//	//memset(buffer, 0, length);
//	b64 = BIO_new(BIO_f_base64());
//	BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);
//	bmem = BIO_new_mem_buf((uint8_t *)txt_dec.get(), src.length());
//	bmem = BIO_push(b64, bmem);
//	BIO_read(bmem, (uint8_t *)src.data(), src.length());
//	BIO_free_all(bmem);
//	return std::string(txt_dec.get(), src.length());
//}

std::string HttpCode::Md5(const std::string& data, bool upper)
{
	MD5_CTX ctx;
	std::string result;
	unsigned char md[MD5_DIGEST_LENGTH] = { 0 };
	MD5_Init(&ctx);
	MD5_Update(&ctx, data.c_str(), data.size());
	MD5_Final(md, &ctx);
	return ToHexStr(md, sizeof(md), upper);
}
std::string HttpCode::Md5(const char* data, size_t len, bool upper)
{
	MD5_CTX ctx;
	std::string result;
	unsigned char md[MD5_DIGEST_LENGTH] = { 0 };
	MD5_Init(&ctx);
	MD5_Update(&ctx, data, len);
	MD5_Final(md, &ctx);
	return ToHexStr(md, sizeof(md), upper);
}
std::string HttpCode::Sha1(const std::string& data, bool upper)
{
	SHA_CTX ctx;
	unsigned char md[SHA_DIGEST_LENGTH] = { 0 };
	SHA1_Init(&ctx);
	SHA1_Update(&ctx, data.c_str(), data.size());
	SHA1_Final(md, &ctx);
	return ToHexStr(md, sizeof(md), upper);
}
std::string HttpCode::ToHexStr(const unsigned char *data, int len, bool upper)
{
	if (!data || len <= 0) {
		return std::string();
	}
	std::string out;
	std::string format = upper ? "%02X" : "%02x";
	char tmp[3] = { 0 };
	for (int i = 0; i < len; ++i) {
		sprintf(tmp, format.c_str(), (unsigned char)data[i]);
		out += tmp;
	}
	return out;
}

#include <stdint.h>
#include <string.h> // CBC mode, for memset
namespace {

#define CBC 1
	/*****************************************************************************/
	/* Defines:                                                                  */
	/*****************************************************************************/
	// The number of columns comprising a state in AES. This is a constant in AES. Value=4
#define Nb 4
// The number of 32 bit words in a key.
#define Nk 4
// Key length in bytes [128 bit]
#define KEYLEN 16
// The number of rounds in AES Cipher.
#define Nr 10

// jcallan@github points out that declaring Multiply as a function 
// reduces code size considerably with the Keil ARM compiler.
// See this link for more information: https://github.com/kokke/tiny-AES128-C/pull/3
#ifndef MULTIPLY_AS_A_FUNCTION
#define MULTIPLY_AS_A_FUNCTION 0
#endif


/*****************************************************************************/
/* Private variables:                                                        */
/*****************************************************************************/
// state - array holding the intermediate results during decryption.
	typedef uint8_t state_t[4][4];
	static state_t* state;

	// The array that stores the round keys.
	static uint8_t RoundKey[176];

	// The Key input to the AES Program
	static const uint8_t* Key;

#if defined(CBC) && CBC
	// Initial Vector used only for CBC mode
	static uint8_t* Iv;
#endif

	// The lookup-tables are marked const so they can be placed in read-only storage instead of RAM
	// The numbers below can be computed dynamically trading ROM for RAM - 
	// This can be useful in (embedded) bootloader applications, where ROM is often limited.
	static const uint8_t sbox[256] = {
		//0     1    2      3     4    5     6     7      8    9     A      B    C     D     E     F
		0x63, 0x7c, 0x77, 0x7b, 0xf2, 0x6b, 0x6f, 0xc5, 0x30, 0x01, 0x67, 0x2b, 0xfe, 0xd7, 0xab, 0x76,
		0xca, 0x82, 0xc9, 0x7d, 0xfa, 0x59, 0x47, 0xf0, 0xad, 0xd4, 0xa2, 0xaf, 0x9c, 0xa4, 0x72, 0xc0,
		0xb7, 0xfd, 0x93, 0x26, 0x36, 0x3f, 0xf7, 0xcc, 0x34, 0xa5, 0xe5, 0xf1, 0x71, 0xd8, 0x31, 0x15,
		0x04, 0xc7, 0x23, 0xc3, 0x18, 0x96, 0x05, 0x9a, 0x07, 0x12, 0x80, 0xe2, 0xeb, 0x27, 0xb2, 0x75,
		0x09, 0x83, 0x2c, 0x1a, 0x1b, 0x6e, 0x5a, 0xa0, 0x52, 0x3b, 0xd6, 0xb3, 0x29, 0xe3, 0x2f, 0x84,
		0x53, 0xd1, 0x00, 0xed, 0x20, 0xfc, 0xb1, 0x5b, 0x6a, 0xcb, 0xbe, 0x39, 0x4a, 0x4c, 0x58, 0xcf,
		0xd0, 0xef, 0xaa, 0xfb, 0x43, 0x4d, 0x33, 0x85, 0x45, 0xf9, 0x02, 0x7f, 0x50, 0x3c, 0x9f, 0xa8,
		0x51, 0xa3, 0x40, 0x8f, 0x92, 0x9d, 0x38, 0xf5, 0xbc, 0xb6, 0xda, 0x21, 0x10, 0xff, 0xf3, 0xd2,
		0xcd, 0x0c, 0x13, 0xec, 0x5f, 0x97, 0x44, 0x17, 0xc4, 0xa7, 0x7e, 0x3d, 0x64, 0x5d, 0x19, 0x73,
		0x60, 0x81, 0x4f, 0xdc, 0x22, 0x2a, 0x90, 0x88, 0x46, 0xee, 0xb8, 0x14, 0xde, 0x5e, 0x0b, 0xdb,
		0xe0, 0x32, 0x3a, 0x0a, 0x49, 0x06, 0x24, 0x5c, 0xc2, 0xd3, 0xac, 0x62, 0x91, 0x95, 0xe4, 0x79,
		0xe7, 0xc8, 0x37, 0x6d, 0x8d, 0xd5, 0x4e, 0xa9, 0x6c, 0x56, 0xf4, 0xea, 0x65, 0x7a, 0xae, 0x08,
		0xba, 0x78, 0x25, 0x2e, 0x1c, 0xa6, 0xb4, 0xc6, 0xe8, 0xdd, 0x74, 0x1f, 0x4b, 0xbd, 0x8b, 0x8a,
		0x70, 0x3e, 0xb5, 0x66, 0x48, 0x03, 0xf6, 0x0e, 0x61, 0x35, 0x57, 0xb9, 0x86, 0xc1, 0x1d, 0x9e,
		0xe1, 0xf8, 0x98, 0x11, 0x69, 0xd9, 0x8e, 0x94, 0x9b, 0x1e, 0x87, 0xe9, 0xce, 0x55, 0x28, 0xdf,
		0x8c, 0xa1, 0x89, 0x0d, 0xbf, 0xe6, 0x42, 0x68, 0x41, 0x99, 0x2d, 0x0f, 0xb0, 0x54, 0xbb, 0x16 };

	static const uint8_t rsbox[256] =
	{ 0x52, 0x09, 0x6a, 0xd5, 0x30, 0x36, 0xa5, 0x38, 0xbf, 0x40, 0xa3, 0x9e, 0x81, 0xf3, 0xd7, 0xfb,
	  0x7c, 0xe3, 0x39, 0x82, 0x9b, 0x2f, 0xff, 0x87, 0x34, 0x8e, 0x43, 0x44, 0xc4, 0xde, 0xe9, 0xcb,
	  0x54, 0x7b, 0x94, 0x32, 0xa6, 0xc2, 0x23, 0x3d, 0xee, 0x4c, 0x95, 0x0b, 0x42, 0xfa, 0xc3, 0x4e,
	  0x08, 0x2e, 0xa1, 0x66, 0x28, 0xd9, 0x24, 0xb2, 0x76, 0x5b, 0xa2, 0x49, 0x6d, 0x8b, 0xd1, 0x25,
	  0x72, 0xf8, 0xf6, 0x64, 0x86, 0x68, 0x98, 0x16, 0xd4, 0xa4, 0x5c, 0xcc, 0x5d, 0x65, 0xb6, 0x92,
	  0x6c, 0x70, 0x48, 0x50, 0xfd, 0xed, 0xb9, 0xda, 0x5e, 0x15, 0x46, 0x57, 0xa7, 0x8d, 0x9d, 0x84,
	  0x90, 0xd8, 0xab, 0x00, 0x8c, 0xbc, 0xd3, 0x0a, 0xf7, 0xe4, 0x58, 0x05, 0xb8, 0xb3, 0x45, 0x06,
	  0xd0, 0x2c, 0x1e, 0x8f, 0xca, 0x3f, 0x0f, 0x02, 0xc1, 0xaf, 0xbd, 0x03, 0x01, 0x13, 0x8a, 0x6b,
	  0x3a, 0x91, 0x11, 0x41, 0x4f, 0x67, 0xdc, 0xea, 0x97, 0xf2, 0xcf, 0xce, 0xf0, 0xb4, 0xe6, 0x73,
	  0x96, 0xac, 0x74, 0x22, 0xe7, 0xad, 0x35, 0x85, 0xe2, 0xf9, 0x37, 0xe8, 0x1c, 0x75, 0xdf, 0x6e,
	  0x47, 0xf1, 0x1a, 0x71, 0x1d, 0x29, 0xc5, 0x89, 0x6f, 0xb7, 0x62, 0x0e, 0xaa, 0x18, 0xbe, 0x1b,
	  0xfc, 0x56, 0x3e, 0x4b, 0xc6, 0xd2, 0x79, 0x20, 0x9a, 0xdb, 0xc0, 0xfe, 0x78, 0xcd, 0x5a, 0xf4,
	  0x1f, 0xdd, 0xa8, 0x33, 0x88, 0x07, 0xc7, 0x31, 0xb1, 0x12, 0x10, 0x59, 0x27, 0x80, 0xec, 0x5f,
	  0x60, 0x51, 0x7f, 0xa9, 0x19, 0xb5, 0x4a, 0x0d, 0x2d, 0xe5, 0x7a, 0x9f, 0x93, 0xc9, 0x9c, 0xef,
	  0xa0, 0xe0, 0x3b, 0x4d, 0xae, 0x2a, 0xf5, 0xb0, 0xc8, 0xeb, 0xbb, 0x3c, 0x83, 0x53, 0x99, 0x61,
	  0x17, 0x2b, 0x04, 0x7e, 0xba, 0x77, 0xd6, 0x26, 0xe1, 0x69, 0x14, 0x63, 0x55, 0x21, 0x0c, 0x7d };


	// The round constant word array, Rcon[i], contains the values given by 
	// x to th e power (i-1) being powers of x (x is denoted as {02}) in the field GF(2^8)
	// Note that i starts at 1, not 0).
	static const uint8_t Rcon[255] = {
	  0x8d, 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x1b, 0x36, 0x6c, 0xd8, 0xab, 0x4d, 0x9a,
	  0x2f, 0x5e, 0xbc, 0x63, 0xc6, 0x97, 0x35, 0x6a, 0xd4, 0xb3, 0x7d, 0xfa, 0xef, 0xc5, 0x91, 0x39,
	  0x72, 0xe4, 0xd3, 0xbd, 0x61, 0xc2, 0x9f, 0x25, 0x4a, 0x94, 0x33, 0x66, 0xcc, 0x83, 0x1d, 0x3a,
	  0x74, 0xe8, 0xcb, 0x8d, 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x1b, 0x36, 0x6c, 0xd8,
	  0xab, 0x4d, 0x9a, 0x2f, 0x5e, 0xbc, 0x63, 0xc6, 0x97, 0x35, 0x6a, 0xd4, 0xb3, 0x7d, 0xfa, 0xef,
	  0xc5, 0x91, 0x39, 0x72, 0xe4, 0xd3, 0xbd, 0x61, 0xc2, 0x9f, 0x25, 0x4a, 0x94, 0x33, 0x66, 0xcc,
	  0x83, 0x1d, 0x3a, 0x74, 0xe8, 0xcb, 0x8d, 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x1b,
	  0x36, 0x6c, 0xd8, 0xab, 0x4d, 0x9a, 0x2f, 0x5e, 0xbc, 0x63, 0xc6, 0x97, 0x35, 0x6a, 0xd4, 0xb3,
	  0x7d, 0xfa, 0xef, 0xc5, 0x91, 0x39, 0x72, 0xe4, 0xd3, 0xbd, 0x61, 0xc2, 0x9f, 0x25, 0x4a, 0x94,
	  0x33, 0x66, 0xcc, 0x83, 0x1d, 0x3a, 0x74, 0xe8, 0xcb, 0x8d, 0x01, 0x02, 0x04, 0x08, 0x10, 0x20,
	  0x40, 0x80, 0x1b, 0x36, 0x6c, 0xd8, 0xab, 0x4d, 0x9a, 0x2f, 0x5e, 0xbc, 0x63, 0xc6, 0x97, 0x35,
	  0x6a, 0xd4, 0xb3, 0x7d, 0xfa, 0xef, 0xc5, 0x91, 0x39, 0x72, 0xe4, 0xd3, 0xbd, 0x61, 0xc2, 0x9f,
	  0x25, 0x4a, 0x94, 0x33, 0x66, 0xcc, 0x83, 0x1d, 0x3a, 0x74, 0xe8, 0xcb, 0x8d, 0x01, 0x02, 0x04,
	  0x08, 0x10, 0x20, 0x40, 0x80, 0x1b, 0x36, 0x6c, 0xd8, 0xab, 0x4d, 0x9a, 0x2f, 0x5e, 0xbc, 0x63,
	  0xc6, 0x97, 0x35, 0x6a, 0xd4, 0xb3, 0x7d, 0xfa, 0xef, 0xc5, 0x91, 0x39, 0x72, 0xe4, 0xd3, 0xbd,
	  0x61, 0xc2, 0x9f, 0x25, 0x4a, 0x94, 0x33, 0x66, 0xcc, 0x83, 0x1d, 0x3a, 0x74, 0xe8, 0xcb };


	/*****************************************************************************/
	/* Private functions:                                                        */
	/*****************************************************************************/
	static uint8_t getSBoxValue(uint8_t num)
	{
		return sbox[num];
	}

	static uint8_t getSBoxInvert(uint8_t num)
	{
		return rsbox[num];
	}

	// This function produces Nb(Nr+1) round keys. The round keys are used in each round to decrypt the states. 
	static void KeyExpansion(void)
	{
		uint32_t i, j, k;
		uint8_t tempa[4]; // Used for the column/row operations

		// The first round key is the key itself.
		for (i = 0; i < Nk; ++i)
		{
			RoundKey[(i * 4) + 0] = Key[(i * 4) + 0];
			RoundKey[(i * 4) + 1] = Key[(i * 4) + 1];
			RoundKey[(i * 4) + 2] = Key[(i * 4) + 2];
			RoundKey[(i * 4) + 3] = Key[(i * 4) + 3];
		}

		// All other round keys are found from the previous round keys.
		for (; (i < (Nb * (Nr + 1))); ++i)
		{
			for (j = 0; j < 4; ++j)
			{
				tempa[j] = RoundKey[(i - 1) * 4 + j];
			}
			if (i % Nk == 0)
			{
				// This function rotates the 4 bytes in a word to the left once.
				// [a0,a1,a2,a3] becomes [a1,a2,a3,a0]

				// Function RotWord()
				{
					k = tempa[0];
					tempa[0] = tempa[1];
					tempa[1] = tempa[2];
					tempa[2] = tempa[3];
					tempa[3] = (uint8_t)k;
				}

				// SubWord() is a function that takes a four-byte input word and 
				// applies the S-box to each of the four bytes to produce an output word.

				// Function Subword()
				{
					tempa[0] = getSBoxValue(tempa[0]);
					tempa[1] = getSBoxValue(tempa[1]);
					tempa[2] = getSBoxValue(tempa[2]);
					tempa[3] = getSBoxValue(tempa[3]);
				}

				tempa[0] = tempa[0] ^ Rcon[i / Nk];
			}
			else if (Nk > 6 && i % Nk == 4)
			{
				// Function Subword()
				{
					tempa[0] = getSBoxValue(tempa[0]);
					tempa[1] = getSBoxValue(tempa[1]);
					tempa[2] = getSBoxValue(tempa[2]);
					tempa[3] = getSBoxValue(tempa[3]);
				}
			}
			RoundKey[i * 4 + 0] = RoundKey[(i - Nk) * 4 + 0] ^ tempa[0];
			RoundKey[i * 4 + 1] = RoundKey[(i - Nk) * 4 + 1] ^ tempa[1];
			RoundKey[i * 4 + 2] = RoundKey[(i - Nk) * 4 + 2] ^ tempa[2];
			RoundKey[i * 4 + 3] = RoundKey[(i - Nk) * 4 + 3] ^ tempa[3];
		}
	}

	// This function adds the round key to state.
	// The round key is added to the state by an XOR function.
	static void AddRoundKey(uint8_t round)
	{
		uint8_t i, j;
		for (i = 0; i < 4; ++i)
		{
			for (j = 0; j < 4; ++j)
			{
				(*state)[i][j] ^= RoundKey[round * Nb * 4 + i * Nb + j];
			}
		}
	}

	// The SubBytes Function Substitutes the values in the
	// state matrix with values in an S-box.
	static void SubBytes(void)
	{
		uint8_t i, j;
		for (i = 0; i < 4; ++i)
		{
			for (j = 0; j < 4; ++j)
			{
				(*state)[j][i] = getSBoxValue((*state)[j][i]);
			}
		}
	}

	// The ShiftRows() function shifts the rows in the state to the left.
	// Each row is shifted with different offset.
	// Offset = Row number. So the first row is not shifted.
	static void ShiftRows(void)
	{
		uint8_t temp;

		// Rotate first row 1 columns to left  
		temp = (*state)[0][1];
		(*state)[0][1] = (*state)[1][1];
		(*state)[1][1] = (*state)[2][1];
		(*state)[2][1] = (*state)[3][1];
		(*state)[3][1] = temp;

		// Rotate second row 2 columns to left  
		temp = (*state)[0][2];
		(*state)[0][2] = (*state)[2][2];
		(*state)[2][2] = temp;

		temp = (*state)[1][2];
		(*state)[1][2] = (*state)[3][2];
		(*state)[3][2] = temp;

		// Rotate third row 3 columns to left
		temp = (*state)[0][3];
		(*state)[0][3] = (*state)[3][3];
		(*state)[3][3] = (*state)[2][3];
		(*state)[2][3] = (*state)[1][3];
		(*state)[1][3] = temp;
	}

	static uint8_t xtime(uint8_t x)
	{
		return ((x << 1) ^ (((x >> 7) & 1) * 0x1b));
	}

	// MixColumns function mixes the columns of the state matrix
	static void MixColumns(void)
	{
		uint8_t i;
		uint8_t Tmp, Tm, t;
		for (i = 0; i < 4; ++i)
		{
			t = (*state)[i][0];
			Tmp = (*state)[i][0] ^ (*state)[i][1] ^ (*state)[i][2] ^ (*state)[i][3];
			Tm = (*state)[i][0] ^ (*state)[i][1]; Tm = xtime(Tm);  (*state)[i][0] ^= Tm ^ Tmp;
			Tm = (*state)[i][1] ^ (*state)[i][2]; Tm = xtime(Tm);  (*state)[i][1] ^= Tm ^ Tmp;
			Tm = (*state)[i][2] ^ (*state)[i][3]; Tm = xtime(Tm);  (*state)[i][2] ^= Tm ^ Tmp;
			Tm = (*state)[i][3] ^ t;        Tm = xtime(Tm);  (*state)[i][3] ^= Tm ^ Tmp;
		}
	}

	// Multiply is used to multiply numbers in the field GF(2^8)
#if MULTIPLY_AS_A_FUNCTION
	static uint8_t Multiply(uint8_t x, uint8_t y)
	{
		return (((y & 1) * x) ^
			((y >> 1 & 1) * xtime(x)) ^
			((y >> 2 & 1) * xtime(xtime(x))) ^
			((y >> 3 & 1) * xtime(xtime(xtime(x)))) ^
			((y >> 4 & 1) * xtime(xtime(xtime(xtime(x))))));
	}
#else
#define Multiply(x, y)                                \
      (  ((y & 1) * x) ^                              \
      ((y>>1 & 1) * xtime(x)) ^                       \
      ((y>>2 & 1) * xtime(xtime(x))) ^                \
      ((y>>3 & 1) * xtime(xtime(xtime(x)))) ^         \
      ((y>>4 & 1) * xtime(xtime(xtime(xtime(x))))))   \

#endif

// MixColumns function mixes the columns of the state matrix.
// The method used to multiply may be difficult to understand for the inexperienced.
// Please use the references to gain more information.
	static void InvMixColumns(void)
	{
		int i;
		uint8_t a, b, c, d;
		for (i = 0; i < 4; ++i)
		{
			a = (*state)[i][0];
			b = (*state)[i][1];
			c = (*state)[i][2];
			d = (*state)[i][3];

			(*state)[i][0] = Multiply(a, 0x0e) ^ Multiply(b, 0x0b) ^ Multiply(c, 0x0d) ^ Multiply(d, 0x09);
			(*state)[i][1] = Multiply(a, 0x09) ^ Multiply(b, 0x0e) ^ Multiply(c, 0x0b) ^ Multiply(d, 0x0d);
			(*state)[i][2] = Multiply(a, 0x0d) ^ Multiply(b, 0x09) ^ Multiply(c, 0x0e) ^ Multiply(d, 0x0b);
			(*state)[i][3] = Multiply(a, 0x0b) ^ Multiply(b, 0x0d) ^ Multiply(c, 0x09) ^ Multiply(d, 0x0e);
		}
	}


	// The SubBytes Function Substitutes the values in the
	// state matrix with values in an S-box.
	static void InvSubBytes(void)
	{
		uint8_t i, j;
		for (i = 0; i < 4; ++i)
		{
			for (j = 0; j < 4; ++j)
			{
				(*state)[j][i] = getSBoxInvert((*state)[j][i]);
			}
		}
	}

	static void InvShiftRows(void)
	{
		uint8_t temp;

		// Rotate first row 1 columns to right  
		temp = (*state)[3][1];
		(*state)[3][1] = (*state)[2][1];
		(*state)[2][1] = (*state)[1][1];
		(*state)[1][1] = (*state)[0][1];
		(*state)[0][1] = temp;

		// Rotate second row 2 columns to right 
		temp = (*state)[0][2];
		(*state)[0][2] = (*state)[2][2];
		(*state)[2][2] = temp;

		temp = (*state)[1][2];
		(*state)[1][2] = (*state)[3][2];
		(*state)[3][2] = temp;

		// Rotate third row 3 columns to right
		temp = (*state)[0][3];
		(*state)[0][3] = (*state)[1][3];
		(*state)[1][3] = (*state)[2][3];
		(*state)[2][3] = (*state)[3][3];
		(*state)[3][3] = temp;
	}


	// Cipher is the main function that encrypts the PlainText.
	static void Cipher(void)
	{
		uint8_t round = 0;

		// Add the First round key to the state before starting the rounds.
		AddRoundKey(0);

		// There will be Nr rounds.
		// The first Nr-1 rounds are identical.
		// These Nr-1 rounds are executed in the loop below.
		for (round = 1; round < Nr; ++round)
		{
			SubBytes();
			ShiftRows();
			MixColumns();
			AddRoundKey(round);
		}

		// The last round is given below.
		// The MixColumns function is not here in the last round.
		SubBytes();
		ShiftRows();
		AddRoundKey(Nr);
	}

	static void InvCipher(void)
	{
		uint8_t round = 0;

		// Add the First round key to the state before starting the rounds.
		AddRoundKey(Nr);

		// There will be Nr rounds.
		// The first Nr-1 rounds are identical.
		// These Nr-1 rounds are executed in the loop below.
		for (round = Nr - 1; round > 0; round--)
		{
			InvShiftRows();
			InvSubBytes();
			AddRoundKey(round);
			InvMixColumns();
		}

		// The last round is given below.
		// The MixColumns function is not here in the last round.
		InvShiftRows();
		InvSubBytes();
		AddRoundKey(0);
	}

	static void BlockCopy(uint8_t* output, uint8_t* input)
	{
		uint8_t i;
		for (i = 0; i < KEYLEN; ++i)
		{
			output[i] = input[i];
		}
	}

	static void XorWithIv(uint8_t* buf)
	{
		uint8_t i;
		for (i = 0; i < KEYLEN; ++i)
		{
			buf[i] ^= Iv[i];
		}
	}
}

//void HttpCode::AES128CBCEncrypt(uint8_t* output,
//	uint8_t* input, uint32_t length, const uint8_t* key, const uint8_t* iv)
//{
//	//AES_KEY aesKey;
//	//AES_set_encrypt_key(key, 128, &aesKey);
//	////memcpy(data_hex + datalen, "\x80\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00", 16);
//	////datalen = datalen + 16;
//	//AES_cbc_encrypt(input, output, length, &aesKey, const_cast<uint8_t*>(iv), AES_ENCRYPT);
//
//}
//void HttpCode::AES128CBCDecrypt(uint8_t* output,
//	uint8_t* input, uint32_t length, const uint8_t* key, const uint8_t* iv)
//{
//	//AES_KEY aesKey;
//	//AES_set_decrypt_key(key, 128, &aesKey);
//	//AES_cbc_encrypt(input, output, length, &aesKey, const_cast<uint8_t*>(iv), AES_DECRYPT);
//}

void HttpCode::AES128CBCEncrypt(uint8_t* output, 
	uint8_t* input, uint32_t length, const uint8_t* key, const uint8_t* iv)
{
	uint32_t i;
	uint8_t remainders = length % KEYLEN; /* Remaining bytes in the last non-full block */

	BlockCopy(output, input);
	state = (state_t*)output;

	// Skip the key expansion if key is passed as 0
	if (0 != key)
	{
		Key = key;
		KeyExpansion();
	}

	if (iv != 0)
	{
		Iv = (uint8_t*)iv;
	}

	for (i = 0; i < length; i += KEYLEN)
	{
		XorWithIv(input);
		BlockCopy(output, input);
		state = (state_t*)output;
		Cipher();
		Iv = output;
		input += KEYLEN;
		output += KEYLEN;
	}

	if (remainders)
	{
		BlockCopy(output, input);
		memset(output + remainders, 0, KEYLEN - remainders); /* add 0-padding */
		state = (state_t*)output;
		Cipher();
	}
}

void HttpCode::AES128CBCDecrypt(uint8_t* output, uint8_t* input, uint32_t length, const uint8_t* key, const uint8_t* iv)
{
	uint32_t i;
	uint8_t remainders = length % KEYLEN; /* Remaining bytes in the last non-full block */

	BlockCopy(output, input);
	state = (state_t*)output;

	// Skip the key expansion if key is passed as 0
	if (0 != key)
	{
		Key = key;
		KeyExpansion();
	}

	// If iv is passed as 0, we continue to encrypt without re-setting the Iv
	if (iv != 0)
	{
		Iv = (uint8_t*)iv;
	}

	for (i = 0; i < length; i += KEYLEN)
	{
		BlockCopy(output, input);
		state = (state_t*)output;
		InvCipher();
		XorWithIv(output);
		Iv = input;
		input += KEYLEN;
		output += KEYLEN;
	}

	if (remainders)
	{
		BlockCopy(output, input);
		memset(output + remainders, 0, KEYLEN - remainders); /* add 0-padding */
		state = (state_t*)output;
		InvCipher();
	}
}

//static void HandleOpenSSLErrors(void)
//{
//	unsigned long errCode = ERR_get_error();
//	printf("An error occurred\n");
//	if (errCode) {
//		printf("%s\n", ERR_error_string(errCode, NULL));
//	}
//}
//
//void HttpCode::AesCbcPkcs5Encrypt(unsigned char *src, int srcLen, unsigned char *key,
//	int keyLen, unsigned char *iv, unsigned char *out, int *outLen)
//{
//	EVP_CIPHER_CTX *ctx = NULL;
//	//char * res = NULL;
//	int blockCount = 0;
//	int quotient = srcLen / AES_BLOCK_SIZE;
//	int mod = srcLen % AES_BLOCK_SIZE;
//	blockCount = quotient + 1;
//
//	int padding = AES_BLOCK_SIZE - mod;
//	unsigned char *in = (unsigned char *)malloc(AES_BLOCK_SIZE*blockCount);
//	memset(in, padding, AES_BLOCK_SIZE*blockCount);
//	memcpy(in, src, srcLen);
//
//	//out
//	//char *out = (char *)malloc(AES_BLOCK_SIZE*blockCount);
//	//memset(out, 0x00, AES_BLOCK_SIZE*blockCount);
//	*outLen = AES_BLOCK_SIZE * blockCount;
//	do {
//		ctx = EVP_CIPHER_CTX_new();
//		if (!ctx) {
//			HandleOpenSSLErrors();
//			break;
//		}
//
//		if (1 != EVP_EncryptInit_ex(ctx, EVP_aes_128_cbc(), NULL, key, iv)) {
//			HandleOpenSSLErrors();
//			break;
//		}
//
//		if (1 != EVP_EncryptUpdate(ctx, (unsigned char*)out, outLen, in, AES_BLOCK_SIZE*blockCount)) {
//			HandleOpenSSLErrors();
//			break;
//		}
//	} while (0);
//	free(in);
//	if (!ctx) {
//		EVP_CIPHER_CTX_free(ctx);
//	}	
//}
//
//void HttpCode::AesCbcPkcs5Decrypt(unsigned char *src, int srcLen, unsigned char *key,
//	int keyLen, unsigned char *iv, unsigned char *out, int *outLen)
//{
//	AES_KEY aeskey;
//
//	/* set the encryption key */
//	AES_set_decrypt_key(key, 128, &aeskey);
//	AES_cbc_encrypt((unsigned char*)src, out, srcLen, &aeskey, iv, AES_DECRYPT);
//}


#endif // USE_OPENSSL
