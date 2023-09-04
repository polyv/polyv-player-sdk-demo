#include "PlayerWidget.h"

#include <QMenu>
#include <QTime>
#include <QMovie>
#include <QDateTime>
#include <QPointer>
#include <QSlider>
#include <QPainter>
#include <QStackedLayout>
#include <QWidgetAction>

#include "AppDef.h"
#include "SdkManager.h"
#include "StatusButton.h"
#include "Application.h"
#include "WidgetHelper.h"
#include "SliderControl.h"
#include "VolumeControl.h"
#include "TipsWidget.h"
#include "ParamDialog.h"
#include "MsgBoxDialog.h"
#include "GlobalConfig.h"

///////////////////////////////////////////////////////
PlayerControl::PlayerControl(Player* _player, QWidget* parent/* = nullptr*/)
	: QStackedWidget(parent),
	player(_player)
{
	Qt::WindowFlags f = Qt::FramelessWindowHint;
#ifdef _WIN32
	f |= Qt::SplashScreen;// Qt::SubWindow, Qt::SplashScreen
#else
	f |= Qt::Tool;
	f |= Qt::NoDropShadowWindowHint;
	//f |= Qt::WindowStaysOnTopHint;
	//f |= Qt::BypassWindowManagerHint;
	//f |= Qt::WindowDoesNotAcceptFocus;
#endif
	setWindowFlags(f);
	setFocusPolicy(Qt::NoFocus);
	setAttribute(Qt::WA_NoSystemBackground);
	setAttribute(Qt::WA_TranslucentBackground);
	setAttribute(Qt::WA_ShowWithoutActivating);
	setAttribute(Qt::WA_MacAlwaysShowToolWindow);

	QStackedLayout* mainLayout = (QStackedLayout*)this->layout();
	mainLayout->setSpacing(0);
	mainLayout->setContentsMargins(0, 0, 0, 0);
	mainLayout->setStackingMode(QStackedLayout::StackAll);

	shotButton = new StatusButton(this, "shotButton", QTStr("ShotScreen"), QSize(40, 40));
	shotButton->setVisible(false);
	connect(shotButton, &QPushButton::clicked, this, [this] {
		auto path = GlobalConfig::GetSaveScreenshotPath();
		if (path.isEmpty()) {
			return;
		}
		QString filename = QString("%1/%2.jpg").arg(path).
			arg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh-mm-ss"));
		player->Screenshot(filename);
		QString tip = QString(tr("ShotSucess")).arg(filename);
		TipsWidget::ShowToast((QWidget*)this->parent(), tip);
	});

	shotPanelHideTimer = new QTimer(this);
	shotPanelHideTimer->setInterval(500);
	connect(shotPanelHideTimer, &QTimer::timeout, this, [&] {
		if (!rect().contains(mapFromGlobal(QCursor::pos()))) {
			shotPanelHideTimer->stop();
			shotButton->setVisible(false);
		}
	});

	mediaSlide = new SliderControl(this);
	mediaSlide->setObjectName(QStringLiteral("timeSlide"));
	mediaSlide->setFixedHeight(14);
	mediaSlide->setCursor(QCursor(Qt::PointingHandCursor));
	mediaSlide->setOrientation(Qt::Horizontal);
	mediaSlide->setTracking(false);
	mediaSlide->setMinimum(0);
	mediaSlide->setMaximum(100);

	connect(mediaSlide, &QSlider::valueChanged, this, [this](int pos) {
		if (!player->IsLoaded()) {
			return;
		}
		player->SetSeek(pos);
	});

	auto startButton = new StatusButton(this,
		"playButton", QTStr("Play"), QTStr("Pause"), QSize(24, 24));
	connect(startButton, &QPushButton::clicked, this, [this, startButton] {
		if (player->IsLoaded()) {
			player->Pause(!player->IsPause());
		}
		else if (isLocalPlay && curVideo) {
			player->PlayLocal(0, GlobalConfig::IsAutoDownRate());
		}
		else if (curVideo){
			QMetaObject::invokeMethod((QWidget*)filterObject, "OnChangeRatePlayVideo", Qt::QueuedConnection,
				Q_ARG(int, curRate), Q_ARG(int, 0), Q_ARG(const SharedVideoPtr&, curVideo));
		}
		else {
			QMetaObject::invokeMethod((QWidget*)filterObject, "OnShowTips", Qt::QueuedConnection,
				Q_ARG(int, TipsWidget::TIP_INFO), Q_ARG(const QString&, QTStr("TokenTip")));
			startButton->setChecked(false);
		}
	});
	auto stopButton = new StatusButton(this,
		"stopButton", QTStr("Stop"), QSize(24, 24));
	connect(stopButton, &QPushButton::clicked, this, [this] {
		player->Stop();
	});
	auto timeLabel = new QLabel(this);
	timeLabel->setObjectName("timeLabel");
	timeLabel->setText("00:00:00/00:00:00");
	timeLabel->setProperty("duration", "00:00:00");

	speedButton = new StatusButton(this, "speedButton",
		GetSpeedName(curSpeed));
	connect(speedButton, &QPushButton::clicked, this, [this] {
		popup = new QMenu(this);
		popup->setObjectName("playerMenu");
		double value = curSpeed;
		auto AddAction = [this, value](QPointer<QMenu>& menu, double type) {
			bool select = (type == value);
			QWidgetAction* action = new QWidgetAction(this);
			QPushButton* item = new QPushButton(this);
			item->setCursor(QCursor(Qt::PointingHandCursor));
			item->setObjectName(select ? "selectItem" : "normalItem");
			item->setText(GetSpeedName(type));
			action->setDefaultWidget(item);
			menu->addAction(action);
			connect(item, &QPushButton::clicked, this, [&, type, menu](bool pause) {
				menu->hide();
				menu->deleteLater();
				SetSpeed(type);
			});
		};
		AddAction(popup, kSpeed05);
		AddAction(popup, kSpeed10);
		AddAction(popup, kSpeed12);
		AddAction(popup, kSpeed15);
		AddAction(popup, kSpeed20);
		QPoint ptY = mediaSlide->mapToGlobal(QPoint(0, 0));
		QPoint ptShow = speedButton->mapToGlobal(QPoint(speedButton->width() >> 1, 0));
		popup->show();
		ptShow.setX(ptShow.x() - (popup->width() >> 1));
		ptShow.setY(ptY.y() - popup->height());
		popup->move(ptShow);
	});
	videoButton = new StatusButton(this, "videoButton",
		GetVideoName(curRate));
	connect(videoButton, &QPushButton::clicked, this, [this] {
		popup = new QMenu(this);
		popup->setObjectName("playerMenu");
		int value = curRate;
		
		auto AddAction = [this, value](QPointer<QMenu>& menu, int type, bool enable = true) {
			bool select = (value == type);
			QWidgetAction* action = new QWidgetAction(this);
			QPushButton* item = new QPushButton(this);
			item->setCursor(QCursor(Qt::PointingHandCursor));
			item->setObjectName(select ? "selectItem" : "normalItem");
			item->setText(GetVideoName(type));
			action->setDefaultWidget(item);
			action->setEnabled(enable);
			menu->addAction(action);
			connect(item, &QPushButton::clicked, this, [&, menu, type](bool pause) {
				menu->hide();
				menu->deleteLater();
				SetRate(type, true);
			});
		};
		if (isLocalPlay) {
			for (int i = VIDEO_RATE_LD; i <= VIDEO_RATE_HD; ++i) {
				AddAction(popup, i, curVideo ? (SdkManager::GetManager()->CheckFileComplete(
					QString::fromStdString(curVideo->vid), QString::fromStdString(curVideo->filePath), i)): false);
			}
		}
		else{		
			int rateCount = player->GetCurrentRateCount();
			if (1 == rateCount && curRate == VIDEO_RATE_SOURCE) {
				for (int i = VIDEO_RATE_AUTO; i <= VIDEO_RATE_SOURCE; ++i) {
					AddAction(popup, i, curRate == i);
				}
			}
			else {
				AddAction(popup, VIDEO_RATE_AUTO, true);
				for (int i = VIDEO_RATE_LD; i <= VIDEO_RATE_SOURCE; ++i) {
					AddAction(popup, i, rateCount >= i);
				}
			}
		}	
		QPoint ptY = mediaSlide->mapToGlobal(QPoint(0, 0));
		QPoint ptShow = videoButton->mapToGlobal(QPoint(videoButton->width() >> 1, 0));
		popup->show();
		ptShow.setX(ptShow.x() - (popup->width() >> 1));
		ptShow.setY(ptY.y() - popup->height());
		popup->move(ptShow);
	});

	volumePanel = new VolumeControl(this);
	volumePanel->hide();
	volumePanel->SetValue(50);
	connect(volumePanel, &VolumeControl::SignalVolume, this, [this](int volume) {
		volumeButton->setChecked(0 == volume ? true : false);
		player->SetVolume(volume);
		player->SetMute(0 == volume ? true : false);
	});
	volumeButton = new StatusButton(this, "volumeButton", QTStr("Mute"), QTStr("UnMute"), QSize(24, 24));
	connect(volumeButton, &QPushButton::clicked, this, [this] {
		bool mute = true;
		if (player->IsLoaded()) {
			mute = player->IsMute();
		}
		else {
			mute = volumeButton->isChecked();
		}
		player->SetMute(!mute);
		mute = player->IsMute();
		if (mute) {
			volumePanel->SetValue(0);
		}
		else {
			int old = volumePanel->GetOldValue();
			volumePanel->SetValue(0 == old ? 50 : old);
			player->SetVolume(volumePanel->GetValue());
		}
		volumeButton->setChecked(mute);
	});
	volumePanelHideTimer = new QTimer(this);
	volumePanelHideTimer->setInterval(500);
	connect(volumePanelHideTimer, &QTimer::timeout, this, [this] {
		if (!volumePanel->geometry().contains(QCursor::pos())) {
			volumePanelHideTimer->stop();
			volumePanel->hide();
		}
	});

	auto fullButton = new StatusButton(this, "fullButton", QTStr("FullScreen"), QTStr("ExitFullScreen"), QSize(24, 24));
	connect(fullButton, &QPushButton::clicked, this, [this](bool full) {	
		QMetaObject::invokeMethod((QWidget*)filterObject, full ? "OnFullScreen" : "OnExitFullScreen");
		//isFullScreen = full;
		//activeTime = time(NULL);
		//full ? fullScreenTimer->start() : fullScreenTimer->stop();
		//if (!full) {
		//	setVisible(true);
		//}
	});

	QHBoxLayout* shotLayout = new QHBoxLayout();
	shotLayout->setContentsMargins(0, 0, 20, 0);
	shotLayout->addWidget(shotButton, 0, Qt::AlignCenter);

	QHBoxLayout* topLayout = new QHBoxLayout();
	topLayout->addStretch(1);
	topLayout->setSpacing(0);
	topLayout->setContentsMargins(1, 0, 1, 0);
	topLayout->addLayout(shotLayout);

	QHBoxLayout* barLayout = new QHBoxLayout();
	barLayout->setSpacing(16);
	barLayout->setContentsMargins(20, 13, 20, 13);
	barLayout->addWidget(startButton);
	barLayout->addWidget(stopButton);
	barLayout->addWidget(timeLabel);
	barLayout->addStretch(1);
	barLayout->addWidget(speedButton);
	barLayout->addWidget(videoButton);
	barLayout->addWidget(volumeButton);
	barLayout->addWidget(fullButton);

	QVBoxLayout* bottomLayout = new QVBoxLayout();
	bottomLayout->setSpacing(0);
	bottomLayout->setContentsMargins(0, 0, 0, 0);
	bottomLayout->addWidget(mediaSlide);
	bottomLayout->addLayout(barLayout);

	barWidget = new QWidget(this);
	barWidget->setObjectName("bar");
	barWidget->setFixedHeight(50 + 14);
	barWidget->setLayout(bottomLayout);

	auto controlWidget = new QWidget(this);
	QVBoxLayout* layout = new QVBoxLayout();
	layout->setSpacing(0);
	layout->setContentsMargins(0, 0, 0, 0);
	layout->addLayout(topLayout, 1);
	layout->addWidget(barWidget);
	controlWidget->setLayout(layout);
	controlWidget->setFocusPolicy(Qt::NoFocus);
	controlWidget->setAttribute(Qt::WA_ShowWithoutActivating);
	addWidget(controlWidget);

	loadingWidget = new QWidget(this);
	loadingWidget->setObjectName("loading");
	loadingMovie = new QMovie(":/res/images/player/loading.gif");
	auto loadingLabel = new QLabel(loadingWidget);
	loadingLabel->setFixedSize(QSize(96, 96));
	loadingLabel->setMovie(loadingMovie);
	layout = new QVBoxLayout();
	layout->setSpacing(0);
	layout->setContentsMargins(0, 0, 0, 1);
	layout->addWidget(loadingLabel, 0, Qt::AlignCenter);
	loadingWidget->setLayout(layout);
	addWidget(loadingWidget);
	loadingWidget->setVisible(true);
	loadingMovie->start();
	
	connect(player, &Player::SignalState, this, [this](int state) {
		switch (state)
		{
		case MEDIA_STATE_PLAY:
			isEnd = false;
			findChild<QPushButton*>("playButton")->setChecked(true);
			break;
		case MEDIA_STATE_PAUSE:
			findChild<QPushButton*>("playButton")->setChecked(false);
			break;
		case MEDIA_STATE_BEGIN_CACHE:
			QMetaObject::invokeMethod((QWidget*)filterObject, "OnShowTips", Qt::QueuedConnection,
				Q_ARG(int, TipsWidget::TIP_INFO), Q_ARG(const QString&, QTStr("BeingCache")));
			break;
		case MEDIA_STATE_END_CACHE:
			QMetaObject::invokeMethod((QWidget*)filterObject, "OnShowTips", Qt::QueuedConnection,
				Q_ARG(int, TipsWidget::TIP_INFO), Q_ARG(const QString&, QTStr("EndCache")));
			break;
		case MEDIA_STATE_BEGIN_SEEKING:
			//QMetaObject::invokeMethod((QWidget*)filterObject, "OnShowTips", Qt::QueuedConnection,
			//	Q_ARG(int, TipsWidget::TIP_INFO), Q_ARG(const QString&, QTStr("BeingSeeking")));
			loadingWidget->setVisible(true);
			loadingMovie->start();
			break;
		case MEDIA_STATE_END_SEEKING:
			//QMetaObject::invokeMethod((QWidget*)filterObject, "OnShowTips", Qt::QueuedConnection,
			//	Q_ARG(int, TipsWidget::TIP_INFO), Q_ARG(const QString&, QTStr("EndSeeking")));
			loadingWidget->setVisible(false);
			loadingMovie->stop();
			break;
		case MEDIA_STATE_LOADING:
			isEnd = false;
			loadingWidget->setVisible(true);
			loadingMovie->start();
			break;
		case MEDIA_STATE_LOADED:
			isEnd = false;
			loadingWidget->setVisible(false);
			loadingMovie->stop();
			break;
		case MEDIA_STATE_FAIL:
			isEnd = true;
			Reset();
			QMetaObject::invokeMethod((QWidget*)filterObject, "OnShowTips", Qt::QueuedConnection,
				Q_ARG(int, TipsWidget::TIP_ERROR), Q_ARG(const QString&, QTStr("PlayerError")));
			break;
		case MEDIA_STATE_END:
			isEnd = true;
			Reset();
			mapProperty.clear();
			emit SignalPropReset();
			break;
		default:
			break;
		}
	});
	connect(player, &Player::SignalProgress, this, [this, timeLabel](int millisecond) {
		if (isEnd) {
			return;
		}
		mediaSlide->blockSignals(true);
		mediaSlide->setValue(millisecond);
		mediaSlide->blockSignals(false);
		int seconds = millisecond / 1000;
		int h = seconds / (60 * 60);
		int m = (seconds % (60 * 60)) / 60;
		int s = seconds % 60;
		QString text = QTime(h, m, s).toString("hh:mm:ss");
		if (h >= 24) {
			text = QString("%1:%2:%3").arg(h).
				arg(m, 2, 10, QLatin1Char('0')).arg(s, 2, 10, QLatin1Char('0'));
		}
		timeLabel->setText(QString("%1/%2").arg(text).arg(timeLabel->property("duration").toString()));

		mapProperty[MEDIA_PROPERTY_POSTION] = MediaProperty{ MEDIA_PROPERTY_POSTION, MEDIA_FORMAT_STRING, text };
		emit SignalPropChange(MEDIA_PROPERTY_POSTION, text);
	});
	connect(player, &Player::SignalProperty, this, [this, timeLabel](int property, int format, QString value) {
		switch (property)
		{
		case MEDIA_PROPERTY_DURATION:
		{
			auto millisecond = value.toInt();
			mediaSlide->setMaximum(millisecond);
			int seconds = millisecond / 1000;
			int h = seconds / (60 * 60);
			int m = (seconds % (60 * 60)) / 60;
			int s = seconds % 60;
			QString duration = QTime(h, m, s).toString("hh:mm:ss");
			if (h >= 24) {
				duration = QString("%1:%2:%3").arg(h).
					arg(m, 2, 10, QLatin1Char('0')).arg(s, 2, 10, QLatin1Char('0'));
			}
			timeLabel->setText(QString("00:00:00/%1").arg(duration));
			timeLabel->setProperty("duration", duration);

			value = duration;
		}
		break;
		case MEDIA_PROPERTY_CACHE_TIME:
			mediaSlide->SetPreValue(value.toInt());
			break;
		default:
			break;
		}
		mapProperty[property] = MediaProperty{ property, format, value };
		emit SignalPropChange(property, value);
	});
	connect(player, &Player::SignalRateChange, this, [this](int inputBitRate, int realBitRate) {
		SetRate(realBitRate, false);
		if (inputBitRate != realBitRate && VIDEO_RATE_AUTO != inputBitRate) {
			QString tip = QString(QTStr("RateChange")).arg(GetVideoName(inputBitRate)).arg(GetVideoName(realBitRate));
			QMetaObject::invokeMethod((QWidget*)filterObject, "OnShowTips", Qt::QueuedConnection,
				Q_ARG(int, TipsWidget::TIP_INFO), Q_ARG(const QString&, tip));
		}	
	});
	fullScreenTimer = new QTimer(this);
	fullScreenTimer->setInterval(1000);
	connect(fullScreenTimer, &QTimer::timeout, this, [this] {
		if (!((QWidget*)filterObject)->isFullScreen()) {
			if (time(NULL) - activeTime > 2) {
				SetBarVisible(false);
			}
		}
		else {
			auto pos = QCursor::pos();
			if (((QWidget*)filterObject)->geometry().contains(pos)) {
				static QPoint spos = pos;
				static QPoint szore(0, 0);
				if (spos - pos != szore) {
					spos = pos;
					SetBarVisible(true);
				}
				else {
					SetBarVisible(false);
				}
			}
			else {
				SetBarVisible(false);
			}			
		}		
	});

	volumePanel->installEventFilter(this);
	volumeButton->installEventFilter(this);
	this->installEventFilter(this);
	if (parent) {
		parent->installEventFilter(this);
	}
	SetFilter(GetRootParent(this));
}
PlayerControl::~PlayerControl(void)
{
	if (paramDialog) {
		paramDialog->close();
		paramDialog = nullptr;
	}
}

void PlayerControl::SetFilter(const QObject* object)
{
	if (filterObject) {
		filterObject->removeEventFilter(this);
	}
	filterObject = const_cast<QObject*>(object);
	filterObject->installEventFilter(this);
}
void PlayerControl::UpdatePos()
{
	QWidget* widget = (QWidget*)this->parent();
	if (widget && widget->isVisible()) {
		this->resize(widget->size());
		this->move(widget->mapToGlobal(QPoint(0, 0)));
	}
}
void PlayerControl::OpenParamWindow()
{
	if (!paramDialog) {
		paramDialog = new ParamDialog(this);		
#ifdef _WIN32
		HWND hwnd = (HWND)paramDialog->winId();
		auto oldStyle = ::GetWindowLongW(hwnd, GWL_EXSTYLE);
		const DWORD newWindowStyle = oldStyle | WS_EX_APPWINDOW;
		::SetWindowLongW(hwnd, GWL_EXSTYLE, static_cast<LONG_PTR>(newWindowStyle));
#else
		Qt::WindowFlags f = paramDialog->windowFlags();
		f |= Qt::WindowStaysOnTopHint;
		paramDialog->setWindowFlags(f);
#endif // !_WIN32		
		paramDialog->setAttribute(Qt::WA_DeleteOnClose, true);
		connect(paramDialog, &ParamDialog::finished, this, [&](int) {
			paramDialog = nullptr;
		});
		connect(this, SIGNAL(SignalPropChange(int, const QString&)),
			paramDialog, SLOT(OnPropChange(int, const QString&)));
		connect(this, SIGNAL(SignalPropReset()),
			paramDialog, SLOT(OnPropReset()));
		for (auto& it : mapProperty) {
			paramDialog->SetPropValue(it.property, it.value);
		}
		paramDialog->show();
		if (filterObject && parent() && !((QWidget*)parent())->isVisible()) {
			auto screen = ((QWidget*)filterObject)->geometry();
			QRect rc = paramDialog->geometry();
			paramDialog->move(screen.left() + (screen.width() - rc.width()) / 2,
				screen.top() + (screen.height() - rc.height()) / 2);
		}
	}
	else {
		paramDialog->showNormal();
	}
	paramDialog->activateWindow();
	paramDialog->raise();
	if (curVideo) {
		paramDialog->SetVideoName(QString::fromStdString(curVideo->title));
	}
}

void PlayerControl::SetInfo(bool local, int rate, const SharedVideoPtr& video)
{
	//Reset();
	isLocalPlay = local;
	curRate = rate;
	curVideo = video;
	SetRate(rate, false);
	player->SetVolume(volumePanel->GetValue());
	player->SetMute(volumeButton->isChecked());
	player->SetSpeed(curSpeed);
}
void PlayerControl::Reset()
{
	volumeButton->setChecked(false);
	mediaSlide->blockSignals(true);
	mediaSlide->setValue(0);
	mediaSlide->SetPreValue(0);
	mediaSlide->blockSignals(false);
	findChild<QPushButton*>("playButton")->setChecked(false);
	findChild<QLabel*>("timeLabel")->setText("00:00:00/00:00:00");
	findChild<QLabel*>("timeLabel")->setProperty("duration", "00:00:00");
}

bool PlayerControl::eventFilter(QObject* obj, QEvent* e)
{
	do
	{
		if (this == obj || parent() == obj) {
			if (QEvent::Enter == e->type()) {
				shotButton->setVisible(true);
			}
			else if (QEvent::Leave == e->type()) {
			shotPanelHideTimer->start();
			}
			else if (QEvent::MouseButtonDblClick == e->type()) {
			QMouseEvent* me = (QMouseEvent*)e;
			if (Qt::LeftButton == me->button()) {
				findChild<QPushButton*>("fullButton")->click();
			}
			}
			else if (QEvent::MouseButtonRelease == e->type()) {
			QMouseEvent* me = (QMouseEvent*)e;
			if (Qt::RightButton == me->button()) {
				QPointer<QMenu> popup = new QMenu(this);
				auto AddAction = [this](QPointer<QMenu>& popup, const QString& name, const std::function<void()>& fun) {
					QAction* item = new QAction(name, this);
					popup->addAction(item);
					connect(item, &QAction::triggered, this, fun);
				};
				AddAction(popup, findChild<QPushButton*>("playButton")->isChecked() ? QTStr("Pause") : QTStr("Play"), [this] {
					findChild<QPushButton*>("playButton")->click(); });
				AddAction(popup, QTStr("Stop"), [&] {
					findChild<QPushButton*>("stopButton")->click(); });
				popup->addSeparator();
				AddAction(popup, QTStr("OpenListParam"), [this] {
					OpenParamWindow();
					//QMetaObject::invokeMethod((QWidget*)filterObject, "OnOpenParamWindow", Q_ARG(QString, QString::fromStdString(curVideo->vid)));
				});
				AddAction(popup, findChild<QPushButton*>("fullButton")->isChecked() ? QTStr("ExitFullScreen") : QTStr("FullScreen"), [&] {
					findChild<QPushButton*>("fullButton")->click(); });
				popup->exec(QCursor::pos());
			}
			}
		}
		else if (volumePanel == obj) {
		if (QEvent::Enter == e->type()) {
			volumePanelHideTimer->stop();
		}
		else if (QEvent::Leave == e->type()) {
			volumePanelHideTimer->start();
		}
		}
		else if (volumeButton == obj) {
		if (QEvent::Enter == e->type()) {
			if (!volumeButton->isEnabled()) {
				break;
			}
			volumePanelHideTimer->stop();
			QPoint ptY = barWidget->mapToGlobal(QPoint(0, 0));
			QPoint ptShow = volumeButton->mapToGlobal(QPoint(volumeButton->width() >> 1, 0));
			volumePanel->show();
			ptShow.setX(ptShow.x() - (volumePanel->width() >> 1));
			ptShow.setY(ptY.y() - volumePanel->height());
			volumePanel->move(ptShow);
		}
		else if (QEvent::Leave == e->type()) {
			volumePanelHideTimer->start();
		}
		}
	} while (false);
	do
	{
		if (!(this == obj || filterObject == obj || this->parent() == obj)) {
			break;
		}
		switch (e->type())
		{
		case QEvent::Hide:
			SetVisible(false);
			break;
		case QEvent::Show:
			SetVisible(true);
			break;
		case QEvent::Move:
		case QEvent::Resize:
			if (this != obj) {
				UpdatePos();
			}
			break;
		case QEvent::WindowActivate:
			UpdatePos();
			break;
		case QEvent::WindowStateChange:
			if (this->nativeParentWidget() &&
				Qt::WindowMinimized == (this->nativeParentWidget()->windowState() & Qt::WindowMinimized)) {
				SetVisible(false);
			}
			break;
		case QEvent::Enter:
			//case QEvent::HoverMove:
				//if (filterObject && ((QWidget*)filterObject)->isFullScreen()) {
				//	activeTime = time(NULL);
				//	setVisible(true);
				//	UpdatePos();
				//}
			if (!((QWidget*)filterObject)->isFullScreen()){
				fullScreenTimer->stop();
			}
			else {
				fullScreenTimer->start(1000);
			}
			SetBarVisible(true);
			break;
		case QEvent::Leave:
		//case QEvent::HoverLeave:
			activeTime = time(NULL);
			fullScreenTimer->start(1000);
			break;
		case QEvent::KeyRelease: {
			QKeyEvent* key = (QKeyEvent*)e;
			if (Qt::Key_Escape == key->key() && filterObject && ((QWidget*)filterObject)->isFullScreen()) {
				findChild<QPushButton*>("fullButton")->click();
			}
		}
			break;
		}
	} while (false);
	return QStackedWidget::eventFilter(obj, e);
}

void PlayerControl::SetRate(int rate, bool notify)
{
	findChild<QPushButton*>("videoButton")->setText(GetVideoName(rate));
	if (rate == curRate) { 
		return;
	}
	curRate = rate;
	if (!notify) {
		return;
	}
	int pos = mediaSlide->value();
	if (isLocalPlay) {
		if (curVideo && SdkManager::GetManager()->CheckFileComplete(
			QString::fromStdString(curVideo->vid), QString::fromStdString(curVideo->filePath), rate)) {
			int ret = player->SetInfo(QString::fromStdString(curVideo->vid),
				QString::fromStdString(curVideo->filePath), rate);
			if (E_NO_ERR != ret) {
				slog_error("[sdk]:set player info error:%d", ret);
				QMetaObject::invokeMethod((QWidget*)filterObject, "OnShowTips", Qt::QueuedConnection,
					Q_ARG(int, TipsWidget::TIP_ERROR), Q_ARG(const QString&, QTStr("PlayerError")));
				Reset();
				return;
			}
			ret = player->PlayLocal(pos, GlobalConfig::IsAutoDownRate());
			if (E_NO_ERR != ret) {
				slog_error("[sdk]:play local error:%d", ret);
				QMetaObject::invokeMethod((QWidget*)filterObject, "OnShowTips", Qt::QueuedConnection,
					Q_ARG(int, TipsWidget::TIP_ERROR), Q_ARG(const QString&, QTStr("PlayerError")));
				Reset();
				return;
			}
			curRate = rate;			
		}
		else {
			QMetaObject::invokeMethod((QWidget*)filterObject, "OnShowTips", Qt::QueuedConnection,
				Q_ARG(int, TipsWidget::TIP_ERROR), Q_ARG(const QString&, QTStr("NoDownloadVideo")));
		}
	}
	else {
		QMetaObject::invokeMethod((QWidget*)filterObject, "OnChangeRatePlayVideo", Qt::QueuedConnection,
			Q_ARG(int, rate), Q_ARG(int, pos), Q_ARG(const SharedVideoPtr&, curVideo));
	}
}

void PlayerControl::SetSpeed(double speed)
{
	findChild<QPushButton*>("speedButton")->setText(GetSpeedName(speed));
	curSpeed = speed;
	player->SetSpeed(speed);
}
void PlayerControl::SetVisible(bool visible)
{
	do
	{
		if (!visible) {
			setVisible(visible);
			break;
		}
		if (parent() && !((QWidget*)parent())->isVisible()) {
			break;
		}
		if (this->nativeParentWidget() &&
			Qt::WindowMinimized == (this->nativeParentWidget()->windowState() & Qt::WindowMinimized)) {
			break;
		}
		setVisible(visible);
		UpdatePos();
	} while (false);
}

void PlayerControl::SetBarVisible(bool visible)
{
	do
	{
		if (!visible) {
			barWidget->setVisible(visible);
			break;
		}
		if (parent() && !((QWidget*)parent())->isVisible()) {
			break;
		}
		barWidget->setVisible(visible);
	} while (false);
}


///////////////////////////////////////////////////////
PlayerWidget::PlayerWidget(QWidget* parent)
	: QWidget(parent)
{	
	setAttribute(Qt::WA_NativeWindow);
	setAttribute(Qt::WA_DontCreateNativeAncestors);
	//setAttribute(Qt::WA_PaintOnScreen);
	//setAttribute(Qt::WA_StaticContents);
	//setAttribute(Qt::WA_NoSystemBackground);
	//setAttribute(Qt::WA_OpaquePaintEvent);
	player = new Player((void*)this->winId(), this);
	controller = new PlayerControl(player, this);
	controller->setObjectName("control");
	
	QVBoxLayout* layout = new QVBoxLayout();
	layout->setSpacing(0);
	layout->setContentsMargins(0, 0, 0, 0);
	layout->addWidget(controller);
	setLayout(layout);
}

PlayerWidget::~PlayerWidget(void)
{
	delete player;
	player = nullptr;
	qDebug() << __FUNCTION__;
}

void PlayerWidget::SetFilter(const QObject* object)
{
	controller->SetFilter(object);
}
QObject* PlayerWidget::GetFilter()
{
	return controller->GetFilter();
}
void PlayerWidget::UpdatePos()
{
	controller->UpdatePos();
}

bool PlayerWidget::Play(bool local, const QString& token, const SharedVideoPtr& video, int rate, int seekMillisecond)
{
	return StartPlay(local, rate, token, video, seekMillisecond);
}

bool PlayerWidget::OnlineRePlay(int rate, int seekMillisecond, const QString& token, const SharedVideoPtr& video)
{
	return StartPlay(false, rate, token, video, seekMillisecond);
}

bool PlayerWidget::LoadLocal(const SharedVideoPtr& video)
{
	SetInfo(true, VIDEO_RATE_AUTO, video);
	player->LoadLocal(0, GlobalConfig::IsAutoDownRate());
	return true;
}

void PlayerWidget::RefreshPlayer()
{
	player->UpdateCacheConfig();
	player->UpdateOSDConfig();
	player->UpdateLogoConfig();
}

void PlayerWidget::OpenParamWindow()
{
	controller->OpenParamWindow();
}

bool PlayerWidget::IsLoading()
{
	return player->IsLoading();
}

void PlayerWidget::Stop(void)
{
	player->Stop();
}
void PlayerWidget::Destroy(void)
{
	player->disconnect();
	player->Reset();
	Stop();
}

void PlayerWidget::paintEvent(QPaintEvent* e)
{
	(void)e;
	QPainter p(this);
	p.setPen(Qt::NoPen);
	p.setBrush(QColor(12, 12, 12));
	p.drawRect(rect());
}
void PlayerWidget::closeEvent(QCloseEvent* e)
{
	Destroy();
	QWidget::closeEvent(e);
}

bool PlayerWidget::SetInfo(bool local, int rate, const SharedVideoPtr& video)
{
	int ret = player->SetInfo(QString::fromStdString(video->vid), QString::fromStdString(video->filePath), rate);
	if (E_NO_ERR != ret) {
		slog_error("[sdk]:set player info error:%d", ret);
		return false;
	}
	controller->SetInfo(local, rate, video);
	return true;
}

bool PlayerWidget::StartPlay(bool local, int rate, const QString& token, const SharedVideoPtr& video, int seekMillisecond)
{
	if (!SetInfo(local, rate, video)) {
		return false;
	}
	int ret = 0;
	if (local) {
		ret = player->PlayLocal(seekMillisecond, GlobalConfig::IsAutoDownRate());
	}
	else {
		ret = player->Play(token, seekMillisecond, GlobalConfig::IsAutoDownRate(), GlobalConfig::IsPlayWithToken(), false);
	}
	if (E_NO_ERR != ret) {
		slog_error("[sdk]:play %s error:%d", (local ? "local" : "online"), ret);
		return false;
	}
	return true;
}
