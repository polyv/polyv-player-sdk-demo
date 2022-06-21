#include "WindowHelper.h"

#include <QHash>
#include <QEvent>
#include <QDebug>
#include <QMouseEvent>
#include <QRubberBand>

class NcWidgetData;
///////////////////////////////////////////////////////
class WindowHelperPrivate
{
public:
	QHash< QWidget*, NcWidgetData* > m_HashWidgetData;
	uchar m_usWidgetMovable : 1;
	uchar m_usWidgetResizable : 1;
	uchar m_usUseRubberBandOnResize : 1;
	uchar m_usUseRubberBandOnMove : 1;
};


class NcCursorPosCalculator
{
public:
	NcCursorPosCalculator(void);
public:
	void Reset(void);
	void Recalculate(const QPoint& globalMousePos, const QRect& frameRect);
public:
	// unsign short 2 byte, 16bit, used 9, remain 7.
	ushort onEdges : 1;
	ushort onLeftEdge : 1;
	ushort onRightEdge : 1;
	ushort onTopEdge : 1;
	ushort onBottomEdge : 1;
	ushort onTopLeftEdge : 1;
	ushort onBottomLeftEdge : 1;
	ushort onTopRightEdge : 1;
	ushort onBottomRightEdge : 1;

	static int s_borderWidth;
};

int NcCursorPosCalculator::s_borderWidth = 3;

class NcWidgetData
{
public:
	NcWidgetData(WindowHelperPrivate* d, QWidget* pTopLevelWidget);
	~NcWidgetData(void);

public:
	const QWidget* Widget() const;
	void HandleWidgetEvent(QEvent* event);
	void UpdateRubberBandStatus(void);
private:
	void UpdateCursorShape(const QPoint& gMousePos);
	void ResizeWidget(const QPoint& gMousePos);
	void MoveWidget(const QPoint& gMousePos);
	void HandleMousePressEvent(QMouseEvent* event);

	void HandleMouseReleaseEvent(QMouseEvent* event);
	void HandleMouseMoveEvent(QMouseEvent* event);
	void HandleLeaveEvent(QEvent* event);
	void HandleHoverMoveEvent(QHoverEvent* event);

private:
	WindowHelperPrivate     * d_ptr;
	QRubberBand * m_pRubberBand;
	QWidget *m_pWidget;
	QPoint m_ptDragPos;
	NcCursorPosCalculator m_PressedMousePos;
	NcCursorPosCalculator m_MoveMousePos;
	uchar m_bLeftButtonPressed : 1;
	uchar m_bCursorShapeChanged : 1;
	Qt::WindowFlags m_eWindowFlags;
};
///--------------------------------------------------///

NcCursorPosCalculator::NcCursorPosCalculator(void)
{
	Reset();
}

void NcCursorPosCalculator::Reset(void)
{
	onEdges = 0;
	onLeftEdge = 0;
	onRightEdge = 0;
	onTopEdge = 0;
	onBottomEdge = 0;
	onTopLeftEdge = 0;
	onBottomLeftEdge = 0;
	onTopRightEdge = 0;
	onBottomRightEdge = 0;
}

void NcCursorPosCalculator::Recalculate(const QPoint& gMousePos, const QRect& frameRect)
{
	const int & globalMouseX = gMousePos.x();
	const int & globalMouseY = gMousePos.y();

	const int & frameX = frameRect.x();
	const int & frameY = frameRect.y();

	const int & frameWidth = frameRect.width();
	const int & frameHeight = frameRect.height();

	onLeftEdge = (globalMouseX >= frameX &&
		globalMouseX <= frameX + s_borderWidth);

	//qDebug() << "11111111:" << onLeftEdge;

	onRightEdge = (globalMouseX >= frameX + frameWidth - s_borderWidth &&
		globalMouseX <= frameX + frameWidth);

	onTopEdge = (globalMouseY >= frameY &&
		globalMouseY <= frameY + s_borderWidth);


	onBottomEdge = (globalMouseY >= frameY + frameHeight - s_borderWidth &&
		globalMouseY <= frameY + frameHeight);

	onTopLeftEdge = onTopEdge && onLeftEdge;
	onBottomLeftEdge = onBottomEdge && onLeftEdge;
	onTopRightEdge = onTopEdge && onRightEdge;
	onBottomRightEdge = onBottomEdge && onRightEdge;

	onEdges = onLeftEdge || onRightEdge || onTopEdge || onBottomEdge;
}

///--------------------------------------------------///
NcWidgetData::NcWidgetData(WindowHelperPrivate * d, QWidget *pTopLevelWidget)
	:d_ptr(d)
	, m_pWidget(pTopLevelWidget)
	, m_pRubberBand(nullptr)
	, m_bLeftButtonPressed(false)
	, m_bCursorShapeChanged(false) {

	m_eWindowFlags = m_pWidget->windowFlags();

	m_pWidget->setMouseTracking(true);

	m_pWidget->setAttribute(Qt::WA_Hover);

	UpdateRubberBandStatus();
}

NcWidgetData::~NcWidgetData(void) 
{

	bool visible = m_pWidget->isVisible();

	m_pWidget->setMouseTracking(false);
	m_pWidget->setWindowFlags(m_eWindowFlags);
	m_pWidget->setAttribute(Qt::WA_Hover, false);
	m_pWidget->setVisible(visible);
	delete m_pRubberBand;
}

const QWidget* NcWidgetData::Widget() const 
{
	return m_pWidget;
}

void NcWidgetData::HandleWidgetEvent(QEvent* event)
{
	switch (event->type())
	{
	default:
		break;
	case QEvent::MouseButtonPress:
		HandleMousePressEvent(static_cast<QMouseEvent*>(event));
		break;

	case QEvent::MouseButtonRelease:
		HandleMouseReleaseEvent(static_cast<QMouseEvent*>(event));
		break;

	case QEvent::MouseMove:
		HandleMouseMoveEvent(static_cast<QMouseEvent*>(event));
		break;

	case QEvent::Leave:
		HandleLeaveEvent(event);
		break;

	case QEvent::HoverMove:
		HandleHoverMoveEvent(static_cast<QHoverEvent*>(event));
		break;
	}
}

void NcWidgetData::UpdateRubberBandStatus(void) 
{
	if (d_ptr->m_usUseRubberBandOnMove || d_ptr->m_usUseRubberBandOnResize) {
		if (nullptr == m_pRubberBand)
			m_pRubberBand = new QRubberBand(QRubberBand::Rectangle);
	}
	else {
		delete m_pRubberBand;
		m_pRubberBand = nullptr;
	}
}

void NcWidgetData::UpdateCursorShape(const QPoint& gMousePos) 
{
	if (m_pWidget->isFullScreen() || m_pWidget->isMaximized())
	{
		if (m_bCursorShapeChanged) {
			m_pWidget->unsetCursor();
		}
		return;
	}

	m_MoveMousePos.Recalculate(gMousePos, m_pWidget->frameGeometry());

	if (m_MoveMousePos.onTopLeftEdge || m_MoveMousePos.onBottomRightEdge)
	{
		m_pWidget->setCursor(Qt::SizeFDiagCursor);
		m_bCursorShapeChanged = true;
	}
	else if (m_MoveMousePos.onTopRightEdge || m_MoveMousePos.onBottomLeftEdge)
	{
		m_pWidget->setCursor(Qt::SizeBDiagCursor);
		m_bCursorShapeChanged = true;
	}
	else if (m_MoveMousePos.onLeftEdge || m_MoveMousePos.onRightEdge)
	{
		m_pWidget->setCursor(Qt::SizeHorCursor);
		m_bCursorShapeChanged = true;
	}
	else if (m_MoveMousePos.onTopEdge || m_MoveMousePos.onBottomEdge)
	{
		m_pWidget->setCursor(Qt::SizeVerCursor);
		m_bCursorShapeChanged = true;
	}
	else
	{
		if (m_bCursorShapeChanged) {
			m_pWidget->unsetCursor();
			m_bCursorShapeChanged = false;
		}
	}
}

void NcWidgetData::ResizeWidget(const QPoint& gMousePos) 
{
	QRect origRect = (d_ptr->m_usUseRubberBandOnResize ? m_pRubberBand->geometry() : m_pWidget->geometry());

	int left = origRect.left();
	int top = origRect.top();
	int right = origRect.right();
	int bottom = origRect.bottom();
	origRect.getCoords(&left, &top, &right, &bottom);

	//qDebug() << "00000000000000:" << origRect << ",top:" << top <<",bottom:" << bottom;

	const int & minWidth = m_pWidget->minimumWidth();
	const int & minHeight = m_pWidget->minimumHeight();

	if (m_PressedMousePos.onTopLeftEdge) {
		left = gMousePos.x();
		top = gMousePos.y();
	}
	else if (m_PressedMousePos.onBottomLeftEdge) {
		left = gMousePos.x();
		bottom = gMousePos.y();
	}
	else if (m_PressedMousePos.onTopRightEdge) {
		right = gMousePos.x();
		top = gMousePos.y();
	}
	else if (m_PressedMousePos.onBottomRightEdge) {
		right = gMousePos.x();
		bottom = gMousePos.y();
	}
	else if (m_PressedMousePos.onLeftEdge) {
		left = gMousePos.x();
	}
	else if (m_PressedMousePos.onRightEdge) {
		right = gMousePos.x();
	}
	else if (m_PressedMousePos.onTopEdge) {
		top = gMousePos.y();
		//qDebug() << "1111111111111111 :" << top;
	}
	else if (m_PressedMousePos.onBottomEdge) {
		bottom = gMousePos.y();
	}

	QRect newRect(QPoint(left, top), QPoint(right, bottom));
	//qDebug() << "height:" << bottom - top << ":bottom:" << bottom;
	if (newRect.isValid()) {
		if (minWidth >= newRect.width()) {
			newRect.setWidth(minWidth);
			//if (left != origRect.left())
			//	newRect.setLeft(origRect.left());
			//else
			//	newRect.setRight(origRect.right());
		}
		if (minHeight >= newRect.height()) {
			newRect.setHeight(minHeight);
			//if (top != origRect.top()) {
			//	newRect.setTop(origRect.top());
			//}	
			//else {
			//	newRect.setBottom(origRect.bottom());
			//}		
		}
		//qDebug() << "111111:" << newRect;
		d_ptr->m_usUseRubberBandOnResize ? m_pRubberBand->setGeometry(newRect) : m_pWidget->setGeometry(newRect);
	}
}

void NcWidgetData::MoveWidget(const QPoint& gMousePos) 
{
	if (d_ptr->m_usUseRubberBandOnMove) {
		m_pRubberBand->move(gMousePos - m_ptDragPos);
	}
	else {
		m_pWidget->move(gMousePos - m_ptDragPos);
	}
}

void NcWidgetData::HandleMousePressEvent(QMouseEvent* event)
{
	if (event->button() == Qt::LeftButton) {
		m_bLeftButtonPressed = true;

		QRect frameRect = m_pWidget->frameGeometry();
		m_PressedMousePos.Recalculate(event->globalPos(), frameRect);

		m_ptDragPos = event->globalPos() - frameRect.topLeft();

		if (m_PressedMousePos.onEdges) {
			if (d_ptr->m_usUseRubberBandOnResize) {
				m_pRubberBand->setGeometry(frameRect);
				m_pRubberBand->show();
			}
		}
		else if (d_ptr->m_usUseRubberBandOnMove) {
			m_pRubberBand->setGeometry(frameRect);
			m_pRubberBand->show();
		}
	}
}

void NcWidgetData::HandleMouseReleaseEvent(QMouseEvent* event) 
{
	if (event->button() == Qt::LeftButton) {
		m_bLeftButtonPressed = false;
		m_PressedMousePos.Reset();
		if (m_pRubberBand && m_pRubberBand->isVisible()) {
			m_pRubberBand->hide();
			m_pWidget->setGeometry(m_pRubberBand->geometry());
		}
	}
}

void NcWidgetData::HandleMouseMoveEvent(QMouseEvent* event) 
{
	if (m_bLeftButtonPressed) {
		//qDebug() << "3333333333333";
		if (d_ptr->m_usWidgetResizable && m_PressedMousePos.onEdges) {
			//qDebug() << "44444444444444 :" << event;
			ResizeWidget(event->globalPos());
		}
		else if (d_ptr->m_usWidgetMovable) {
			MoveWidget(event->globalPos());
		}
	}
	else if (d_ptr->m_usWidgetResizable) {
		UpdateCursorShape(event->globalPos());
	}
}

void NcWidgetData::HandleLeaveEvent(QEvent* event) 
{
	Q_UNUSED(event)
	if (!m_bLeftButtonPressed) {
		m_pWidget->unsetCursor();
	}
}

void NcWidgetData::HandleHoverMoveEvent(QHoverEvent* event)
{
	if (d_ptr->m_usWidgetResizable) {
		UpdateCursorShape(m_pWidget->mapToGlobal(event->pos()));
	}
}




////////////////////////////////////////////////////////////////
WindowHelper::WindowHelper(QObject *parent)
	:QObject(parent)
	, d_ptr(new WindowHelperPrivate()) 
{
	d_ptr->m_usWidgetMovable = 1;
	d_ptr->m_usWidgetResizable = 1;
	d_ptr->m_usUseRubberBandOnResize = 0;
	d_ptr->m_usUseRubberBandOnMove = 0;
}

WindowHelper::~WindowHelper(void) 
{
	QList<QWidget*> keys = d_ptr->m_HashWidgetData.keys();
	const int& size = keys.size();
	for (int i = 0; i < size; ++i) {
		delete d_ptr->m_HashWidgetData.take(keys[i]);
	}
	delete d_ptr;
}

bool WindowHelper::eventFilter(QObject* obj, QEvent* event)
{
	QEvent::Type type = event->type();
	if (type == QEvent::MouseMove ||
		type == QEvent::HoverMove ||
		type == QEvent::MouseButtonPress ||
		type == QEvent::MouseButtonRelease ||
		type == QEvent::Leave
		) {
		NcWidgetData* data = d_ptr->m_HashWidgetData.value(static_cast<QWidget*>(obj));
		if (data) {
			data->HandleWidgetEvent(event);
			//qDebug() << event;
		}
	}

	return false;
}

void WindowHelper::ActivateOn(QWidget* pTopLevelWidget)
{
	if (d_ptr->m_HashWidgetData.contains(pTopLevelWidget)) {
		return;
	}

	NcWidgetData* data = new NcWidgetData(d_ptr, pTopLevelWidget);
	d_ptr->m_HashWidgetData.insert(pTopLevelWidget, data);

	pTopLevelWidget->installEventFilter(this);
}

void WindowHelper::RemoveFrom(QWidget* pTopLevelWidget) 
{
	NcWidgetData* data = d_ptr->m_HashWidgetData.take(pTopLevelWidget);
	if (data) {
		pTopLevelWidget->removeEventFilter(this);
		delete data;
	}
}

void WindowHelper::SetWidgetMovable(bool value) 
{
	d_ptr->m_usWidgetMovable = value;
}

bool WindowHelper::IsWidgetMovable(void) const
{
	return d_ptr->m_usWidgetMovable;
}

void WindowHelper::SetWidgetResizable(bool value)
{
	d_ptr->m_usWidgetResizable = value;
}

bool WindowHelper::IsWidgetResizable(void) const
{
	return d_ptr->m_usWidgetResizable;
}

void WindowHelper::SetRubberBandOnMove(bool value)
{
	d_ptr->m_usUseRubberBandOnMove = value;
	QList<NcWidgetData*> list = d_ptr->m_HashWidgetData.values();
	int size = list.size();
	for (int i = 0; i < size; ++i) {
		list[i]->UpdateRubberBandStatus();
	}
}

bool WindowHelper::IsRubberBandOnMove(void) const
{
	return d_ptr->m_usUseRubberBandOnMove;
}

void WindowHelper::SetRubberBandOnResize(bool value) 
{
	d_ptr->m_usUseRubberBandOnResize = value;
	QList<NcWidgetData*> list = d_ptr->m_HashWidgetData.values();
	const int & size = list.size();
	for (int i = 0; i < size; ++i) {
		list[i]->UpdateRubberBandStatus();
	}
}

bool WindowHelper::IsRubberBandOnResize(void) const
{
	return d_ptr->m_usUseRubberBandOnResize;
}

void WindowHelper::SetBorderWidth(int value) 
{
	if (value > 0) {
		NcCursorPosCalculator::s_borderWidth = value;
	}
}

int WindowHelper::BorderWidth(void) const
{
	return NcCursorPosCalculator::s_borderWidth;
}


