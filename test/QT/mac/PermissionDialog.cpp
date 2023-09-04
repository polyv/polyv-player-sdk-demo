#include "PermissionDialog.h"
#include <QLabel>

#include "../Application.h"


PermissionDialog::PermissionDialog(QWidget *parent, MacPermissionStatus allFiles,
			       MacPermissionStatus accessibility)
	: QDialog(parent), ui(new Ui::PermissionDialog)
{
	ui->setupUi(this);
	SetStatus(ui->allFilesPermissionButton, allFiles,
		  QTStr("MacPermissions.Item.AllFiles"));
	SetStatus(ui->accessibilityPermissionButton, accessibility,
		  QTStr("MacPermissions.Item.Accessibility"));
}

void PermissionDialog::SetStatus(QPushButton *btn, MacPermissionStatus status,
			       const QString &preference)
{
	if (status == kPermissionAuthorized) {
		btn->setText(QTStr("MacPermissions.AccessGranted"));
	} else if (status == kPermissionNotDetermined) {
		btn->setText(QTStr("MacPermissions.RequestAccess"));
	} else {
		btn->setText(
			QTStr("MacPermissions.OpenPreferences").arg(preference));
	}
	btn->setEnabled(status != kPermissionAuthorized);
	btn->setProperty("status", status);
}


void PermissionDialog::on_allFilesPermissionButton_clicked()
{
	MacPermissionStatus status =
		(MacPermissionStatus)ui->allFilesPermissionButton
			->property("status")
			.toInt();
	if (status == kPermissionNotDetermined) {
		status = RequestPermission(kAllFilesAccess);
		SetStatus(ui->allFilesPermissionButton, status,
			  QTStr("MacPermissions.Item.allFiles"));
	} else {
		OpenMacOSPrivacyPreferences("AllFiles");
	}
}

void PermissionDialog::on_accessibilityPermissionButton_clicked()
{
	OpenMacOSPrivacyPreferences("Accessibility");
	//RequestPermission(kAccessibility);
}

void PermissionDialog::on_continueButton_clicked()
{
	App()->GlobalConfig().Set("General",
		       "MacOSPermissionsDialogLastShown",
		       MACOS_PERMISSIONS_DIALOG_VERSION);
	close();
}
