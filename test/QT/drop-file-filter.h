#pragma once
#include <QWidget>

#include <functional>
class DropFileFilter : public QObject
{
    Q_OBJECT
public:
    explicit DropFileFilter(QWidget *widget, 
        std::function<void(const QList<QUrl> &)> cb);

protected:
    bool eventFilter(QObject *obj, QEvent *e) override;

private:
    std::function<void(const QList<QUrl> &)> callback;
};
