#pragma once
#include <QObject>
#include <QWidget>
class WindowHelperPrivate;
class WindowHelper : public QObject {
	Q_OBJECT
public:
	WindowHelper(QObject * parent = 0);
	~WindowHelper(void);
public:
	void ActivateOn(QWidget* pTopLevelWidget);
	void RemoveFrom(QWidget* pTopLevelWidget);

	void SetWidgetMovable(bool movable);
	bool IsWidgetMovable(void) const;

	void SetWidgetResizable(bool resizable);
	bool IsWidgetResizable(void) const;

	void SetRubberBandOnMove(bool use);
	bool IsRubberBandOnMove(void) const;

	void SetRubberBandOnResize(bool use);
	bool IsRubberBandOnResize(void) const;

	void SetBorderWidth(int width);
	int BorderWidth(void) const;

protected:
	virtual bool eventFilter(QObject* obj, QEvent* event);

private:
	WindowHelperPrivate* d_ptr;
};