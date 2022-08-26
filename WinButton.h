#pragma once

#include <QPushButton>
#include <QPixmap>

class QPaintEvent;

class WinButton : public QPushButton
{
	Q_OBJECT

public:
	WinButton(QWidget *parent);
	~WinButton();

	void setPixmap(const QPixmap& pixmap);

protected:
	void paintEvent(QPaintEvent* e) override;

private:
	QPixmap _pixmap_normal;
};
