#pragma once

#include <QDialog>
#include <QMap>
#include <QTableWidgetItem>

namespace Ui {
class ParamDialog;
}

class ParamDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ParamDialog(QWidget *parent = 0);
    ~ParamDialog();

	void SetVideoName(const QString& name);
	void SetPropValue(int prop, const QString& value);
private slots:
	void OnPropReset(void);
	void OnPropChange(int prop, const QString& value);
	
private:
	void InsertItem(int prop, const QString& name);
private:


    Ui::ParamDialog *ui;

	QMap<int, QTableWidgetItem*> mapItems;
};

