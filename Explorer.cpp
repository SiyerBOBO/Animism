#include "Explorer.h"
#include <QVBoxLayout>
#include <QButtonGroup>
#include <QTimer>
#include <QScrollBar>
#include "IconButton.h"
#include "ModelPool.h"

Explorer::Explorer(QWidget* parent)
	: QWidget(parent)
{
	this->initUi();
}

Explorer::~Explorer()
{
}

int Explorer::modelCount()
{
	return this->ui.stackedWidget_view->count();
}

void Explorer::push(Model& model)
{
	if (!Pool.contains(model.Uuid))
		return;
	// Create Page
	ModelView* view = new ModelView(model, this->ui.stackedWidget_view);
	this->ui.stackedWidget_view->addWidget(view);

	IconButton* button = new IconButton(this->ui.scrollAreaWidgetContents_map);
	button->setMinimumHeight(40);
	button->setMaximumHeight(40);
	button->setText(Pool.file_path(model.Uuid).section("/", -1, -1).split('.')[0]);
	button->setCheckable(true);
	button->setBgRadius(5);
	button->setNormalBgColor(Qt::transparent);
	button->setHoverBgColor(QColor("#F0F8FF"));
	button->setPressBgColor(QColor("#CBDEF6"));
	button->setCheckBgColor(QColor("#CBDEF6"));
	button->setNormalTextColor(QColor("#2c2c2c"));
	button->setHoverTextColor(QColor("#2c2c2c"));
	button->setPressTextColor(QColor("#2c2c2c"));
	button->setCheckTextColor(QColor("#2c2c2c"));
	button->setIconNormal(model.Process.Icon);
	button->setIconHover(model.Process.Icon);
	button->setIconPress(model.Process.Icon);
	button->setIconCheck(model.Process.Icon);
	button->setIconSize(QSize(24, 24));
	button->setShowIcon(true);
	button->setPaddingLeft(16);
	button->setPaddingRight(35);
	button->setToolTip(QString("PID: %1\nName: %2\nPath = %3\nMd5 = %4")
		.arg(model.Process.Pid).arg(model.Process.Name).arg(model.Process.FilePath).arg(model.Process.Md5));
	QObject::connect(button, &IconButton::clicked, this, [=](bool checked) {
		this->ui.stackedWidget_view->setCurrentWidget(view);
		});
	QObject::connect(view, &ModelView::over, this, [=]() {
		this->ui.stackedWidget_view->removeWidget(view);
		this->_ptr_group_navi->removeButton(button);
		this->_ptr_layout_navi->removeWidget(button);
		view->deleteLater();
		button->deleteLater();
		this->_hash_view2button.remove(view);
		auto _cur_view = this->ui.stackedWidget_view->currentWidget();
		if (_cur_view)
			this->_hash_view2button.value(_cur_view)->click();
		emit this->modelCountChanged(this->ui.stackedWidget_view->count());
		});
	this->_ptr_group_navi->addButton(button);
	this->_ptr_layout_navi->addWidget(button);
	button->setChecked(true);
	this->ui.stackedWidget_view->setCurrentWidget(view);
	_hash_view2button.insert(view, button);
	QScrollBar* scrollbar = this->ui.scrollArea_map->verticalScrollBar();
	QTimer::singleShot(30, [=]() {scrollbar->setValue(scrollbar->maximum()); });

	emit this->modelCountChanged(this->ui.stackedWidget_view->count());
}

void Explorer::initUi()
{
	ui.setupUi(this);

	this->_ptr_layout_navi = new QVBoxLayout(this->ui.scrollAreaWidgetContents_map);
	this->_ptr_layout_navi->setAlignment(Qt::AlignTop);
	this->_ptr_layout_navi->setDirection(QBoxLayout::Direction::TopToBottom);
	this->_ptr_layout_navi->setContentsMargins(10, 10, 10, 10);
	this->_ptr_layout_navi->setSpacing(2);

	this->_ptr_group_navi = new QButtonGroup(this);

	QList<QAbstractButton*> _list_buttons = this->findChildren<QAbstractButton*>();
	foreach(auto button, _list_buttons) {
		button->setFocusPolicy(Qt::NoFocus);
		button->setCursor(Qt::PointingHandCursor);
	}
}
