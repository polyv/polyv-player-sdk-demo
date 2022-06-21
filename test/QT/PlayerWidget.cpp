#include "PlayerWidget.h"

#include <QMenu>
#include <QTime>
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
#include "SettingDialog.h"



PlayerWidget::PlayerWidget(QWidget* parent)
	: QWidget(parent)
{	
	playerWidget = new QWidget(this);
	playerWidget->setObjectName("player");
	playerWidget->setAttribute(Qt::WA_NativeWindow);
	//playerWidget->setAttribute(Qt::WA_PaintOnScreen);
	//playerWidget->setAttribute(Qt::WA_DontCreateNativeAncestors);

	controlWidget = new QWidget(this);
	controlWidget->setObjectName("control");
	controlWidget->setFocusPolicy(Qt::NoFocus);
	controlWidget->setAttribute(Qt::WA_NoSystemBackground);
	controlWidget->setAttribute(Qt::WA_TranslucentBackground);
	controlWidget->setAttribute(Qt::WA_ShowWithoutActivating);
	controlWidget->setWindowFlags(Qt::FramelessWindowHint | Qt::Tool);
	controlWidget->setVisible(isShowControl);
#ifdef _WIN32
	// remove top-level window
	SetWindowLongPtr((HWND)(controlWidget->winId()), GWL_EXSTYLE,
		GetWindowLong((HWND)(controlWidget->winId()), GWL_EXSTYLE) & ~WS_EX_APPWINDOW);
#endif
	
	QVBoxLayout* layout = new QVBoxLayout();
	layout->setSpacing(0);
	layout->setContentsMargins(0, 0, 0, 0);
	layout->addWidget(playerWidget);
	//layout->addWidget(controlWidget);
	setLayout(layout);

	QPushButton* shotImage = new StatusButton(controlWidget, "shotButton", QTStr("ShotScreen"), QSize(40, 40));
	connect(shotImage, &QPushButton::clicked, [this] {
		auto path = App()->GlobalConfig().Get("Download", "ScreenshotPath").toString();
		if (path.isEmpty()) {
			return;
		}
		QString filename = QString("%1/%2.jpg").arg(path).
			arg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh-mm-ss"));
		PLVPlayerScreenshot(mediaPlayer, QT_TO_UTF8(filename));
	});
	mediaSlide = new SliderControl(controlWidget);
	mediaSlide->setObjectName(QStringLiteral("timeSlide"));
	mediaSlide->setFixedHeight(14);
	mediaSlide->setCursor(QCursor(Qt::PointingHandCursor));
	mediaSlide->setOrientation(Qt::Horizontal);
	mediaSlide->setTracking(false);
	mediaSlide->setMinimum(0);
	mediaSlide->setMaximum(100);

	connect(mediaSlide, &QSlider::valueChanged, [this](int pos) {
		if (!PLVPlayerIsLoaded(mediaPlayer)) {
			return;
		}
		PLVPlayerSetSeek(mediaPlayer, pos, true);
	});

	startButton = new StatusButton(controlWidget,
		"playButton", QTStr("Play"), QTStr("Pause"), QSize(24, 24));
	connect(startButton, &QPushButton::clicked, [this] {
		if (PLVPlayerIsLoaded(mediaPlayer)) {
			bool pause = !PLVPlayerIsPause(mediaPlayer);
			PLVPlayerPause(mediaPlayer, pause);
		}
		else if(videoInfo){
			if (isLocal) {
				PLVPlayerPlayLocal(mediaPlayer, 0);
			}
			else {
				//PLVPlayerPlay("");
				QMetaObject::invokeMethod((QWidget*)App()->GetMainWindow(), "OnShowTips", Qt::QueuedConnection,
					Q_ARG(int, TipsWidget::TIP_INFO), Q_ARG(const QString&, QTStr("TokenTip")));
				startButton->setChecked(false);
			}
		}
		else {
			startButton->setChecked(false);
		}
	});
	stopButton = new StatusButton(controlWidget,
		"stopButton", QTStr("Stop"), QSize(24, 24));
	connect(stopButton, &QPushButton::clicked, [this] {
		PLVPlayerStop(mediaPlayer);
	});
	timeLabel = new QLabel(this);
	timeLabel->setObjectName("timeLabel");
	timeLabel->setText("00:00:00");

	//curRate = App()->GlobalConfig().GetInt("Player", "Video");
	//curSpeed = App()->GlobalConfig().GetDouble("Player", "Speed");

	QPushButton* speedButton = new StatusButton(controlWidget, "speedButton",
		GetSpeedName(curSpeed));
	connect(speedButton, &QPushButton::clicked, [this, speedButton] {
		QPointer<QMenu> popup = new QMenu(this);
		popup->setObjectName("player");
		double value = curSpeed;// App()->GlobalConfig().GetDouble("Player", "Speed");
		auto AddAction = [this, speedButton, value](QPointer<QMenu>& menu, const QString& name, double type) {
			bool select = (type == value);
			//if (select) {
			//	curSpeed = type;
			//}
			QWidgetAction *action = new QWidgetAction(this);
			QPushButton* item = new QPushButton(this);
			item->setCursor(QCursor(Qt::PointingHandCursor));
			item->setObjectName(select ? "selectItem" : "normalItem");
			item->setText(name);
			//itemName->setAlignment(Qt::AlignCenter);
			action->setDefaultWidget(item);
			//QAction *item = new QAction(name, this);
			menu->addAction(action);
			//connect(itemName, SIGNAL(clicked()), itemAction, SLOT(trigger()));
			//connect(item, &QPushButton::clicked,
			//	std::bind(&QMenu::triggered, menu, action));
			connect(item, &QPushButton::clicked, [&, speedButton, name, type, menu](bool pause) {
				menu->hide();
				menu->deleteLater();
				speedButton->setText(name);
				curSpeed = type;
				//App()->GlobalConfig().SetDouble("Player", "Speed", type);

				if (PLVPlayerIsLoaded(mediaPlayer)) {
					PLVPlayerSetSpeed(mediaPlayer, curSpeed);
				}
			});
		};
		AddAction(popup, QTStr("0.5x"), kSpeed05);
		AddAction(popup, QTStr("1.0x"), kSpeed10);
		AddAction(popup, QTStr("1.2x"), kSpeed12);
		AddAction(popup, QTStr("1.5x"), kSpeed15);
		AddAction(popup, QTStr("2.0x"), kSpeed20);
		/*auto pos = multipleButton->mapToGlobal(multipleButton->pos());
		popup->exec(QPoint(pos.x() - 12, pos.y() - 12));*/
		QPoint ptY = mediaSlide->mapToGlobal(QPoint(0, 0));
		QPoint ptShow = speedButton->mapToGlobal(QPoint(speedButton->width() >> 1, 0));
		popup->show();
		ptShow.setX(ptShow.x() - (popup->width() >> 1));
		ptShow.setY(ptY.y() - popup->height());
		popup->move(ptShow);
	});
	videoButton = new StatusButton(controlWidget, "videoButton",
		GetVideoName(curRate));
	connect(videoButton, &QPushButton::clicked, [this] {
		QPointer<QMenu> popup = new QMenu(this);
		popup->setObjectName("player");
		int value = curRate;// App()->GlobalConfig().GetInt("Player", "Video");
		auto AddAction = [this, value](QPointer<QMenu>& menu, const QString& name, int type) {
			bool select = (value == type);
			//if (select) {
			//	curRate = type;
			//}
			QWidgetAction *action = new QWidgetAction(this);
			QPushButton* item = new QPushButton(this);
			item->setCursor(QCursor(Qt::PointingHandCursor));
			item->setObjectName(select ? "selectItem" : "normalItem");
			item->setText(name);
			//itemName->setAlignment(Qt::AlignCenter);
			action->setDefaultWidget(item);

			if (type <= rateCount) {
				action->setEnabled(true);
			}
			else {
				action->setEnabled(false);
			}
			//QAction *item = new QAction(name, this);
			menu->addAction(action);
			//connect(itemName, SIGNAL(clicked()), itemAction, SLOT(trigger()));
			//connect(item, &QPushButton::clicked,
			//	std::bind(&QMenu::triggered, menu, action));
			connect(item, &QPushButton::clicked, [&, name, menu, type](bool pause) {
				menu->hide();
				menu->deleteLater();
				int pos = mediaSlide->value();
				if (isLocal) {
					if (videoInfo && PLVCheckFileComplete(videoInfo->vid.c_str(), videoInfo->filePath.c_str(), type)) {
						PLVPlayerStop(mediaPlayer);
						int ret = PLVPlayerSetVideo(mediaPlayer, videoInfo->vid.c_str(), videoInfo->filePath.c_str(), type);
						if (E_NO_ERR != ret) {
							slog_error("[sdk]:set player info error:%d", ret);
							QMetaObject::invokeMethod((QWidget*)App()->GetMainWindow(), "OnShowTips", Qt::QueuedConnection,
								Q_ARG(int, TipsWidget::TIP_ERROR), Q_ARG(const QString&, QTStr("PlayerError")));
							return;
						}
						UpdateOSD();
						ret = PLVPlayerPlayLocal(mediaPlayer, pos);
						if (E_NO_ERR != ret) {
							slog_error("[sdk]:play local error:%d", ret);
							QMetaObject::invokeMethod((QWidget*)App()->GetMainWindow(), "OnShowTips", Qt::QueuedConnection,
								Q_ARG(int, TipsWidget::TIP_ERROR), Q_ARG(const QString&, QTStr("PlayerError")));
							return;
						}
						videoButton->setText(name);
						curRate = type;
					}
					else {
						QMetaObject::invokeMethod((QWidget*)App()->GetMainWindow(), "OnShowTips", Qt::QueuedConnection,
							Q_ARG(int, TipsWidget::TIP_ERROR), Q_ARG(const QString&, QTStr("NoDownloadVideo")));
					}
				}
				else {
					QMetaObject::invokeMethod((QWidget*)App()->GetMainWindow(), "OnChangeRatePlayVideo", Qt::QueuedConnection,
						Q_ARG(int, type), Q_ARG(int, pos), Q_ARG(const SharedVideoPtr&, videoInfo));
				}
			});
		};
		AddAction(popup, QTStr("Auto"), VIDEO_RATE_AUTO);
		AddAction(popup, QTStr("SD"), VIDEO_RATE_SD);
		AddAction(popup, QTStr("HD"), VIDEO_RATE_HD);
		AddAction(popup, QTStr("BD"), VIDEO_RATE_BD);

		QPoint ptY = mediaSlide->mapToGlobal(QPoint(0, 0));
		QPoint ptShow = videoButton->mapToGlobal(QPoint(videoButton->width() >> 1, 0));
		popup->show();
		ptShow.setX(ptShow.x() - (popup->width() >> 1));
		ptShow.setY(ptY.y() - popup->height());
		popup->move(ptShow);
	});
	volumeButton = new StatusButton(this, "volumeButton", QTStr("Mute"), QTStr("UnMute"), QSize(24, 24));
	connect(volumeButton, &QPushButton::clicked, [this] {
		bool mute = true;
		if (PLVPlayerIsLoaded(mediaPlayer)) {
			mute = PLVPlayerIsMute(mediaPlayer);
		}
		else {
			mute = volumeButton->isChecked();
		}
		PLVPlayerSetMute(mediaPlayer, !mute);
		mute = PLVPlayerIsMute(mediaPlayer);
		if (mute) {
			volumePanel->SetValue(0);
		}
		else {
			int old = volumePanel->GetOldValue();
			volumePanel->SetValue(0 == old ? 50 : old);
			PLVPlayerSetVolume(mediaPlayer, volumePanel->GetValue());
		}
		volumeButton->setChecked(mute);
	});
	fullButton = new StatusButton(controlWidget, "fullButton", QTStr("FullScreen"), QTStr("ExitFullScreen"), QSize(24, 24));
	connect(fullButton, &QPushButton::clicked, [&](bool full) {
		QMetaObject::invokeMethod((QWidget*)App()->GetMainWindow(), full ? "OnFullScreen" : "OnExitFullScreen");
	});

	rightWidget = new QWidget(controlWidget);
	rightWidget->setObjectName("rightWidget");
	rightWidget->setFixedWidth(60);

	QHBoxLayout* rightLayout = new QHBoxLayout();
	rightLayout->setContentsMargins(0, 0, 20, 0);
	rightLayout->addWidget(shotImage, 0, Qt::AlignCenter);
	rightWidget->setLayout(rightLayout);

	QHBoxLayout* topLayout = new QHBoxLayout();
	topLayout->addStretch(1);
	topLayout->setSpacing(0);
	topLayout->setContentsMargins(0, 0, 0, 0);
	topLayout->addWidget(rightWidget);

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

	controlBarWidget = new QWidget(controlWidget);
	controlBarWidget->setObjectName("barWidget");
	controlBarWidget->setFixedHeight(50 + 14);
	controlBarWidget->setLayout(bottomLayout);

	QVBoxLayout* controlLayout = new QVBoxLayout();
	controlLayout->setSpacing(0);
	controlLayout->setContentsMargins(0, 0, 0, 0);
	controlLayout->addLayout(topLayout, 1);
	controlLayout->addWidget(controlBarWidget);

	controlWidget->setLayout(controlLayout);

	hideTimer = new QTimer(this);
	hideTimer->setInterval(200);
	connect(hideTimer, &QTimer::timeout, [&] {
		if (!rect().contains(mapFromGlobal(QCursor::pos()))) {
			hideTimer->stop();
			SetPanelVisible(false);
		}
	});
	
	volumeHideTimer = new QTimer(this);
	volumeHideTimer->setInterval(200);
	connect(volumeHideTimer, &QTimer::timeout, [&] {
		if (!volumePanel->geometry().contains(QCursor::pos())) {
			volumeHideTimer->stop();
			volumePanel->hide();
		}
	});
	volumePanel = new VolumeControl(this);
	volumePanel->hide();
	volumePanel->SetValue(50);
	connect(volumePanel, &VolumeControl::SignalVolume, [this](int volume) {
		volumeButton->setChecked(0 == volume ? true : false);
		PLVPlayerSetVolume(mediaPlayer, volume);
		PLVPlayerSetMute(mediaPlayer, 0 == volume ? true : false);
	});

	volumeButton->installEventFilter(this);
	volumePanel->installEventFilter(this);
	this->installEventFilter(this);

	SetPanelVisible(false);

	mediaPlayer = PLVPlayerCreate((void*)playerWidget->winId());

	PLVPlayerSetStateHandler(mediaPlayer, [](const char* vid, int state, void* data) {
		(void)vid;
		PlayerWidget* obj = (PlayerWidget*)data;
		QMetaObject::invokeMethod(obj, "OnPlayerStateHandler", Qt::QueuedConnection,
			Q_ARG(int, state));
	}, this);
	PLVPlayerSetPropertyHandler(mediaPlayer, [](const char* vid, int property, int format, const char* value, void* data) {
		(void)vid;
		PlayerWidget* obj = (PlayerWidget*)data;
		QMetaObject::invokeMethod(obj, "OnPlayerPropertyHandler", Qt::QueuedConnection,
			Q_ARG(int, property), Q_ARG(int, format), Q_ARG(QString, value));
	}, this);
	PLVPlayerSetRateChangeHandler(mediaPlayer, [](const char* vid, int inputBitRate, int realBitRate, void* data) {
		(void)vid;
		PlayerWidget* obj = (PlayerWidget*)data;
		QMetaObject::invokeMethod(obj, "OnPlayerRateChangeHandler", Qt::QueuedConnection,
			Q_ARG(int, inputBitRate), Q_ARG(int, realBitRate));
	}, this);
	PLVPlayerSetProgressHandler(mediaPlayer, [](const char* vid, int millisecond, void* data) {
		(void)vid;
		PlayerWidget* obj = (PlayerWidget*)data;
		QMetaObject::invokeMethod(obj, "OnPlayerProgressHandler", Qt::QueuedConnection,
			Q_ARG(int, millisecond));
	}, this);
    PLVPlayerSetAudioDeviceHandler(mediaPlayer, [](const char* vid, int audioDeviceCount, void* data) {
        (void)vid;
        PlayerWidget* obj = (PlayerWidget*)data;
        QMetaObject::invokeMethod(obj, "OnPlayerAudioDeviceHandler", Qt::QueuedConnection,
            Q_ARG(int, audioDeviceCount));
    }, this);
}

PlayerWidget::~PlayerWidget(void)
{
	//PLVPlayerDestroy(mediaPlayer);
	//mediaPlayer = nullptr;
}

void PlayerWidget::OnPlayerStateHandler(int state)
{
	qDebug() << "the state:" << state;
	switch (state)
	{
	case MEDIA_STATE_PLAY:
		startButton->setChecked(true);

		//QTimer::singleShot(1000, [&] {
		//	PLVPlayerSetSeek(mediaPlayer, PLVPlayerGetDuration(mediaPlayer), false);
		//});

		//QTimer::singleShot(1000, [&] {
		//	PLVPlayerSeekToEnd(mediaPlayer);
		//});
		break;
	case MEDIA_STATE_PAUSE:
		startButton->setChecked(false);

		//QTimer::singleShot(5000, [&] {
		//	PLVPlayerPause(mediaPlayer, false);
		//});
		break;
	case MEDIA_STATE_BEGIN_CACHE:
		QMetaObject::invokeMethod((QWidget*)App()->GetMainWindow(), "OnShowTips", Qt::QueuedConnection,
			Q_ARG(int, TipsWidget::TIP_INFO), Q_ARG(const QString&, QTStr("BeingCache")));
		break;
	case MEDIA_STATE_END_CACHE:
		QMetaObject::invokeMethod((QWidget*)App()->GetMainWindow(), "OnShowTips", Qt::QueuedConnection,
			Q_ARG(int, TipsWidget::TIP_INFO), Q_ARG(const QString&, QTStr("EndCache")));
		break;
	case MEDIA_STATE_BEGIN_SEEKING:
		QMetaObject::invokeMethod((QWidget*)App()->GetMainWindow(), "OnShowTips", Qt::QueuedConnection,
			Q_ARG(int, TipsWidget::TIP_INFO), Q_ARG(const QString&, QTStr("BeingSeeking")));
		break;
	case MEDIA_STATE_END_SEEKING:
		QMetaObject::invokeMethod((QWidget*)App()->GetMainWindow(), "OnShowTips", Qt::QueuedConnection,
			Q_ARG(int, TipsWidget::TIP_INFO), Q_ARG(const QString&, QTStr("EndSeeking")));
		break;
	case MEDIA_STATE_LOADED:
		if (!isLocal) {
			rateCount = PLVPlayerGetRateCount(mediaPlayer);
		}
		else {
			rateCount = VIDEO_RATE_BD;
		}
		break;
	case MEDIA_STATE_FAIL:
		QMetaObject::invokeMethod((QWidget*)App()->GetMainWindow(), "OnShowTips", Qt::QueuedConnection,
			Q_ARG(int, TipsWidget::TIP_ERROR), Q_ARG(const QString&, QTStr("PlayerError")));
		break;
	case MEDIA_STATE_END:
		Reset();
		mapProp.clear();
		emit SignalPropReset();
		break;
	}
}
void PlayerWidget::OnPlayerPropertyHandler(int property, int format, QString value)
{	
	switch (property)
	{
	case MEDIA_PROPERTY_DURATION:
	{
		mediaDuration = value.toInt();
		mediaSlide->setMaximum(mediaDuration);
		int seconds = mediaDuration / 1000;
		int h = seconds / (60 * 60);
		int m = (seconds % (60 * 60)) / 60;
		duration = QTime(h, m, seconds % 60).toString("hh:mm:ss");
		timeLabel->setText(QString("00:00:00/%1").arg(duration));
		value = duration;
	}
		break;
	default:
		break;
	}
	mapProp[property] = MediaProp{ property, format, value };
	emit SignalPropChange(property, value);
}
void PlayerWidget::OnPlayerRateChangeHandler(int inputBitRate, int realBitRate)
{
	(void)inputBitRate;
	(void)realBitRate;
}
void PlayerWidget::OnPlayerProgressHandler(int millisecond)
{
	mediaSlide->blockSignals(true);
	mediaSlide->setValue(millisecond);
	mediaSlide->blockSignals(false);
	int seconds = millisecond / 1000;
	int h = seconds / (60 * 60);
	int m = (seconds % (60 * 60)) / 60;
	QString text = QTime(h, m, seconds % 60).toString("hh:mm:ss");
	timeLabel->setText(QString("%1/%2").arg(text).arg(duration));
	mapProp[MEDIA_PROPERTY_POSTION] = MediaProp{ MEDIA_PROPERTY_POSTION, MEDIA_FORMAT_STRING, text };
	emit SignalPropChange(MEDIA_PROPERTY_POSTION, text);
}

bool PlayerWidget::LoadLocal(int opt, const SharedVideoPtr& video)
{
	if (1 == opt) {
		if (!PLVPlayerIsLoaded(mediaPlayer)) {
			QMetaObject::invokeMethod((QWidget*)App()->GetMainWindow(), "OnShowTips", Qt::QueuedConnection,
				Q_ARG(int, TipsWidget::TIP_ERROR), Q_ARG(const QString&, QTStr("MustFirstLoad")));
			return false;
		}
		PLVPlayerPause(mediaPlayer, false);
	}
	else {
		PLVPlayerSetVideo(mediaPlayer, video->vid.c_str(), video->filePath.c_str(), VIDEO_RATE_AUTO);
		PLVPlayerLoadLocal(mediaPlayer, 0);
	}
	UpdatePanel(video->rate, video);
	return true;
}
void PlayerWidget::UpdateOSD(void)
{
	auto & osd = SettingDialog::GetOSDConfig();
	if (!osd.enable) {
		PLVPlayerSetOSDConfig(mediaPlayer, false, nullptr);
		return;
	}
	OSDConfigInfo config;
	config.animationEffect = (OSD_DISPLAY_TYPE)osd.animationEffect;
	config.border = osd.border;
	config.borderAlpha = osd.borderAlpha;
	std::string borderColor = QT_TO_UTF8(osd.borderColor);
	config.borderColor = borderColor.c_str();
	config.borderWidth = osd.borderWidth;
	config.displayDuration = osd.displayDuration;
	config.displayInterval = osd.displayInterval;
	config.fadeDuration = osd.fadeDuration;
	std::string text = QT_TO_UTF8(osd.text);
	config.text = text.c_str();
	config.textAlpha = osd.textAlpha;
	std::string textColor = QT_TO_UTF8(osd.textColor);
	config.textColor = textColor.c_str();
	config.textSize = osd.textSize;
	PLVPlayerSetOSDConfig(mediaPlayer, true, &config);
}

void PlayerWidget::UpdateCache(void)
{
	auto & cache = SettingDialog::GetCacheConfig();
	if (!cache.change) {
		return;
	}
	cache.change = false;
	if (!cache.enable) {
		PLVPlayerSetCacheConfig(mediaPlayer, false, 0, 0);
		return;
	}
	PLVPlayerSetCacheConfig(mediaPlayer, true, cache.maxCacheBytes, cache.maxCacheSeconds);
}

void PlayerWidget::OnPlayerAudioDeviceHandler(int audioDeviceCount)
{
    if (audioDeviceCount > 0) {
        if (!hasAudioPlayDevice) {
            PLVPlayerReloadAudio(mediaPlayer);
        }
        hasAudioPlayDevice = true;
    }
    else {
        hasAudioPlayDevice = false;
    }
}

void PlayerWidget::SetMainWindow(QWidget* win)
{
	mainWindow = win;
	mainWindow->installEventFilter(this);
}

bool PlayerWidget::Play(bool local, const QString& token, const SharedVideoPtr& video, int seekMillisecond)
{
	int ret = PLVPlayerSetVideo(mediaPlayer, video->vid.c_str(), video->filePath.c_str(), video->rate);
	if (E_NO_ERR != ret) {
		slog_error("[sdk]:set player info error:%d", ret);
		return false;
	}
	UpdateOSD();
	isLocal = local;
	if (local) {
		ret = PLVPlayerPlayLocal(mediaPlayer, seekMillisecond);
	}
	else {
		ret = PLVPlayerPlay(mediaPlayer, QT_TO_UTF8(token), seekMillisecond, false);
	}
	if (E_NO_ERR != ret) {
		slog_error("[sdk]:play %s error:%d", (local ? "local" : "online"), ret);
		return false;
	}
	UpdatePanel(video->rate, video);
	return true;
}

bool PlayerWidget::RePlay(int rate, int seekMillisecond, const QString& token, const SharedVideoPtr& video)
{
	int ret = PLVPlayerSetVideo(mediaPlayer, video->vid.c_str(), video->filePath.c_str(), rate);
	if (E_NO_ERR != ret) {
		slog_error("[sdk]:set player info error:%d", ret);
		return false;
	}
	UpdateOSD();
	ret = PLVPlayerPlay(mediaPlayer, QT_TO_UTF8(token), seekMillisecond, false);
	if (E_NO_ERR != ret) {
		slog_error("[sdk]:play online error:%d", ret);
		return false;
	}
	UpdatePanel(rate, video);
	return true;
}

void PlayerWidget::Stop(void)
{
	videoInfo = nullptr;
	PLVPlayerStop(mediaPlayer);
	isShowControl = false;
	UpdateControlPanel(false, false);
	Reset();
}
void PlayerWidget::Destroy(void)
{
	PLVPlayerResetHandler(mediaPlayer);
	Stop();
	//PLVPlayerDestroy(mediaPlayer);
	//mediaPlayer = NULL;
}

bool PlayerWidget::eventFilter(QObject* obj, QEvent *e)
{
	do
	{
		if (QEvent::Enter == e->type() && this == obj) {
			SetPanelVisible(true);
		}
		else if (QEvent::MouseButtonDblClick == e->type() && this == obj) {
			QMouseEvent *me = (QMouseEvent*)e;
			if (Qt::LeftButton == me->button()) {
				fullButton->click();
			}
		}
		else if (QEvent::MouseButtonRelease == e->type() && this == obj) {
			QMouseEvent *me = (QMouseEvent*)e;
			if (Qt::RightButton == me->button()) {
				QPointer<QMenu> popup = new QMenu(this);
				auto AddAction = [this](QPointer<QMenu>& popup, const QString& name, int type) {
					QAction *item = new QAction(name, this);
					popup->addAction(item);
					connect(item, &QAction::triggered, [this, type](bool pause) {
						switch (type)
						{
						case 1:
							startButton->click();
							break;
						case 2:
							stopButton->click();
							break;
						case 3:
							QMetaObject::invokeMethod((QWidget*)App()->GetMainWindow(), "on_paramButton_clicked");
							break;
						case 4:
							fullButton->click();
							break;
						}						
					});
				};
				AddAction(popup, startButton->isChecked() ? QTStr("Pause") : QTStr("Play"), 1);
				AddAction(popup, QTStr("Stop"), 2);
				popup->addSeparator();
				AddAction(popup, QTStr("OpenListParam"), 3);
				AddAction(popup, fullButton->isChecked() ? QTStr("ExitFullScreen") : QTStr("FullScreen"), 4);
				popup->exec(QCursor::pos());
			}
		}
		else if (QEvent::Leave == e->type() && this == obj) {
			hideTimer->start();
		}
		else if (QEvent::Enter == e->type() && volumePanel == obj) {
			volumeHideTimer->stop();
		}
		else if (QEvent::Leave == e->type() && volumePanel == obj) {
			volumeHideTimer->start();
		}
		else if (QEvent::Leave == e->type() && volumeButton == obj) {
			volumeHideTimer->start();
		}
		else if (QEvent::Enter == e->type() && volumeButton == obj) {
			if (!volumeButton->isEnabled()) {
				break;
			}
			QPoint ptY = mediaSlide->mapToGlobal(QPoint(0, 0));
			QPoint ptShow = volumeButton->mapToGlobal(QPoint(volumeButton->width() >> 1, 0));
			volumePanel->show();
			ptShow.setX(ptShow.x() - (volumePanel->width() >> 1));
			ptShow.setY(ptY.y() - volumePanel->height());
			volumePanel->move(ptShow);
		}
	} while (false);

	do
	{
		//if (obj == this && QEvent::Show == e->type()) {
		//	UpdateControlPanel(true, true);
		//}
		if (!(obj == mainWindow || obj == this)) {
			break;
		}
		switch (e->type())
		{
		case QEvent::Hide:
			controlWidget->setVisible(false);
			break;
		case QEvent::Show:
		case QEvent::WindowStateChange:
			UpdateControlPanel(false, false);
			break;
		case QEvent::Move:
		case QEvent::Resize:
			UpdateControlPanel(true, true);
			break;
		}
	} while (false);
	return QWidget::eventFilter(obj, e);
}

void PlayerWidget::SetPanelVisible(bool visible)
{
	rightWidget->setVisible(visible);
}

void PlayerWidget::UpdatePanel(int rate, const SharedVideoPtr& video)
{
	Reset();
	startButton->setChecked(true);
	curRate = rate;
	videoButton->setText(GetVideoName(curRate));
	videoInfo = video;
	PLVPlayerSetVolume(mediaPlayer, volumePanel->GetValue());
	PLVPlayerSetMute(mediaPlayer, volumeButton->isChecked());
	PLVPlayerSetSpeed(mediaPlayer, curSpeed);
	isShowControl = true;
	if (!showControlTimer) {
		showControlTimer = new QTimer(this);
		connect(showControlTimer, &QTimer::timeout, [&] {
			showControlTimer->stop();
			UpdateControlPanel(true, true);
			//Test();
		});
	}
	showControlTimer->start(50);
}

void PlayerWidget::UpdateControlPanel(bool move, bool resize)
{
	if (mainWindow) {
		auto state = mainWindow->windowState();
		if (Qt::WindowMinimized == (Qt::WindowMinimized & state)) {
			controlWidget->setVisible(false);
			return;
		}
	}
	controlWidget->setVisible(isShowPlayer && isShowControl);
	if (!controlWidget->isVisible()) {
		return;
	}
	if (move) {
		auto itemPos = this->mapToGlobal(QPoint(0, 0));
		controlWidget->move(itemPos.x(), itemPos.y());
	}
	if (resize) {
		controlWidget->setFixedSize(this->width(), this->height());
	}
}

void PlayerWidget::Reset(void)
{
	startButton->setChecked(false);
	volumeButton->setChecked(false);
	mediaSlide->blockSignals(true);
	mediaSlide->setValue(0);
	mediaSlide->blockSignals(false);
	if (!PLVPlayerIsLoaded(mediaPlayer)) {
		mediaDuration = 0;
		duration = "00:00:00";
		timeLabel->setText(duration);
	}
}

void PlayerWidget::Test(void)
{
	QVariantList debug;
	debug << PLVPlayerIsLoaded(mediaPlayer) << PLVPlayerIsMute(mediaPlayer) << PLVPlayerIsPause(mediaPlayer) << PLVPlayerGetSpeed(mediaPlayer)
		<< PLVPlayerGetVolume(mediaPlayer) << PLVPlayerGetDuration(mediaPlayer) << PLVPlayerGetCurrentRate(mediaPlayer);
	qDebug() << debug;
}
