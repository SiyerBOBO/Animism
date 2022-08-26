#pragma once

#include <QDialog>
#include <QUuid>
#include <QPointer>
#include "MessageDialog.h"
#include "ui_SaveDialog.h"

class SaveDialog : public QDialog
{
	Q_OBJECT

public:
	SaveDialog(const QUuid& uuid, QWidget *parent = Q_NULLPTR);
	~SaveDialog();

protected:
	void initUi();
	bool event(QEvent* e);
	bool eventFilter(QObject* obj, QEvent* e);

private:
	Ui::SaveDialog ui;
	const QUuid _uuid;
	QString _name;
	QPointer<MessageDialog> _ptr_cancel_dlg = nullptr;
};
