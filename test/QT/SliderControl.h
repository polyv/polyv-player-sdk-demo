#pragma once
#include <QSlider>



class SliderControl :public QSlider {

	Q_OBJECT
public:
	SliderControl(QWidget* parent = nullptr);

protected:
	virtual void mousePressEvent(QMouseEvent *e) override;
};







