#pragma once
#include <QWidget>
#include <QLabel>
#include <QTimer>
#include <QSlider>
#include <QPaintEvent>
#include <QPushButton>
#include <QStackedWidget>

#include "AppDef.h"
#include "VideoControl.h"

class VolumeControl;
class IPLVMediaPlayer;


class PlayerWidget : public QWidget {
	Q_OBJECT
public:
	PlayerWidget(QWidget* parent = nullptr);
	~PlayerWidget(void);

	void SetMainWindow(QWidget* win);

	bool Play(bool local, const QString& token, const SharedVideoPtr& video, int seekMillisecond);
	bool RePlay(int rate, int seekMillisecond, const QString& token, const SharedVideoPtr& video);
	void Stop(void);
	void Destroy(void);
	bool LoadLocal(int opt, const SharedVideoPtr& video);
	void UpdateOSD(void);
	void UpdateCache(void);

	void SetShowPlayer(bool show) {
		isShowPlayer = show;
	}

	struct MediaProp {
		int property;
		int format;
		QString value;
	};
	QMap<int, MediaProp> GetProps(void) const {
		return mapProp;
	}
	
signals:
	void SignalPropChange(int prop, const QString& value);
	void SignalPropReset(void);
protected:
	virtual bool eventFilter(QObject *, QEvent *) override;
private slots:
	void OnPlayerStateHandler(int state);
	void OnPlayerPropertyHandler(int property, int format, QString value);
	void OnPlayerRateChangeHandler(int inputBitRate, int realBitRate);
	void OnPlayerProgressHandler(int millisecond);
    void OnPlayerAudioDeviceHandler(int audioDeviceCount);
private:
	void SetPanelVisible(bool visible);
	void UpdatePanel(int rate, const SharedVideoPtr& video);
	void UpdateControlPanel(bool move, bool resize);
	void Reset(void);
	void Test(void);
private:
	QLabel* timeLabel;
	QWidget* rightWidget;
	QWidget* playerWidget;
	QWidget* controlWidget;
	QWidget* controlBarWidget;
	QPushButton* videoButton;
	QPushButton* startButton;
	QPushButton* stopButton;
	QPushButton* volumeButton;
	QPushButton* fullButton;
	QTimer* hideTimer;
	QTimer* volumeHideTimer;
	QTimer* showControlTimer = nullptr;
	QSlider *mediaSlide;
	VolumeControl *volumePanel = nullptr;

	QWidget* mainWindow = nullptr;

	SharedVideoPtr videoInfo;
	int mediaDuration = 0;
	QString duration;
	int rateCount = VIDEO_RATE_BD;
	int curRate = VIDEO_RATE_AUTO;
	double curSpeed = kSpeed10;
	bool isLocal = false;
	bool isShowControl = false;
    bool hasAudioPlayDevice = true;
	bool isShowPlayer = false;
	PLVPlayerPtr mediaPlayer = nullptr;

	
	QMap<int, MediaProp> mapProp;
};