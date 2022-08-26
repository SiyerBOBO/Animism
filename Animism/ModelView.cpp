#pragma execution_character_set("utf-8")
#include "ModelView.h"
#include <QPainter>
#include <QLinearGradient>
#include <QMovie>
#include <QTimer>
#include <QVBoxLayout>

#include <QPixmap>
#include <QBuffer>
#include <QFileInfo>
#include <QButtonGroup>
#include <QGraphicsDropShadowEffect>

#include <QProcess>
#include "Utils.h"

#include "MessageDialog.h"
#include "FlowLayout.h"
#include "ModelCreater.h"
#include "EventTypeDescription.h"
#include "ModelPool.h"
#include "SaveDialog.h"

#include <QtCharts>
#include <QtCharts/QScatterSeries>
#include <QtCharts/QLineSeries>
#include <QtCharts/QAreaSeries>

#include <QDebug>

ModelView::ModelView(const Model& model, QWidget* parent)
	: QWidget(parent),
	_ptr_task(new ModelTask(model, this))
{
	this->initUi();
}

ModelView::~ModelView()
{
	if (this->_ptr_task)
	{
		if (this->_ptr_task->isRunning())
		{
			emit this->_ptr_task->stop();
			this->_ptr_task->wait(3000);
			this->_ptr_task->terminate();
		}
		this->_ptr_task->deleteLater();
	}
}

void ModelView::initUi()
{
	ui.setupUi(this);

	this->ui.label_pstate_text->setText(tr("初始化观测环境..."));

	this->ui.widget_info->installEventFilter(this);
	this->ui.label_info_icon->setPixmap(_ptr_task->_model.Process.Icon);
	this->ui.label_info_name->setText(QFontMetrics(this->ui.label_info_name->font()).elidedText(_ptr_task->_model.Process.Name, Qt::ElideRight, this->ui.label_info_name->width()));
	this->ui.label_info_name->setToolTip(_ptr_task->_model.Process.Name);
	this->ui.label_info_path->setText(QFontMetrics(this->ui.label_info_path->font()).elidedText(_ptr_task->_model.Process.FilePath, Qt::ElideRight, this->ui.label_info_path->width()));
	this->ui.label_info_path->setToolTip(_ptr_task->_model.Process.FilePath);

	auto _button_group = new QButtonGroup(this);
	_button_group->addButton(this->ui.pushButton_state, 0);
	_button_group->addButton(this->ui.pushButton_cpu, 1);
	_button_group->addButton(this->ui.pushButton_ram, 2);
	_button_group->addButton(this->ui.pushButton_event, 3);
	QObject::connect(_button_group, &QButtonGroup::idClicked, this->ui.stackedWidget_data_view, &QStackedWidget::setCurrentIndex);

	for (auto _button : _button_group->buttons())
	{
		auto button = qobject_cast<IconButton*>(_button);
		button->installEventFilter(this);
		button->setAttribute(Qt::WA_Hover);
		button->setShowIcon(false);
		button->setBgRadius(5);
		button->setNormalBgColor(QColor(225, 230, 235));
		button->setHoverBgColor(QColor(230, 235, 240));
		button->setPressBgColor(QColor(225, 230, 235));
		button->setNormalTextColor(Qt::black);
		button->setHoverTextColor(Qt::black);
		button->setPressTextColor(Qt::black);
		button->setTextAlign(IconButton::TextAlign_Center);
		button->setPadding(0);
		button->setShowBorder(true);
		button->setBorderColor(QColor(210, 210, 210));
		button->setBorderWidth(1);
	}

	this->_ptr_movie_state = new QMovie(this);
	this->_ptr_movie_state->setFileName(":/Animism/Resource/ani_model.gif");
	this->_ptr_movie_state->setScaledSize(QSize(200, 200));
	this->_ptr_movie_state->setCacheMode(QMovie::CacheAll);

	QObject::connect(this->_ptr_movie_state, &QMovie::frameChanged, this, [=](int frameNumber) {
		if (frameNumber == 59)
			this->_ptr_movie_state->jumpToFrame(30);
		});
	this->ui.label_pstate_ani->setMovie(this->_ptr_movie_state);

	this->ui.widget_data->installEventFilter(this);
	QGraphicsDropShadowEffect* effect = new QGraphicsDropShadowEffect(this);
	effect->setOffset(0, 0);
	effect->setColor(Qt::gray);
	effect->setBlurRadius(8);
	this->ui.widget_data->setGraphicsEffect(effect);

	auto _item_layout = new FlowLayout(this->ui.widget_pstate_items, 0, 5, 5);
	this->ui.widget_pstate_items->setLayout(_item_layout);
	_item_layout->setAlignment(Qt::AlignLeft | Qt::AlignTop);

	for (auto& _focus : this->_ptr_task->_model.Focus)
	{
		EventCategory category;
		if (!EventCategory::FindEventCategory(_focus, category))
			return;
		EventTag* tag = new EventTag(category, this->ui.widget_pstate_items);
		_item_layout->addWidget(tag);
	}

	// Cpu Usage Chart
	this->_ptr_chart_cpu = new QChart();
	this->_ptr_chart_cpu->setTheme(QChart::ChartTheme::ChartThemeLight);
	this->_ptr_chart_cpu->setAnimationOptions(QChart::AnimationOption::SeriesAnimations);
	this->_ptr_chart_cpu->setContentsMargins(0, 0, 0, 0);
	this->_ptr_chart_cpu->layout()->setContentsMargins(0, 0, 0, 0);
	this->_ptr_chart_cpu->setBackgroundRoundness(0);

	this->_ptr_series_cpu = new QLineSeries(this->_ptr_chart_cpu);
	this->_ptr_series_cpu->setPen(QPen(QColor("#0780cf"), 1));
	this->_ptr_series_cpu->setBrush(QColor("#0780cf"));

	this->_ptr_series_cpu_area = new QAreaSeries(this->_ptr_series_cpu);
	this->_ptr_series_cpu_area->setBrush(QColor(7, 128, 207, 100));

	this->_ptr_axis_x_cpu = new QValueAxis;
	this->_ptr_axis_x_cpu->setRange(0, 30);
	this->_ptr_axis_x_cpu->setGridLineColor(QColor("#0780cf"));
	this->_ptr_axis_x_cpu->setVisible(false);

	this->_ptr_axis_y_cpu = new QValueAxis;
	this->_ptr_axis_y_cpu->setRange(0, 100);
	this->_ptr_axis_y_cpu->setGridLineColor(QColor("#0780cf"));
	this->_ptr_axis_y_cpu->setVisible(false);

	this->_ptr_chart_cpu->addSeries(this->_ptr_series_cpu_area);
	this->_ptr_chart_cpu->addSeries(this->_ptr_series_cpu);

	this->_ptr_chart_cpu->addAxis(this->_ptr_axis_x_cpu, Qt::AlignBottom);
	this->_ptr_series_cpu->attachAxis(this->_ptr_axis_x_cpu);
	this->_ptr_series_cpu_area->attachAxis(this->_ptr_axis_x_cpu);
	this->_ptr_chart_cpu->addAxis(this->_ptr_axis_y_cpu, Qt::AlignLeft);
	this->_ptr_series_cpu->attachAxis(this->_ptr_axis_y_cpu);
	this->_ptr_series_cpu_area->attachAxis(this->_ptr_axis_y_cpu);

	this->_ptr_chart_cpu->legend()->hide();
	this->_ptr_chart_cpu->setBackgroundPen(QColor("#0780cf"));

	this->ui.graphicsView_pcpu_chart->setRenderHint(QPainter::Antialiasing, true);
	this->ui.graphicsView_pcpu_chart->setChart(this->_ptr_chart_cpu);
	this->ui.graphicsView_pcpu_chart->setContentsMargins(0, 0, 0, 0);

	// Ram Usage Chart
	this->_ptr_chart_ram = new QChart();
	this->_ptr_chart_ram->setTheme(QChart::ChartTheme::ChartThemeLight);
	this->_ptr_chart_ram->setAnimationOptions(QChart::AnimationOption::SeriesAnimations);
	this->_ptr_chart_ram->setContentsMargins(0, 0, 0, 0);
	this->_ptr_chart_ram->layout()->setContentsMargins(0, 0, 0, 0);
	this->_ptr_chart_ram->setBackgroundRoundness(0);

	this->_ptr_series_ram = new QLineSeries(this->_ptr_chart_ram);
	this->_ptr_series_ram->setPen(QPen(QColor("#765005"), 1));
	this->_ptr_series_ram->setBrush(QColor("#765005"));

	this->_ptr_series_ram_area = new QAreaSeries(this->_ptr_series_ram);
	this->_ptr_series_ram_area->setBrush(QColor(118, 80, 5, 100));

	this->_ptr_axis_x_ram = new QValueAxis;
	this->_ptr_axis_x_ram->setRange(0, 30);
	this->_ptr_axis_x_ram->setVisible(false);
	this->_ptr_axis_x_ram->setGridLineColor(QColor("#765005"));

	this->_ptr_axis_y_ram = new QValueAxis;
	this->_ptr_axis_y_ram->setRange(0, 100);
	this->_ptr_axis_y_ram->setVisible(false);
	this->_ptr_axis_y_ram->setGridLineColor(QColor("#765005"));

	this->_ptr_chart_ram->addSeries(this->_ptr_series_ram_area);
	this->_ptr_chart_ram->addSeries(this->_ptr_series_ram);

	this->_ptr_chart_ram->addAxis(this->_ptr_axis_x_ram, Qt::AlignBottom);
	this->_ptr_series_ram->attachAxis(this->_ptr_axis_x_ram);
	this->_ptr_series_ram_area->attachAxis(this->_ptr_axis_x_ram);
	this->_ptr_chart_ram->addAxis(this->_ptr_axis_y_ram, Qt::AlignLeft);
	this->_ptr_series_ram->attachAxis(this->_ptr_axis_y_ram);
	this->_ptr_series_ram_area->attachAxis(this->_ptr_axis_y_ram);

	this->_ptr_chart_ram->legend()->hide();
	this->_ptr_chart_ram->setBackgroundPen(QColor("#765005"));

	this->ui.graphicsView_pram_chart->setRenderHint(QPainter::Antialiasing, true);
	this->ui.graphicsView_pram_chart->setChart(this->_ptr_chart_ram);
	this->ui.graphicsView_pram_chart->setContentsMargins(0, 0, 0, 0);

	// Event Filter Chart
	this->_ptr_chart_event = new QChart();
	this->_ptr_chart_event->setTheme(QChart::ChartTheme::ChartThemeLight);
	this->_ptr_chart_event->setAnimationOptions(QChart::AnimationOption::SeriesAnimations);
	this->_ptr_chart_event->setContentsMargins(0, 0, 0, 0);
	this->_ptr_chart_event->layout()->setContentsMargins(0, 0, 0, 0);
	this->_ptr_chart_event->setBackgroundRoundness(0);
	this->_ptr_chart_event->legend()->show();
	this->_ptr_chart_event->legend()->setAlignment(Qt::AlignLeft);

	this->_ptr_series_event = new QPieSeries(this->_ptr_chart_event);
	this->_ptr_series_event->setPieSize(0.5);
	for (const auto& categort : EventCategory::GetEventCategoryMap())
	{
		QPieSlice* slice = this->_ptr_series_event->append(categort.Name, 0);
		_list_slice_event << slice;

		QObject::connect(slice, &QPieSlice::hovered, [=](bool state) {
			slice->setLabelVisible(state);
			slice->setExploded(state);
			});

		QObject::connect(slice, &QPieSlice::clicked, [=]() {
			EventTypeDescription::EventTypeDescription(categort, this).exec();
			});
	}

	this->_ptr_chart_event->addSeries(this->_ptr_series_event);

	this->ui.graphicsView_pevent_chart->setRenderHint(QPainter::Antialiasing, true);
	this->ui.graphicsView_pevent_chart->setChart(this->_ptr_chart_event);
	this->ui.graphicsView_pevent_chart->setContentsMargins(0, 0, 0, 0);

	this->ui.pushButton_model_control->setCheckable(false);
	this->ui.pushButton_model_control->setShowIcon(false);
	this->ui.pushButton_model_control->setBgRadius(5);
	this->ui.pushButton_model_control->setNormalBgColor(QColor(225, 230, 235));
	this->ui.pushButton_model_control->setHoverBgColor(QColor(230, 235, 240));
	this->ui.pushButton_model_control->setPressBgColor(QColor(225, 230, 235));
	this->ui.pushButton_model_control->setNormalTextColor(Qt::black);
	this->ui.pushButton_model_control->setHoverTextColor(Qt::black);
	this->ui.pushButton_model_control->setPressTextColor(Qt::black);
	this->ui.pushButton_model_control->setTextAlign(IconButton::TextAlign_Center);
	this->ui.pushButton_model_control->setPadding(0);
	this->ui.pushButton_model_control->setShowBorder(true);
	this->ui.pushButton_model_control->setBorderColor(QColor(210, 210, 210));
	this->ui.pushButton_model_control->setBorderWidth(1);

	this->_ptr_timer_elapsed = new QTimer(this);
	QObject::connect(this->_ptr_timer_elapsed, &QTimer::timeout, this, [=]() {
		uint _interval = QDateTime::currentDateTime().toTime_t() - this->_datetime_start.toTime_t();
		auto _datetime = QDateTime::fromTime_t(_interval).toUTC();
		this->ui.label_pstate_elapsedtime->setText(tr("已用时长 %1").arg(_datetime.toString("hh:mm:ss")));
		});

	QTimer::singleShot(1000, this, &ModelView::initData);
}

void ModelView::initData()
{
	QProcess proc(this);
	proc.start("cmd", QStringList() << "/c" << "wmic cpu get name");
	if (proc.waitForFinished())
		this->ui.label_pcpu_text->setText(QString::fromLocal8Bit(proc.readAllStandardOutput()).simplified().mid(5));
	proc.terminate();

	QObject::connect(this->_ptr_task, &ModelTask::started, this, [=]() {
		this->ui.label_pstate_text->setText(tr("Animism AI正在观测目标进程"));
		this->ui.pushButton_model_control->setVisible(true);
		this->_datetime_start = QDateTime::currentDateTime();
		this->_ptr_timer_elapsed->start(1000);
		});
	QObject::connect(this->_ptr_task, &ModelTask::finished, this, [=]() {
		this->_ptr_timer_elapsed->stop();
		auto _error = this->_ptr_task->lastError();
		if (_error != ModelTask::ErrorType::None)
			this->ui.label_pstate_text->setText(tr("发生异常：%1").arg(_error));
		else
			this->ui.label_pstate_text->setText(tr("目标任务已完成"));
		});
	QObject::connect(this->_ptr_task, &ModelTask::monitor, this, [=](double cpu, qulonglong ram) {
		this->ui.label_pcpu_value->setText(QString::number(cpu, 'f', 3));
		this->_ptr_series_cpu->append((this->_ptr_series_cpu->count() + 1) / 2, cpu);
		if (this->_ptr_series_cpu->count() > 61)
			this->_ptr_chart_cpu->scroll(this->_ptr_chart_cpu->plotArea().width() / 60, 0);

		if (this->_ptr_axis_y_ram->max() * 0.9 < ram)
			this->_ptr_axis_y_ram->setMax(ram / 2.0 * 3.0);
		this->ui.label_pram_value->setText(QString::number(ram / 1024));
		this->_ptr_series_ram->append((this->_ptr_series_ram->count() + 1) / 2, ram);
		if (this->_ptr_series_ram->count() > 61)
			this->_ptr_chart_ram->scroll(this->_ptr_chart_ram->plotArea().width() / 60, 0);
		});
	QObject::connect(this->_ptr_task, &ModelTask::counter, this, [=](
		unsigned long long _process,
		unsigned long long _thread,
		unsigned long long _imageload,
		unsigned long long _fileio,
		unsigned long long _tcpip,
		unsigned long long _udpip,
		unsigned long long _registry)
		{
			unsigned long long _ull_total_old = 0, _ull_total_new = 0;

			_ull_total_old += this->_arr_event_count[0].exchange(_process);
			_ull_total_new += _process;
			_list_slice_event[0]->setValue(this->_arr_event_count[0].load());

			_ull_total_old += this->_arr_event_count[1].exchange(_thread);
			_ull_total_new += _thread;
			_list_slice_event[1]->setValue(this->_arr_event_count[1].load());

			_ull_total_old += this->_arr_event_count[2].exchange(_imageload);
			_ull_total_new += _imageload;
			_list_slice_event[2]->setValue(this->_arr_event_count[2].load());

			_ull_total_old += this->_arr_event_count[3].exchange(_fileio);
			_ull_total_new += _fileio;
			_list_slice_event[3]->setValue(this->_arr_event_count[3].load());

			_ull_total_old += this->_arr_event_count[4].exchange(_tcpip);
			_ull_total_new += _tcpip;
			_list_slice_event[4]->setValue(this->_arr_event_count[4].load());

			_ull_total_old += this->_arr_event_count[5].exchange(_udpip);
			_ull_total_new += _udpip;
			_list_slice_event[5]->setValue(this->_arr_event_count[5].load());

			_ull_total_old += this->_arr_event_count[6].exchange(_registry);
			_ull_total_new += _registry;
			_list_slice_event[6]->setValue(this->_arr_event_count[6].load());

			this->ui.label_pevent_value->setText(QString::number(_ull_total_new - _ull_total_old));
		});
	this->ui.label_pstate_text->setText(tr("Animism AI正在聚焦..."));
	this->_ptr_task->start();
}

bool ModelView::event(QEvent* e)
{
	switch (e->type())
	{
	case QEvent::Resize:
	{
		this->ui.label_info_name->setText(QFontMetrics(this->ui.label_info_name->font()).elidedText(_ptr_task->_model.Process.Name, Qt::ElideRight, this->ui.label_info_name->width()));
		this->ui.label_info_path->setText(QFontMetrics(this->ui.label_info_path->font()).elidedText(_ptr_task->_model.Process.FilePath, Qt::ElideRight, this->ui.label_info_path->width()));
		break;
	}
	case QEvent::Show:
	{
		this->ui.label_info_name->setText(QFontMetrics(this->ui.label_info_name->font()).elidedText(_ptr_task->_model.Process.Name, Qt::ElideRight, this->ui.label_info_name->width()));
		this->ui.label_info_path->setText(QFontMetrics(this->ui.label_info_path->font()).elidedText(_ptr_task->_model.Process.FilePath, Qt::ElideRight, this->ui.label_info_path->width()));
		break;
	}
	case QEvent::Hide: {
		break;
	}
	default:
		break;
	}
	return QWidget::event(e);
}

bool ModelView::eventFilter(QObject* obj, QEvent* e)
{
	if (obj == this->ui.widget_info)
	{
		QWidget* _ptr_obj = qobject_cast<QWidget*>(obj);
		switch (e->type())
		{
		case QEvent::Paint:
		{
			QLinearGradient linear(0, _ptr_obj->height() / 2.0, _ptr_obj->width(), _ptr_obj->height() / 2.0);
			linear.setColorAt(1, QColor("#0F2027"));
			linear.setColorAt(0.5, QColor("#203A43"));
			linear.setColorAt(0, QColor("#2C5364"));
			QPainter painter(_ptr_obj);
			painter.setRenderHints(QPainter::Antialiasing);
			painter.setPen(Qt::NoPen);
			painter.setBrush(linear);
			painter.drawRoundedRect(_ptr_obj->rect(), 10, 10);
			break;
		}
		default:
			break;
		}
	}
	else if (obj == this->ui.pushButton_state
		|| obj == this->ui.pushButton_cpu
		|| obj == this->ui.pushButton_ram
		|| obj == this->ui.pushButton_event)
	{
		auto button = qobject_cast<IconButton*>(obj);
		switch (e->type())
		{
		case QEvent::HoverEnter:
		{
			button->click();
			break;
		}
		default:
			break;
		}
	}
	else if (obj == this->ui.widget_data)
	{
		QWidget* _ptr_obj = qobject_cast<QWidget*>(obj);
		switch (e->type())
		{
		case QEvent::Paint:
		{
			QPainter painter(_ptr_obj);
			painter.setRenderHints(QPainter::Antialiasing);
			painter.setPen(Qt::NoPen);
			painter.setBrush(Qt::white);
			painter.drawRoundedRect(_ptr_obj->rect(), 10, 10);
			break;
		}
		case QEvent::Show: {
			this->_ptr_movie_state->start();
			break;
		}
		case QEvent::Hide: {
			this->_ptr_movie_state->stop();
			break;
		}
		default:
			break;
		}
	}
	return QWidget::eventFilter(obj, e);
}

void ModelView::on_pushButton_model_control_clicked()
{
	static QPointer<SaveDialog> dlg;
	if (this->_atomic_bool_lock.load())
	{
		if(!dlg.isNull())
			Utils::ActiveUi(dlg);
		return;
	}
	_atomic_bool_lock.store(true);
	this->_ptr_task->callStop();

	dlg = new SaveDialog(this->_ptr_task->_model.Uuid, this);
	dlg->show();
	auto result = dlg->exec();

	Pool.release(this->_ptr_task->_model.Uuid);

	if (result)
		MessageDialog::MessageDialog(this, tr("提示"), tr("模型已创建")).exec();
	else
		MessageDialog::MessageDialog(this, tr("提示"), tr("模型创建失败")).exec();

	_atomic_bool_lock.store(false);
	emit this->over();
}
