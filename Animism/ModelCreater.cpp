#pragma execution_character_set("utf-8")
#include "ModelCreater.h"
#include "Utils.h"
#include "ProcessFinder.h"
#include "EventTypeDescription.h"
#include <QPainter>
#include <QPainterPath>
#include <QtMath>
#include <QMouseEvent>
#include <QAction>
#include <QFileDialog>
#include <QStandardPaths>
#include "MessageDialog.h"
#include "ModelPool.h"

ModelCreater::ModelCreater(Model& model, QWidget* parent)
	: QDialog(parent), _model(model)
{
	this->initUi();
}

ModelCreater::~ModelCreater()
{
}

void ModelCreater::initUi()
{
	ui.setupUi(this);
	Utils::CenterUi(this);
	this->setAttribute(Qt::WA_Hover);
	this->setAttribute(Qt::WA_TranslucentBackground);
	this->setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog);

	this->setWindowTitle(tr("新建Animism行为模型"));

	this->ui.widget_app_icon->installEventFilter(this);
	this->ui.widget_app_head->installEventFilter(this);
	this->ui.lineEdit_proc_info->installEventFilter(this);
	this->ui.scrollArea_event->installEventFilter(this);

	this->ui.pushButton_app_close->setPixmap(QPixmap(":/Animism/Resource/wm_close.png"));
	QObject::connect(this->ui.pushButton_app_close, &QPushButton::clicked, this, &ModelCreater::reject);
	QObject::connect(this->ui.pushButton_cancel, &QPushButton::clicked, this, &ModelCreater::reject);
	QObject::connect(this->ui.pushButton_submit, &QPushButton::clicked, this, &ModelCreater::submit);

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

	QObject::connect(this->ui.pushButton_proc_open, &QPushButton::clicked, this, [=]() {
		ProcessFinder finder(this);
		if (finder.exec())
		{
			_proc_info = finder.info();
			this->updateProc();
		}
		});

	QObject::connect(this->ui.pushButton_model_path_open, &QPushButton::clicked, this, [=]() {
		this->_path = QFileDialog::getSaveFileName(
			this,
			tr("保存到..."),
			QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation),
			tr("Animism Model File(*.asm)")
		);
		this->updateFile();
		});

	auto _ptr_layout = new FlowLayout(this->ui.scrollAreaWidgetContents_eventmap, 5, 5, 5);
	this->ui.scrollAreaWidgetContents_eventmap->setLayout(_ptr_layout);
	_ptr_layout->setAlignment(Qt::AlignLeft | Qt::AlignTop);

	for (const auto& category : EventCategory::GetEventCategoryMap())
	{
		EventTag* tag = new EventTag(category, this->ui.scrollAreaWidgetContents_eventmap);
		tag->setCheckable(true);
		QObject::connect(tag, &EventTag::clicked, this, [=](bool checked) {
			if (checked)
			{
				this->_set_event_uuids.insert(tag->uuid());
				if (this->_set_event_uuids.size() == EventCategory::GetEventCategoryMap().size())
					this->ui.checkBox_event_ctrl->setCheckState(Qt::CheckState::Checked);
				else
					this->ui.checkBox_event_ctrl->setCheckState(Qt::CheckState::PartiallyChecked);
			}
			else
			{
				this->_set_event_uuids.remove(tag->uuid());
				if (this->_set_event_uuids.isEmpty())
					this->ui.checkBox_event_ctrl->setCheckState(Qt::CheckState::Unchecked);
				else
					this->ui.checkBox_event_ctrl->setCheckState(Qt::CheckState::PartiallyChecked);
			}
			});
		_ptr_layout->addWidget(tag);
	}

	QObject::connect(this->ui.checkBox_event_ctrl, &QCheckBox::clicked, this, [=](bool state) {
		if (Qt::CheckState::PartiallyChecked == this->ui.checkBox_event_ctrl->checkState())
			this->ui.checkBox_event_ctrl->setCheckState(Qt::CheckState::Checked);

		auto tags = this->ui.scrollAreaWidgetContents_eventmap->findChildren<EventTag*>();
		for (auto& tag : tags)
			if (tag->isChecked() != state)
				tag->click();
		});

	this->ui.lineEdit_proc_info->setCursor(Qt::OpenHandCursor);
	this->ui.lineEdit_proc_info->setFocusPolicy(Qt::FocusPolicy::NoFocus);

	QList<QAbstractButton*> _list_buttons = this->findChildren<QAbstractButton*>();
	foreach(auto button, _list_buttons) {
		button->setFocusPolicy(Qt::NoFocus);
		button->setCursor(Qt::PointingHandCursor);
	}
}

bool ModelCreater::event(QEvent* e)
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

bool ModelCreater::eventFilter(QObject* obj, QEvent* e)
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
	else if (obj == this->ui.lineEdit_proc_info)
	{
		QLineEdit* _ptr_obj = qobject_cast<QLineEdit*>(obj);
		static bool _isMouseButtonClicked = false;
		switch (e->type())
		{
		case QEvent::MouseButtonPress:
		{
			QMouseEvent* _ptr_mouse_e = (QMouseEvent*)e;
			if (_ptr_mouse_e->button() == Qt::MouseButton::LeftButton)
			{
				_ptr_obj->setCursor(Qt::ClosedHandCursor);
				_isMouseButtonClicked = true;
			}
			break;
		}
		case QEvent::MouseButtonRelease:
		{
			if (_isMouseButtonClicked)
			{
				_isMouseButtonClicked = false;
				_ptr_obj->setCursor(Qt::OpenHandCursor);
				ProcessInfo info;
				if (!Utils::getInformationByWindow(info))
					break;
				_proc_info = info;
				this->updateProc();
			}
			break;
		}
		case QEvent::MouseMove:
		{
			if (_isMouseButtonClicked)
				Utils::declareWindow();
			break;
		}
		default:
			break;
		}
	}
	else if (obj == this->ui.scrollArea_event)
	{
		QScrollArea* _ptr_obj = qobject_cast<QScrollArea*>(this->ui.scrollArea_event);
		switch (e->type())
		{
		case QEvent::Paint:
		{
			QPainter painter(_ptr_obj);
			painter.setRenderHint(QPainter::TextAntialiasing);
			QPen pen(Qt::darkGray);
			pen.setStyle(Qt::PenStyle::DashLine);
			painter.setPen(pen);
			painter.setBrush(Qt::NoBrush);
			painter.drawRect(_ptr_obj->rect().marginsRemoved(QMargins(0, 0, 1, 1)));
			break;
		}
		default:
			break;
		}
	}
	return QDialog::eventFilter(obj, e);
}

void ModelCreater::updateProc()
{
	if (this->_ptr_action_icon.isNull())
	{
		this->_ptr_action_icon = new QAction(this);
		this->ui.lineEdit_proc_info->addAction(this->_ptr_action_icon, QLineEdit::ActionPosition::LeadingPosition);
		this->ui.lineEdit_proc_info->setStyleSheet("");
	}
	this->_ptr_action_icon->setIcon(_proc_info.Icon);
	QString _str_procinfo_complete = QString("[%1] %2").arg(_proc_info.Pid).arg(_proc_info.Name);
	QString _str_procinfo = QFontMetrics(this->ui.lineEdit_proc_info->font()).elidedText(_str_procinfo_complete, Qt::ElideRight, this->ui.lineEdit_proc_info->width() - 30);
	this->ui.lineEdit_proc_info->setText(_str_procinfo);
	this->ui.lineEdit_proc_info->setToolTip(_str_procinfo_complete);
}

void ModelCreater::updateFile()
{
	QString _str_savepath_complete = this->_path;
	QString _str_savepath = QFontMetrics(this->ui.lineEdit_model_path->font()).elidedText(_str_savepath_complete, Qt::ElideRight, this->ui.lineEdit_model_path->width() - 15);
	this->ui.lineEdit_model_path->setText(_str_savepath);
	this->ui.lineEdit_model_path->setToolTip(_str_savepath_complete);
}

void ModelCreater::submit()
{
	if (this->ui.lineEdit_proc_info->text().isEmpty())
	{
		MessageDialog::MessageDialog(this, tr("提示"), tr("请选择目标进程")).exec();
		return;
	}
	if (this->ui.lineEdit_model_path->text().isEmpty())
	{
		MessageDialog::MessageDialog(this, tr("提示"), tr("请设置模型保存位置")).exec();
		return;
	}
	if (this->_set_event_uuids.isEmpty())
	{
		MessageDialog::MessageDialog(this, tr("提示"), tr("至少选择一个目标行为")).exec();
		return;
	}
	Model _model_temp
	{
		QUuid::createUuid(),
		QDateTime::currentDateTime(),
		0,
		this->_set_event_uuids,
		this->_proc_info
	};
	if (!Pool.create(_model_temp, this->_path))
	{
		MessageDialog::MessageDialog(this, tr("错误"), tr("模型资源创建失败")).exec();
		return;
	}
	this->_model = _model_temp;
	this->accept();
}

EventTag::EventTag(const EventCategory& category, QWidget* parent)
	:QPushButton(parent), _category(category)
{
	this->initUi();
}

EventTag::~EventTag()
{
}

QUuid EventTag::uuid() const
{
	return this->_category.Guid;
}

void EventTag::initUi()
{
	this->setCursor(Qt::PointingHandCursor);

	this->setText(this->_category.Name);
	this->setToolTip(this->_category.Guid.toString(QUuid::WithoutBraces));

	QFont font("Microsoft YaHei");
	font.setPixelSize(15);
	this->setFont(font);
	QFontMetrics fm(font);
	int width = fm.width(this->_category.Name);
	this->setMinimumSize(width < 20 ? 40 : width + 20, 40);
	this->setMaximumSize(width < 20 ? 40 : width + 20, 40);
	this->update();
}

void EventTag::paintEvent(QPaintEvent* e)
{
	QPainter painter(this);
	painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);

	painter.setPen(Qt::lightGray);
	painter.setBrush((this->isChecked() || !this->isCheckable()) ? QColor(48, 206, 76) : Qt::gray);
	painter.drawRoundedRect(this->rect(), 3, 3);

	QFont font = this->font();
	font.setPixelSize(13);
	painter.setFont(font);
	painter.setPen(Qt::white);
	painter.setBrush(Qt::NoBrush);
	painter.drawText(this->rect(), Qt::AlignCenter, this->text());
}

void EventTag::mousePressEvent(QMouseEvent* e)
{
	if (e->button() == Qt::MouseButton::RightButton)
		EventTypeDescription::EventTypeDescription(this->_category, this).exec();
	return QPushButton::mousePressEvent(e);
}
