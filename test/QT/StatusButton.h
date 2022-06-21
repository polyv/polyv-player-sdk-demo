#pragma once
#include <QPushButton>

class StatusButton : public QPushButton {
	Q_OBJECT
public:
	explicit StatusButton(QWidget *parent = nullptr);
	explicit StatusButton(QWidget *parent, const QString& name, const QString& text);
	explicit StatusButton(QWidget *parent, const QString& name, const QString& normal, const QString& check);
	explicit StatusButton(QWidget *parent, const QString& name, const QString& tooltip, const QSize& size);
	explicit StatusButton(QWidget *parent, const QString& name, const QString& normal, const QString& check, const QSize& size);

public:
	void SetShowText(bool showText);
	void SetContent(const QString& normal, const QString& check);
protected:
	virtual void checkStateSet(void) override;
	virtual void mouseReleaseEvent(QMouseEvent *e) override;

protected:
	void UpdateStatus(void);

protected:
	bool isShowText = false;
	QString normalText;
	QString checkText;
};
