#include "WinButton.h"
#include <QPainter>
#include <QPaintEvent>

WinButton::WinButton(QWidget *parent)
	: QPushButton(parent)
{
}

WinButton::~WinButton()
{
}

void WinButton::setPixmap(const QPixmap& pixmap)
{
	this->_pixmap_normal = pixmap;
	this->update();
}

void WinButton::paintEvent(QPaintEvent* e)
{
	QPainter painter(this);
	painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
	painter.setPen(Qt::NoPen);
	if (this->isDown())
		painter.setBrush(QColor(226, 232, 240));
	else if (this->underMouse())
		painter.setBrush(QColor(237, 242, 247));
	else
		painter.setBrush(Qt::NoBrush);
	painter.drawRoundedRect(this->rect(), 5, 5);

	painter.drawPixmap(QRect(10, 10, 16, 16), this->_pixmap_normal);
	return;
}
