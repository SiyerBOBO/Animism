#include "ScatterDiagram.h"
#include <QPainter>
#include <QEvent>
#include <QMouseEvent>
#include <QtMath>
#include <QDebug>

ScatterDiagram::ScatterDiagram(QWidget* parent)
	: QWidget(parent),
	_range_min(0), _range_max(100),
	_color_border(QColor(160, 160, 160)),
	_color_background(Qt::transparent),
	_color_foreground(QColor("#886ab5")),
	_color_highlight(Qt::red)
{
	this->initUi();
}

ScatterDiagram::~ScatterDiagram()
{
}

void ScatterDiagram::setRange(qint64 min, qint64 max)
{
	this->_range_min = qMin(min, max);
	this->_range_max = qMax(min, max);
	this->updateData();
}

QPair<qint64, qint64> ScatterDiagram::range() const
{
	return qMakePair<qint64, qint64>(this->_range_min, _range_max);
}

void ScatterDiagram::insert(const QList<qint64>& data)
{
	this->_src_data += data.toSet();
	this->updateData();
}

void ScatterDiagram::remove(qint64 data)
{
	this->_src_data.remove(data);
	this->updateData();
}

void ScatterDiagram::clear()
{
	this->_src_data.clear();
	this->updateData();
}

void ScatterDiagram::initUi()
{
	this->setAttribute(Qt::WA_Hover);
}

void ScatterDiagram::updateData()
{
	this->_pre_data = QPixmap(this->_range_max - this->_range_min, 1);
	QPainter painter(&this->_pre_data);
	painter.setPen(Qt::NoPen);
	painter.setBrush(Qt::white);
	painter.drawRect(this->_pre_data.rect());
	painter.setPen(this->_color_foreground);
	painter.setBrush(Qt::NoBrush);
	for (auto it = this->_src_data.begin(); it != this->_src_data.end(); it++)
	{
		qint64 _value = *it - this->_range_min;
		painter.drawLine(_value, 0, _value, 1);
	}

	this->update();
}

bool ScatterDiagram::event(QEvent* e)
{
	static bool _is_mouse_pressed = false;
	static int _pos_mouse_px = -1;
	static int _pos_mouse_rx = -1;
	switch (e->type())
	{
	case QEvent::MouseButtonPress:
	{
		QMouseEvent* _mevent = (QMouseEvent*)e;
		if (Qt::MouseButton::LeftButton == _mevent->button())
		{
			_is_mouse_pressed = true;
			_pos_mouse_px = this->mapFromGlobal(QCursor::pos()).x();
			_pos_mouse_rx = _pos_mouse_px;
			this->update();
			emit this->drag(
				(_pos_mouse_px < _pos_mouse_rx ? _pos_mouse_px : _pos_mouse_rx) * 100.0 / this->width(),
				(_pos_mouse_px > _pos_mouse_rx ? _pos_mouse_px : _pos_mouse_rx) * 100.0 / this->width()
			);
		}
		break;
	}
	case QEvent::MouseButtonRelease:
	{
		QMouseEvent* _mevent = (QMouseEvent*)e;
		if (Qt::MouseButton::LeftButton == _mevent->button())
		{
			_is_mouse_pressed = false;
			this->update();
			emit this->drop(
				(_pos_mouse_px < _pos_mouse_rx ? _pos_mouse_px : _pos_mouse_rx) * 100.0 / this->width(),
				(_pos_mouse_px > _pos_mouse_rx ? _pos_mouse_px : _pos_mouse_rx) * 100.0 / this->width()
			);

			_pos_mouse_px = -1;
			_pos_mouse_rx = -1;
		}
		break;
	}
	case QEvent::HoverMove:
	{
		if (_is_mouse_pressed)
		{
			_pos_mouse_rx = this->mapFromGlobal(QCursor::pos()).x();
			_pos_mouse_rx = _pos_mouse_rx < 0 ? 0 : _pos_mouse_rx;
			_pos_mouse_rx = _pos_mouse_rx > this->width() ? this->width() : _pos_mouse_rx;

			emit this->drag(
				(_pos_mouse_px < _pos_mouse_rx ? _pos_mouse_px : _pos_mouse_rx) * 100.0 / this->width(),
				(_pos_mouse_px > _pos_mouse_rx ? _pos_mouse_px : _pos_mouse_rx) * 100.0 / this->width()
			);
		}
		this->update();
		break;
	}
	case QEvent::Paint:
	{
		QPainter painter(this);
		painter.setPen(Qt::NoPen);
		painter.setBrush(this->_color_background);
		painter.drawRect(this->rect());

		auto _width = this->width();
		auto _height = this->height();
		//auto _range = this->_range_max - this->_range_min;
		//auto _subwidth = _width * 1.0 / _range;

		//painter.setPen(QPen(this->_color_foreground, qFloor(_subwidth)));
		//painter.setBrush(Qt::NoBrush);
		//for (auto it = this->_src_data.begin(); it != this->_src_data.end(); it++)
		//{
		//	auto _value = qFloor((*it - this->_range_min) * _subwidth);
		//	painter.drawLine(_value, 0, _value, _height);
		//}

		if (!this->_pre_data.isNull())
		{
			painter.setPen(Qt::NoPen);
			painter.setBrush(Qt::NoBrush);
			painter.drawPixmap(this->rect(), this->_pre_data.scaled(_width, _height));
		}

		if (_is_mouse_pressed)
		{
			QColor _color_area = this->_color_highlight;
			_color_area.setAlpha(100);
			painter.setPen(Qt::NoPen);
			painter.setBrush(_color_area);
			painter.drawRect(_pos_mouse_px, 0, _pos_mouse_rx - _pos_mouse_px, _height);

			painter.setPen(QPen(this->_color_highlight, 3));
			painter.setBrush(Qt::NoBrush);
			painter.drawLine(_pos_mouse_px, 0, _pos_mouse_px, _height);
			painter.drawLine(_pos_mouse_rx, 0, _pos_mouse_rx, _height);
		}
		else if (this->underMouse())
		{
			painter.setPen(this->_color_highlight);
			painter.setBrush(Qt::NoBrush);
			qint64 _cursor_x = this->mapFromGlobal(QCursor::pos()).x();
			painter.drawLine(_cursor_x, 0, _cursor_x, _height);
		}

		painter.setPen(this->_color_border);
		painter.setBrush(Qt::NoBrush);
		painter.drawRect(this->rect().marginsRemoved(QMargins(0, 0, 1, 1)));
		break;
	}
	default:
		break;
	}
	return QWidget::event(e);
}
