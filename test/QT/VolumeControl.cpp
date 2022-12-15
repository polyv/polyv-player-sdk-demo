#include "VolumeControl.h"

#include <QHBoxLayout>

#include "Application.h"
#include "SliderControl.h"


VolumeControl::VolumeControl(QWidget* parent)
	: QWidget(parent)
{
	setWindowFlags(Qt::FramelessWindowHint | Qt::Tool | Qt::Window | Qt::NoDropShadowWindowHint);
	setFixedSize(40, 120);

	volumeSlider = new QSlider(this);
	volumeSlider->setOrientation(Qt::Vertical);
	volumeSlider->setMinimum(0);
	volumeSlider->setMaximum(100);
	volumeSlider->setFixedSize(QSize(14, 94));
	volumeSlider->setCursor(QCursor(Qt::PointingHandCursor));
	volumeSlider->setTracking(false);
	volumeSlider->setValue(0);

	QVBoxLayout *mainLayout = new QVBoxLayout();
	mainLayout->addWidget(volumeSlider, 0, Qt::AlignCenter);
	setLayout(mainLayout);

	connect(volumeSlider, SIGNAL(valueChanged(int)), this, SIGNAL(SignalVolume(int)));
	connect(volumeSlider, &QSlider::valueChanged, this, [this](int value) {
		this->setToolTip(QString(QTStr("VolumeValue")).arg(value));
	});
}

void VolumeControl::SetValue(int value)
{
	volumeSlider->blockSignals(true);
	if (0 == value) {
		oldValue = volumeSlider->value();
	}
	volumeSlider->setValue(value);
	this->setToolTip(QString(QTStr("VolumeValue")).arg(volumeSlider->value()));
	volumeSlider->blockSignals(false);
}

int VolumeControl::GetValue(void) const
{
	return volumeSlider->value();
}

