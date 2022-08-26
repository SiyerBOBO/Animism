#include "LayeredUI.h"

LayeredUI::LayeredUI(QWidget *parent)
	: QWidget(parent)
{
}

LayeredUI::~LayeredUI()
{
}

void LayeredUI::setTopWidget(QWidget* widget)
{
	this->_ptr_widget_top = widget;
	this->todo_layout();
}

void LayeredUI::removeTopWidget()
{
	if (this->_ptr_widget_top.isNull())
		return;
	this->_ptr_widget_top.data()->deleteLater();
	this->todo_layout();
}

QWidget* LayeredUI::takeTopWidget()
{
	auto widget = this->_ptr_widget_top.data();
	this->_ptr_widget_top = nullptr;
	this->todo_layout();
	return widget;
}

QWidget* LayeredUI::topWidget()
{
	return this->_ptr_widget_top.data();
}

void LayeredUI::setBottomWidget(QWidget* widget)
{
	this->_ptr_widget_bottom = widget;
	this->todo_layout();
}

void LayeredUI::removeBottomWidget()
{
	if (this->_ptr_widget_bottom.isNull())
		return;
	this->_ptr_widget_bottom.data()->deleteLater();
	this->todo_layout();
}

QWidget* LayeredUI::takeBottomWidget()
{
	auto widget = this->_ptr_widget_bottom.data();
	this->_ptr_widget_bottom = nullptr;
	this->todo_layout();
	return widget;
}

QWidget* LayeredUI::bottomWidget()
{
	return this->_ptr_widget_bottom.data();
}

void LayeredUI::resizeEvent(QResizeEvent* event)
{
	this->todo_layout();
	return QWidget::resizeEvent(event);
}

void LayeredUI::todo_layout()
{
	if (!this->_ptr_widget_bottom.isNull())
	{
		auto widget = this->_ptr_widget_bottom.data();
		widget->setGeometry(0, 0, this->width(), this->height());
	}
	if (!this->_ptr_widget_top.isNull())
	{
		auto widget = this->_ptr_widget_top.data();
		widget->setGeometry(0, 0, this->width(), this->height());
		widget->raise();
	}
}