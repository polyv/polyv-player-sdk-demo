#include "SliderControl.h"
#include <QMouseEvent>
#include <QDebug>

SliderControl::SliderControl(QWidget* parent)
	: QSlider(parent)
{

}

void SliderControl::mousePressEvent(QMouseEvent *e)
{
	int value = 0;
	

	if (Qt::Horizontal == orientation()) {
		int currentX = e->pos().x();
		double per = currentX * 1.0 / this->width();
		value = per * (this->maximum() - this->minimum()) + this->minimum();
	}
	else {
		int currentY = e->pos().y();
		double per = currentY * 1.0 / this->height();
		value = per * (this->maximum() - this->minimum()) + this->minimum();
		value = this->maximum() - value;
	}
	//int value = per * (this->maximum() - this->minimum()) + this->minimum();

	//qDebug() << "1111111111:" << value;

	this->setValue(value);

	QSlider::mousePressEvent(e);
}