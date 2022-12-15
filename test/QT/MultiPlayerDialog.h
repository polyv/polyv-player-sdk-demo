#pragma once
#include <QMap>
#include <QDialog>
#include "VideoControl.h"

namespace Ui {
class MultiPlayerDialog;
}

class MultiPlayerDialog : public QDialog
{
    Q_OBJECT

public:
    explicit MultiPlayerDialog(QWidget *parent = 0);
    ~MultiPlayerDialog();

    static void Open(const SharedVideoPtr& video);
    static void CloseAll();
public:
    QString GetVid() const {
        return videoId;
    }
    void LoadLocal(const SharedVideoPtr& video);
protected:
    virtual void closeEvent(QCloseEvent* e) override;
    bool eventFilter(QObject*, QEvent*) override;
private slots:
    void OnChangeRatePlayVideo(int rate, int seekMillisecond, const SharedVideoPtr& video);
    //void OnOpenParamWindow(QString vid);
    void OnFullScreen(void);
    void OnExitFullScreen(void);
    void OnShowTips(int level, const QString& msg);
private:
    Ui::MultiPlayerDialog *ui;
    QString videoId;
    //QPointer<ParamDialog> paramDialog;
};

