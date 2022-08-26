#pragma execution_character_set("utf-8")
#include <QEvent>
#include <QPaintEvent>
#include <QWindowStateChangeEvent>
#include <QPainter>
#include <QPainterPath>
#include <QFont>
#include <QPushButton>
#include <QButtonGroup>
#include <QFileDialog>
#include <QStandardPaths>
#include <QtMath>
#include <QScrollBar>
#include <QTimer>
#include <QSplitterHandle>
#include <QUuid>
#include "Animism.h"
#include "WinEffectHelper.h"
#include "Utils.h"
#include "ModelCreater.h"
#include "EventTrapper.h"
#include "Welcome.h"
#include "Explorer.h"
#include "Settings.h"
#include "MessageDialog.h"
#include "ModelPool.h"
#include <QDebug>

Animism::Animism(QWidget* parent)
	: QWidget(parent)
{
	this->initUi();
}

Animism::~Animism()
{

}

void Animism::initUi()
{
	ui.setupUi(this);
	this->setAttribute(Qt::WA_Hover);
	this->setAttribute(Qt::WA_TranslucentBackground);
	this->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowSystemMenuHint | Qt::WindowMinimizeButtonHint);
	setWindowAnimation((HWND)this->winId());

	this->_vect_resize_area.fill(qMakePair<QRect, bool>(QRect(0, 0, 0, 0), false), 8);

	this->ui.pushButton_app_close->setPixmap(QPixmap(":/Animism/Resource/wm_close.png"));
	this->ui.pushButton_app_minisize->setPixmap(QPixmap(":/Animism/Resource/wm_minisize.png"));
	this->ui.pushButton_app_maxisize->setPixmap(QPixmap(":/Animism/Resource/wm_maxisize.png"));

	QObject::connect(this->ui.pushButton_app_close, &QPushButton::clicked, this, &Animism::close);
	QObject::connect(this->ui.pushButton_app_minisize, &QPushButton::clicked, this, &Animism::showMinimized);
	QObject::connect(this->ui.pushButton_app_maxisize, &QPushButton::clicked, this, [=]() {
		if (this->isMaximized())
			ShowWindow((HWND)this->winId(), SW_RESTORE);
		else
			ShowWindow((HWND)this->winId(), SW_MAXIMIZE);
		});

	this->ui.widget_app_icon->installEventFilter(this);
	this->ui.widget_app_head->installEventFilter(this);

	QObject::connect(this, &Animism::windowIconChanged, this, [=]() {this->update(); });
	QObject::connect(this, &Animism::windowIconTextChanged, this, [=]() {this->update(); });

	this->_ptr_welcome = new Welcome(this->ui.stackedWidget_container);
	this->ui.stackedWidget_container->addWidget(_ptr_welcome);

	this->_ptr_explorer = new Explorer(this->ui.stackedWidget_container);
	this->ui.stackedWidget_container->addWidget(_ptr_explorer);

	this->_ptr_settings = new Settings(this->ui.stackedWidget_container);
	this->ui.stackedWidget_container->addWidget(_ptr_settings);

	QObject::connect(this->_ptr_explorer, &Explorer::modelCountChanged, this, [=](int num) {
		this->ui.stackedWidget_container->setCurrentIndex(num ? 1 : 0);
		this->ui.widget_app_status->set(0, tr("%1个项目").arg(num));
		});

	this->ui.widget_app_status->append(tr("0个项目"));

	this->ui.widget_app_status->append(tr("已捕获：0次行为"));
	this->ui.widget_app_status->append(tr("吞吐量：0项每秒"));

	this->ui.pushButton_app_create->setPixmap(QPixmap(":/Animism/Resource/app_create.png"));
	QObject::connect(this->ui.pushButton_app_create, &WinButton::clicked, [=]() {
		Model model;
		ModelCreater cm(model, this);
		cm.setModal(false);
		cm.show();
		if (ModelCreater::Accepted != cm.exec())
			return;
		this->_ptr_explorer->push(model);
		});

	this->ui.pushButton_app_open->setPixmap(QPixmap(":/Animism/Resource/app_open.png"));
	QObject::connect(this->ui.pushButton_app_open, &WinButton::clicked, [=]() {
		QStringList _filepaths = QFileDialog::getOpenFileNames(this,
			tr("打开模型"),
			QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation),
			tr("Animism Model File(*.asm)")
		);
		if (_filepaths.isEmpty())
			return;
		QProcess::startDetached(QString::fromLocal8Bit(__argv[0]), _filepaths);
		});

	this->ui.pushButton_app_settings->setPixmap(QPixmap(":/Animism/Resource/app_settings.png"));
	QObject::connect(this->ui.pushButton_app_settings, &WinButton::clicked, this, [=]() {
		if(2 != this->ui.stackedWidget_container->currentIndex())
			this->ui.stackedWidget_container->setCurrentWidget(this->_ptr_settings);
		else if(this->_ptr_explorer->modelCount())
			this->ui.stackedWidget_container->setCurrentWidget(this->_ptr_explorer);
		else
			this->ui.stackedWidget_container->setCurrentWidget(this->_ptr_welcome);
		});

	QObject::connect(this->ui.stackedWidget_container, &QStackedWidget::currentChanged, this, [=](int index) {
		if (index == 2)
		{
			this->ui.pushButton_app_settings->setPixmap(QPixmap(":/Animism/Resource/app_back.png"));
			this->ui.pushButton_app_settings->setToolTip(tr("返回"));
		}
		else
		{
			this->ui.pushButton_app_settings->setPixmap(QPixmap(":/Animism/Resource/app_settings.png"));
			this->ui.pushButton_app_settings->setToolTip(tr("设置"));
		}
		});

	QObject::connect(&Trapper, &EventTrapper::speedPerSecond, this, [=](uint32_t value) {
		this->ui.widget_app_status->set(1, tr("已捕获：%1次行为").arg(Trapper.eventCount()));
		this->ui.widget_app_status->set(2, tr("吞吐量：%1项每秒").arg(value));
		});

	QList<QAbstractButton*> _list_buttons = this->findChildren<QAbstractButton*>();
	foreach(auto button, _list_buttons) {
		button->setFocusPolicy(Qt::NoFocus);
		button->setCursor(Qt::PointingHandCursor);
	}
}

bool Animism::event(QEvent* e) 
{
	static bool _b_windowActive = this->isActiveWindow();
	static QPoint _point_mouse = QPoint(0, 0);
	static QRect _rect_frame = QRect(0, 0, 0, 0);
	static bool _b_mousePressed_Non = false;
	static bool _b_mousePressed_Gbl = false;

	switch (e->type())
	{
	case QEvent::Paint:
	{
		int _int_winShadow = 15;
		int _int_winRadius = 10;
		QRect _rect_winRect = QRect(this->ui.widget_frame->pos(), this->ui.widget_frame->size());

		QPainter painter(this);
		painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing | QPainter::SmoothPixmapTransform);

		QPainterPath path;
		if (this->isMaximized()) {
			path.addRect(_rect_winRect);
			painter.fillPath(path, QColor(255, 255, 255));
		}
		else {
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
		}
		break;
	}
	case QEvent::Resize:
	{
		//重新计算八个描点的区域,描点区域的作用还有就是计算鼠标坐标是否在某一个区域内
		int width = this->width();
		int height = this->height();
		int padding = 15;

		this->_vect_resize_area[0].first = QRect(0, padding, padding, height - padding * 2);
		this->_vect_resize_area[1].first = QRect(width - padding, padding, padding, height - padding * 2);
		this->_vect_resize_area[2].first = QRect(padding, 0, width - padding * 2, padding);
		this->_vect_resize_area[3].first = QRect(padding, height - padding, width - padding * 2, padding);
		this->_vect_resize_area[4].first = QRect(0, 0, padding, padding);
		this->_vect_resize_area[5].first = QRect(width - padding, 0, padding, padding);
		this->_vect_resize_area[6].first = QRect(0, height - padding, padding, padding);
		this->_vect_resize_area[7].first = QRect(width - padding, height - padding, padding, padding);
		break;
	}
	case QEvent::HoverMove:
	{
		QHoverEvent* hoverEvent = (QHoverEvent*)e;
		QPoint point = hoverEvent->pos();

		if (!_b_mousePressed_Gbl) {
			if (this->_vect_resize_area.at(0).first.contains(point))
				this->setCursor(Qt::SizeHorCursor);
			else if (this->_vect_resize_area.at(1).first.contains(point))
				this->setCursor(Qt::SizeHorCursor);
			else if (this->_vect_resize_area.at(2).first.contains(point))
				this->setCursor(Qt::SizeVerCursor);
			else if (this->_vect_resize_area.at(3).first.contains(point))
				this->setCursor(Qt::SizeVerCursor);
			else if (this->_vect_resize_area.at(4).first.contains(point))
				this->setCursor(Qt::SizeFDiagCursor);
			else if (this->_vect_resize_area.at(5).first.contains(point))
				this->setCursor(Qt::SizeBDiagCursor);
			else if (this->_vect_resize_area.at(6).first.contains(point))
				this->setCursor(Qt::SizeBDiagCursor);
			else if (this->_vect_resize_area.at(7).first.contains(point))
				this->setCursor(Qt::SizeFDiagCursor);
			else
				this->setCursor(Qt::ArrowCursor);
		}

		int offsetX = point.x() - _point_mouse.x();
		int offsetY = point.y() - _point_mouse.y();

		int rectX = _rect_frame.x();
		int rectY = _rect_frame.y();
		int rectW = _rect_frame.width();
		int rectH = _rect_frame.height();

		if (this->_vect_resize_area.at(0).second) {
			int resizeW = this->width() - offsetX;
			if (this->minimumWidth() <= resizeW)
				this->setGeometry(this->x() + offsetX, rectY, resizeW, rectH);
		}
		else if (this->_vect_resize_area.at(1).second)
			this->setGeometry(rectX, rectY, rectW + offsetX, rectH);
		else if (this->_vect_resize_area.at(2).second) {
			int resizeH = this->height() - offsetY;
			if (this->minimumHeight() <= resizeH)
				this->setGeometry(rectX, this->y() + offsetY, rectW, resizeH);
		}
		else if (this->_vect_resize_area.at(3).second)
			this->setGeometry(rectX, rectY, rectW, rectH + offsetY);
		else if (this->_vect_resize_area.at(4).second) {
			int resizeW = this->width() - offsetX;
			int resizeH = this->height() - offsetY;
			if (this->minimumWidth() <= resizeW)
				this->setGeometry(this->x() + offsetX, this->y(), resizeW, resizeH);
			if (this->minimumHeight() <= resizeH)
				this->setGeometry(this->x(), this->y() + offsetY, resizeW, resizeH);
		}
		else if (this->_vect_resize_area.at(5).second) {
			int resizeW = rectW + offsetX;
			int resizeH = this->height() - offsetY;
			if (this->minimumHeight() <= resizeH)
				this->setGeometry(this->x(), this->y() + offsetY, resizeW, resizeH);
		}
		else if (this->_vect_resize_area.at(6).second) {
			int resizeW = this->width() - offsetX;
			int resizeH = rectH + offsetY;
			if (this->minimumWidth() <= resizeW)
				this->setGeometry(this->x() + offsetX, this->y(), resizeW, resizeH);
			if (this->minimumHeight() <= resizeH)
				this->setGeometry(this->x(), this->y(), resizeW, resizeH);
		}
		else if (this->_vect_resize_area.at(7).second) {
			int resizeW = rectW + offsetX;
			int resizeH = rectH + offsetY;
			this->setGeometry(this->x(), this->y(), resizeW, resizeH);
		}

		break;
	}
	case QEvent::MouseButtonPress:
	{
		QMouseEvent* mouseEvent = (QMouseEvent*)e;
		_point_mouse = mouseEvent->pos();
		_rect_frame = this->geometry();

		//判断按下的手柄的区域位置
		if (this->_vect_resize_area.at(0).first.contains(_point_mouse))
			this->_vect_resize_area[0].second = true;
		else if (this->_vect_resize_area.at(1).first.contains(_point_mouse))
			this->_vect_resize_area[1].second = true;
		else if (this->_vect_resize_area.at(2).first.contains(_point_mouse))
			this->_vect_resize_area[2].second = true;
		else if (this->_vect_resize_area.at(3).first.contains(_point_mouse))
			this->_vect_resize_area[3].second = true;
		else if (this->_vect_resize_area.at(4).first.contains(_point_mouse))
			this->_vect_resize_area[4].second = true;
		else if (this->_vect_resize_area.at(5).first.contains(_point_mouse))
			this->_vect_resize_area[5].second = true;
		else if (this->_vect_resize_area.at(6).first.contains(_point_mouse))
			this->_vect_resize_area[6].second = true;
		else if (this->_vect_resize_area.at(7).first.contains(_point_mouse))
			this->_vect_resize_area[7].second = true;
		else
			_b_mousePressed_Non = true;

		_b_mousePressed_Gbl = true;
		break;
	}
	case QEvent::MouseButtonRelease:
	{
		this->setCursor(Qt::ArrowCursor);
		_b_mousePressed_Non = false;
		_b_mousePressed_Gbl = false;
		for (int i = 0; i < 8; ++i)
			this->_vect_resize_area[i].second = false;
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
	case QEvent::Close: 
	{
		if (this->_ptr_explorer->modelCount())
		{
			this->ui.stackedWidget_container->setCurrentWidget(this->_ptr_explorer);
			auto result = MessageDialog::MessageDialog(this, tr("提示"), tr("任务正在进行，仍然退出？"), false).exec();
			if (!result)
			{
				e->ignore();
				return false;
			}
		}
		Pool.cleanUnfinished();
		Utils::KillMe();
		break;
	}
	case QEvent::WindowStateChange:
	{
		if (this->isMaximized())
		{
			this->ui.pushButton_app_maxisize->setToolTip(tr("还原"));
			this->ui.pushButton_app_maxisize->setPixmap(QPixmap(":/Animism/Resource/wm_return.png"));
			this->ui.gridLayout_3->setMargin(10);
		}
		else
		{
			this->ui.pushButton_app_maxisize->setToolTip(tr("最大化"));
			this->ui.pushButton_app_maxisize->setPixmap(QPixmap(":/Animism/Resource/wm_maxisize.png"));
			this->ui.gridLayout_3->setMargin(15);
		}
		this->update();
		break;
	}
	default:
		break;
	}
	return QWidget::event(e);
}

bool Animism::eventFilter(QObject* obj, QEvent* e) {
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
	else if (obj == this->ui.widget_app_head)
	{
		QWidget* _ptr_obj = qobject_cast<QWidget*>(obj);
		switch (e->type())
		{
		case QEvent::Paint:
		{
			QPainter painter(_ptr_obj);
			painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);
			painter.setBrush(Qt::NoBrush);
			painter.setPen("#2c2c2c");
			QFont _font_app_head = this->font();
			_font_app_head.setPixelSize(15);
			painter.setFont(_font_app_head);
			painter.drawText(_ptr_obj->rect(), Qt::AlignCenter, qApp->applicationDisplayName());
			break;
		}
		default:
			break;
		}
	}
	return QWidget::eventFilter(obj, e);
}

#if (QT_VERSION >= QT_VERSION_CHECK(6,0,0))
bool Animism::nativeEvent(const QByteArray& eventType, void* message, qintptr* result)
#else
bool Animism::nativeEvent(const QByteArray& eventType, void* message, long* result)
#endif
{
	if (eventType == "windows_generic_MSG") {
		MSG* msg = static_cast<MSG*>(message);

		//不同的消息类型和参数进行不同的处理
		if (msg->message == WM_NCCALCSIZE) {
			*result = 0;
			return true;
		}
		else if (msg->message == WM_NCHITTEST) {
			long x = GET_X_LPARAM(msg->lParam);
			long y = GET_Y_LPARAM(msg->lParam);
			QPoint pos = mapFromGlobal(QPoint(x, y)) - (this->isMaximized() ? QPoint(10, 10) : QPoint(15, 15));
			if (this->ui.widget_app_head->rect().contains(pos)) {
				QWidget* child = this->ui.widget_app_head->childAt(pos);
				if (!child || child == this->ui.widget_app_icon) {
					*result = HTCAPTION;
					QTimer::singleShot(100, this, [=]() {this->update(); });
					return true;
				}
			}
		}
		else if (msg->wParam == PBT_APMSUSPEND && msg->message == WM_POWERBROADCAST) {
			//系统休眠的时候自动最小化可以规避程序可能出现的问题
			ShowWindow((HWND)this->winId(), SW_MINIMIZE);
		}
		else if (msg->wParam == PBT_APMRESUMEAUTOMATIC) {
			//休眠唤醒后自动打开
			ShowWindow((HWND)this->winId(), SW_RESTORE);
		}
	}
	return false;
}