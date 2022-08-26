#pragma execution_character_set("utf-8")
#include "StatusBar.h"
#include <QPainter>
#include <QApplication>

StatusBar::StatusBar(QWidget* parent)
	: QWidget(parent)
{
	
}

StatusBar::~StatusBar()
{
}

void StatusBar::append(const QString& item)
{
	this->_strlist_tags << item;
	this->update();
}

void StatusBar::append(const QStringList& items)
{
	this->_strlist_tags << items;
	this->update();
}

bool StatusBar::set(int index, const QString& item)
{
	if (index < 0 || index >= this->_strlist_tags.count())
		return false;
	this->_strlist_tags[index] = item;
	this->update();
	return true;
}

void StatusBar::removeAt(int index)
{
	this->_strlist_tags.removeAt(index);
	this->update();
}

void StatusBar::clear()
{
	this->_strlist_tags.clear();
	this->update();
}

QStringList StatusBar::items() const
{
	return this->_strlist_tags;
}

QString StatusBar::itemAt(int index) const
{
	if (index < 0 || index >= this->_strlist_tags.count())
		return "";
	return this->_strlist_tags.at(index);
}

void StatusBar::paintEvent(QPaintEvent* e)
{
	QPainter painter(this);
	painter.setPen(QColor("#2c2c2c"));
	painter.setBrush(Qt::NoBrush);
	QFont font = qApp->font();
	font.setPixelSize(15);
	painter.setFont(font);
	QString tags = this->_strlist_tags.join(" ¦ò ");
	painter.drawText(this->rect().marginsAdded(QMargins(-15, 0, -15, 0)), Qt::AlignLeft | Qt::AlignVCenter, tags);
	return QWidget::paintEvent(e);
}