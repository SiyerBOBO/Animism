#pragma execution_character_set("utf-8")
#include "ProcessFinder.h"
#include "Utils.h"
#include <QPainter>
#include <QPainterPath>
#include <QEvent>
#include <QPaintEvent>
#include <QMouseEvent>
#include <QtMath>
#include <QTimer>
#include <QMap>
#include <QScrollBar>

ProcessFinder::ProcessFinder(QWidget* parent)
	: QDialog(parent)
{
	this->initUi();
}

ProcessFinder::~ProcessFinder()
{
}

ProcessInfo ProcessFinder::info()
{
	ProcessInfo info;
	QList<QListWidgetItem*> items = this->ui.listWidget_proc->selectedItems();
	if (items.isEmpty())
		return {};
	QListWidgetItem* item = items[0];
	QString filemd5;
	Utils::GetFileMd5ByPath(item->data(Qt::UserRole + 2).toString(), filemd5);
	return {
		(DWORD)item->data(Qt::UserRole).toInt(), // PID
		item->data(Qt::UserRole + 1).toString(), // Name
		item->data(Qt::UserRole + 2).toString(), // Path
		item->data(Qt::UserRole + 3).value<QIcon>().pixmap(256, 256), // Icon
		filemd5
	};
}

void ProcessFinder::initUi()
{
	ui.setupUi(this);
	Utils::CenterUi(this);
	this->setAttribute(Qt::WA_Hover);
	this->setAttribute(Qt::WA_TranslucentBackground);
	this->setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog);

	this->setWindowTitle(tr("选择目标进程"));
	this->ui.widget_app_status->append(tr("请选择目标进程"));

	this->ui.widget_app_icon->installEventFilter(this);
	this->ui.widget_app_head->installEventFilter(this);

	this->ui.pushButton_app_close->setPixmap(QPixmap(":/Animism/Resource/wm_close.png"));
	QObject::connect(this->ui.pushButton_app_close, &QPushButton::clicked, this, &ProcessFinder::reject);
	QObject::connect(this->ui.pushButton_cancel, &QPushButton::clicked, this, &ProcessFinder::reject);
	QObject::connect(this->ui.pushButton_refresh, &QPushButton::clicked, this, &ProcessFinder::refresh);

	auto _buttons = this->ui.widget_container->findChildren<IconButton*>();
	for (auto _button : _buttons)
	{
		_button->setCheckable(false);
		_button->setShowIcon(false);
		_button->setBgRadius(5);
		_button->setNormalBgColor(QColor(225, 230, 235));
		_button->setHoverBgColor(QColor(230, 235, 240));
		_button->setPressBgColor(QColor(225, 230, 235));
		_button->setNormalTextColor(Qt::black);
		_button->setHoverTextColor(Qt::black);
		_button->setPressTextColor(Qt::black);
		_button->setTextAlign(IconButton::TextAlign_Center);
		_button->setPadding(0);
		_button->setShowBorder(true);
		_button->setBorderColor(QColor(210, 210, 210));
		_button->setBorderWidth(1);
	}

	QObject::connect(this->ui.pushButton_submit, &QPushButton::clicked, this, [=]() {
		QList<QListWidgetItem*> items = this->ui.listWidget_proc->selectedItems();
		if (items.isEmpty()) {
			this->ui.widget_app_status->set(0, tr("请选择目标进程"));
			return;
		}
		this->accept();
		});

	QObject::connect(this->ui.listWidget_proc, &QListWidget::itemClicked, this, [=](QListWidgetItem* item) {
		this->ui.widget_app_status->set(0, tr("已选择进程：%1").arg(item->data(Qt::UserRole).toInt()));
		});

	QObject::connect(this->ui.lineEdit_find, &QLineEdit::textChanged, this, &ProcessFinder::find);

	QTimer::singleShot(500, this, &ProcessFinder::refresh);

	QList<QAbstractButton*> _list_buttons = this->findChildren<QAbstractButton*>();
	foreach(auto button, _list_buttons) {
		button->setFocusPolicy(Qt::NoFocus);
		button->setCursor(Qt::PointingHandCursor);
	}
}

bool ProcessFinder::event(QEvent* e)
{
	static bool _b_windowActive = this->isActiveWindow();
	switch (e->type())
	{
	case QEvent::Paint: {
		int _int_winShadow = 15;
		int _int_winRadius = 10;
		QRect _rect_winRect = QRect(this->ui.widget_frame->pos(), this->ui.widget_frame->size());

		QPainter painter(this);
		painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);

		QPainterPath path;
		path.addRoundedRect(_rect_winRect, _int_winRadius, _int_winRadius);
		painter.fillPath(path, QColor(255, 255, 255));
		QColor color = Qt::black;
		for (int i = 0; i < _int_winShadow; i++)
		{
			_rect_winRect = _rect_winRect.marginsAdded(QMargins(1, 1, 1, 1));
			double _double_alpha = _b_windowActive ? 30 - 11 * qSqrt(i) : 20 - 8 * qSqrt(i);
			path.clear();
			path.setFillRule(Qt::WindingFill);
			path.addRoundedRect(_rect_winRect, _int_winRadius, _int_winRadius++);
			color.setAlpha(_double_alpha > 0 ? _double_alpha : 0);
			painter.setPen(color);
			painter.drawPath(path);
		}
		break;
	}
	case QEvent::WindowActivate:
	{
		_b_windowActive = true;
		this->update();
		break;
	}
	case QEvent::WindowDeactivate:
	{
		_b_windowActive = false;
		this->update();
		break;
	}
	default:
		break;
	}
	return QDialog::event(e);
}

bool ProcessFinder::eventFilter(QObject* obj, QEvent* e)
{
	if (obj == this->ui.widget_app_icon)
	{
		QWidget* _ptr_obj = qobject_cast<QWidget*>(obj);
		switch (e->type())
		{
		case QEvent::Paint:
		{
			QPainter painter(_ptr_obj);
			painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
			painter.setBrush(Qt::NoBrush);
			painter.setPen(Qt::NoPen);
			painter.drawPixmap(QRect(3, 3, 26, 26), this->windowIcon().pixmap(_ptr_obj->size()));
			break;
		}
		default:
			break;
		}
	}
	else if (obj == this->ui.widget_app_head) {
		static bool event_bLPressed = false;
		static QPoint event_DragPosition;
		QWidget* _ptr_obj = qobject_cast<QWidget*>(obj);
		switch (e->type()) {
		case QEvent::Paint: {
			QPainter painter(_ptr_obj);
			painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);
			painter.setPen("#2c2c2c");
			painter.setBrush(Qt::NoBrush);
			QFont _font_app_head = this->font();
			_font_app_head.setPixelSize(15);
			painter.setFont(_font_app_head);
			painter.drawText(_ptr_obj->rect(), Qt::AlignCenter, this->windowTitle());
			break;
		}
		case QEvent::MouseButtonPress: {
			if (((QMouseEvent*)e)->button() == Qt::LeftButton) {
				event_bLPressed = true;
				event_DragPosition = ((QMouseEvent*)e)->globalPos() - this->pos();
			}
			break;
		}
		case QEvent::MouseMove: {
			if (event_bLPressed && _ptr_obj->rect().contains(event_DragPosition - QPoint(15, 15)))
				move(((QMouseEvent*)e)->globalPos() - event_DragPosition);
			break;
		}
		case QEvent::MouseButtonRelease: {
			event_bLPressed = false;
			break;
		}
		}
	}
	return QDialog::eventFilter(obj, e);
}

void ProcessFinder::find()
{
	const QString& text = this->ui.lineEdit_find->text();

	// Clear
	this->ui.listWidget_proc->clearSelection();
	while (this->ui.listWidget_proc->count())
		this->ui.listWidget_proc->takeItem(0);

	// If Empty, Do Back
	if (text.isEmpty()) {
		for (QList<QListWidgetItem*>::iterator it = this->_list_items.begin(); it != this->_list_items.end(); it++)
			this->ui.listWidget_proc->addItem(*it);
		this->ui.widget_app_status->set(0, tr("请选择目标进程"));
		return;
	}

	// Find & Update
	for (QList<QListWidgetItem*>::iterator it = this->_list_items.begin(); it != this->_list_items.end(); it++) {
		bool _is_target = false;
		QListWidgetItem* item = *it;
		_is_target |= QString::number(item->data(Qt::UserRole).toInt()).contains(text);
		_is_target |= item->data(Qt::UserRole + 1).toString().toLower().contains(text.toLower());
		if (_is_target)
			this->ui.listWidget_proc->addItem(item);
	}
	this->ui.listWidget_proc->verticalScrollBar()->setValue(0);
	this->ui.widget_app_status->set(0, tr("查找到%1条进程").arg(this->ui.listWidget_proc->count()));
}

void ProcessFinder::refresh()
{
	this->ui.lineEdit_find->clear();
	while (this->ui.listWidget_proc->count())
		this->ui.listWidget_proc->takeItem(0);
	for (QList<QListWidgetItem*>::iterator it = this->_list_items.begin(); it != this->_list_items.end(); it++) {
		QListWidgetItem* item = *it;
		delete item;
	}
	this->_list_items.clear();

	QMap<qint64, QString> app_pid;
	if (!Utils::GetCurrentProcessMap(app_pid)) {
		this->ui.widget_app_status->set(0, tr("获取进程列表失败"));
		return;
	}
	for (QMap<qint64, QString>::iterator it = app_pid.begin(); it != app_pid.end(); it++) {
		if (it.key() == qApp->applicationPid())
			continue;

		QListWidgetItem* item = new QListWidgetItem(QString("[%1] %2").arg(it.key()).arg(it.value()), this->ui.listWidget_proc);
		item->setSizeHint(QSize(10, 20));
		QString filepath;
		QIcon icon(":/Animism/Resource/filetype_exe.png");
		if (Utils::GetPathByProcessID(it.key(), filepath))
			Utils::GetIconByPath(filepath, icon);
		item->setIcon(icon);

		item->setData(Qt::UserRole, it.key()); // PID
		item->setData(Qt::UserRole + 1, it.value()); // Name
		item->setData(Qt::UserRole + 2, filepath); // Path
		item->setData(Qt::UserRole + 3, icon); // Icon

		this->_list_items.append(item);
		this->ui.listWidget_proc->addItem(item);
	}
	this->ui.listWidget_proc->verticalScrollBar()->setValue(0);
}
