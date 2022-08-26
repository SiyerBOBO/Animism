#pragma once

#include <QDialog>
#include "Model.h"
#include "ui_ProcessFinder.h"

class ProcessFinder : public QDialog
{
	Q_OBJECT

public:
	ProcessFinder(QWidget *parent = Q_NULLPTR);
	~ProcessFinder();
	ProcessInfo info();

protected:
	void initUi();
	bool event(QEvent* e) override;
	bool eventFilter(QObject* obj, QEvent* e) override;
	Q_SLOT void find();
	Q_SLOT void refresh();

private:
	Ui::ProcessFinder ui;
	QList<QListWidgetItem*> _list_items;
};
