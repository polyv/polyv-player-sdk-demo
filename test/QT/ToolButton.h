#pragma once

#include <QIcon>
#include <QToolButton>


class ToolButton : public QToolButton {
	Q_OBJECT

public:
	explicit ToolButton(QWidget*parent = nullptr);
	void SetNormalIcon(const QString& normalIcon, const QString& activeIcon);
	void SetCheckIcon(const QString& NormalIcon, const QString& activeIcon);
	void SetDisableIcon(const QString& disableIcon);
protected:
	virtual void checkStateSet(void) override;
	virtual bool eventFilter(QObject *watched, QEvent *event) override;
private:
	QIcon normalIcon;
	QIcon activeIcon;
	QIcon checkedNormalIcon;
	QIcon checkedActiveIcon;
	QIcon disableIcon;
};
