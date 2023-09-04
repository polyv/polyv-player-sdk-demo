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
typedef enum {
	kAllFilesAccess = 0,
	kAccessibility
} MacPermissionType;

typedef enum {
	kPermissionNotDetermined = 0,
	kPermissionRestricted = 1,
	kPermissionDenied = 2,
	kPermissionAuthorized = 3,
} MacPermissionStatus;

MacPermissionStatus CheckPermissionWithPrompt(MacPermissionType type,
	bool prompt_for_permission);
#define CheckPermission(x) CheckPermissionWithPrompt(x, false)
#define RequestPermission(x) CheckPermissionWithPrompt(x, true)
void OpenMacOSPrivacyPreferences(const char* tab);
#endif
