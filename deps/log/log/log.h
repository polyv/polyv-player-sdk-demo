#pragma once
/// simple log => slog

#if defined(_MSC_VER) && defined(LOG_SHARE)
#ifdef LOG_EXPORTS
#define LOG_API      extern "C" __declspec(dllexport)
#define LOG_CLASS    __declspec(dllexport)
#define LOG_TEMPLATE __declspec(dllexport)
#else
#define LOG_API   extern "C" __declspec(dllimport)
#define LOG_CLASS __declspec(dllimport)
#define LOG_TEMPLATE
#endif // end LOG_EXPORTS
#else
#define LOG_API
#define LOG_CLASS
#define LOG_TEMPLATE
#endif // end defined(_MSC_VER) && defined(LOG_SHARE)



#include <stdarg.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define SLOG_STRINGIFY(x) #x
#define SLOG_STRINGIFY_(x) SLOG_STRINGIFY(x)
#define SLOG__LINE__ SLOG_STRINGIFY_(__LINE__)
#define SLOG_FILE_LINE __FILE__ "(" SLOG__LINE__ ")"
#define SLOG_FUNCTION_LINE __FUNCTION__ "(" SLOG__LINE__ ")"

enum SLOG_LEVEL_TYPE{
    /**
    * Debug message to be used mostly by developers.
    */
    SLOG_FATAL = 1,
    SLOG_ERROR = 100,
    SLOG_WARN = 200,
    SLOG_INFO = 300,
    SLOG_DEBUG = 400,

    SLOG_OFF = 0,/*no log*/
};

enum SLOG_MODE_TYPE {
    SLOG_MODE_FILE = 0x01, ///> to file
    SLOG_MODE_CONSOLE = 0x02,///> to console
    SLOG_MODE_CALLBACK = 0x04,///> to callback hook
    SLOG_MODE_DEBUGVIEW = 0x08,///> to debugview ide
    SLOG_MODE_DEFAULT = SLOG_MODE_FILE | SLOG_MODE_CONSOLE | SLOG_MODE_DEBUGVIEW,
    SLOG_MODE_ALL = SLOG_MODE_DEFAULT | SLOG_MODE_CALLBACK,
};

enum SLOG_FORMAT_TYPE {
    SLOG_FORMAT_TIME = 0x01, ///> out time
    SLOG_FORMAT_LEVEL = 0x02, ///> out level
    SLOG_FORMAT_THREAD = 0x4, ///> out thread id
    SLOG_FORMAT_FUNCTION = 0x08, ///> out function name, must be macro
    SLOG_FORMAT_DEFAULT =
        SLOG_FORMAT_TIME | SLOG_FORMAT_LEVEL | SLOG_FORMAT_THREAD, ///> default format out
    SLOG_FORMAT_ALL = SLOG_FORMAT_DEFAULT | SLOG_FORMAT_FUNCTION,
};

typedef void(*slog_handler_t)(int lvl, const char *msg, va_list args, void *param);
LOG_API void slog_set_handler(slog_handler_t handler, void *param);
LOG_API void slog_get_handler(slog_handler_t *handler, void **param);

typedef void(*slog_hook_t)(int lvl, const char *msg, void* param);
LOG_API void slog_set_hook(slog_hook_t hook, void* param);
LOG_API void slog_get_hook(slog_hook_t *hook, void** param);

LOG_API bool slog_init(const char* path, const char* filename);
LOG_API void slog_release(void);

LOG_API int slog_get_level(void);
LOG_API void slog_set_level(int lvl);

LOG_API const char* slog_get_path(void);
LOG_API const char* slog_get_file(void);

LOG_API int slog_get_mode(void);
LOG_API void slog_set_mode(int mode);

LOG_API int slog_get_format(void);
LOG_API void slog_set_format(int format);

LOG_API int slog_get_limit_memory(void);
LOG_API void slog_set_limit_memory(int memory);// default 5m

LOG_API void slog_msg(int lvl, const char *msg);

#if !defined(_MSC_VER) && !defined(SWIG)
#define PRINTFATTR(f, a) __attribute__((__format__(__printf__, f, a)))
#else
#define PRINTFATTR(f, a)
#endif
PRINTFATTR(2, 3)
LOG_API void slog(int lvl, const char *format, ...);

PRINTFATTR(1, 2)
LOG_API void scrash(const char *format, ...);

#undef PRINTFATTR


#ifdef __cplusplus
}
#endif

#define slog_fatal(format, ...) slog(SLOG_FATAL, format, ##__VA_ARGS__)
#define slog_error(format, ...) slog(SLOG_ERROR, format, ##__VA_ARGS__)
#define slog_warn(format, ...)  slog(SLOG_WARN, format, ##__VA_ARGS__)
#define slog_info(format, ...)  slog(SLOG_INFO, format, ##__VA_ARGS__)
#define slog_debug(format, ...) slog(SLOG_DEBUG, format, ##__VA_ARGS__)
