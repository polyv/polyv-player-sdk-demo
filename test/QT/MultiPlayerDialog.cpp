#include "MultiPlayerDialog.h"
#include "ui_MultiPlayerDialog.h"

#include <QDebug>

#include "Application.h"
#include "MainWindow.h"
#include "PlayerWidget.h"
#include "TipsWidget.h"


static QList<MultiPlayerDialog*> s_listPlayer;

MultiPlayerDialog::MultiPlayerDialog(QWidget *parent) 
	: QDialog(parent)
	, ui(new Ui::MultiPlayerDialog)
{
    ui->setupUi(this);
	Qt::WindowFlags f;
	f |= Qt::Dialog;
	f |= Qt::WindowCloseButtonHint;
	f |= Qt::WindowMinMaxButtonsHint;
	f |= Qt::WindowSystemMenuHint;
#ifdef _WIN32
	f |= Qt::FramelessWindowHint;
#endif
	setWindowFlags(f);

	ui->titleBar->Init(this, TITLE_ALL_BTN);
	ui->titleBar->SetResizable(true);
	ui->titleBar->SetTitleable(true);
	ui->titleBar->SetLogoable(false, QSize());
	ui->titleBar->SetTitleName(tr("MultiPlayer"));
#ifdef _WIN32
	ui->titleBar->SetResizeFrameColor(QColor(12, 12, 12));
#endif

	setMinimumWidth(430);
	setMinimumHeight(400);

	this->installEventFilter(this);
}

MultiPlayerDialog::~MultiPlayerDialog()
{
	s_listPlayer.removeOne(this);
	ui->player->Destroy();
    delete ui;
	qDebug() << __FUNCTION__;
}

void MultiPlayerDialog::Open(const SharedVideoPtr& video)
{
	QString vid = QString::fromStdString(video->vid);
	MultiPlayerDialog* player = nullptr;
	for (auto& it : s_listPlayer) {
		if (it->GetVid() == vid) {
			player = it;
			break;
		}
	}
	if (player) {
		player->activateWindow();
		player->raise();
	}
	else {
		player = new MultiPlayerDialog();
		player->setAttribute(Qt::WA_DeleteOnClose, true);
		player->show();
		s_listPlayer.append(player);
	}
	player->LoadLocal(video);
}

void MultiPlayerDialog::CloseAll()
{
	auto list = s_listPlayer;
	s_listPlayer.clear();
	for (auto& it : list) {
		it->close();
	}
}

void MultiPlayerDialog::LoadLocal(const SharedVideoPtr& video)
{
	videoId = QString::fromStdString(video->vid);
	ui->titleBar->SetTitleName(QString::fromStdString(video->title));
	ui->player->SetFilter(this);
	ui->player->LoadLocal(video);
}
void MultiPlayerDialog::closeEvent(QCloseEvent* e)
{
	ui->player->Destroy();
	QDialog::closeEvent(e);
}
bool MultiPlayerDialog::eventFilter(QObject* obj, QEvent* e)
{
	switch (e->type()) {
	case QEvent::KeyPress: {
		QKeyEvent* key = (QKeyEvent*)e;
		if (Qt::Key_Escape == key->key() && isFullScreen()) {
			e->ignore();
			OnExitFullScreen();
			return true;
		}
		break;
	}
	}
	return QDialog::eventFilter(obj, e);
}

void MultiPlayerDialog::OnChangeRatePlayVideo(int rate, int seekMillisecond, const SharedVideoPtr& video)
{
	QString vid = QString::fromStdString(video->vid);
	QString token;
	if (1 == video->seed) {
		token = App()->GetMainWindow()->GetToken(vid);
	}
	ui->player->OnlineRePlay(rate, seekMillisecond, token, video);
}
void MultiPlayerDialog::OnFullScreen(void)
{
	ui->titleBar->ShowFullScreen(true);
}
void MultiPlayerDialog::OnExitFullScreen(void)
{
	ui->titleBar->ShowFullScreen(false);
}
void MultiPlayerDialog::OnShowTips(int level, const QString& msg)
{
	TipsWidget::ShowLevel(ui->player, msg, (TipsWidget::TipType)level);
}

