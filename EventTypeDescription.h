#pragma once
#include "Model.h"
#include <QDialog>
#include "ui_EventTypeDescription.h"

class EventTypeDescription : public QDialog
{
	Q_OBJECT

public:
	EventTypeDescription(const EventCategory& category, QWidget *parent = Q_NULLPTR);
	~EventTypeDescription();

protected:
	void initUi();
	bool event(QEvent* e) override;
	bool eventFilter(QObject* obj, QEvent* e) override;

private:
	Ui::EventTypeDescription ui;
	const EventCategory category;
};
