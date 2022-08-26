#pragma execution_character_set("utf-8")
#include "EventTypeDescription.h"
#include <QPixmap>
#include <QPainter>
#include <QPainterPath>
#include <QtMath>
#include <QClipBoard>
#include "Utils.h"
#include "MessageDialog.h"

EventTypeDescription::EventTypeDescription(const EventCategory& category, QWidget *parent)
	: QDialog(parent), category(category)
{
	this->initUi();
}

EventTypeDescription::~EventTypeDescription()
{
}

void EventTypeDescription::initUi()
{
	ui.setupUi(this);
	Utils::CenterUi(this);
	this->setAttribute(Qt::WA_Hover);
	this->setAttribute(Qt::WA_TranslucentBackground);
	this->setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog);

	this->setWindowTitle(tr("关于 %1 事件").arg(category.Name));
	this->ui.widget_app_icon->installEventFilter(this);
	this->ui.widget_app_head->installEventFilter(this);

	this->ui.pushButton_app_close->setPixmap(QPixmap(":/Animism/Resource/wm_close.png"));
	QObject::connect(this->ui.pushButton_app_close, &QPushButton::clicked, this, &EventTypeDescription::reject);
	QObject::connect(this->ui.pushButton_close, &QPushButton::clicked, this, &EventTypeDescription::reject);
	QObject::connect(this->ui.pushButton_copy, &QPushButton::clicked, this, [=]() {
		QStringList _str_items;
		_str_items.append(QString("name=%1").arg(this->category.Name));
		_str_items.append(QString("guid=%1").arg(this->category.Guid.toString(QUuid::StringFormat::WithoutBraces)));
		_str_items.append(QString("description=%1").arg(this->category.Description));
		QString _str_text = _str_items.join("\n");
		QApplication::clipboard()->setText(_str_text);
		MessageDialog::MessageDialog(this, tr("提示"), tr("信息已复制")).exec();
		});

	this->ui.label_icon->setPixmap(QPixmap(QString(":/Animism/%1").arg(category.Name)));
	this->ui.label_name->setText(category.Name);
	this->ui.label_guid->setText(category.Guid.toString(QUuid::Id128));
	this->ui.plainTextEdit_description->setPlainText(category.Description);
	
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

	QList<QAbstractButton*> _list_buttons = this->findChildren<QAbstractButton*>();
	foreach(auto button, _list_buttons) {
		button->setFocusPolicy(Qt::NoFocus);
		button->setCursor(Qt::PointingHandCursor);
	}
}

bool EventTypeDescription::event(QEvent* e)
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

bool EventTypeDescription::eventFilter(QObject* obj, QEvent* e)
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
