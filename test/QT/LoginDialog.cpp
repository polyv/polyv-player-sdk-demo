#include "LoginDialog.h"
#include "ui_LoginDialog.h"

#include <QMovie>
#include <QListWidget>
#include <QCompleter>
#include <QStringListModel>
#include <QKeyEvent>

#include <QTimer>
#include <QDebug>

#include "SdkManager.h"
#include "Application.h"
#include "SettingDialog.h"
static const char KStrKey[] = { 'K','P','l','v','P','l','a','y','e','r','S','t','r','K','e','y' };


/***************************************************************/
LoginDialog::LoginDialog(QWidget *parent) 
	: QDialog(parent)
	, ui(new Ui::LoginDialog)
{
	ui->setupUi(this);
	Qt::WindowFlags f;
	f |= Qt::Dialog;
	f |= Qt::WindowCloseButtonHint;
	f |= Qt::WindowMinimizeButtonHint;
	f |= Qt::CustomizeWindowHint;
	f |= Qt::WindowSystemMenuHint;
#ifdef _WIN32
	f |= Qt::FramelessWindowHint;
#endif
	setWindowFlags(f);
	ui->titleBar->Init(this, TITLE_CLOSE_BTN | TITLE_MIN_BTN);
	ui->titleBar->SetTitleable(false);
	ui->titleBar->SetLogoable(false, QSize(188, 20));

	ui->userId->installEventFilter(this);
	ui->secretKey->installEventFilter(this);
	ui->readToken->installEventFilter(this);
	
	ClearError();

	loginTimeout = new QTimer(this);
	loginTimeout->setInterval(50 * 1000);
	connect(loginTimeout, &QTimer::timeout, [this] {
		loginTimeout->stop();
		ui->login->setEnabled(true);
		SetError(QTStr("InitTimeout"));
	});

	QFile infofile("info.debug");
	if (infofile.open(QIODevice::ReadOnly | QIODevice::Text)) {
		while (!infofile.atEnd()) {
			auto line = QByteArray::fromBase64(infofile.readLine());
			auto v = QString(line).split(":");
			if (2 == v.size()) {
				if ("UserId" == v.at(0)) {
					ui->userId->setText(Decrypt(v.at(1)));
				}
				else if ("SecretKey" == v.at(0)) {
					ui->secretKey->setText(Decrypt(v.at(1)));
				}
				else if ("ReadToken" == v.at(0)) {
					ui->readToken->setText(Decrypt(v.at(1)));
				}
			}
		}
		infofile.close();
	}
	else {
		if (App()->GlobalConfig().Get("App", "Remember").toBool()) {
			ui->remember->setChecked(true);
			ui->userId->setText(Decrypt(App()->GlobalConfig().Get("App", "UserId").toString()));
			ui->secretKey->setText(Decrypt(App()->GlobalConfig().Get("App", "SecretKey").toString()));
			ui->readToken->setText(Decrypt(App()->GlobalConfig().Get("App", "ReadToken").toString()));
		}
	}

#ifdef _DEBUG

#endif// _DEBUG
}

LoginDialog::~LoginDialog()
{
    delete ui;
}

bool LoginDialog::eventFilter(QObject *obj, QEvent *e)
{
	do {
		if (QEvent::KeyPress != e->type()) {
			break;
		}
		if (ui->userId == obj || ui->secretKey == obj || ui->readToken == obj) {
			ClearError();
		}
	} while (false);

	return QDialog::eventFilter(obj, e);
}


void LoginDialog::on_login_clicked(void)
{
	ClearError();
	QString userId = ui->userId->text().trimmed();
	if (userId.isEmpty()) {
		SetError(QTStr("EmptyUserId"));
		return;
	}
	QString secretKey = ui->secretKey->text().trimmed();
	if (secretKey.isEmpty()) {
		SetError(QTStr("EmptySecretKey"));
		return;
	}
	QString readToken = ui->readToken->text().trimmed();
	if (readToken.isEmpty()) {
		SetError(QTStr("EmptyReadToken"));
		return;
	}
	loginTimeout->start();
	ui->login->setEnabled(false);

	static bool init = false;
	if (!init) {
		init = true;
		connect(SdkManager::GetManager(), &SdkManager::SignalInitResult, [this](bool result, const QString& msg) {
			if (!loginTimeout->isActive()) {
				return;
			}
			loginTimeout->stop();
			ui->login->setEnabled(true);
			if (result) {
				App()->GlobalConfig().Set("App", "Remember", ui->remember->isChecked());
				App()->GlobalConfig().Set("App", "UserId", Encrypt(ui->userId->text()).toStdString().c_str());
				App()->GlobalConfig().Set("App", "SecretKey", Encrypt(ui->secretKey->text()).toStdString().c_str());
				App()->GlobalConfig().Set("App", "ReadToken", Encrypt(ui->readToken->text()).toStdString().c_str());
				App()->GlobalConfig().Save();
				if (QFile::exists("toinfo.debug")) {
					QFile infofile("info.debug");
					if (infofile.open(QIODevice::ReadWrite | QIODevice::Text)) {
						QByteArray userId(QString("UserId:%1").arg(Encrypt(ui->userId->text())).toUtf8());
						infofile.write(userId.toBase64());
						infofile.write("\n");
						QByteArray secretKey(QString("SecretKey:%1").arg(Encrypt(ui->secretKey->text())).toUtf8());
						infofile.write(secretKey.toBase64());
						infofile.write("\n");
						QByteArray readToken(QString("ReadToken:%1").arg(Encrypt(ui->readToken->text())).toUtf8());
						infofile.write(readToken.toBase64());
						infofile.flush();
						infofile.close();
					}
				}
				SettingDialog::SetOSDText(SdkManager::GetManager()->GetAccount().viewerId);
				accept();
				return;
			}
			SetError(msg);
		});
	}
	
	SdkManager::GetManager()->Init(userId, secretKey, readToken);
	SdkManager::GetManager()->SetViewer("test11111111", "testname11", "");
}


void LoginDialog::ClearError(void)
{
	ui->errorButton->setText(QString());
	ui->errorButton->setVisible(false);
}

void LoginDialog::SetError(const QString& error)
{
	ui->errorButton->setText(error);
	ui->errorButton->setVisible(true);
}

QString Encrypt(const QString& src)
{
	if (src.isEmpty()) {
		return src;
	}
	QByteArray dst = src.toUtf8();
	int size = sizeof(KStrKey) - 1;
	for (int i = 0; i < dst.size(); ++i) {
		dst[i] = dst[i] ^ KStrKey[i % size];
	}
	return QString::fromUtf8(dst.toBase64());
}
QString Decrypt(const QString& src)
{
	if (src.isEmpty()) {
		return src;
	}
	QByteArray dst = src.toUtf8();
	dst = QByteArray::fromBase64(dst);
	int size = sizeof(KStrKey) - 1;
	for (int i = 0; i < dst.size(); ++i) {
		dst[i] = dst[i] ^ KStrKey[i % size];
	}
	return QString::fromUtf8(dst);
}