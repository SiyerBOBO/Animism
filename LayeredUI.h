#pragma once

#include <QWidget>
#include <QPointer>
#include <QResizeEvent>

class LayeredUI : public QWidget
{
	Q_OBJECT

public:
	LayeredUI(QWidget *parent);
	~LayeredUI();

	void setTopWidget(QWidget* widget);
	void removeTopWidget();
	QWidget* takeTopWidget();
	QWidget* topWidget();

	void setBottomWidget(QWidget* widget);
	void removeBottomWidget();
	QWidget* takeBottomWidget();
	QWidget* bottomWidget();

protected:
	void resizeEvent(QResizeEvent* event) override;
	void todo_layout();

private:
	QPointer<QWidget> _ptr_widget_top;
	QPointer<QWidget> _ptr_widget_bottom;
};
