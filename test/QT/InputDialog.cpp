#include "InputDialog.h"
#include "ui_InputDialog.h"

#include <QClipboard>

#include "Application.h"
#include "WidgetHelper.h"
#include "TipsWidget.h"
#include "SdkManager.h"


InputDialog::InputDialog(INPUT_TYPE type, QWidget *parent)
	: QDialog(parent) 
	, ui(new Ui::InputDialog)
	, inputType(type)
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
	ui->titleBar->SetTitleName(QTStr("InputParam"));
	ui->titleBar->SetLogoable(false, QSize(188, 20));

	ui->param->setFocus();
	InitParam();
}

InputDialog::~InputDialog()
{
    delete ui;
}

QString InputDialog::GetParam() const
{
	return ui->param->toPlainText();
}


void InputDialog::on_ok_clicked(void)
{
	if (ui->param->toPlainText().isEmpty()) {
		TipsWidget::ShowToast(this, QTStr("ParamEmpty"));
		return;
	}
	accept();
}

void InputDialog::InitParam()
{
	auto CheckVid = [](const QString& vid) {
		do
		{
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
			return true;
		} while (false);
		return false;
	};
	auto InitVid = [&] {
		do
		{
			QClipboard* clipboard = QGuiApplication::clipboard();
			auto vid = clipboard->text();
			auto vids = vid.split(",");
			if (vids.count() > 0) {
				for (auto& it : vids) {
					if (!CheckVid(it)) {
						return;
					}
				}
				ui->param->setText(vid);
			}
			else if (CheckVid(vid)) {
				ui->param->setText(vid);
			}
		} while (false);
	};

	auto InitChannel = [&] {
		QClipboard* clipboard = QGuiApplication::clipboard();
		auto channel = clipboard->text();
		if (channel.contains(QRegExp("^\\d+$"))) {
			ui->param->setText(channel);
		}
	};
	INPUT_TYPE::VID == inputType ? InitVid() : InitChannel();
}

