#pragma once
#include <QTimer>
#include <QDialog>

namespace Ui {
class LoginDialog;
}

class LoginDialog : public QDialog
{
    Q_OBJECT

public:
    explicit LoginDialog(QWidget *parent = 0);
    ~LoginDialog();


protected:
	virtual bool eventFilter(QObject *obj, QEvent *e) override;
private slots:
	void on_login_clicked(void);

private:
	void ClearError(void);
	void SetError(const QString& error);
private:
    Ui::LoginDialog *ui;
	
	QTimer* loginTimeout;
};

QString Encrypt(const QString& str);
QString Decrypt(const QString& str);
