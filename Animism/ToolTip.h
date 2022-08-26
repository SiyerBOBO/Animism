#pragma once

#include <QWidget>
#include "ui_ToolTip.h"

class ToolTip : public QWidget
{
	Q_OBJECT

public:
	static ToolTip* Instance(const QString& title, const QString& text);
	~ToolTip();

protected:
	ToolTip(QWidget *parent = Q_NULLPTR);
	void initUi();
	bool event(QEvent* e) override;

private:
	Ui::ToolTip ui;
	static std::unique_ptr<ToolTip> _spointer_instance;
};
