#pragma once

#include <QWidget>
#include <QSet>

class ScatterDiagram : public QWidget
{
	Q_OBJECT

public:
	ScatterDiagram(QWidget *parent);
	~ScatterDiagram();

	void setRange(qint64 min, qint64 max);
	QPair<qint64, qint64> range() const;

	void insert(const QList<qint64>& data);
	void remove(qint64 data);
	void clear();

	Q_SIGNAL void drag(qreal min, qreal max);
	Q_SIGNAL void drop(qreal min, qreal max);

protected:
	void initUi();
	void updateData();
	bool event(QEvent* e) override;

private:
	QSet<qint64> _src_data;
	QPixmap _pre_data;
	qint64 _range_min;
	qint64 _range_max;
	QColor _color_border;
	QColor _color_background;
	QColor _color_foreground;
	QColor _color_highlight;
};
