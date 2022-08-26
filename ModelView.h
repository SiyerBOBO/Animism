#pragma once
#include "ModelTask.h"
#include <QWidget>
#include <QPointer>
#include "ui_ModelView.h"

class QTimer;
class QMovie;

class ModelView : public QWidget
{
	Q_OBJECT

public:
	ModelView(const Model& model, QWidget* parent = Q_NULLPTR);
	~ModelView();
	Q_SIGNAL void over();

protected:
	void initUi();
	Q_SLOT void initData();
	bool event(QEvent* e) override;
	bool eventFilter(QObject* obj, QEvent* e) override;
	Q_SLOT void on_pushButton_model_control_clicked();

private:
	Ui::ModelView ui;
	QPointer<ModelTask> _ptr_task;
	QDateTime _datatime_start;
	QPointer<QMovie> _ptr_movie_state;

	std::atomic<unsigned long long> _arr_event_count[7] = { 0 };

	QPointer<QChart> _ptr_chart_cpu;
	QPointer<QValueAxis> _ptr_axis_x_cpu;
	QPointer<QValueAxis> _ptr_axis_y_cpu;
	QPointer<QLineSeries> _ptr_series_cpu;
	QPointer<QAreaSeries> _ptr_series_cpu_area;

	QPointer<QChart> _ptr_chart_ram;
	QPointer<QValueAxis> _ptr_axis_x_ram;
	QPointer<QValueAxis> _ptr_axis_y_ram;
	QPointer<QLineSeries> _ptr_series_ram;
	QPointer<QAreaSeries> _ptr_series_ram_area;

	QPointer<QChart> _ptr_chart_event;
	QPointer<QPieSeries> _ptr_series_event;
	QList<QPointer<QPieSlice>> _list_slice_event;

	QPointer<QTimer> _ptr_timer_elapsed;
	QDateTime _datetime_start;

	std::atomic_bool _atomic_bool_lock = false;
};
