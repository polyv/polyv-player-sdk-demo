#include "VideoTableWidget.h"

#include <QStyle>
#include <QHeaderView>
#include <QApplication>
#include <QStackedLayout>

#include <QPainter>
#include <QAbstractTextDocumentLayout>
#include <QTextDocument>

//TableDelegete::TableDelegete(QObject *parent)
//	: QStyledItemDelegate(parent)
//{
//
//}
//
//void TableDelegete::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
//{
//	if (1 != index.column()) {
//		return QStyledItemDelegate::paint(painter, option, index);
//	}
//	QString text = index.data().toString();
//	
//
//
//	QStyleOptionViewItem viewOption(option);
//	initStyleOption(&viewOption, index);
//	viewOption.widget->style()->drawControl(QStyle::CE_ItemViewItem, &viewOption, painter, viewOption.widget);
//	QTextDocument doc;
//	doc.setHtml(text);
//	QAbstractTextDocumentLayout::PaintContext paintContext;
//	QRect textRect = viewOption.widget->style()->subElementRect(QStyle::SE_ItemViewItemText, &viewOption);
//	painter->save();
//	painter->translate(textRect.topLeft());
//	painter->setClipRect(textRect.translated(-textRect.topLeft()));
//	doc.documentLayout()->draw(painter, paintContext);
//	painter->restore();
//}

VideoTableWidget::VideoTableWidget(QWidget* parent)
	: QStackedWidget(parent)
{
	QStackedLayout * layout = (QStackedLayout*)this->layout();
	//QStackedLayout* layout = new QStackedLayout();
	layout->setSpacing(0);
	layout->setContentsMargins(0, 0, 0, 0);
	layout->setStackingMode(QStackedLayout::StackOne);
	//setLayout(layout);
}


void VideoTableWidget::InitTableWidget(QTableWidget* table)
{
	table->horizontalHeader()->setHighlightSections(false);
	table->horizontalHeader()->setFixedHeight(48);
	table->horizontalHeader()->setDefaultAlignment(Qt::AlignCenter);
	table->horizontalHeader()->setStretchLastSection(false);

	table->verticalHeader()->setDefaultSectionSize(80);
	table->verticalHeader()->setVisible(false);
	table->verticalHeader()->setHighlightSections(false);

	table->setSelectionBehavior(QAbstractItemView::SelectRows);
	table->setSelectionMode(QAbstractItemView::SingleSelection);
	table->setFrameShape(QFrame::NoFrame);
	table->setFocusPolicy(Qt::NoFocus);
	table->setShowGrid(false);
	table->setEditTriggers(QAbstractItemView::NoEditTriggers);
	table->setAlternatingRowColors(false);
	table->setSelectionMode(QAbstractItemView::NoSelection);

	QStackedLayout* layout = (QStackedLayout*)this->layout();
	int count = this->count();
	for (int i = 0; i < count; ++i) {
		layout->addWidget(this->widget(i));
	}
	SetTableEmpty(true);	
}

void VideoTableWidget::SetTableEmpty(bool empty)
{
	QStackedLayout* layout = (QStackedLayout*)this->layout();
	if (empty) {
		//layout->setStackingMode(QStackedLayout::StackOne);
		setCurrentIndex(0);
	}
	else {
		//layout->setStackingMode(QStackedLayout::StackOne);
		setCurrentIndex(1);
	}
}