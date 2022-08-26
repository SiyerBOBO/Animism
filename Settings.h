#pragma once

#include <QWidget>
#include "ui_Settings.h"
#include <QHash>

class Settings : public QWidget
{
	Q_OBJECT

public:
	Settings(QWidget *parent = Q_NULLPTR);
	~Settings();

protected:
	void initUi();
	Q_SLOT void initData();
	void showEvent(QShowEvent* event) override;

private:
	Ui::Settings ui;
	QHash<QString, QString> _hash_model2detail;
};
