#pragma once

#include <QWidget>
#include <QTimer>
#include <QLabel>
#include <QSlider>
#include <QPushButton>


class VolumeControl :public QWidget {

	Q_OBJECT
public:
	VolumeControl(QWidget* parent = nullptr);

	void SetValue(int value);
	int GetValue(void) const;
	int GetOldValue(void) const {
		return oldValue;
	}
signals:
	void SignalVolume(int volume);
private:
	QSlider* volumeSlider;
	int oldValue = 0;
};







