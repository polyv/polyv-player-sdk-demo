#pragma once

#include <QDialog>

namespace Ui {
class InputDialog;
}

class InputDialog : public QDialog
{
    Q_OBJECT

public:
    enum class INPUT_TYPE{
        VID = 0,
        CHANNEL,
    };
    explicit InputDialog(INPUT_TYPE type, QWidget *parent = 0);
    ~InputDialog();


	QString GetParam() const;
private slots:
	void on_ok_clicked(void);
	
private:
    void InitParam();
private:
    Ui::InputDialog *ui;
    INPUT_TYPE inputType = INPUT_TYPE::VID;
};

