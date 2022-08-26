#pragma once

#include <QWidget>
#include <QPointer>
#include <QButtonGroup>
#include "ui_Analysis.h"

class QGraphicsDropShadowEffect;
class SummaryView;

class Analysis : public QWidget
{
	Q_OBJECT

public:
	Analysis(const QString& modelfile);
	~Analysis();

protected:
	void initUi();
	Q_SLOT void initData();
	bool event(QEvent* e) override;
	bool eventFilter(QObject* obj, QEvent* e) override;
#if (QT_VERSION >= QT_VERSION_CHECK(6,0,0))
	bool nativeEvent(const QByteArray& eventType, void* message, qintptr* result);
#else
	bool nativeEvent(const QByteArray& eventType, void* message, long* result);
#endif

private:
	Ui::Analysis ui;
	QVector<QPair<QRect, bool>> _vect_resize_area;
	const QString _str_filepath;

	QButtonGroup _buttonGroup;
	QPointer<QGraphicsDropShadowEffect> _ptr_shadow;

	QPointer<SummaryView> _ptr_summary_view;
};
