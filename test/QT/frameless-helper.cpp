#include "frameless-helper.h"

#include <QApplication>
#include <QPoint>
#include <QSize>
#include <QWindow>
#include <QDebug>
#include <QPainter>
#include <QMouseEvent>
#include <QPainterPath>
#include <QBitmap>

#include <windows.h>
#include <WinUser.h>
#include <windowsx.h>
#include <dwmapi.h>
#include <objidl.h> // Fixes error C2504: 'IUnknown' : base class undefined
#include <gdiplus.h>
#include <GdiPlusColor.h>
#pragma comment(lib,"Dwmapi.lib") // Adds missing library, fixes error LNK2019: unresolved external symbol __imp__DwmExtendFrameIntoClientArea
#pragma comment(lib,"user32.lib")

//
//// Utility functions without warnings
//constexpr WORD GetLOWORD(unsigned long long l)
//{
//	return static_cast<WORD>(static_cast<DWORD_PTR>(l) & 0xffff);
//}
//
//constexpr WORD GetLOWORD(long long l)
//{
//	return static_cast<WORD>(static_cast<DWORD_PTR>(l) & 0xffff);
//}
//
//constexpr WORD GetHIWORD(unsigned long long l)
//{
//	return static_cast<WORD>((static_cast<DWORD_PTR>(l) >> 16) & 0xffff);
//}
//
//constexpr WORD GetHIWORD(long long l)
//{
//	return static_cast<WORD>((static_cast<DWORD_PTR>(l) >> 16) & 0xffff);
//}

extern void SetDpiAwareness(int value) 
{
	HRESULT hr = CoInitialize(NULL);
	if (SUCCEEDED(hr)){
		HMODULE m = LoadLibraryA("Shcore.dll");
		if (m) {
			typedef HRESULT(WINAPI* TSetProcessDpiAwareness)(int value);
			TSetProcessDpiAwareness proc = (TSetProcessDpiAwareness)GetProcAddress(m, "SetProcessDpiAwareness");
			if (proc){
				hr = proc(value);
				qInfo() << "set dpi result:" << hr;
			}
			FreeLibrary(m);
		}
		CoUninitialize();
	}
}


FramelessHelper::FramelessHelper(QObject* parent)
	: QObject(parent)
	, QAbstractNativeEventFilter()
{
}

void FramelessHelper::Attach(QWidget* window)
{
	if (window == attachWindow) {
		return;
	}
	Detach();
	if (!window) {
		return;
	}

	//wndShadow = new WndShadow(window);
	attachWindow = window;
	wid = attachWindow->winId();
	HWND hwnd = (HWND)wid;
	oldStyle = ::GetWindowLongW(hwnd, GWL_STYLE);
	const auto oldClassStyle = static_cast<DWORD>(GetClassLongPtrW(hwnd, GCL_STYLE));

	//const DWORD newClassStyle = (oldClassStyle & ~(CS_HREDRAW | CS_VREDRAW));
	//::SetClassLongPtrW(hwnd, GCL_STYLE, static_cast<LONG_PTR>(newClassStyle));

	static constexpr const DWORD normalWindowStyle =
		(WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_THICKFRAME | WS_MAXIMIZEBOX | CS_DBLCLKS);
	const DWORD newWindowStyle = ((oldStyle & ~WS_POPUP) | normalWindowStyle);
	//this line will get titlebar/thick frame/Aero back, which is exactly what we want
	//we will get rid of titlebar and thick frame again in nativeEvent() later	
	::SetWindowLongW(hwnd, GWL_STYLE, static_cast<LONG_PTR>(newWindowStyle));

	DWMNCRENDERINGPOLICY ncrp = DWMNCRP_ENABLED;
	::DwmSetWindowAttribute(hwnd, DWMWA_NCRENDERING_POLICY, &ncrp, sizeof(ncrp));

	const MARGINS shadow = { 1, 1, 1, 1 };
	::DwmExtendFrameIntoClientArea(hwnd, &shadow);

	//RECT frame = { 0, 0, 0, 0 };
	//AdjustWindowRectEx(&frame, WS_OVERLAPPEDWINDOW, FALSE, 0);
	////record frame area data
	//double dpr = this->devicePixelRatioF();
	//m_frames.setLeft(abs(frame.left) / dpr + 0.5);
	//m_frames.setTop(abs(frame.bottom) / dpr + 0.5);
	//m_frames.setRight(abs(frame.right) / dpr + 0.5);
	//m_frames.setBottom(abs(frame.bottom) / dpr + 0.5);

	//attachWindow->setAttribute(Qt::WA_TranslucentBackground);// for rounded corners
	attachWindow->setContentsMargins(borderSize, borderSize, borderSize, borderSize);
	attachWindow->installEventFilter(this);
	qApp->installNativeEventFilter(this);
}
void FramelessHelper::Detach(void)
{
	if (!attachWindow) {
		return;
	}
	HWND hwnd = (HWND)wid;
	qApp->removeNativeEventFilter(this);
	if (0 != oldStyle) {
		::SetWindowLong(hwnd, GWL_STYLE, oldStyle);
		oldStyle = 0;
	}
	const MARGINS shadow = { 0, 0, 0, 0 };
	::DwmExtendFrameIntoClientArea(hwnd, &shadow);

	attachWindow->setContentsMargins(0, 0, 0, 0);
	attachWindow->removeEventFilter(this);
	attachWindow = nullptr;
	wid = 0;
}

void FramelessHelper::SetResizeable(bool resizeable)
{
	isResizeable = resizeable;
	//if (resizeable) {
	//	attachWindow->setContentsMargins(borderSize, borderSize, borderSize, borderSize);
	//}
	//else {
	//	attachWindow->setContentsMargins(0, 0, 0, 0);
	//}
}

void FramelessHelper::SetBorderSize(int size)
{
	borderSize = size;
}

void FramelessHelper::SetResizeableBorder(int size)
{
	if (1 > size) {
		size = 1;
	}
	resizeSize = size;
}
void FramelessHelper::SetTitleBar(QWidget* title)
{
	if (titleBar) {
		titleBar->removeEventFilter(this);
	}
	titleBar = title;
	if (titleBar) {	
		title->installEventFilter(this);
	}
}
void FramelessHelper::SetTitleColor(const QColor& color)
{
	titleColor = color;
	if (attachWindow) {
		attachWindow->repaint();
	}
}
void FramelessHelper::SetFrameColor(const QColor& color)
{
	frameColor = color;
	if (attachWindow) {
		attachWindow->repaint();
	}
}

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
bool FramelessHelper::nativeEventFilter(const QByteArray& eventType,
	void* message,
	qintptr* result)
#else
bool FramelessHelper::nativeEventFilter(
	const QByteArray& eventType,
	void* message,
	long* result)
#endif
{
	// "result" can't be null in theory and I don't see any projects check
	// this, everyone is assuming it will never be null, including Microsoft,
	// but according to Lucas, frameless applications crashed on many Win7
	// machines because it's null. The temporary solution is also strange:
	// upgrade drivers or switch to the basic theme.
	if (!result) {
		return false;
	}
	//Workaround for known bug -> check Qt forum : https://forum.qt.io/topic/93141/qtablewidget-itemselectionchanged/13
#if (QT_VERSION == QT_VERSION_CHECK(5, 11, 1))
	MSG* msg = *reinterpret_cast<MSG**>(message);
#else
	MSG* msg = reinterpret_cast<MSG*>(message);
#endif
	if (!attachWindow || !msg || (msg && !msg->hwnd)) {
		// Why sometimes the window msg->hwnd is null? Is it designed to be?
		// Anyway, we should skip it in this case.
		return false;
	}
	if ((HWND)wid != msg->hwnd) {
		return false;
	}
	switch (msg->message)
	{
	case WM_NCCALCSIZE: {
		// This must always be first or else the one of other two ifs will execute
		//  when window is in full screen and we don't want that

		//this kills the window frame and title bar we added with WS_THICKFRAME and WS_CAPTION
		//auto client_area_needs_calculating = static_cast<bool>(msg->wParam);

		//if (client_area_needs_calculating) {
		//	auto parameters = reinterpret_cast<NCCALCSIZE_PARAMS*>(msg->lParam);
		//	auto& requested_client_area = parameters->rgrc[0];
		//	requested_client_area.right -= GetSystemMetrics(SM_CXFRAME) + GetSystemMetrics(SM_CXPADDEDBORDER);
		//	requested_client_area.left += GetSystemMetrics(SM_CXFRAME) + GetSystemMetrics(SM_CXPADDEDBORDER);
		//	//requested_client_area.top += GetSystemMetrics(SM_CYFRAME) + GetSystemMetrics(SM_CXPADDEDBORDER);// title will back
		//	requested_client_area.bottom -= GetSystemMetrics(SM_CYFRAME) + GetSystemMetrics(SM_CXPADDEDBORDER);

		//	// We got this!
		//	*result = 0;
		//	return true;
		//}
		*result = 0;
		return true;
	}// end case WM_NCCALCSIZE
	case WM_NCHITTEST: {
		*result = 0;
		const LONG border = resizeSize; //in pixels
		RECT winrect;
		::GetWindowRect(msg->hwnd, &winrect);

		long x = GET_X_LPARAM(msg->lParam);
		long y = GET_Y_LPARAM(msg->lParam);
		if (isResizeable) {
			bool resizeWidth = attachWindow->minimumWidth() != attachWindow->maximumWidth();
			bool resizeHeight = attachWindow->minimumHeight() != attachWindow->maximumHeight();
			if (resizeWidth) {
				//left border
				if (x >= winrect.left && x < winrect.left + border) {
					*result = HTLEFT;
				}
				//right border
				if (x < winrect.right && x >= winrect.right - border) {
					*result = HTRIGHT;
				}
			}
			if (resizeHeight) {
				//bottom border
				if (y < winrect.bottom && y >= winrect.bottom - border) {
					*result = HTBOTTOM;
				}
				//top border
				if (y >= winrect.top && y < winrect.top + border) {
					*result = HTTOP;
				}
			}
			if (resizeWidth && resizeHeight) {
				//bottom left corner
				if (x >= winrect.left && x < winrect.left + border &&
					y < winrect.bottom && y >= winrect.bottom - border) {
					*result = HTBOTTOMLEFT;
				}
				//bottom right corner
				if (x < winrect.right && x >= winrect.right - border &&
					y < winrect.bottom && y >= winrect.bottom - border) {
					*result = HTBOTTOMRIGHT;
				}
				//top left corner
				if (x >= winrect.left && x < winrect.left + border &&
					y >= winrect.top && y < winrect.top + border) {
					*result = HTTOPLEFT;
				}
				//top right corner
				if (x < winrect.right && x >= winrect.right - border &&
					y >= winrect.top && y < winrect.top + border) {
					*result = HTTOPRIGHT;
				}
			}
		}		
		if (0 != *result) {
			return true;
		}
		return false;
	} //end case WM_NCHITTEST
	case WM_GETMINMAXINFO:{
		const QSize minSize = attachWindow->minimumSize() * attachWindow->devicePixelRatioF();
		const QSize maxSize = attachWindow->maximumSize() * attachWindow->devicePixelRatioF();
		MINMAXINFO* p = (MINMAXINFO*)(msg->lParam);
		p->ptMinTrackSize.x = minSize.width();
		p->ptMinTrackSize.y = minSize.height();
		p->ptMaxTrackSize.x = maxSize.width();
		p->ptMaxTrackSize.y = maxSize.height();
		//*result = ::DefWindowProc(msg->hwnd, msg->message, msg->wParam, msg->lParam);
		*result = 0;
		return false;
	}// end case WM_GETMINMAXINFO
	default:
		return false;
	}
	return false;
}
bool FramelessHelper::eventFilter(QObject* obj, QEvent* event)
{
	do
	{
		if (obj != attachWindow) {
			break;
		}
		if (!titleBar) {
			break;
		}
		if (QEvent::WindowStateChange == event->type()) {
			if (Qt::WindowFullScreen ==
				(attachWindow->windowState() & Qt::WindowFullScreen)) {
				attachWindow->setContentsMargins(0, 0, 0, 0);
			}
			else {
				attachWindow->setContentsMargins(borderSize, borderSize, borderSize, borderSize);
			}
			break;
		}
		else if (QEvent::Resize == event->type()) {
			//const int radius = 10;
			//QPainterPath path;
			//path.addRoundedRect(attachWindow->rect(), radius, radius);
			//QRegion mask = QRegion(path.toFillPolygon().toPolygon());
			//attachWindow->setMask(mask);
			break;
		}
		else if (QEvent::Paint == event->type()) {
			if (attachWindow->isFullScreen()) {
				break;
			}
			if (!titleColor.isValid() && !frameColor.isValid()) {
				break;
			}

			QColor color = titleColor;
			QPen pen = {};
			//pen.setStyle(Qt::SolidLine);
			pen.setWidth(borderSize + 1);
			QSize titleSize = titleBar->size();
			QSize frameSize = attachWindow->size();

			QPainter painter(attachWindow);	
			// draw title border
			if (!titleColor.isValid()) {
				color = frameColor;
			}
			pen.setColor(color);
			//pen.setColor(QColor(255, 0, 0));			
			painter.setPen(pen);
			// In fact, we should use "width() - 1" here but we can't
			// because Qt's drawing system has some rounding errors internally and if
			// we minus one here we'll get a one pixel gap, so sad. But drawing a line
			// with a little extra pixels won't hurt anyway.
			painter.drawLine(0, 0, frameSize.width(), 0);
			painter.drawLine(0, 0, 0, titleSize.height() + 1);
			painter.drawLine(titleSize.width() + borderSize, 0, titleSize.width() + borderSize, titleSize.height() + 1);

			// draw frame border
			color = frameColor;
			if (!frameColor.isValid()) {
				color = QColor(255, 255, 255);
			}
			pen.setColor(color);
			//pen.setColor(QColor(255, 0, 0));
			painter.setPen(pen);
			painter.drawLine(0, titleSize.height() + borderSize + 1, 0, frameSize.height());
			painter.drawLine(titleSize.width() + borderSize, titleSize.height() + borderSize + 1, 
				titleSize.width() + borderSize, frameSize.height());
			painter.drawLine(0, frameSize.height(), frameSize.width(), frameSize.height());		
			break;
		}
	} while (false);
	do
	{
		if (obj != titleBar) {
			break;
		}
		if (!attachWindow) {
			break;
		}
		
		if (event->type() == QEvent::MouseButtonPress || event->type() == QEvent::MouseMove ||
			event->type() == QEvent::MouseButtonRelease || event->type() == QEvent::MouseButtonDblClick) {
			QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);

			static bool isStartDragWindow = false;
			static QPoint startPos;
			static QPoint startMousePos;
			if (event->type() == QEvent::MouseButtonPress) {
				if (mouseEvent->button() == Qt::LeftButton) {
					isStartDragWindow = true;
					//moveWindow->windowHandle()->startSystemMove();
					startPos = attachWindow->pos();
					startMousePos = mouseEvent->globalPos();
				}
			}
			else if (event->type() == QEvent::MouseMove) {
				if (isResizeable && isStartDragWindow && attachWindow->isMaximized()) {
					attachWindow->showNormal();
					startPos = attachWindow->pos();
					startMousePos = mouseEvent->globalPos();
				}
				else if (isStartDragWindow) {
					attachWindow->move(startPos + mouseEvent->globalPos() - startMousePos);
				}
			}
			else if (event->type() == QEvent::MouseButtonRelease) {
				if (isStartDragWindow) {
					isStartDragWindow = false;
				}
			}
			else {
				if (isResizeable) {
					attachWindow->isMaximized() ? attachWindow->showNormal() : attachWindow->showMaximized();
				}				
			}
		}
	} while (false);
	
	return QObject::eventFilter(obj, event);
}