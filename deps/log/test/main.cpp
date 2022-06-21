#include <iostream>

#include <string>
#include <log/log.h>

#ifdef _WIN32
#include <Windows.h>
#pragma warning(disable:4996) //depricated warnings
#include <Shlobj.h>
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
#include <sys/types.h>
#include <libgen.h>
#include <limits.h>
#include <unistd.h>
#include <pwd.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#define  util_access(a)  ::access(a, 0)
#define  util_mkdir(a)   ::mkdir((a), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH)
#define  util_rmdir(a)   ::rmdir(a)
#define  util_stat       stat
#endif
#ifdef _WIN32
const char PATH_DELIMITER = '\\';
const wchar_t WPATH_DELIMITER = L'\\';
#else
const char PATH_DELIMITER = '/';
const wchar_t WPATH_DELIMITER = L'/';
#endif

bool Mkdirs(const std::string& folder)
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

int main(int argc, char **argv)
{
    std::string path = "log";
    Mkdirs(path);
    slog_init(path.c_str(), "test.log");
    slog_info("test");
    slog_release();
    getchar();
    return 0;
}
