#pragma once

#include <QDialog>
#include <QUrl>
#include <QMap>
#include <QTableWidgetItem>

namespace Ui {
class MigrateDialog;
}

class MigrateDialog : public QDialog
{
    Q_OBJECT

public:
    explicit MigrateDialog(QWidget *parent = 0);
    ~MigrateDialog();
	
//private:
//	void InsertItem(int prop, const QString& name);
private slots:
    void on_addPathButton_clicked(void);
    void on_migrateButton_clicked(void);
    void on_clearButton_clicked(void);
private:
    bool CheckExist(const QString& keyFile);
    void DropFile(const QList<QUrl>& urls);
    void DeleteItem(const QString& keyFile);
    void GetAllFiles(const QString& folder, QStringList& files);
    
private:
    Ui::MigrateDialog *ui;
};

