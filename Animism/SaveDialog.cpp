#pragma execution_character_set("utf-8")
#include "SaveDialog.h"
#include "ModelPool.h"
#include "Utils.h"
#include <QtMath>
#include <QPainter>
#include <QPainterPath>

SaveDialog::SaveDialog(const QUuid& uuid, QWidget* parent)
	: QDialog(parent), _uuid(uuid)
{
	this->initUi();
}

SaveDialog::~SaveDialog()
{
}

void SaveDialog::initUi()
{
	ui.setupUi(this);
	Utils::CenterUi(this);
	this->setAttribute(Qt::WA_Hover);
	this->setAttribute(Qt::WA_TranslucentBackground);
	this->setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog);
	this->setAttribute(Qt::WA_DeleteOnClose);

	this->ui.label_text->setText(tr("正在保存观测记录..."));

	QFileInfo info(Pool.file_path(this->_uuid));
	this->_name = info.baseName();
	this->setWindowTitle(this->_name);
	this->ui.widget_app_head->setToolTip(this->_name);

	this->ui.widget_app_icon->installEventFilter(this);
	this->ui.widget_app_head->installEventFilter(this);

	this->ui.pushButton_app_close->setPixmap(QPixmap(":/Animism/Resource/wm_close.png"));
	QObject::connect(this->ui.pushButton_app_close, &QPushButton::clicked, this, [=]() {
		if (this->_ptr_cancel_dlg.isNull())
			this->_ptr_cancel_dlg = new MessageDialog(this, tr("警告"), tr("任务未完成，坚持取消？"), false);
		auto c = this->_ptr_cancel_dlg->exec();
		if (c && this->_ptr_cancel_dlg)
			this->reject();
	});

	this->ui.progressBar_bar->setValue(0);
	this->ui.progressBar_bar->setAutoRadius(true);
	this->ui.progressBar_bar->setShowProgressRadius(true);
	this->ui.progressBar_bar->setBgColor(qRgba(255, 255, 255, 255));
	QLinearGradient lineGradient(0, 0, 370, 15);
	lineGradient.setColorAt(0.0, QColor("#5EFCE8"));
	lineGradient.setColorAt(1.0, QColor("#736EFE"));
	this->ui.progressBar_bar->setValueBrush(lineGradient);

	QList<QAbstractButton*> _list_buttons = this->findChildren<QAbstractButton*>();
	foreach(auto button, _list_buttons) {
		button->setFocusPolicy(Qt::NoFocus);
		button->setCursor(Qt::PointingHandCursor);
	}

	QObject::connect(&Pool, &ModelPool::progress_of_save, this, [=](const QUuid& uuid, qreal value) {
		if (uuid != this->_uuid)
			return;
		this->ui.progressBar_bar->setValue(value);
		this->ui.label_value->setText(QString::number(value, 'f', 2) + '%');
		});
	QObject::connect(&Pool, &ModelPool::result_of_save, this, [=](const QUuid& uuid, bool isComplete) {
		if (uuid != this->_uuid)
			return;
		Pool.release(_uuid);
		this->_ptr_cancel_dlg->deleteLater();
		if (isComplete)
			this->accept();
		else
			this->reject();
		});
	Pool.save(this->_uuid);
}

bool SaveDialog::event(QEvent* e)
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

bool SaveDialog::eventFilter(QObject* obj, QEvent* e)
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
			QString _text = QFontMetrics(_font_app_head).elidedText(this->windowTitle(), Qt::ElideRight, _ptr_obj->rect().width());
			painter.drawText(_ptr_obj->rect(), Qt::AlignCenter, _text);
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