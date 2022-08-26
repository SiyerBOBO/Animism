#pragma once

#include <QWidget>
#include <QPointer>
#include <QtConcurrent>
#include "Model.h"
#include "ui_MiniView.h"

class MiniOpcodeView : public QWidget
{
	Q_OBJECT

public:
	static MiniOpcodeView* Instance(const QString& task_name, const QString& opcode_name);
	~MiniOpcodeView();

protected:
	MiniOpcodeView(QWidget *parent = Q_NULLPTR);
	void initUi();
	bool event(QEvent* e) override;
	Q_SLOT void loadData();
	Q_SLOT void reset();

private:
	Ui::MiniView ui;
	static std::unique_ptr<MiniOpcodeView> _spointer_instance;
	QString _str_task_name;
	QString _str_opcode_name;

	QPointer<QFutureWatcher<QList<EventInfo>>> _ptr_watcher_event;
	QList<QString> _list_param;
};
