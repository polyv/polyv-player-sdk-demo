#include "TitleBar.h"

#include <QHBoxLayout>
#include <QPushButton>
#include <QMouseEvent>

#include "Application.h"
#include "StatusButton.h"
#include "WidgetHelper.h"
//#include "WindowHelper.h"
//#include "shadow-helper.h"
#ifdef _WIN32
#include "frameless-helper.h"
#endif

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
}

TitleBar::~TitleBar()
{
#ifdef _WIN32
	if (framelessHelper) {
		framelessHelper->deleteLater();
		framelessHelper = nullptr;
	}
#endif
	
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
	maximizeBtn->disconnect();
	minimizeBtn->disconnect();
	closeBtn->disconnect();
	if (minable) {
		windowWidget->setWindowFlags(windowWidget->windowFlags() | Qt::WindowMinimizeButtonHint);
		connect(minimizeBtn, SIGNAL(clicked()), windowWidget, SLOT(showMinimized()));
	}
	else {
		windowWidget->setWindowFlags(windowWidget->windowFlags() & ~Qt::WindowMinimizeButtonHint);
	}
	if (maxable) {
		windowWidget->setWindowFlags(windowWidget->windowFlags() | Qt::WindowMaximizeButtonHint);
		connect(maximizeBtn, &QPushButton::clicked, this, [this](bool max) {
			max ? ShowMaximize() : ShowNormal();
		});
	}
	else {
		windowWidget->setWindowFlags(windowWidget->windowFlags() & ~Qt::WindowMaximizeButtonHint);
	}
	if (closeable) {
		windowWidget->setWindowFlags(windowWidget->windowFlags() | Qt::WindowCloseButtonHint);
		connect(closeBtn, SIGNAL(clicked()), this, SIGNAL(SignalClose()));
		connect(closeBtn, SIGNAL(clicked()), windowWidget, SLOT(close()));
	}
	else {
		windowWidget->setWindowFlags(windowWidget->windowFlags() & ~Qt::WindowCloseButtonHint);
	}
	windowWidget->installEventFilter(this);

#ifdef _WIN32
	if (framelessHelper) {
		framelessHelper->Attach(windowWidget);
	}
	else {
		SetResizable(false);
	}
#endif
	
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
#ifdef _WIN32
	if (framelessHelper) {
		framelessHelper->SetResizeable(resizable);
		if (resizable) {
			framelessHelper->SetTitleColor(QColor(15, 117, 255));
		}
	}
	else {
		framelessHelper = new FramelessHelper(this);
		framelessHelper->SetTitleBar(this);
		if (windowWidget) {
			framelessHelper->Attach(windowWidget);
		}	
		framelessHelper->SetResizeable(resizable);
		if (resizable) {
			framelessHelper->SetTitleColor(QColor(15, 117, 255));
		}
		framelessHelper->SetFrameColor(QColor(255, 255, 255));
	}
#endif
}
#ifdef _WIN32
void TitleBar::SetResizeTitleColor(const QColor& color)
{
	if (framelessHelper) {
		framelessHelper->SetTitleColor(color);
	}
}
void TitleBar::SetResizeFrameColor(const QColor& color)
{
	if (framelessHelper) {
		framelessHelper->SetFrameColor(color);
	}
}
#endif


void TitleBar::ShowNormal(void)
{
	windowWidget->showNormal();
}

void TitleBar::ShowMaximize(void)
{
	windowWidget->showMaximized();
}

void TitleBar::ShowFullScreen(bool fullscreen)
{
	if (fullscreen) {
		if (IsMaximize()) {
			isFullscreenMax = true;
		}
		setVisible(false);
		windowWidget->showFullScreen();
	}
	else {
		if (isFullscreenMax) {
			ShowMaximize();
		}
		else {
			ShowNormal();
		}
		isFullscreenMax = false;
		setVisible(true);
	}
	

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

//
//
//void TitleBar::mouseMoveEvent(QMouseEvent* e)
//{
//	do
//	{
//		if (!windowWidget || !isPressed) {
//			break;
//		}
//		if (!isMoving) {
//			isMoving = true;
//			if (IsMaximize()) {
//				windowWidget->showNormal();
//			}
//		}			
//		windowWidget->move(e->globalPos() - clickPos);
//	} while (false);
//	return QWidget::mousePressEvent(e);
//}
//

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
		SetMaximizeStatus(Qt::WindowMaximized == 
			(windowWidget->windowState() & Qt::WindowMaximized) ? true : false);
	} while (false);
	return QWidget::eventFilter(obj, e);
}

///////////////////////////////////////////////////
CustomTitle::CustomTitle(QWidget* parent) :
	TitleBar(parent)
{
	SetResizable(false);
	SetLogoable(false, QSize());
	SetTitleable(true);
}