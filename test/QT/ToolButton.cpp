#include "ToolButton.h"

#include <QMouseEvent>

ToolButton::ToolButton(QWidget*parent)
	: QToolButton(parent)
{
	installEventFilter(this);
	setCursor(Qt::PointingHandCursor);
}

void ToolButton::SetNormalIcon(const QString& iconNormal, const QString& iconActive)
{
	normalIcon.addFile(iconNormal);
	activeIcon.addFile(iconActive);
	setIcon(normalIcon);
}
void ToolButton::SetCheckIcon(const QString& iconNormal, const QString& iconActive)
{
	setCheckable(true);
	checkedNormalIcon.addFile(iconNormal);
	checkedActiveIcon.addFile(iconActive);
}

void ToolButton::SetDisableIcon(const QString& disableIcon_)
{
	disableIcon.addFile(disableIcon_);
}

void ToolButton::checkStateSet(void)
{
	if (isCheckable()) {
		setIcon(isChecked() ? checkedNormalIcon : normalIcon);
	}
}

bool ToolButton::eventFilter(QObject *watched, QEvent *event)
{
	do {
		if (watched != this) {
			break;
		}
		if (event->type() == QEvent::Show || event->type() == QEvent::Leave || event->type() == QEvent::EnabledChange) {
			if (isEnabled()) {
				if (isCheckable() && isChecked()) {
					setIcon(checkedNormalIcon);
				} else {
					setIcon(normalIcon);
				}
			} else {
				setIcon(disableIcon);
			}
		} else if (event->type() == QEvent::Enter) {
			if (isEnabled()) {
				if (isCheckable() && isChecked()) {
					setIcon(checkedActiveIcon);
				} else {
					setIcon(activeIcon);
				}
			} else {
				setIcon(disableIcon);
			}
		} else if (event->type() == QEvent::MouseButtonRelease) {
			if (isCheckable()) {
				setIcon(isChecked() ? checkedNormalIcon : normalIcon);
			}
		} 
	} while (0);
	return QToolButton::eventFilter(watched, event);
}
