#include "VidDialog.h"
#include "ui_VidDialog.h"

#include <QClipboard>

#include "Application.h"
#include "WidgetHelper.h"
#include "TipsWidget.h"
#include "SdkManager.h"


VidDialog::VidDialog(QWidget *parent) 
	: QDialog(parent)
	, ui(new Ui::VidDialog)
{
    ui->setupUi(this);
	Qt::WindowFlags f;
	f |= Qt::Dialog;
	f |= Qt::WindowCloseButtonHint;
	f |= Qt::CustomizeWindowHint;
	f |= Qt::WindowSystemMenuHint;
#ifdef _WIN32
	f |= Qt::FramelessWindowHint;
#endif
	setWindowFlags(f);
	//setAttribute(Qt::WA_DeleteOnClose, false);
	ui->titleBar->Init(this, TITLE_CLOSE_BTN);
	ui->titleBar->SetTitleable(true);
	ui->titleBar->SetTitleName(QTStr("VIDPlay"));
	ui->titleBar->SetLogoable(false, QSize(188, 20));

	ui->vid->setFocus();
	InitVid();
}

VidDialog::~VidDialog()
{
    delete ui;
}

QString VidDialog::GetVID() const
{
	return ui->vid->text();
}


void VidDialog::on_get_clicked(void)
{
	if (ui->vid->text().isEmpty()) {
		TipsWidget::ShowToast(this, QTStr("VIDEmpty"));
		return;
	}
	accept();
}

void VidDialog::InitVid()
{
	do
	{
		QClipboard* clipboard = QGuiApplication::clipboard();
		auto vid = clipboard->text();
		auto ret = vid.split("_");
		if (2 != ret.size()) {
			break;
		}
		if (32 != ret.at(0).size()) {
			break;
		}
		if (1 != ret.at(1).size()) {
			break;
		}
		ui->vid->setText(vid);

	} while (false);
}

