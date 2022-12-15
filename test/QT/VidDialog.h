#pragma once

#include <QDialog>

namespace Ui {
class VidDialog;
}

class VidDialog : public QDialog
{
    Q_OBJECT

public:
    explicit VidDialog(QWidget *parent = 0);
    ~VidDialog();


	QString GetVID() const;
private slots:
	void on_get_clicked(void);
	
private:
    void InitVid();
private:
    Ui::VidDialog *ui;
};

