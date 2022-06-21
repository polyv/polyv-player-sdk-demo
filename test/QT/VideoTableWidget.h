#pragma once
#include <QStackedWidget>
#include <QTableWidget>
#include <QStyledItemDelegate>


//class TableDelegete :public QStyledItemDelegate {
//	Q_OBJECT;
//public:
//	explicit TableDelegete(QObject *parent = nullptr);
//
//	virtual void paint(QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index) const;
//};

class VideoTableWidget : public QStackedWidget {
	Q_OBJECT
public:
	VideoTableWidget(QWidget *parent = 0);

public:
	void InitTableWidget(QTableWidget* table);
	void SetTableEmpty(bool empty);
};
