#include "ParamDialog.h"
#include "ui_ParamDialog.h"

#include "Application.h"
#include "SdkManager.h"


ParamDialog::ParamDialog(QWidget *parent) 
	: QDialog(parent)
	, ui(new Ui::ParamDialog)
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
	setAttribute(Qt::WA_DeleteOnClose, true);
	ui->titleBar->Init(this, TITLE_CLOSE_BTN);
	ui->titleBar->SetLogoable(false, QSize(188, 20));
	ui->titleBar->SetTitleable(true);
	ui->titleBar->SetTitleName(QTStr("ListParam"));

	ui->paramTable->setColumnWidth(0, 120);

	ui->paramTable->horizontalHeader()->setVisible(false);
	ui->paramTable->horizontalHeader()->setFixedHeight(48);
	ui->paramTable->horizontalHeader()->setDefaultAlignment(Qt::AlignCenter);
	ui->paramTable->horizontalHeader()->setStretchLastSection(true);

	ui->paramTable->verticalHeader()->setDefaultSectionSize(48);
	ui->paramTable->verticalHeader()->setCascadingSectionResizes(false);
	ui->paramTable->verticalHeader()->setFixedWidth(16);
	ui->paramTable->verticalHeader()->setVisible(true);
	ui->paramTable->verticalHeader()->setHighlightSections(false);

	ui->paramTable->setSelectionBehavior(QAbstractItemView::SelectRows);
	ui->paramTable->setSelectionMode(QAbstractItemView::SingleSelection);
	ui->paramTable->setFrameShape(QFrame::NoFrame);
	ui->paramTable->setFocusPolicy(Qt::NoFocus);
	ui->paramTable->setShowGrid(false);
	ui->paramTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
	ui->paramTable->setAlternatingRowColors(true);
	ui->paramTable->setSelectionMode(QAbstractItemView::NoSelection);

	InsertItem(MEDIA_PROPERTY_DURATION, QTStr("MediaDuration"));
	InsertItem(MEDIA_PROPERTY_POSTION, QTStr("MediaPostion"));
	InsertItem(MEDIA_PROPERTY_HWDEC, QTStr("MediaHwdec"));
	InsertItem(MEDIA_PROPERTY_VIDEO_CODEC, QTStr("MediaVideoCodec"));
	InsertItem(MEDIA_PROPERTY_VIDEO_BITRATE, QTStr("MediaVideoBitrate"));
	InsertItem(MEDIA_PROPERTY_VIDEO_FPS, QTStr("MediaVideoFPS"));
	InsertItem(MEDIA_PROPERTY_VIDEO_WIDTH, QTStr("MediaVideoWidth"));
	InsertItem(MEDIA_PROPERTY_VIDEO_HEIGHT, QTStr("MediaVideoHeight"));
	InsertItem(MEDIA_PROPERTY_AUDIO_CODEC, QTStr("MediaAudioCodec"));
	InsertItem(MEDIA_PROPERTY_AUDIO_BITRATE, QTStr("MediaAudioBitrate"));
	InsertItem(MEDIA_PROPERTY_CACHE_SPEED, QTStr("MediaCacheSpeed"));
	InsertItem(MEDIA_PROPERTY_CACHE_PROGRESS, QTStr("MediaCacheProgress"));
	InsertItem(MEDIA_PROPERTY_CACHE_TIME, QTStr("MediaCacheTime"));
}

ParamDialog::~ParamDialog()
{
    delete ui;
}

void ParamDialog::OnPropReset(void)
{
	for (auto & it : mapItems) {
		it->setText(QString());
	}
}
void ParamDialog::SetPropValue(int prop, const QString& value)
{
	auto item = mapItems.value(prop);
	if (!item) {
		return;
	}
	switch (prop)
	{
	case MEDIA_PROPERTY_CACHE_SPEED:
	{
		auto speed = value.toInt();
		static const int c_size = 1024;
		speed = speed / c_size;
		if (0 >= speed) {
			speed = 0;
		}
		QString speedStr;
		if (speed < 1024) {
			speedStr = QString("%1%2").arg(speed).arg("KB/S");
		}
		else {
			speedStr = QString("%1%2").arg(QString::number((double)speed / c_size, 'f', 2)).arg("MB/S");
		}
		item->setText(speedStr);
	}
		break;
	case MEDIA_PROPERTY_CACHE_PROGRESS:
		item->setText(value + "%");
		break;
	case MEDIA_PROPERTY_CACHE_TIME:
		item->setText(value + " s");
		break;
	default:
		item->setText(value);
		break;
	}
	
}
void ParamDialog::OnPropChange(int prop, const QString& value)
{
	SetPropValue(prop, value);
}

void ParamDialog::InsertItem(int prop, const QString& name)
{
	int row = ui->paramTable->rowCount();
	ui->paramTable->insertRow(row);
	int col = 0;
	ui->paramTable->setItem(row, col, new QTableWidgetItem());
	auto item = new QTableWidgetItem(name);
	item->setTextAlignment(Qt::AlignCenter);
	ui->paramTable->setItem(row, col++, item);
	item = new QTableWidgetItem();
	item->setTextAlignment(Qt::AlignCenter);
	ui->paramTable->setItem(row, col++, item); 
	
	mapItems[prop] = item;
}
