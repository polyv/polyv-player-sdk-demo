#pragma once

#include <string>
#include <vector>

class QWidget;

std::vector<std::wstring> GetPreferredLocales(void);

bool IsAlwaysOnTop(QWidget *window);
void SetAlwaysOnTop(QWidget *window, bool enable);

#ifdef _WIN32
void SetAeroEnabled(bool enable);
void SetProcessPriority(const char *priority);
#endif

#ifdef __APPLE__
namespace ns {
enum NSWindowCollectionBehavior {
    NSWindowCollectionBehaviorDefault = 0,
    NSWindowCollectionBehaviorCanJoinAllSpaces = 1 << 0,
    NSWindowCollectionBehaviorMoveToActiveSpace = 1 << 1,
    NSWindowCollectionBehaviorManaged = 1 << 2,
    NSWindowCollectionBehaviorTransient = 1 << 3,
    NSWindowCollectionBehaviorStationary = 1 << 4,
    NSWindowCollectionBehaviorParticipatesInCycle = 1 << 5,
    NSWindowCollectionBehaviorIgnoresCycle = 1 << 6,
    NSWindowCollectionBehaviorFullScreenPrimary = 1 << 7,
    NSWindowCollectionBehaviorFullScreenAuxiliary = 1 << 8,
    NSWindowCollectionBehaviorFullScreenAllowsTiling = 1 << 11,
    NSWindowCollectionBehaviorFullScreenDisallowsTiling = 1 << 12
};
}
void SetCollectionBehavior(QWidget *window, ns::NSWindowCollectionBehavior behavior, bool on = true);
void EnableOSXVSync(bool enable);
void EnableOSXDockIcon(bool enable);
bool isInBundle();
std::string GetBundlePath();
std::string GetBundleResourcesPath();
#endif
