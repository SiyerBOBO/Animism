#pragma once

#include <QWidget>
#include <QPointer>
#include <QtConcurrent>
#include "Model.h"
#include "ui_MiniView.h"

class MiniDurationView : public QWidget
{
	Q_OBJECT

public:
	static MiniDurationView* Instance(const QString& task_name, const QDateTime& from, const QDateTime& to);
	~MiniDurationView();

protected:
	MiniDurationView(QWidget *parent = Q_NULLPTR);
	void initUi();
	bool event(QEvent* e) override;
	Q_SLOT void loadData();
	Q_SLOT void reset();

private:
	Ui::MiniView ui;
	static std::unique_ptr<MiniDurationView> _spointer_instance;
	QString _str_task_name;
	QDateTime _timestamp_from;
	QDateTime _timestamp_to;

	QPointer<QFutureWatcher<QList<EventInfo>>> _ptr_watcher_event;
	QPointer<QFutureWatcher<QMap<QString, qulonglong>>> _ptr_watcher_count;
	QList<QString> _list_param;
};
