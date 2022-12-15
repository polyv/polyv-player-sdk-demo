#include "TipsWidget.h"

#include <QLabel>
#include <QPushButton>
#include <QHBoxLayout>

#include "Application.h"
#include "WidgetHelper.h"

TipsWidget::TipsWidget(QWidget* parent, const QString& Tips, TipType level, PosType pos,
	int duration, bool mouseNoHide, bool showCloseBtn, bool autoMove, bool toast)
	: QWidget(parent)
	, isMouseNoHide(mouseNoHide)
	, animationDuration(duration * 1000)
{
	setMouseTracking(true);
	setAttribute(Qt::WA_DeleteOnClose);
	setAttribute(Qt::WA_TranslucentBackground);
	setAttribute(Qt::WA_NoSystemBackground, false);

	setWindowFlags(Qt::FramelessWindowHint | Qt::Tool | Qt::NoDropShadowWindowHint);
	setMinimumHeight(32);

	QWidget* mainWidget = new QWidget(this);
	if (toast) {
		mainWidget->setObjectName("toastWidget");
		setMinimumHeight(40);
	}
	else {
		mainWidget->setObjectName(QString("%1%2").arg(QStringLiteral("tipWidget")).arg(level));
	}
	

	QHBoxLayout* layout = new QHBoxLayout(this);
	layout->setContentsMargins(0, 0, 0, 0);
	layout->addWidget(mainWidget);
	setLayout(layout);

	QLabel*tipsIcon = new QLabel(mainWidget);
	tipsIcon->setObjectName(toast ? QStringLiteral("toastIcon") : QStringLiteral("tipsIcon"));
	tipsIcon->setMinimumSize(16, 16);
	tipsIcon->setProperty("level", level);
	if (TIP_NOICON == level) {
		tipsIcon->setVisible(false);
	}

	QLabel*tipsLabel = new QLabel(Tips, mainWidget);
	tipsLabel->setObjectName(toast ? QStringLiteral("toastLabel") : QStringLiteral("tipsLabel"));
	tipsLabel->setProperty("level", level);
	tipsLabel->adjustSize();
	//tipsLabel->setMinimumHeight(20);
	tipsLabel->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);

	QPushButton*closeButton = new QPushButton(mainWidget);
	closeButton->setObjectName(QStringLiteral("closeButton"));
	closeButton->setMinimumSize(QSize(16, 16));
	closeButton->setToolTip(QTStr("Close"));
	closeButton->setCursor(QCursor(Qt::PointingHandCursor));
	connect(closeButton, SIGNAL(clicked()), this, SLOT(close()));
	closeButton->setVisible(showCloseBtn);

	QHBoxLayout*mainLayout = new QHBoxLayout();
	mainLayout->setContentsMargins(16, 8, 16, 8);
	mainLayout->setSpacing(8);
	mainLayout->addWidget(tipsIcon);
	mainLayout->addWidget(tipsLabel);
	mainLayout->addWidget(closeButton);
	mainWidget->setLayout(mainLayout);
	//setMinimumSize(218, 32);

	setFixedWidth(sizeHint().width());

	if (0 != animationDuration) {
		animation = new QPropertyAnimation(this, "windowOpacity");
		if (!showTimer) {
			showTimer = new QTimer(this);
			connect(showTimer, &QTimer::timeout, this, [this] {
				showTimer->stop();
				Start();
			});
		}
		showTimer->start(1 * 1000);
	}
	if (parent) {
		parent->installEventFilter(this);
	}
	if (autoMove) {
		moveFilter = GetRootParent(parent);
	}
	if (moveFilter) {
		moveFilter->installEventFilter(this);
	}
	SetPos(pos);
}
TipsWidget::~TipsWidget()
{
	if (parent()) {
		parent()->removeEventFilter(this);
	}
	if (showTimer) {
		showTimer->stop();
		showTimer->deleteLater();
	}
	if (moveFilter) {
		moveFilter->removeEventFilter(this);
	}
	if (animation) {
		animation->deleteLater();
	}
}
	
bool TipsWidget::eventFilter(QObject* obj, QEvent* e)
{
	do
	{
		if (obj != parent()) {
			break;
		}
		if (e->type() == QEvent::Resize ||
			e->type() == QEvent::Move) {
			SetPos(posType);
		}
	} while (false);
	do
	{
		if (!moveFilter || moveFilter != obj) {
			break;
		}
		if (e->type() == QEvent::Resize ||
			e->type() == QEvent::Move) {
			SetPos(posType);
		}
	} while (false);

	return QWidget::eventFilter(obj, e);
}

void TipsWidget::enterEvent(QEvent *event)
{
	QWidget::enterEvent(event);
	if (!isMouseNoHide) {
		return;
	}
	if (!geometry().contains(QCursor::pos())) {
		return;
	}
	Stop();
}
void TipsWidget::leaveEvent(QEvent *event)
{
	QWidget::leaveEvent(event);
	if (!isMouseNoHide) {
		return;
	}
	if (geometry().contains(QCursor::pos())) {
		return;
	}
	Start();
}

void TipsWidget::Start()
{
	if (!animation) {
		return;
	}
	connect(animation, SIGNAL(finished()), this, SLOT(close()));
	animation->setDuration(animationDuration);
	animation->setKeyValueAt(0, 1);
	animation->setKeyValueAt(0.5, 1);
	animation->setKeyValueAt(0.8, 0.5);
	animation->setKeyValueAt(1, 0);
	animation->start(QAbstractAnimation::KeepWhenStopped);
}
void TipsWidget::Stop()
{
	if (!animation) {
		return;
	}
	animation->disconnect();
	animation->stop();
	animation->resume();
	animation->setDuration(10);
	animation->setKeyValueAt(0, 1);
	animation->setKeyValueAt(0.5, 1);
	animation->setKeyValueAt(0.8, 1);
	animation->setKeyValueAt(1, 1);
	animation->start(QAbstractAnimation::KeepWhenStopped);
	animation->stop();
}

void TipsWidget::SetPos(PosType point)
{
	posType = point;
	QWidget* widget = (QWidget*)parent();
	if (!widget) {
		return;
	}
	QPoint itemPos = widget->mapToGlobal(QPoint(0, 0));
	switch (point)
	{
	case TIP_POS_TOP:
		move((itemPos.x() + (widget->width() - width()) / 2), itemPos.y() + 10);
		break;
	case TIP_POS_BOTTOM:
		move(itemPos.x() + (widget->width() - width()) / 2, itemPos.y() + widget->height() - height() - 70);
		break;
	case TIP_POS_CENTER:
		move(itemPos.x() + (widget->width() - width()) / 2, itemPos.y() + (widget->height() - height()) / 2);
		break;
	}
}

TipsWidget* TipsWidget::Show(QWidget *parent, const QString& tips)
{
	return ShowWithParam(parent, tips, TIP_INFO, TIP_POS_TOP, 1, false, false, true);
}

TipsWidget* TipsWidget::ShowLevel(QWidget *parent, const QString& tips, TipType level)
{
	return ShowWithParam(parent, tips, level, TIP_POS_TOP, 1, false, false, true);
}

TipsWidget* TipsWidget::ShowWithParam(QWidget *parent, const QString& tips, TipType level,
	PosType pos, int duration, bool mouseNoHide, bool showCloseBtn, bool autoMove)
{
	TipsWidget *widget = new TipsWidget(parent, tips, level, pos, duration, mouseNoHide, showCloseBtn, autoMove, false);
	widget->show();
	return widget;
}

TipsWidget* TipsWidget::ShowToast(QWidget* parent, const QString& tips)
{
	TipsWidget *widget = new TipsWidget(parent, tips, TIP_INFO, TIP_POS_CENTER, 1, false, false, true, true);
	widget->show();
	return widget;
}



