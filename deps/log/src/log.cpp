#include "log/log.h"
#include "log-def.h"
#include <fstream>
#include <iostream>
#include <mutex>
#include <ratio>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <string>

#ifdef _WIN32
#include <Shlobj.h>
#include <Windows.h>
#pragma warning(disable:4996) //depricated warnings
#include <direct.h>
#include <io.h>
#include <sys/stat.h>
#define  util_access(a)  ::_access((a), 0)
#define  util_mkdir(a)   ::_mkdir(a)
#define  util_rmdir(a)   ::_rmdir(a)
#define  util_stat       ::_stat64
#define  util_waccess(a)  ::_waccess((a), 0)
#define  util_wmkdir(a)   ::_wmkdir(a)
#define  util_wrmdir(a)   ::_wrmdir(a)
#define  util_wstat       ::_wstat64
#define  util_wremove     ::_wremove
#define  util_wrename     ::_wrename
#else
#include <fcntl.h>
#include <libgen.h>
#include <limits.h>
#include <pwd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#define util_access(a) ::access(a, 0)
#define util_mkdir(a)  ::mkdir((a), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH)
#define util_rmdir(a)  ::rmdir(a)
#define util_stat      stat
#endif
#ifdef _WIN32
const char PATH_DELIMITER = '\\';
const wchar_t WPATH_DELIMITER = L'\\';

const char PATH_DELIMITER_BAK = '/';
const wchar_t WPATH_DELIMITER_BAK = L'/';
#else
const char PATH_DELIMITER = '/';
const wchar_t WPATH_DELIMITER = L'/';

const char PATH_DELIMITER_BAK = '\\';
const wchar_t WPATH_DELIMITER_BAK = L'\\';
#endif

static int g_crashing = 0;
static std::mutex g_logMutex;
static std::string g_logFilePath;
static std::string g_logFileName;
static std::string g_logFileBaseName;
static std::string g_logLastFileName;
static std::fstream g_logFileStream;

static int g_logLevel = SLOG_INFO;
static int g_logMode = SLOG_MODE_DEFAULT;
static int g_logMemory = kLogDefaultMemory;
static int g_logFormat = SLOG_FORMAT_DEFAULT;
static int g_logCurrentMemory = 0;

static unsigned int g_maxLogCounts = 0;

static void* g_logHookParam = nullptr;
static slog_hook_t g_logHook = nullptr;

static bool GetPathInfo(const std::string& path, time_t& t, std::streamsize& size)
{
	struct util_stat buf;
	auto filename = path.c_str();
#if defined(_WIN32) && _MSC_VER < 1900
	std::string tmp;
	if (!path.empty() && (path.rbegin()[0] == PATH_DELIMITER)) {
		tmp = path.substr(0, path.size() - 1);
		filename = tmp.c_str();
	}
#endif
	if (util_stat(filename, &buf) != 0) {
		return false;
	}

	t = buf.st_mtime;
	size = static_cast<std::streamsize>(buf.st_size);
	return true;
}

static bool Mkdirs(const std::string& folder)
{
    std::string folder_builder;
    std::string sub;
    sub.reserve(folder.size());
    for (auto it = folder.begin(); it != folder.end(); ++it) {
        const char c = *it;
        sub.push_back(c);
        if (c == PATH_DELIMITER || it == folder.end() - 1) {
            folder_builder.append(sub);
            if (util_access(folder_builder.c_str()) != 0) {
                if (util_mkdir(folder_builder.c_str()) != 0) {
                    return false;
                }
            }
            sub.clear();
        }
    }
    return true;
}

static bool IsFileExist(const std::string& filePath)
{
    std::streamsize size;
	time_t t;
	return GetPathInfo(filePath, t, size);
}

/// eg: C:/test/file.h -> file.h
static std::string GetFileName(const std::string &filePath)
{
    const char *base = strrchr(filePath.c_str(), '/');
#ifdef WIN32 // Look for either path separator in Windows
    if (!base) {
        base = strrchr(filePath.c_str(), '\\');
    }
#endif
    return base ? (base + 1) : filePath;
}
/// eg: C:/test/file.h -> file
static std::string GetBaseName(const std::string& filePath)
{
    std::string fileName = GetFileName(filePath);
    const char* begin = fileName.c_str();
    const char* base = strrchr(fileName.c_str(), '.');
    if (base) {
        return fileName.substr(0, base - begin);
    }
    return fileName;
}

/// eg: C:/test/file.h -> C:/test
static std::string GetFilePath(const std::string &filePath)
{
    const char* begin = filePath.c_str();
    const char* base = strrchr(filePath.c_str(), '/');
#ifdef WIN32  // Look for either path separator in Windows
    if (!base) {
        base = strrchr(filePath.c_str(), '\\');
    }
#endif
    if (base) {
        return filePath.substr(0, base - begin);
    }
    return filePath;
}
/// eg: C:/test/file.h -> .h
static std::string GetExtension(const std::string &filePath)
{
    const char* begin = filePath.c_str();
    size_t pos = 0;

    const char* slash = strrchr(filePath.c_str(), '/');
#ifdef WIN32  // Look for either path separator in Windows
    if (!slash) {
        slash = strrchr(filePath.c_str(), '\\');
    }
#endif
    const char* period = strrchr(filePath.c_str(), '.');
    if (period) {
        pos = (size_t)(period - begin);
    }

    if (!period || slash > period) {
        return "";
    }
    return begin + pos;
}
/// eg: C:/test/file.h, return C:/test/file(2).h
static std::string GetNextName(const std::string& filePath)
{
    if (IsFileExist(filePath.c_str())) {
        auto dir = GetFilePath(filePath);
        auto name = GetBaseName(filePath);
        auto ext = GetExtension(filePath);
        std::string ret;
        long long i = 1;
        do
        {
            ret = dir + "/" + name + "(" + std::to_string(++i) + ")" + ext;
        } while (IsFileExist(ret.c_str()));
        return ret;
    }
    return filePath;
}

#ifdef _WIN32
static void load_debug_privilege(void)
{
    const DWORD flags = TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY;
    TOKEN_PRIVILEGES tp;
    HANDLE token;
    LUID val;

    if (!OpenProcessToken(GetCurrentProcess(), flags, &token)) {
        return;
    }

    if (!!LookupPrivilegeValue(NULL, SE_DEBUG_NAME, &val)) {
        tp.PrivilegeCount = 1;
        tp.Privileges[0].Luid = val;
        tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

        AdjustTokenPrivileges(token, false, &tp, sizeof(tp), NULL, NULL);
    }

    CloseHandle(token);
}
enum { ANSI = 0, UTF8 = 65001 }; // string dcode
static std::string WideToMBCP(const std::wstring &src, unsigned int cp)
{
    if (src.length() == 0) {
        return std::string();
    }
    // compute the length of the buffer we'll need
    int charcount = WideCharToMultiByte(cp, 0, src.c_str(), -1, NULL, 0, NULL, NULL);
    if (charcount == 0) {
        return std::string();
    }
    // convert
    char *buf = new char[charcount];
    WideCharToMultiByte(cp, 0, src.c_str(), -1, buf, charcount, NULL, NULL);
    std::string result(buf);
    delete[] buf;
    return std::move(result);
}
static std::wstring MBCPToWide(const std::string &src, unsigned int cp)
{
    if (src.length() == 0) {
        return std::wstring();
    }
    // compute the length of the buffer we'll need
    int charcount = MultiByteToWideChar(cp, 0, src.c_str(), -1, NULL, 0);
    if (charcount == 0) {
        return std::wstring();
    }
    // convert
    wchar_t *buf = new wchar_t[charcount];
    MultiByteToWideChar(cp, 0, src.c_str(), -1, buf, charcount);
    std::wstring result(buf);
    delete[] buf;
    return std::move(result);
}
static bool GetPathInfo(const std::wstring &path, time_t &t, std::streamsize &size)
{
    struct util_stat buf;
    auto filename = path.c_str();
#if defined(_WIN32) && _MSC_VER < 1900
    std::wstring tmp;
    if (!path.empty() && (path.rbegin()[0] == WPATH_DELIMITER)) {
        tmp = path.substr(0, path.size() - 1);
        filename = tmp.c_str();
    }
#endif
    if (util_wstat(filename, &buf) != 0) {
        return false;
    }

    t = buf.st_mtime;
    size = static_cast<std::streamsize>(buf.st_size);
    return true;
}
static bool IsFileExist(const std::wstring &filePath)
{
    std::streamsize size;
    time_t t;
    return GetPathInfo(filePath, t, size);
}
static bool Mkdirs(const std::wstring &filePath)
{
    std::wstring folder = filePath;
    std::wstring folder_builder;
    std::wstring sub;
    sub.reserve(folder.size());
    for (auto it = folder.begin(); it != folder.end(); ++it) {
        auto c = *it;
        sub.push_back(c);
        if ((c == WPATH_DELIMITER || c == WPATH_DELIMITER_BAK) || it == folder.end() - 1) {
            folder_builder.append(sub);
            if (util_waccess(folder_builder.c_str()) != 0) {
                if (util_wmkdir(folder_builder.c_str()) != 0) {
                    return false;
                }
            }
            sub.clear();
        }
    }
    return true;
}
/// eg: C:/test/file.h, return C:/test/file(2).h
static std::wstring GetNextName(const std::wstring &filePath)
{
    if (IsFileExist(filePath)) {
        auto temp = WideToMBCP(filePath, ANSI);
        auto dir = GetFilePath(temp);
        auto name = GetBaseName(temp);
        auto ext = GetExtension(temp);
        std::wstring ret;
        long long i = 1;
        do {
            ret = MBCPToWide(dir + "/" + name + "(" + std::to_string(++i) + ")" + ext, ANSI);

        } while (IsFileExist(ret));
        return ret;
    }
    return filePath;
}
#endif


static std::string format_time_string(const struct tm& time)
{
    char buf[80] = { 0 };
    strftime(buf, sizeof(buf), "%X", &time);
    return buf;
}
static std::string current_time_string()
{
    using namespace std::chrono;

    char  buf[80];
    struct tm  tstruct;

    auto tp = system_clock::now();
    auto now = system_clock::to_time_t(tp);
    tstruct = *localtime(&now);

    size_t written = strftime(buf, sizeof(buf), "%X", &tstruct);
    if (std::ratio_less<system_clock::period, seconds::period>::value && written &&
        (sizeof(buf) - written) > 5) {
        auto tp_secs = time_point_cast<seconds>(tp);
        auto millis = duration_cast<milliseconds>(tp - tp_secs).count();

        snprintf(buf + written, sizeof(buf) - written, ".%03u", static_cast<unsigned>(millis));
    }
    return buf;
}


static std::string generate_timedate_filename(const char *extension, bool noSpace)
{
    time_t    now = time(0);
    struct tm nowtime;
    nowtime = *localtime(&now);
    char      file[256] = {};
    snprintf(file, sizeof(file), "%d-%02d-%02d%c%02d-%02d-%02d.%s",
        nowtime.tm_year + 1900,
        nowtime.tm_mon + 1,
        nowtime.tm_mday,
        noSpace ? '_' : ' ',
        nowtime.tm_hour,
        nowtime.tm_min,
        nowtime.tm_sec,
        extension);

    return std::string(file);
}
static const char* log_level_string(int lvl)
{
    switch (lvl) {
    case SLOG_DEBUG:
        return "[debug]";
    case SLOG_INFO:
        return "[info]";
    case SLOG_WARN:
        return "[warn]";
    case SLOG_ERROR:
        return "[error]";
    case SLOG_FATAL:
        return "[fatal]";
    }
    return "[unknown]";
}
static void def_log_handler(int lvl, const char *format, va_list args, void *param)
{
    (void)param;
    if (lvl > g_logLevel) {
        return;
    }
    if (args) {
        char out[kLogSize] = {0};
        vsnprintf(out, kLogSize, format, args);

        fprintf(stdout, "%s%s:%s\n", current_time_string().c_str(), log_level_string(lvl), out);
    } else {
        fprintf(stdout, "%s%s:%s\n", current_time_string().c_str(), log_level_string(lvl), format);
    }
    fflush(stdout);
}

static void* g_logHandlerParam = nullptr;
static slog_handler_t g_logHandler = def_log_handler;

inline bool is_mode_output(int mode)
{
    return mode == (g_logMode & mode);
}
inline bool is_format_output(int format)
{
    return format == (g_logFormat & format);
}
static inline int sum_chars(const char *str)
{
    int val = 0;
    for (; *str != 0; str++) {
        val += *str;
    }

    return val;
}
#define MAX_REPEATED_LINES 30
#define MAX_CHAR_VARIATION (255 * 3)
static inline bool too_many_repeated_entries(std::fstream &file,
                                             const char *format,
                                             const char *msg,
                                             const char *output_str)
{
    static const char *last_msg_ptr = nullptr;
    static int last_char_sum = 0;
    static char cmp_str[4096];
    static int rep_count = 0;

    int new_sum = sum_chars(output_str);

    if (last_msg_ptr == msg) {
        int diff = std::abs(new_sum - last_char_sum);
        if (diff < MAX_CHAR_VARIATION) {
            return (rep_count++ >= MAX_REPEATED_LINES);
        }
    }

    if (rep_count > MAX_REPEATED_LINES) {
        if (format) {
            file << format;
        }
        std::string ext = "Last log entry repeated for " +
                          std::to_string(rep_count - MAX_REPEATED_LINES) + " more lines";
        file << ext << std::endl;
        g_logCurrentMemory += (int)ext.size();
    }
    last_msg_ptr = msg;
    strcpy(cmp_str, output_str);
    last_char_sum = new_sum;
    rep_count = 0;
    return false;
}
static inline void log_string(std::fstream& file, const char* format, char* str)
{
    if (format) {
        file << format;
    }
    file << str << std::endl;
}
static inline void log_string_chunk(std::fstream& file, const char* format, char *str)
{
    char *pNextLine = str;
    while (*pNextLine) {
        char *nextLine = strchr(str, '\n');
        if (!nextLine)
            break;

        if (nextLine != str && nextLine[-1] == '\r') {
            nextLine[-1] = 0;
        }
        else {
            nextLine[0] = 0;
        }

        log_string(file, format, str);
        nextLine++;
        str = nextLine;
    }
    if (str && *str != '\0') {
        log_string(file, format, str);
    }
}

static void do_log(int lvl, const char *msg, va_list args, void *param)
{
    do_log_with_param(lvl, msg, std::this_thread::get_id(), args, NULL, NULL, param);
}
static bool open_log(const char* path)
{
    if (path) {
#ifdef _WIN32
        std::wstring wpath = MBCPToWide(path, ANSI);
        if (!Mkdirs(wpath)) {
            return false;
        }
#else
        if (!Mkdirs(path)) {
            return false;
        }
#endif
    } 
    std::stringstream dst;
    std::string name = g_logFileBaseName;
    dst << g_logFilePath << "/" << name;
#ifdef _WIN32
    std::wstring wdst = MBCPToWide(dst.str(), ANSI);
    auto logFileName = GetNextName(wdst);
    g_logFileName = WideToMBCP(logFileName, ANSI);
    g_logFileStream.open(logFileName.c_str(),
                         std::ios_base::in | std::ios_base::out | std::ios_base::trunc);
#else
    g_logFileName = GetNextName(dst.str());
    g_logFileStream.open(g_logFileName,
                         std::ios_base::in | std::ios_base::out | std::ios_base::trunc);
#endif   
    if (g_logFileStream.is_open()) {
        slog_set_handler(do_log, &g_logFileStream);
#ifdef _WIN32
        load_debug_privilege();
#endif
    } else {
        g_logFileName.clear();
        slog(SLOG_ERROR, "failed to open log file");
    }
    return g_logFileStream.is_open();
}
static void close_log(bool clear)
{
    if (clear) {
        slog_set_handler(def_log_handler, nullptr);
    }
    if (g_logFileStream.is_open()) {
        g_logFileStream.flush();
        g_logFileStream.close();
    }
    if (clear) {
        g_logFileName.clear();
        g_logFilePath.clear();
    }
}
void do_log_with_param(int lvl,
                       const char *msg,
                       const std::thread::id &id,
                       va_list args /* = NULL*/,
                       struct tm *time /* = NULL*/,
                       const char *function /* = NULL*/,
                       void *param /* = NULL*/)
{
    std::lock_guard<std::mutex> lock(g_logMutex);
    std::stringstream str;
    if (is_format_output(SLOG_FORMAT_TIME)) {
        str << (time ? format_time_string(*time) : current_time_string()) << ' ';
    }
    if (is_format_output(SLOG_FORMAT_LEVEL)) {
        str << log_level_string(lvl) << ' ';
    }
    if (is_format_output(SLOG_FORMAT_THREAD)) {
        str << "TID=" << id << ' ';
    }
    if (is_format_output(SLOG_FORMAT_FUNCTION) && function) {
        str << function << ' ';
    }
    std::string format = str.str();
    if (!format.empty()) {
        format += ": ";
    }
    int size = 0;
    static char log[kLogSize] = {0};
    if (args) {
#ifndef _WIN32
        va_list args2;
        va_copy(args2, args);
#endif
        size = vsnprintf(log, kLogSize - 1, msg, args);
    } else {
        strcpy(log, msg);
        size = (int)strlen(log);
    }
    size += (int)format.size();

#ifdef _WIN32
    if (IsDebuggerPresent() && is_mode_output(SLOG_MODE_DEBUGVIEW)) {
        int wNum = MultiByteToWideChar(CP_UTF8, 0, log, -1, NULL, 0);
        if (wNum > 1) {
            static std::wstring wide_buf;
            wide_buf.reserve(wNum + 1);
            wide_buf.resize(wNum - 1);
            MultiByteToWideChar(CP_UTF8, 0, log, -1, &wide_buf[0], wNum);
            auto end = wide_buf.end() - 1;
            if (*end != '\n') {
                wide_buf.push_back('\n');
            }
            if (!format.empty()) {
                OutputDebugStringA(format.c_str());
            }
            OutputDebugStringW(wide_buf.c_str());
        }
    }
#endif
    if (lvl > g_logLevel) {
        return;
    }
    if (is_mode_output(SLOG_MODE_CONSOLE)) {
        if (!format.empty()) {
            std::cout << format.c_str();
        }
        std::cout << log << std::endl;
    }
    if (!param) {
        param = g_logHandlerParam;
    }
    if (param) {
        std::fstream &file = *static_cast<std::fstream *>(param);

        do {
            if (g_logMemory == kLogDefaultMemory) {
                break;
            }
            g_logCurrentMemory += size;
            if (g_logCurrentMemory > g_logMemory) {
                close_log(false);
                open_log(g_logFilePath.c_str());
                g_logCurrentMemory = size;
            }
        } while (false);

        if (file.is_open() && is_mode_output(SLOG_MODE_FILE)) {
            if (too_many_repeated_entries(file, format.c_str(), msg, log)) {
                return;
            }
            log_string_chunk(file, format.c_str(), log);
        }
    }

    if (g_logHook && is_mode_output(SLOG_MODE_CALLBACK)) {
        g_logHook(lvl, log, g_logHookParam);
    }

#if defined(_WIN32)
    if (lvl <= SLOG_FATAL && IsDebuggerPresent()) {
        __debugbreak();
    }
#endif
}

LOG_API void slog_set_handler(slog_handler_t handler, void *param)
{
    if (!handler) {
        handler = def_log_handler;
    }
    g_logHandlerParam = param;
    g_logHandler = handler;
}
LOG_API void slog_get_handler(slog_handler_t *handler, void **param)
{
    if (handler) {
        *handler = g_logHandler;
    }
    if (param) {
        *param = g_logHandlerParam;
    }
}

LOG_API void slog_set_hook(slog_hook_t hook, void* user)
{
    std::lock_guard<std::mutex> lock(g_logMutex);
    g_logHook = hook;
    g_logHookParam = user;
}
LOG_API void slog_get_hook(slog_hook_t *hook, void** param)
{
    if (hook) {
        *hook = g_logHook;
    }
    if (param) {
        *param = g_logHookParam;
    }
}

LOG_API bool slog_init(const char* path, const char* filename)
{
    slog_release();
    std::lock_guard<std::mutex> lock(g_logMutex);
    g_logFilePath = path ? path : ".";
    g_logFileBaseName = filename ? std::string(filename) : generate_timedate_filename("txt", false);
    return open_log(path);
}
LOG_API void slog_release(void)
{
    std::lock_guard<std::mutex> lock(g_logMutex);
    close_log(true);
}

LOG_API int slog_get_level(void)
{
    return g_logLevel;
}
LOG_API void slog_set_level(int lvl)
{
    std::lock_guard<std::mutex> lock(g_logMutex);
    g_logLevel = lvl;
}

LOG_API const char* slog_get_path(void)
{
    return g_logFilePath.c_str();
}
LOG_API const char* slog_get_file(void)
{
    return g_logFileName.c_str();
}

LOG_API int slog_get_mode(void)
{
    return g_logMode;
}
LOG_API void slog_set_mode(int mode)
{
    std::lock_guard<std::mutex> lock(g_logMutex);
    g_logMode = mode;
}

LOG_API int slog_get_format(void)
{
    return g_logFormat;
}
LOG_API void slog_set_format(int format)
{
    std::lock_guard<std::mutex> lock(g_logMutex);
    g_logFormat = format;
}

LOG_API int slog_get_limit_memory(void)
{
    return g_logMemory;
}
LOG_API void slog_set_limit_memory(int memory)
{
    std::lock_guard<std::mutex> lock(g_logMutex);
    if (kLogDefaultMemory != memory && memory < kLogMinMemory) {
        memory = kLogMinMemory;
    }
    g_logMemory = memory;
}

LOG_API void slog_msg(int lvl, const char *msg)
{
    if (SLOG_OFF == g_logLevel) {
        return;
    }
    g_logHandler(lvl, msg, nullptr, g_logHandlerParam);
}

LOG_API void slogva(int lvl, const char *format, va_list args)
{
    if (SLOG_OFF == g_logLevel) {
        return;
    }
    g_logHandler(lvl, format, args, g_logHandlerParam);
}

LOG_API void slog(int lvl, const char *format, ...)
{
    if (SLOG_OFF == g_logLevel) {
        return;
    }
    va_list args;

    va_start(args, format);
    slogva(lvl, format, args);
    va_end(args);
}

LOG_API void scrash(const char *format, ...)
{
    va_list args;

    if (g_crashing) {
        fputs("Crashed in the crash handler", stderr);
    }

    g_crashing = 1;
    va_start(args, format);

    vfprintf(stderr, format, args);
    exit(0);
    //va_end(args);
}
