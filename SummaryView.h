#pragma once

#include <QWidget>
#include <QPointer>
#include "ui_SummaryView.h"

class SummaryView : public QWidget
{
	Q_OBJECT

public:
	SummaryView(QWidget *parent = Q_NULLPTR);
	~SummaryView();

	Q_SIGNAL void categoryClicked(const QUuid& uuid, int index);

protected:
	void initUi();
	Q_SLOT void loadData();
	bool eventFilter(QObject* object, QEvent* e) override;

private:
	Ui::SummaryView ui;

	QPointer<QPolarChart> _ptr_chart;
	QPointer<QCategoryAxis> _ptr_axis_angula;
	QPointer<QValueAxis> _ptr_axis_radial;

	QPointer<QScatterSeries> _ptr_series_scatter;
	QPointer<QLineSeries> _ptr_series_line;
	QPointer<QAreaSeries> _ptr_series_area;

	QMap<QString, qulonglong> _map_event_count;
};
