#pragma once
#include <QSlider>

#include <QProxyStyle>

class SliderAbsoluteSetStyle : public QProxyStyle {
public:
	SliderAbsoluteSetStyle(const QString& baseStyle);
	SliderAbsoluteSetStyle(QStyle* baseStyle = Q_NULLPTR);

public:
	int styleHint(QStyle::StyleHint hint, const QStyleOption* option,
		const QWidget* widget,
		QStyleHintReturn* returnData) const override;
};


class SliderControl :public QSlider {

	Q_OBJECT
public:
	SliderControl(QWidget* parent = nullptr);

	void SetPreValue(int value);
protected:
	void paintEvent(QPaintEvent* e) override;
	//void mousePressEvent(QMouseEvent *e) override;

private:
	int preValue = 0;
};







