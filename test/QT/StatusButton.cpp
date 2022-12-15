#include "StatusButton.h"



StatusButton::StatusButton(QWidget *parent/* = nullptr*/)
	: QPushButton(parent)
{
	setAutoDefault(false);
	setDefault(false);
	setCursor(QCursor(Qt::PointingHandCursor));
	setFocusPolicy(Qt::NoFocus);
}

StatusButton::StatusButton(QWidget *parent, const QString& name, const QString& text)
	: QPushButton(parent)
{
	setAutoDefault(false);
	setDefault(false);
	setCursor(QCursor(Qt::PointingHandCursor));
	setFocusPolicy(Qt::NoFocus);
	setObjectName(name);
	setText(text);
	SetShowText(true);
}


StatusButton::StatusButton(QWidget *parent, const QString& name, const QString& normal, const QString& check)
	: QPushButton(parent)
{
	setAutoDefault(false);
	setDefault(false);
	setCursor(QCursor(Qt::PointingHandCursor));
	setFocusPolicy(Qt::NoFocus);
	setObjectName(name);
	setCheckable(true);
	setChecked(false);
	SetContent(normal, check);
	SetShowText(true);
	setText(normal);
}

StatusButton::StatusButton(QWidget *parent, const QString& name, const QString& tooltip, const QSize& size)
	: QPushButton(parent)
{
	setAutoDefault(false);
	setDefault(false);
	setCursor(QCursor(Qt::PointingHandCursor));
	setFocusPolicy(Qt::NoFocus);
	setObjectName(name);
	setToolTip(tooltip);
	SetShowText(false);
	setFixedSize(size);
}


StatusButton::StatusButton(QWidget *parent, const QString& name, const QString& normal, const QString& check, const QSize& size)
	: QPushButton(parent)
{
	setAutoDefault(false);
	setDefault(false);
	setCursor(QCursor(Qt::PointingHandCursor));
	setFocusPolicy(Qt::NoFocus);
	setObjectName(name);
	setCheckable(true);
	setChecked(false);
	SetContent(normal, check);
	SetShowText(false);
	setToolTip(normal);
	setFixedSize(size);
}

void StatusButton::SetShowText(bool showText)
{
	isShowText = showText;
}
void StatusButton::SetContent(const QString& normal, const QString& check)
{
	normalText = normal;
	checkText = check;
	SetShowText(true);
}

void StatusButton::checkStateSet(void)
{
	UpdateStatus();
	QPushButton::checkStateSet();
}

void StatusButton::mouseReleaseEvent(QMouseEvent *e)
{
	UpdateStatus();
	QPushButton::mouseReleaseEvent(e);
}

void StatusButton::UpdateStatus(void)
{
	if (!isCheckable()) {
		return;
	}
	if (isShowText) {
		setText(isChecked() ? checkText : normalText);
	}
	else {
		setToolTip(isChecked() ? checkText : normalText);
	}
}
