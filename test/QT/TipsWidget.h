#pragma once
#include <QWidget>
#include <QTimer>
#include <QPointer>
#include <QPropertyAnimation>

class TipsWidget :public QWidget
{
	Q_OBJECT
public:
	typedef enum {
		TIP_NOICON = 0,
		TIP_INFO,
		TIP_SUCCESS,
		TIP_WARN,
		TIP_ERROR
	}TipType;
	typedef enum {
		TIP_POS_TOP,
		TIP_POS_BOTTOM,
		TIP_POS_CENTER,
	}PosType;

	// if @duration=0 no auto hide
	TipsWidget(QWidget* parent, const QString& Tips, TipType level, PosType pos,
		int duration, bool mouseNoHide, bool showCloseBtn, bool autoMove, bool toast);
	~TipsWidget();
public:
	static TipsWidget* Show(QWidget *parent, const QString& tips);
	static TipsWidget* ShowLevel(QWidget *parent, const QString& tips, TipType level);
	static TipsWidget* ShowWithParam(QWidget *parent, const QString& tips, TipType level,
		PosType pos, int duration, bool mouseNoHide, bool showCloseBtn, bool autoMove);

	static TipsWidget* ShowToast(QWidget* parent, const QString& tips);
protected:
	virtual bool eventFilter(QObject* obj, QEvent* e) override;
	virtual void enterEvent(QEvent *event) override;
	virtual void leaveEvent(QEvent *event) override;
private:
	void Start();
	void Stop();
	void SetPos(PosType pos);
private:
	QPointer<QPropertyAnimation> animation;
	bool isMouseNoHide;
	PosType posType;
	int animationDuration;// second
	QObject* moveFilter = nullptr;
	QTimer* showTimer = nullptr;
};