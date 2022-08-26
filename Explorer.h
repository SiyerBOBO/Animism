#pragma once

#include <QWidget>
#include <QPointer>
#include "Model.h"
#include "ModelView.h"
#include "ui_Explorer.h"

class QVBoxLayout;
class QButtonGroup;

class Explorer : public QWidget
{
	Q_OBJECT

public:
	Explorer(QWidget *parent = Q_NULLPTR);
	~Explorer();
	int modelCount();
	Q_SLOT void push(Model& model);
	Q_SIGNAL void modelCountChanged(int count);

protected:
	void initUi();

private:
	Ui::Explorer ui;
	QPointer<QVBoxLayout> _ptr_layout_navi;
	QPointer<QButtonGroup> _ptr_group_navi;
	QHash<QWidget*, QAbstractButton*> _hash_view2button;
};
