#include "TitleBar.h"

#include <QHBoxLayout>
#include <QPushButton>
#include <QMouseEvent>

#include "Application.h"
#include "StatusButton.h"
#include "WidgetHelper.h"
#include "WindowHelper.h"

TitleBar::TitleBar(QWidget *parent) 
	: QWidget(parent)
{
	setObjectName(QStringLiteral("titleBar"));
	titleBg = new QWidget(this);
	titleBg->setObjectName(QStringLiteral("titleBg")); 
	QHBoxLayout *layout = new QHBoxLayout();
	layout->addWidget(titleBg);
	layout->setContentsMargins(0, 0, 0, 0);
	setLayout(layout); 

	logoLabel = new QLabel(titleBg);
	logoLabel->setObjectName(QStringLiteral("logoLabel"));
	logoLabel->setVisible(titleBg);

	titleLabel = new QLabel(titleBg);
	titleLabel->setObjectName(QStringLiteral("titleLabel"));
	titleLabel->setMinimumHeight(16);
	titleLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
	titleLabel->setVisible(false);

	auto CreateWidget = [](QWidget* parent) {
		QWidget* widget = new QWidget(parent);
		QHBoxLayout*layout = new QHBoxLayout();
		layout->setSpacing(16);
		layout->setContentsMargins(0, 0, 0, 0);
		widget->setLayout(layout);
		return widget;
	};

	leftExtendWidget = CreateWidget(titleBg);
	centerEntendWidget = CreateWidget(titleBg);
	rightExtendWidget = CreateWidget(titleBg);

	QHBoxLayout* hlayout = GetLeftLayout();
	hlayout->addWidget(logoLabel);
	hlayout->addWidget(titleLabel);

	minimizeBtn = new StatusButton(titleBg, QStringLiteral("minimize"), QTStr("Minimize"), QSize(24, 24));
	minimizeBtn->setVisible(false);
	maximizeBtn = new StatusButton(titleBg, QStringLiteral("maximize"), QTStr("Maximize"), QTStr("Restore"), QSize(24, 24));
	maximizeBtn->setVisible(false);
	closeBtn = new StatusButton(titleBg, QStringLiteral("close"), QTStr("Close"), QSize(24, 24));

	hlayout = GetRightLayout();
	hlayout->addWidget(minimizeBtn);
	hlayout->addWidget(maximizeBtn);
	hlayout->addWidget(closeBtn);

	QHBoxLayout *mainLayout = new QHBoxLayout(); 
	mainLayout->setSpacing(16);
	mainLayout->setContentsMargins(16, 0, 16, 0);
	mainLayout->addWidget(leftExtendWidget);
	mainLayout->addStretch();
	mainLayout->addWidget(centerEntendWidget);
	mainLayout->addStretch();
	mainLayout->addWidget(rightExtendWidget);

#ifdef __APPLE__
	minimizeBtn->setVisible(false);
	maximizeBtn->setVisible(false);
	closeBtn->setVisible(false);
#endif
	titleBg->setLayout(mainLayout);

	windowWidget = (QWidget*)GetRootParent(parent);
}

void TitleBar::Init(QWidget* parent, TitleBtnTypes type)
{
	windowWidget = parent;
	bool minable = (TITLE_MIN_BTN == (type & TITLE_MIN_BTN));
	bool maxable = (TITLE_MAX_BTN == (type & TITLE_MAX_BTN));
	bool closeable = (TITLE_CLOSE_BTN == (type & TITLE_CLOSE_BTN));
#ifdef _WIN32
	minimizeBtn->setVisible(minable);
	maximizeBtn->setVisible(maxable);
	closeBtn->setVisible(closeable);
#endif
	if (minable) {
		connect(minimizeBtn, SIGNAL(clicked()), windowWidget, SLOT(showMinimized()));
	}
	if (maxable) {
		connect(maximizeBtn, &QPushButton::clicked, [&](bool max) {
			max ? ShowMaximize() : ShowNormal();
		});
		connect(maximizeBtn, SIGNAL(clicked(bool)), this, SIGNAL(SignalMaximize(bool)));
	}
	if (closeable) {
		connect(closeBtn, SIGNAL(clicked()), this, SIGNAL(SignalClose()));
		connect(closeBtn, SIGNAL(clicked()), windowWidget, SLOT(close()));
	}
	windowWidget->installEventFilter(this);
}

QPushButton* TitleBar::Button(TitleBtnTypes type)
{
	switch (type) 
	{
	case TITLE_MIN_BTN:
		return minimizeBtn;
	case TITLE_MAX_BTN:
		return maximizeBtn;
	case TITLE_CLOSE_BTN:
		return closeBtn;
	}
	return nullptr;
}

void TitleBar::SetResizable(bool resizable)
{
	do
	{
		if (!windowWidget) {
			break;
		}
		if (!resizable) {
			if (!windowHelper) {
				break;
			}
			windowHelper->RemoveFrom(windowWidget);
			delete windowHelper;
			windowHelper = nullptr;
			break;
		}
		windowHelper = new WindowHelper(this);
		windowHelper->ActivateOn(windowWidget);
		windowHelper->SetWidgetMovable(false);
		windowHelper->SetWidgetResizable(true);
		windowHelper->SetBorderWidth(1);
	} while (false);
}
bool TitleBar::IsResizable(void) const
{
	return windowHelper ? true : false;
}

void TitleBar::SetSimplifyMode(bool mode)
{
	isSimplifyMode = mode;
	if (windowHelper) {
		windowHelper->SetWidgetResizable(!mode);
	}
}
bool TitleBar::IsSimplifyMode(void) const
{
	return isSimplifyMode;
}

void TitleBar::SetFullScreen(bool full)
{
	if (windowHelper) {
		windowHelper->SetWidgetResizable(!full);
	}
}

void TitleBar::ShowNormal(void)
{
	windowWidget->showNormal();
}

void TitleBar::ShowMaximize(void)
{
	windowWidget->showMaximized();
}

void TitleBar::SetLogoable(bool enable, const QSize& size)
{
	logoLabel->setVisible(enable);
	logoLabel->setFixedSize(size);
}
void TitleBar::SetTitleable(bool enable)
{
	titleLabel->setVisible(enable);
}

void TitleBar::SetMaximize(bool max)
{
	if (maximizeBtn->isVisible()) {
		maximizeBtn->clicked(max);
		maximizeBtn->setChecked(max);
	}
}
void TitleBar::SetMaximizeStatus(bool status)
{
	if (maximizeBtn->isVisible()) {
		maximizeBtn->setChecked(status);
	}
}
bool TitleBar::IsMaximize(void) const
{
	return maximizeBtn->isChecked();
}
void TitleBar::SetTitleName(const QString& name)
{
	titleLabel->setText(name);
}
QString TitleBar::GetTitleName(void) const
{
	return titleLabel->text();
}

QHBoxLayout* TitleBar::GetMainLayout(void) const
{
	return (QHBoxLayout*)titleBg->layout();
}
QHBoxLayout* TitleBar::GetLeftLayout(void) const
{
	return (QHBoxLayout*)leftExtendWidget->layout();
}
QHBoxLayout* TitleBar::GetCenterLayout(void) const
{
	return (QHBoxLayout*)centerEntendWidget->layout();
}
QHBoxLayout* TitleBar::GetRightLayout(void) const
{
	return (QHBoxLayout*)rightExtendWidget->layout();
}


void TitleBar::mousePressEvent(QMouseEvent *e)
{
	if (Qt::LeftButton == (e->buttons() & Qt::LeftButton)) {
		isPressed = true;
	}
	startPos = e->globalPos();
	clickPos = mapToParent(e->pos())/* + QPoint(16, 16)*/;
	//e->ignore();
	return QWidget::mousePressEvent(e);
}
void TitleBar::mouseMoveEvent(QMouseEvent* e)
{
	do
	{
		if (!windowWidget || !isPressed) {
			break;
		}
		if (!isMoving) {
			isMoving = true;
			if (IsMaximize() && !IsSimplifyMode()) {
				//auto pos = e->globalPos() - clickPos;
				//if (pos.x() > 3 || pos.y() > 3) {
				windowWidget->showNormal();
				//SetMaximizeStatus(false);
				//}
			}
		}			
		windowWidget->move(e->globalPos() - clickPos);
	} while (false);
	return QWidget::mousePressEvent(e);
}
void TitleBar::mouseReleaseEvent(QMouseEvent *e)
{
	isMoving = false;
	isPressed = false;
	//isDoubleClick = false;
	return QWidget::mouseMoveEvent(e);
}

void TitleBar::mouseDoubleClickEvent(QMouseEvent *e)
{
	if (!IsSimplifyMode() && (maximizeBtn && maximizeBtn->isVisible())) {
		isMoving = false;
		isPressed = false;
		//isDoubleClick = true;
		maximizeBtn->click();
		return;
	}
	return QWidget::mouseDoubleClickEvent(e);
}

bool TitleBar::eventFilter(QObject* obj, QEvent* e)
{
	do
	{
		if (windowWidget != obj) {
			break;
		}
		if (QEvent::WindowStateChange != e->type()) {
			break;
		}
		switch (windowWidget->windowState())
		{
		case Qt::WindowMaximized:
			SetMaximizeStatus(true);
			//ShowMaximize();
			break;
		default:
			SetMaximizeStatus(false);
			break;
		}
	} while (false);
	return QWidget::eventFilter(obj, e);
}

///////////////////////////////////////////////////
CustomTitle::CustomTitle(QWidget* parent) :
	TitleBar(parent)
{
	Init((QWidget*)GetRootParent(parent), TITLE_CLOSE_BTN);
	SetResizable(false);
	SetLogoable(false, QSize());
	SetTitleable(true);
}