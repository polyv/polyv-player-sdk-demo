#pragma once

#include "ui_PermissionDialog.h"
#include "platform.h"

#define MACOS_PERMISSIONS_DIALOG_VERSION 2

class PermissionDialog : public QDialog {
	Q_OBJECT

private:
	std::unique_ptr<Ui::PermissionDialog> ui;
	void SetStatus(QPushButton *btn, MacPermissionStatus status,
		       const QString &preference);

public:
	PermissionDialog(QWidget *parent, MacPermissionStatus allFiles,
		       MacPermissionStatus accessibility);

private slots:
	void on_allFilesPermissionButton_clicked();
	void on_accessibilityPermissionButton_clicked();
	void on_continueButton_clicked();
};
