#pragma once
#include <memory>
#include <functional>
#include <QDialog>
#include <QPointer>
namespace Ui {
	class MsgBoxDialog;
}

class MsgBoxDialog : public QDialog {
	Q_OBJECT
public:
	typedef enum {
		MSGBOX_RESULT_CLOSE = -1,
		MSGBOX_RESULT_OK = 1,
		MSGBOX_RESULT_CANCEL = 0,
	}MsgBoxResult;// result 

	typedef enum {
		MSGBOX_OK,
		MSGBOX_CANCEL,
		MSGBOX_CONTENT,
		MSGBOX_DIALOG,
	}MsgBoxControl;// control type QWidget

	using ModifyControl = std::function<void(MsgBoxControl type, QWidget* control)>;
	//general 
	MsgBoxDialog(QWidget *parent, const QString& text, 
		const QString& title, const QString& ok, const QString& cancel, ModifyControl modify, quint32 seconds);
	~MsgBoxDialog();

	// return MsgBoxResult
	static MsgBoxResult MsgBox(QWidget *parent, const QString& text, const QString& okText, const QString& cancelText,
		const QString& title, ModifyControl modify, quint32 seconds = 0);
	static MsgBoxResult ConfirmOK(QWidget *parent, const QString& text, const QString& okText, const QString& cancelText,
		const QString& title, ModifyControl modify);
	static MsgBoxResult ConfirmCancel(QWidget *parent, const QString& text, const QString& okText, const QString& cancelText,
		const QString& title, ModifyControl modify);
	static MsgBoxResult ConfirmNotice(QWidget *parent, const QString& text, const QString& okText, ModifyControl modify);
private:
	std::unique_ptr<Ui::MsgBoxDialog> ui;
};