#pragma once

#include "Model.h"
#include <QDialog>
#include <QPointer>
#include <QSet>
#include "FlowLayout.h"
#include "ui_ModelCreater.h"

class ModelCreater;
class EventSelector;
class EventTag;

class ModelCreater : public QDialog
{
	Q_OBJECT

public:
	ModelCreater(Model& model, QWidget* parent = Q_NULLPTR);
	~ModelCreater();

protected:
	void initUi();
	bool event(QEvent* e) override;
	bool eventFilter(QObject* obj, QEvent* e) override;
	void updateProc();
	void updateFile();
	Q_SLOT void submit();

private:
	Ui::ModelCreater ui;
	Model& _model;
	ProcessInfo _proc_info;
	QString _path;
	QPointer<QAction> _ptr_action_icon;
	QSet<QUuid> _set_event_uuids;
};

class EventTag : public QPushButton
{
	Q_OBJECT

public:
	EventTag(const EventCategory& category, QWidget* parent);
	~EventTag();

	QUuid uuid() const;

protected:
	void initUi();
	void paintEvent(QPaintEvent* e) override;
	void mousePressEvent(QMouseEvent* e) override;

private:
	EventCategory _category;
};