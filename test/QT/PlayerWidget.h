#pragma once
#include <QWidget>
#include <QMenu>
#include <QLabel>
#include <QTimer>
#include <QSlider>
#include <QPaintEvent>
#include <QPushButton>
#include <QStackedWidget>

#include "AppDef.h"
#include "VideoControl.h"

#include "Player.h"


class VolumeControl;
class SliderControl;
class ParamDialog;

//////////////////////////////////////////////////
class PlayerControl : public QStackedWidget {
	Q_OBJECT
public:
	explicit PlayerControl(Player* _player, QWidget* parent = nullptr);
	virtual ~PlayerControl(void);

public:
	void SetInfo(bool local, int rate, const SharedVideoPtr& video);
	void Reset();
	void SetFilter(const QObject* object);
	QObject* GetFilter() {
		return filterObject;
	}
	void UpdatePos();

	void OpenParamWindow();

signals:
	void SignalPropChange(int prop, const QString& value);
	void SignalPropReset(void);
protected:
	bool eventFilter(QObject*, QEvent*) override;

private:
	void SetRate(int rate, bool notify);
	void SetSpeed(double speed);
	void SetVisible(bool visible);
	void SetBarVisible(bool visible);

	struct MediaProperty {
		int property;
		int format;
		QString value;
	};
	//QMap<int, MediaProperty> GetProperties(void) const {
	//	return mapProperty;
	//}
private:
	Player* player = nullptr;
	QObject* filterObject = nullptr;

	QWidget* barWidget = nullptr;

	QMovie* loadingMovie = nullptr;
	QWidget* loadingWidget = nullptr;

	QPushButton* shotButton = nullptr;
	QPushButton* volumeButton = nullptr;
	QPushButton* speedButton = nullptr;
	QPushButton* videoButton = nullptr;

	QPointer<QMenu> popup = nullptr;

	SliderControl* mediaSlide = nullptr;
	VolumeControl* volumePanel = nullptr;

	QTimer* shotPanelHideTimer = nullptr;
	QTimer* volumePanelHideTimer = nullptr;

	SharedVideoPtr curVideo;
	int curRate = VIDEO_RATE_AUTO;
	bool isLocalPlay = false;
	bool isEnd = true;
	double curSpeed = kSpeed10;

	qint64 activeTime = 0;
	QTimer* fullScreenTimer = nullptr;

	QPointer<ParamDialog> paramDialog;
	QMap<int, MediaProperty> mapProperty;
};

//////////////////////////////////////////////////
class PlayerWidget : public QWidget {
	Q_OBJECT
public:
	PlayerWidget(QWidget* parent = nullptr);
	virtual ~PlayerWidget(void);

public:
	void SetFilter(const QObject* object);
	QObject* GetFilter();
	void UpdatePos();
	void Destroy();

	bool Play(bool local, const QString& token, const SharedVideoPtr& video, int rate, int seekMillisecond);
	bool OnlineRePlay(int rate, int seekMillisecond, const QString& token, const SharedVideoPtr& video);

	void Stop(void);
	bool LoadLocal(const SharedVideoPtr& video);

	void RefreshPlayer();

	void OpenParamWindow();

	bool IsLoading();
protected:
	void paintEvent(QPaintEvent* e) override;
	void closeEvent(QCloseEvent* e) override;
private:
	bool SetInfo(bool local, int rate, const SharedVideoPtr& video);
	bool StartPlay(bool local, int rate, const QString& token, const SharedVideoPtr& video, int seekMillisecond);

	//void SetPanelVisible(bool visible);
	//void UpdatePanel(int rate, const SharedVideoPtr& video);
	//void UpdateControlPanel(bool move, bool resize);
	//void Reset(void);
private:
	Player* player = nullptr;
	PlayerControl* controller = nullptr;
};