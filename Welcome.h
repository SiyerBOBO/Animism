#pragma once

#include <QWidget>
#include <QPointer>
#include "ui_Welcome.h"

class QMovie;

class Welcome : public QWidget
{
	Q_OBJECT

public:
	Welcome(QWidget *parent = Q_NULLPTR);
	~Welcome();

protected:
	void initUi();
	bool event(QEvent* e) override;

private:
	Ui::Welcome ui;
	QPointer<QMovie> _ptr_movie_home;
};
