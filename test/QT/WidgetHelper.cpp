#include "WidgetHelper.h"

#include <QStyle>
#include <QListWidget>
#include <QStandardPaths>

#include "AppDef.h"
#include "SdkManager.h"
#include "Application.h"

QObject* GetRootParent(QObject* parent)
{
	if (!parent) {
		return NULL;
	}
	auto p = parent->parent();
	if (!p) {
		return parent;
	}
	return GetRootParent(p);
}

void SetControlSheet(QWidget* widget, const QString& objectName)
{
	if (!widget) {
		return;
	}
	widget->setObjectName(objectName);
	widget->style()->unpolish(widget);
	widget->style()->polish(widget);
}

void SetControlSheet(QWidget* widget, const char* propertyName, const QVariant& propertyValue)
{
	if (!widget) {
		return;
	}
	widget->setProperty(propertyName, propertyValue);
	widget->style()->unpolish(widget);
	widget->style()->polish(widget);
}

void InitComboBox(QComboBox*comboBox)
{
	QListWidget *widget = new QListWidget();
	widget->setStyleSheet("QListWidget::item{height:32px;}");
	comboBox->setModel(widget->model());
	comboBox->setView(widget);
}
void SetComboxListView(QComboBox* comboBox)
{
	QListWidget *widget = new QListWidget();
	widget->setStyleSheet("QListWidget::item{height:32px;}");
	widget->setAttribute(Qt::WA_MacShowFocusRect, false);
	comboBox->setModel(widget->model());
	comboBox->setView(widget);
}

QComboBox* CreateComboBox(QWidget* parent, const QString& name)
{
	QComboBox * comboBox = new QComboBox(parent);
	QListWidget *widget = new QListWidget();
	widget->setStyleSheet("QListWidget::item{height:32px;}");
	comboBox->setModel(widget->model());
	comboBox->setView(widget);
	if (!name.isEmpty()) {
		comboBox->setObjectName(name);
	}
	return comboBox;
}

struct NameInfo1 {
	int type;
	QString desc;
};
struct NameInfo2 {
	double type;
	QString desc;
};

QString GetVideoName(int type)
{
	static struct NameInfo1 videoNameInfo[] = {
	{VIDEO_RATE_LD, QTStr("LD")},
	{VIDEO_RATE_SD, QTStr("SD")},
	{VIDEO_RATE_HD, QTStr("HD")},
	{VIDEO_RATE_AUTO, QTStr("Auto")},
	{VIDEO_RATE_SOURCE, QTStr("Source")},
	};

	static int count = sizeof(videoNameInfo) / sizeof(videoNameInfo[0]);
	for (int i = 0; i < count; ++i) {
		if (videoNameInfo[i].type == type) {
			return videoNameInfo[i].desc;
		}
	}
	return QString();
}

QString GetSpeedName(double type)
{
	static struct NameInfo2 speedNameInfo[] = {
	{kSpeed05, "0.5x"},
	{kSpeed10, "1.0x"},
	{kSpeed12, "1.2x"},
	{kSpeed15, "1.5x"},
	{kSpeed20, "2.0x"},
	};

	static int count = sizeof(speedNameInfo) / sizeof(speedNameInfo[0]);
	for (int i = 0; i < count; ++i) {
		if (speedNameInfo[i].type == type) {
			return speedNameInfo[i].desc;
		}
	}
	return QString();
}

QString GetVideoPath(void)
{
	QStringList tempPaths = QStandardPaths::standardLocations(QStandardPaths::MoviesLocation);

	return tempPaths[0];
}

QString GetConfigPath(void)
{
	QStringList tempPaths = QStandardPaths::standardLocations(QStandardPaths::AppDataLocation);

	return QString("%1/..").arg(tempPaths[0]);
	//QString logPath = QString("%1/../%2").arg(tempPaths[0]).arg(path);
	//CFileUtil::mkdirs(QT_TO_UTF8(logPath));
}

