#include "path-edit.h"
#include <QFileDialog>
#include <QHBoxLayout>
#include <QWidgetAction>

#include "StatusButton.h"

PathEdit::PathEdit(QWidget *parent)
    : QLineEdit(parent)
{
    setReadOnly(true);

    auto button = new StatusButton(this, "openDir", tr("ChangeDirectory"), QSize(16, 16));

    QWidgetAction *action = new QWidgetAction(this);
    action->setDefaultWidget(button);
    addAction(action, QLineEdit::TrailingPosition);

    connect(button, &QPushButton::clicked, this, [this] {
        QString dir = QFileDialog::getExistingDirectory(this,
                                                        tr("SelectDirectory"),
                                                        this->text(),
                                                        QFileDialog::ShowDirsOnly |
                                                            QFileDialog::DontResolveSymlinks);
        if (dir.isEmpty()) {
            return;
        }
        this->setText(dir);
    });
}

