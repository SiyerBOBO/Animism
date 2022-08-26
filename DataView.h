#pragma once

#include <QWidget>
#include <QPointer>
#include <QButtonGroup>
#include "Model.h"
#include "ui_DataView.h"
#include <QtConcurrent>

class SqlQueryModel;

class DataView : public QWidget
{
	Q_OBJECT

public:
	DataView(const QUuid& uuid, QWidget *parent = Q_NULLPTR);
	~DataView();

protected:
	void initUi();
	void initData();
	void loadData();
	bool event(QEvent* e) override;
	bool eventFilter(QObject* obj, QEvent* event) override;

private:
	Ui::DataView ui;
	EventCategory _category;
	QButtonGroup _buttonGroup;
	QPointer<QFutureWatcher<QList<EventInfo>>> _ptr_watcher_data;
	std::atomic_bool _viewlock;
	QList<QString> _list_data_param;
	QPointer<QChart> _ptr_chart_opcode;
	QPointer<QPieSeries> _ptr_series_opcode;
	QList<QPointer<QPieSlice>> _list_slice_opcode;
};
