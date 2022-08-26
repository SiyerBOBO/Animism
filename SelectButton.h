#pragma once

#include <QWidget>
#include <QUuid>
#include <QPixmap>
#include <QLabel>
#include <QPushButton>
#include <QPointer>
#include <QHBoxLayout>
#include "SwitchButton.h"

class SelectButton : public QWidget
{
	Q_OBJECT

public:
	SelectButton(QWidget *parent = Q_NULLPTR);
	~SelectButton();

	void setIcon(const QPixmap& icon);
	void setUuid(const QUuid& uuid);
	void setText(const QString& text);

	QString text() const;
	QPixmap icon() const;
	QUuid uuid() const;

	void setChecked(bool state);
	bool isChecked();

	Q_SIGNAL void checked(bool state);
	Q_SIGNAL void help();

protected:
	void initUi();
	void paintEvent(QPaintEvent* e) override;

private:
	QUuid _uuid;
	QPixmap _icon;

	QPointer<QLabel> _ptr_icon;
	QPointer<QPushButton> _ptr_text;
	QPointer<SwitchButton> _ptr_switch;
	
	QPointer<QHBoxLayout> _ptr_layout;
};
