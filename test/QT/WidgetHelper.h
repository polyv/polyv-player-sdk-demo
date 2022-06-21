#pragma once
#include <QWidget>
#include <QComboBox>


QObject* GetRootParent(QObject* parent);
void SetControlSheet(QWidget* widget, const QString& objectName);
void SetControlSheet(QWidget* widget, const char* propertyName, const QVariant& propertyValue);

void InitComboBox(QComboBox*comboBox);
void  SetComboxListView(QComboBox* commbo);
QComboBox* CreateComboBox(QWidget* parent, const QString& name);


QString GetVideoName(int type);
QString GetSpeedName(double type);


QString GetVideoPath(void);
QString GetConfigPath(void);










