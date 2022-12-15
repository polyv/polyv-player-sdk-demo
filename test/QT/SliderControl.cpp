#include "SliderControl.h"
#include <QMouseEvent>
#include <QDebug>
#include <QStyleOptionSlider>
#include <QStylePainter>
#include <QPainterPath>

SliderAbsoluteSetStyle::SliderAbsoluteSetStyle(const QString& baseStyle)
	: QProxyStyle(baseStyle)
{
}
SliderAbsoluteSetStyle::SliderAbsoluteSetStyle(QStyle* baseStyle)
	: QProxyStyle(baseStyle)
{
}

int SliderAbsoluteSetStyle::styleHint(QStyle::StyleHint hint,
	const QStyleOption* option = 0,
	const QWidget* widget = 0,
	QStyleHintReturn* returnData = 0) const
{
	if (hint == QStyle::SH_Slider_AbsoluteSetButtons)
		return (Qt::LeftButton | Qt::MiddleButton);
	return QProxyStyle::styleHint(hint, option, widget, returnData);
}



////////////////////////////////////////////////////////////////
SliderControl::SliderControl(QWidget* parent)
	: QSlider(parent)
{
	setStyle(new SliderAbsoluteSetStyle(this->style()));
}

void SliderControl::SetPreValue(int value)
{
	preValue = value;
	repaint();
}

void SliderControl::paintEvent(QPaintEvent* e)
{
	QSlider::paintEvent(e);

	if (preValue > 0 && preValue != this->value()) {
		QPainter p(this);
		QStyleOptionSlider opt;
		initStyleOption(&opt);

		QRect rect = style()->subControlRect(QStyle::CC_Slider, &opt, QStyle::SC_SliderGroove, this);
		int width = (preValue * 1.0 /this->maximum() * rect.width());
		//rect.setWidth(width);
		p.setPen(Qt::NoPen);
		p.setBrush(QColor(255, 255, 255, 180));
		p.drawRoundedRect(QRect(rect.left(), rect.top(), width, rect.height()), 2, 2);

		opt.subControls = QStyle::SC_SliderHandle;
		style()->drawComplexControl(QStyle::CC_Slider, &opt, &p, this);
	}
}

//void SliderControl::mousePressEvent(QMouseEvent *e)
//{
//	int value = 0;
//	
//
//	if (Qt::Horizontal == orientation()) {
//		int currentX = e->pos().x();
//		double per = currentX * 1.0 / this->width();
//		value = per * (this->maximum() - this->minimum()) + this->minimum();
//	}
//	else {
//		int currentY = e->pos().y();
//		double per = currentY * 1.0 / this->height();
//		value = per * (this->maximum() - this->minimum()) + this->minimum();
//		value = this->maximum() - value;
//	}
//	//int value = per * (this->maximum() - this->minimum()) + this->minimum();
//
//	//qDebug() << "1111111111:" << value;
//
//	this->setValue(value);
//
//	QSlider::mousePressEvent(e);
//}