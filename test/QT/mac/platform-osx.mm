#include "../platform.h"

#include <unistd.h>
#include <QWidget>
#import <pwd.h>
#import <AppKit/AppKit.h>
#import <CoreFoundation/CoreFoundation.h>
#import <AVFoundation/AVFoundation.h>
#import <ApplicationServices/ApplicationServices.h>

#include <log/log.h>

bool IsAlwaysOnTop(QWidget *window)
{
    return (window->windowFlags() & Qt::WindowStaysOnTopHint) != 0;
}

void SetAlwaysOnTop(QWidget *window, bool enable)
{
    Qt::WindowFlags flags = window->windowFlags();

    if (enable) {
        /* Force the level of the window high so it sits on top of
         * full-screen applications like Keynote */
        NSView *nsv = (__bridge NSView *)reinterpret_cast<void *>(
            window->winId());
        NSWindow *nsw = nsv.window;
        [nsw setLevel:1024];

        flags |= Qt::WindowStaysOnTopHint;
    } else {
        flags &= ~Qt::WindowStaysOnTopHint;
    }

    window->setWindowFlags(flags);
    window->show();
}

static NSString* GetUserHomeFolderPath()
{
    NSString* _userHomeFolderPath;

    BOOL isSandboxed = (nil != NSProcessInfo.processInfo.environment[@"APP_SANDBOX_CONTAINER_ID"]);
    if (isSandboxed)
    {
        struct passwd *pw = getpwuid(getuid());
        assert(pw);
        _userHomeFolderPath = [NSString stringWithUTF8String:pw->pw_dir];
    }
    else
    {
        _userHomeFolderPath = NSHomeDirectory();
    }
    return _userHomeFolderPath;
}

static MacPermissionStatus CheckFDAUsingFile(NSString *path)
{
    int fd = open([path cStringUsingEncoding:NSUTF8StringEncoding], O_RDONLY);
    if (fd != -1)
    {
        close(fd);
        return kPermissionAuthorized;
    }
    
    if (errno == EPERM || errno == EACCES)
    {
        return kPermissionDenied;
    }
    
    return kPermissionNotDetermined;
}

MacPermissionStatus CheckPermissionWithPrompt(MacPermissionType type,
                          bool prompt_for_permission)
{
    __block MacPermissionStatus permissionResponse =
        kPermissionNotDetermined;

    switch (type) {
    case kAllFilesAccess:{
        auto userHomeFolderPath = GetUserHomeFolderPath();
        NSArray<NSString *> *testFiles = @[
            [userHomeFolderPath stringByAppendingPathComponent:@"Library/Safari/CloudTabs.db"],
            [userHomeFolderPath stringByAppendingPathComponent:@"Library/Safari/Bookmarks.plist"],
            @"/Library/Application Support/com.apple.TCC/TCC.db",
            @"/Library/Preferences/com.apple.TimeMachine.plist",
        ];
            for (NSString *file in testFiles)
            {
                auto status = CheckFDAUsingFile(file);
                if (status == kPermissionAuthorized)
                {
                    permissionResponse = kPermissionAuthorized;
                    break;
                }
                if (status == kPermissionDenied)
                {
                    permissionResponse = kPermissionDenied;
                }
            }
        
            break;
        }
    case kAccessibility: {
        permissionResponse = (AXIsProcessTrusted()
                          ? kPermissionAuthorized
                          : kPermissionDenied);

        if (permissionResponse != kPermissionAuthorized &&
            prompt_for_permission) {
            NSDictionary *options = @{
                (__bridge id)kAXTrustedCheckOptionPrompt: @YES
            };
            permissionResponse = (AXIsProcessTrustedWithOptions(
                              (CFDictionaryRef)options)
                              ? kPermissionAuthorized
                              : kPermissionDenied);
        }

        slog_info("[macOS] Permission for accessibility %s.",
             permissionResponse == kPermissionAuthorized ? "granted"
                                 : "denied");
        break;
    }
    }

    return permissionResponse;
}

void OpenMacOSPrivacyPreferences(const char *tab)
{
    NSURL *url = [NSURL
        URLWithString:
            [NSString
                stringWithFormat:
                    @"x-apple.systempreferences:com.apple.preference.security?Privacy_%s",
                    tab]];
    [[NSWorkspace sharedWorkspace] openURL:url];
}
