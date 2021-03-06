#pragma once
#include <QLabel>
#include <QWidget>
#include <QPushButton>
#include <QHBoxLayout>

class WindowHelper;


enum TitleBtnType {
	TITLE_MIN_BTN = 0X0001,
	TITLE_MAX_BTN = 0X0002,
	TITLE_CLOSE_BTN = 0X0004,
	TITLE_ALL_BTN = TITLE_MIN_BTN | TITLE_MAX_BTN | TITLE_CLOSE_BTN
};
Q_DECLARE_FLAGS(TitleBtnTypes, TitleBtnType)
Q_DECLARE_OPERATORS_FOR_FLAGS(TitleBtnTypes)

class TitleBar : public QWidget
{
    Q_OBJECT
public:
	/*! This is an overloaded function.
	 *  it will handle window on the top window of this title bar, should be used on designer
	 * \brief TitleBar
	 * \param parent
	 */
	explicit TitleBar(QWidget *parent);

	
	void Init(QWidget* parent, TitleBtnTypes type);
	QPushButton* Button(TitleBtnTypes type);
	void SetResizable(bool resizable);
	bool IsResizable(void) const;
	void SetSimplifyMode(bool mode);
	bool IsSimplifyMode(void) const;


	void SetFullScreen(bool full);
	void SetMaximize(bool max);
	void SetMaximizeStatus(bool status);
	bool IsMaximize(void) const;
	void ShowNormal(void);
	void ShowMaximize(void);

	void SetLogoable(bool enable, const QSize& size);
	void SetTitleable(bool enable);
	

	void SetTitleName(const QString& titleName);
	QString GetTitleName(void) const;

	QHBoxLayout* GetMainLayout(void) const;
	QHBoxLayout* GetLeftLayout(void) const;
	QHBoxLayout* GetCenterLayout(void) const;
	QHBoxLayout* GetRightLayout(void) const;

signals:
	void SignalClose(void);
	void SignalMaximize(bool maximize);

protected:
	virtual void mousePressEvent(QMouseEvent *me);
	virtual void mouseMoveEvent(QMouseEvent* me);
	virtual void mouseReleaseEvent(QMouseEvent *me);
	virtual void mouseDoubleClickEvent(QMouseEvent *event);
	virtual bool eventFilter(QObject* obj, QEvent* e);

protected:
	QWidget* windowWidget = nullptr;
	QWidget* titleBg = nullptr;
	QLabel* logoLabel = nullptr;
	QLabel* titleLabel = nullptr;
	QPushButton* minimizeBtn = nullptr;
	QPushButton* maximizeBtn = nullptr;
	QPushButton* closeBtn = nullptr;

	// title left space center space right min max close
	QWidget* leftExtendWidget = nullptr;
	QWidget* centerEntendWidget = nullptr;
	QWidget* rightExtendWidget = nullptr;

	QPoint startPos;
	QPoint clickPos;
	bool isMoving = false;
	bool isPressed = false;
	//bool isDoubleClick = false;
	bool isSimplifyMode = false;

	QRect rcOldGeometry;
	WindowHelper* windowHelper = nullptr;
};
///////////////////////////////////////////////////
class CustomTitle :public TitleBar
{
	Q_OBJECT
public:
	CustomTitle(QWidget*parent);
};
