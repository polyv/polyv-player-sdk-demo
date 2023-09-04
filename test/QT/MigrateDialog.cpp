#include "MigrateDialog.h"
#include "ui_MigrateDialog.h"

#include <QDir>
#include <QDebug>
#include <QFileInfo>
#include <QFileDialog>

#include "Application.h"
#include "MainWindow.h"
#include "SdkManager.h"
#include "GlobalConfig.h"
#include "StatusButton.h"

#include "drop-file-filter.h"


MigrateDialog::MigrateDialog(QWidget *parent) 
	: QDialog(parent)
	, ui(new Ui::MigrateDialog)
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
	ui->titleBar->Init(this, TITLE_CLOSE_BTN);
	ui->titleBar->SetLogoable(false, QSize(188, 20));
	ui->titleBar->SetTitleable(true);
	ui->titleBar->SetTitleName(QTStr("MigrateVideo"));

	ui->keyfileTable->horizontalHeader()->setVisible(true);
	ui->keyfileTable->horizontalHeader()->setFixedHeight(48);
	ui->keyfileTable->horizontalHeader()->setDefaultAlignment(Qt::AlignVCenter | Qt::AlignLeft);
	ui->keyfileTable->horizontalHeader()->setStretchLastSection(false);

	ui->keyfileTable->model()->setHeaderData(1, Qt::Horizontal, (int)(Qt::AlignCenter),
		Qt::TextAlignmentRole);
	ui->keyfileTable->model()->setHeaderData(2, Qt::Horizontal, (int)(Qt::AlignCenter),
		Qt::TextAlignmentRole);

	int col = 1;
	ui->keyfileTable->setColumnWidth(col++, 140);
	ui->keyfileTable->setColumnWidth(col++, 60);
	col = 0;
	ui->keyfileTable->horizontalHeader()->setSectionResizeMode(col++, QHeaderView::Stretch);
	ui->keyfileTable->horizontalHeader()->setSectionResizeMode(col++, QHeaderView::Fixed);
	ui->keyfileTable->horizontalHeader()->setSectionResizeMode(col++, QHeaderView::Fixed);

	ui->keyfileTable->verticalHeader()->setDefaultSectionSize(48);
	ui->keyfileTable->verticalHeader()->setCascadingSectionResizes(false);
	ui->keyfileTable->verticalHeader()->setFixedWidth(16);
	ui->keyfileTable->verticalHeader()->setVisible(false);
	ui->keyfileTable->verticalHeader()->setHighlightSections(false);

	ui->keyfileTable->setSelectionBehavior(QAbstractItemView::SelectRows);
	ui->keyfileTable->setSelectionMode(QAbstractItemView::SingleSelection);
	ui->keyfileTable->setFrameShape(QFrame::NoFrame);
	ui->keyfileTable->setFocusPolicy(Qt::NoFocus);
	ui->keyfileTable->setShowGrid(false);
	ui->keyfileTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
	ui->keyfileTable->setAlternatingRowColors(true);
	ui->keyfileTable->setSelectionMode(QAbstractItemView::NoSelection);
	ui->keyfileTable->setVisible(false);

	auto cb = std::bind(&MigrateDialog::DropFile, this, std::placeholders::_1);
	new DropFileFilter(this, cb);
}

MigrateDialog::~MigrateDialog()
{
    delete ui;
}

void MigrateDialog::on_addPathButton_clicked(void) 
{
	static QString s_path = GlobalConfig::GetSaveVideoPath();
	QString dir = QFileDialog::getExistingDirectory(this,
		tr("SelectDirectory"),
		s_path,
		QFileDialog::ShowDirsOnly |
		QFileDialog::DontResolveSymlinks);
	if (dir.isEmpty()) {
		return;
	}
	s_path = dir;
	DropFile(QList<QUrl>{QUrl(dir)});
}
void MigrateDialog::on_migrateButton_clicked(void)
{
	for (int i = 0; i < ui->keyfileTable->rowCount(); ++i) {
		if (QTStr("MigrateOk") != ui->keyfileTable->item(i, 1)->text()) {
			auto result = SdkManager::GetManager()->MigrateLocalVideoKeyFile(
				ui->keyfileTable->item(i, 0)->text(), SdkManager::GetManager()->GetAccount().secretKey);
			ui->keyfileTable->item(i, 1)->setText(result ? tr("MigrateOk") : tr("MigrateFail"));
		}
	}
	QMetaObject::invokeMethod((QWidget*)App()->GetMainWindow(), "OnCompleteRequestVideo", Qt::QueuedConnection);
}
void MigrateDialog::on_clearButton_clicked(void)
{
	int count = ui->keyfileTable->rowCount();
	while (0 != count) {
		ui->keyfileTable->removeRow(0);
		count = ui->keyfileTable->rowCount();
	}
	ui->keyfileTable->setVisible(ui->keyfileTable->rowCount() > 0);
}

bool MigrateDialog::CheckExist(const QString& keyFile)
{
	for (int i = 0; i < ui->keyfileTable->rowCount(); ++i) {
		if (keyFile == ui->keyfileTable->item(i, 0)->text()) {
			return true;
		}
	}
	return false;
}
const QString kExtensionFile = "key";
static bool CheckKeyFile(const QString& filePath)
{
	QFileInfo fileInfo(filePath);
	if (!fileInfo.exists()) {
		return false;
	}
	if (0 != fileInfo.suffix().compare(kExtensionFile, Qt::CaseInsensitive)) {
		return false;
	}
	auto vid = fileInfo.baseName();
	if (34 != vid.length()) {
		return false;
	}
	auto ret = vid.split("_");
	if (2 != ret.size()) {
		return false;
	}
	if (!App()->GetMainWindow()->IsExistLocalFile(ret.at(0))) {
		return false;
	}
	return true;
}



void MigrateDialog::DropFile(const QList<QUrl>& urls)
{
	QStringList files;
	for (const auto& url : urls) {
		auto filePath = url.isLocalFile() ? url.toLocalFile() : url.path();
		QFileInfo fileInfo(filePath);
		if (fileInfo.exists()) {
			if (fileInfo.isDir()) {
				GetAllFiles(filePath, files);
			}
			else if(CheckKeyFile(filePath)) {
				files.append(filePath);
			}
		}
	}
	for (auto& it : files) {
		if (CheckKeyFile(it) && !CheckExist(it)) {
			QString keyFile = it;
			int row = ui->keyfileTable->rowCount();
			ui->keyfileTable->insertRow(row);
			int col = 0;
			auto item = new QTableWidgetItem(it);
			item->setToolTip(it);
			item->setTextAlignment(Qt::AlignVCenter | Qt::AlignLeft);
			ui->keyfileTable->setItem(row, col++, item);			
			item = new QTableWidgetItem(tr("MigrateSource"));
			item->setTextAlignment(Qt::AlignCenter);
			if (QFileInfo::exists(it + ".sign")) {
				item->setText(tr("MigrateOk"));
			}
			ui->keyfileTable->setItem(row, col++, item);
			
			auto layout = new QVBoxLayout();
			layout->setContentsMargins(0, 0, 0, 0);
			auto widget = new QWidget(this);
			widget->setLayout(layout);	
			QPushButton* delButton = nullptr;
			delButton = new StatusButton(this, "delButton", QTStr("MigrateDelete"), QSize(24, 24));
			connect(delButton, &QPushButton::clicked, this, [this, keyFile] {
				DeleteItem(keyFile);
				});
			layout->addWidget(delButton, 0, Qt::AlignCenter);
			ui->keyfileTable->setCellWidget(row, col++, widget);
		}
	}
	ui->keyfileTable->setVisible(ui->keyfileTable->rowCount() > 0);
}
void MigrateDialog::DeleteItem(const QString& keyFile)
{
	for (int i = 0; i < ui->keyfileTable->rowCount(); ++i) {
		if (keyFile == ui->keyfileTable->item(i, 0)->text()) {
			ui->keyfileTable->removeRow(i);
			break;
		}
	}
	ui->keyfileTable->setVisible(ui->keyfileTable->rowCount() > 0);
}
void MigrateDialog::GetAllFiles(const QString& folder, QStringList& files)
{
	QDir folderDir(folder);
	//QStringList filters;
	//filters << "*." + kExtensionFile;
	//folderDir.setNameFilters(filters);
	QFileInfoList fileInfoList = folderDir.entryInfoList(QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot);
	foreach(QFileInfo fileInfo, fileInfoList) {		
		if (fileInfo.isDir()) {
			GetAllFiles(fileInfo.absoluteFilePath(), files);
		}
		else if (CheckKeyFile(fileInfo.absoluteFilePath())) {
			files.append(fileInfo.absoluteFilePath());
		}
		else {
		}
	}
}
