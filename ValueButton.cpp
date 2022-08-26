#include "ValueButton.h"
#include <QPaintEvent>
#include <QPainter>
#include <QPixmap>
#include <QFontMetrics>
#include <QApplication>

ValueButton::ValueButton(QWidget *parent)
	: QPushButton(parent)
{
	this->initUi();
}

ValueButton::~ValueButton()
{
}

void ValueButton::setValue(const QString& value)
{
	this->_value = value;
}

QString ValueButton::value() const
{
	return this->_value;
}

void ValueButton::initUi()
{
	this->setMinimumHeight(60);
	this->setMaximumHeight(60);
	this->setCursor(Qt::CursorShape::PointingHandCursor);
}

void ValueButton::paintEvent(QPaintEvent* e)
{
	QPainter painter(this);
	painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform | QPainter::TextAntialiasing);

	// draw background
	painter.setPen(QColor(210, 210, 210));
	if (this->isDown() || (this->isCheckable() && this->isChecked()))
		painter.setBrush(QColor(225, 230, 235));
	else if (this->underMouse())
		painter.setBrush(QColor(230, 235, 240));
	else
		painter.setBrush(QColor(225, 230, 235));
	painter.drawRoundedRect(this->rect(), 10, 10);

	// draw icon
	painter.setPen(Qt::NoPen);
	painter.setBrush(Qt::NoBrush);
	painter.drawPixmap(10, 10, 35, 35, this->icon().pixmap(35, 35));

	// prepare info
	QFont font = qApp->font();
	int _info_width = this->width() - 10 * 2 - 10 - 35;

	// draw text
	painter.setPen(Qt::black);
	painter.setBrush(Qt::NoBrush);
	font.setPixelSize(16);
	painter.setFont(font);
	QString _text = QFontMetrics(font).elidedText(this->text(), Qt::ElideRight, _info_width);
	painter.drawText(10 + 35 + 10, 10, _info_width, 35, Qt::AlignLeft | Qt::AlignVCenter, _text);

	// draw data
	painter.setPen(Qt::NoPen);
	painter.setBrush(Qt::darkGray);
	painter.drawRoundedRect(10, 45, this->width() - 10 * 2, 20, 10, 10);

	painter.setPen(Qt::white);
	painter.setBrush(Qt::NoBrush);
	font.setPixelSize(13);
	painter.setFont(font);
	QString _data = QFontMetrics(font).elidedText(_value, Qt::ElideRight, _info_width);
	painter.drawText(10, 45, this->width() - 10 * 2, 20, Qt::AlignCenter, _data);
}
