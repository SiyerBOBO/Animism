#include "ToolTip.h"
#include <mutex>
#include <QDesktopWidget>
#include <QPainter>
#include <QPainterPath>
#include <QtMath>

std::unique_ptr<ToolTip> ToolTip::_spointer_instance;
ToolTip* ToolTip::Instance(const QString& title, const QString& text)
{
	static std::once_flag oc;
	std::call_once(oc, [&] {  _spointer_instance.reset(new ToolTip()); });

	_spointer_instance->ui.label_title->setText(title);
	_spointer_instance->ui.label_text->setText(text);
	_spointer_instance->update();

	return _spointer_instance.get();
}

ToolTip::ToolTip(QWidget *parent)
	: QWidget(parent)
{
	this->initUi();
}

void ToolTip::initUi()
{
	ui.setupUi(this);
	this->setAttribute(Qt::WA_TranslucentBackground);
	this->setAttribute(Qt::WA_TransparentForMouseEvents);
	this->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::Tool);
}

ToolTip::~ToolTip()
{
}

bool ToolTip::event(QEvent* e)
{
	static bool _b_windowActive = this->isActiveWindow();
	switch (e->type())
	{
	case QEvent::Paint: {
		int _int_winShadow = 15;
		int _int_winRadius = 10;
		QRect _rect_winRect = QRect(this->ui.widget_frame->pos(), this->ui.widget_frame->size());

		QPainter painter(this);
		painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);

		QPainterPath path;
		path.addRoundedRect(_rect_winRect, _int_winRadius, _int_winRadius);
		painter.fillPath(path, QColor(255, 255, 255));
		QColor color = Qt::black;
		for (int i = 0; i < _int_winShadow; i++)
		{
			_rect_winRect = _rect_winRect.marginsAdded(QMargins(1, 1, 1, 1));
			double _double_alpha = _b_windowActive ? 30 - 11 * qSqrt(i) : 20 - 8 * qSqrt(i);
			path.clear();
			path.setFillRule(Qt::WindingFill);
			path.addRoundedRect(_rect_winRect, _int_winRadius, _int_winRadius++);
			color.setAlpha(_double_alpha > 0 ? _double_alpha : 0);
			painter.setPen(color);
			painter.drawPath(path);
		}
		break;
	}
	case QEvent::Show:
	{
		QPoint _ptr_cursor = QCursor::pos();
		if (_ptr_cursor.x() + this->width() > QApplication::desktop()->availableGeometry().width())
			_ptr_cursor = QPoint(QApplication::desktop()->availableGeometry().width() - this->width(), _ptr_cursor.y());
		if (_ptr_cursor.y() + this->height() > QApplication::desktop()->availableGeometry().height())
			_ptr_cursor = QPoint(_ptr_cursor.x(), QApplication::desktop()->availableGeometry().height() - this->height());
		if (_ptr_cursor.x() < 0)
			_ptr_cursor = QPoint(0, _ptr_cursor.y());
		if (_ptr_cursor.y() < 0)
			_ptr_cursor = QPoint(_ptr_cursor.x(), 0);
		this->move(_ptr_cursor);
		break;
	}
	case QEvent::WindowActivate:
	{
		_b_windowActive = true;
		this->update();
		break;
	}
	case QEvent::WindowDeactivate:
	{
		_b_windowActive = false;
		this->update();
		break;
	}
	default:
		break;
	}
	return QWidget::event(e);
}