#pragma once

#include <QDialog>
#include <QWidget>
#include <QLabel>
#include <QEvent>
#include <QPainter>
#include <QPainterPath>
#include <QPushButton>
#include <QMouseEvent>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSpacerItem>

class MessageDialog : public QDialog
{

	Q_OBJECT

public:
	MessageDialog(QWidget* parent, QString title, QString message, bool isSignalBtn = true);
	~MessageDialog();

protected:
	void initUi();
	bool event(QEvent* e) override;
	bool eventFilter(QObject* obj, QEvent* e)  override;

private:
	QString _str_title;
	QString _str_message;
	bool _bool_isSignalBtn;
	QVBoxLayout* _ptr_layout1;
	QHBoxLayout* _ptr_layout2;
	QSpacerItem* _ptr_spacerItem1;
	QSpacerItem* _ptr_spacerItem2;
	QLabel* _ptr_label;
	QPushButton* _ptr_button_accept;
	QPushButton* _ptr_button_reject;
};
