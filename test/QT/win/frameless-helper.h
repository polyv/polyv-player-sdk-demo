// only in window
#pragma once
#include <QWidget>
#include <QAbstractNativeEventFilter>

extern void SetDpiAwareness(int value);

class FramelessHelper : public QObject, public QAbstractNativeEventFilter {
	Q_OBJECT
public:
	explicit FramelessHelper(QObject* parent = nullptr);
public:
	void Attach(QWidget* window);
	void Detach();

	void SetBorderSize(int size);

	//if resizeable is set to false, then the window can not be resized by mouse
	//but still can be resized programtically
	void SetResizeable(bool resizeable);
	bool IsResizeable() const { 
		return isResizeable; 
	}
	//set border width, inside this aera, window can be resized by mouse
	void SetResizeableBorder(int size);

	void SetTitleBar(QWidget* title);
	// draw frame border
	void SetTitleColor(const QColor& color);
	void SetFrameColor(const QColor& color);
	
protected: 
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
    bool nativeEventFilter(const QByteArray &eventType, void *message, qintptr *result) override;
#else
    bool nativeEventFilter(const QByteArray &eventType, void *message, long *result) override;
#endif
	bool eventFilter(QObject* obj, QEvent* event) override;
private:
	WId wid = 0;
	QWidget* attachWindow = nullptr;
	QWidget* titleBar = nullptr;
	int borderSize = 1;
	int resizeSize = 5;
	bool isResizeable = false;
    unsigned long oldStyle = 0;

	QColor titleColor;
	QColor frameColor = QColor(255, 255, 255);
};
