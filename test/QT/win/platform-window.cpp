#include "platform.h"

#include <algorithm>
#include <sstream>
#include <QWidget>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <shellapi.h>
#include <shlobj.h>
#include <Dwmapi.h>

std::vector<std::wstring> GetPreferredLocales()
{
	std::vector<std::wstring> result;

	ULONG num, length = 0;
	if (!GetUserPreferredUILanguages(MUI_LANGUAGE_NAME, &num,
		nullptr, &length))
		return result;

	std::vector<wchar_t> buffer(length);
	if (!GetUserPreferredUILanguages(MUI_LANGUAGE_NAME, &num,
		&buffer.front(), &length))
		return result;

	result.reserve(num);
	auto start = begin(buffer);
	auto end_ = end(buffer);
	decltype(start) separator;
	while ((separator = find(start, end_, 0)) != end_) {
		if (result.size() == num)
			break;

		result.emplace_back(&*start);

		start = separator + 1;
	}

	return result;
}

void SetAeroEnabled(bool enable)
{
	static HRESULT (WINAPI *func)(UINT) = nullptr;
	static bool failed = false;

	if (!func) {
		if (failed) {
			return;
		}

		HMODULE dwm = LoadLibraryW(L"dwmapi");
		if (!dwm) {
			failed = true;
			return;
		}

		func = reinterpret_cast<decltype(func)>(GetProcAddress(dwm,
						"DwmEnableComposition"));
		if (!func) {
			failed = true;
			return;
		}
	}

	func(enable ? DWM_EC_ENABLECOMPOSITION : DWM_EC_DISABLECOMPOSITION);
}

bool IsAlwaysOnTop(QWidget *window)
{
	DWORD exStyle = GetWindowLong((HWND)window->winId(), GWL_EXSTYLE);
	return (exStyle & WS_EX_TOPMOST) != 0;
}

void SetAlwaysOnTop(QWidget *window, bool enable)
{
	Qt::WindowFlags flags = window->windowFlags();
	if (enable)
        flags |= Qt::WindowStaysOnTopHint;
	else
        flags &= ~Qt::WindowStaysOnTopHint;
	window->setWindowFlags(flags);

	HWND hwnd = (HWND)window->winId();
	SetWindowPos(hwnd, enable ? HWND_TOPMOST : HWND_NOTOPMOST, 0, 0, 0, 0,
			SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
}

void SetProcessPriority(const char *priority)
{
	if (!priority)
		return;

	if (strcmp(priority, "High") == 0)
		SetPriorityClass(GetCurrentProcess(), HIGH_PRIORITY_CLASS);
	else if (strcmp(priority, "AboveNormal") == 0)
		SetPriorityClass(GetCurrentProcess(), ABOVE_NORMAL_PRIORITY_CLASS);
	else if (strcmp(priority, "Normal") == 0)
		SetPriorityClass(GetCurrentProcess(), NORMAL_PRIORITY_CLASS);
	else if (strcmp(priority, "Idle") == 0)
		SetPriorityClass(GetCurrentProcess(), IDLE_PRIORITY_CLASS);
}
