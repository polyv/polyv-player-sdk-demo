#include "drop-file-filter.h"

#include <QDebug>
#include <QMimeData>
#include <QDragEnterEvent>


DropFileFilter::DropFileFilter(QWidget *widget,
                                                 std::function<void(const QList<QUrl> &)> cb)
    : QObject(widget)
{
    widget->setAcceptDrops(true);
    widget->installEventFilter(this);
    callback = cb;
}

bool DropFileFilter::eventFilter(QObject *obj, QEvent *e)
{
    if (e->type() == QEvent::DragEnter) {
        auto event = static_cast<QDragEnterEvent *>(e);
        if (event->mimeData()->hasFormat("text/uri-list")) {
            event->acceptProposedAction();
            return true;
        }
    } else if (e->type() == QEvent::Drop) {
        auto event = static_cast<QDropEvent *>(e);
        if (event->mimeData()->hasUrls() && !event->mimeData()->urls().isEmpty()) {
            if (callback) {
                callback(event->mimeData()->urls());
            }   
        } else {
            qWarning() << "acceptProposedAction of DragEnterEvent, but mimeData hasUrls() is false"
                       << "or urls() or urls() is empty, other info:" << event
                       << event->mimeData()->text();
        }
        event->acceptProposedAction();
        return true;
    }
    return false;
}