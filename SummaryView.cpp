#pragma execution_character_set("utf-8")
#include "SummaryView.h"
#include "FlowLayout.h"
#include "ModelCreater.h"
#include "ModelReaderV2.h"
#include <QtCharts>

SummaryView::SummaryView(QWidget* parent)
	: QWidget(parent)
{
	this->initUi();
}

SummaryView::~SummaryView()
{
}

void SummaryView::initUi()
{
	ui.setupUi(this);
	this->ui.widget_proc->installEventFilter(this);
	this->ui.widget_info->installEventFilter(this);
	this->ui.widget_tooltip->installEventFilter(this);

	auto _fevent_layout = new FlowLayout(this->ui.scrollAreaWidgetContents_info_fevent, 0, 5, 5);
	this->ui.scrollAreaWidgetContents_info_fevent->setLayout(_fevent_layout);
	_fevent_layout->setAlignment(Qt::AlignLeft | Qt::AlignTop);

	this->_ptr_chart = new QPolarChart();
	this->_ptr_chart->setTheme(QChart::ChartTheme::ChartThemeLight);
	this->_ptr_chart->setAnimationOptions(QChart::AnimationOption::SeriesAnimations);
	this->_ptr_chart->setContentsMargins(0, 0, 0, 0);
	this->_ptr_chart->layout()->setContentsMargins(0, 0, 0, 0);
	this->_ptr_chart->setBackgroundRoundness(0);
	this->_ptr_chart->legend()->hide();
	// this->_ptr_chart->setBackgroundPen(QColor("#0780cf"));

	this->ui.graphicsView_chart->setRenderHint(QPainter::Antialiasing, true);
	this->ui.graphicsView_chart->setChart(this->_ptr_chart);
	this->ui.graphicsView_chart->setContentsMargins(0, 0, 0, 0);

	this->_ptr_series_scatter = new QScatterSeries(this);
	this->_ptr_series_scatter->setPen(QPen(QColor(51, 133, 255), 2));
	this->_ptr_series_scatter->setBrush(Qt::white);
	QObject::connect(this->_ptr_series_scatter, &QScatterSeries::hovered, this, [=](const QPointF& point, bool state) {
		if (state)
		{
			auto _name = EventCategory::GetEventCategoryMap().at(point.x()).Name;
			this->ui.label_tooltip_event_name->setText(_name);
			this->ui.label_tooltip_event_count->setText(QString::number(this->_map_event_count.value(_name)));
		}
		else
		{
			this->ui.label_tooltip_event_name->setText("--");
			this->ui.label_tooltip_event_count->setText("--");
		}
		});
	QObject::connect(this->_ptr_series_scatter, &QScatterSeries::clicked, this, [=](const QPointF& point) {
		auto _category = EventCategory::GetEventCategoryMap().at(point.x());
		emit this->categoryClicked(_category.Guid, point.x());
		});

	this->_ptr_series_line = new QLineSeries(this);
	this->_ptr_series_line->setPen(QPen(QColor(51, 133, 255), 2));
	this->_ptr_series_line->setBrush(Qt::transparent);

	this->_ptr_series_area = new QAreaSeries(this->_ptr_series_line, 0);
	this->_ptr_series_area->setPen(Qt::NoPen);
	this->_ptr_series_area->setBrush(QColor(51, 133, 255, 100));

	this->_ptr_axis_angula = new QCategoryAxis(this);
	this->_ptr_axis_angula->setTickCount(8);
	this->_ptr_axis_angula->setLabelsVisible(true);
	this->_ptr_axis_angula->setLabelsPosition(QCategoryAxis::AxisLabelsPositionOnValue);
	QFont font("Microsoft YaHei");
	font.setPixelSize(15);
	this->_ptr_axis_angula->setLabelsFont(font);
	auto _list_categories = EventCategory::GetEventCategoryMap();
	for (int i = 0, c = _list_categories.size(); i < c; i++)
		this->_ptr_axis_angula->append(_list_categories.at(i).Name, i);
	this->_ptr_chart->addAxis(this->_ptr_axis_angula, QPolarChart::PolarOrientationAngular);

	this->_ptr_axis_radial = new QValueAxis(this);
	this->_ptr_axis_radial->setTickCount(10);
	this->_ptr_axis_radial->setLabelsVisible(false);
	this->_ptr_chart->addAxis(this->_ptr_axis_radial, QPolarChart::PolarOrientationRadial);

	this->_ptr_chart->addSeries(this->_ptr_series_area);
	this->_ptr_chart->addSeries(this->_ptr_series_line);
	this->_ptr_chart->addSeries(this->_ptr_series_scatter);

	this->_ptr_series_scatter->attachAxis(this->_ptr_axis_angula);
	this->_ptr_series_scatter->attachAxis(this->_ptr_axis_radial);

	this->_ptr_series_line->attachAxis(this->_ptr_axis_angula);
	this->_ptr_series_line->attachAxis(this->_ptr_axis_radial);

	this->_ptr_series_area->attachAxis(this->_ptr_axis_angula);
	this->_ptr_series_area->attachAxis(this->_ptr_axis_radial);

	this->_ptr_axis_angula->setRange(0, 7);
	this->_ptr_axis_radial->setRange(0, 9);

	this->loadData();
}

void SummaryView::loadData()
{
	Model _model = Reader.model();
	this->ui.label_proc_icon->setPixmap(_model.Process.Icon);
	this->ui.label_proc_name->setText(QFontMetrics(this->ui.label_proc_name->font()).elidedText(_model.Process.Name, Qt::ElideRight, 135));
	this->ui.label_proc_name->setToolTip(_model.Process.Name);

	this->ui.label_info_uuid->setToolTip(_model.Uuid.toString(QUuid::Id128));
	this->ui.label_info_timestamp->setToolTip(_model.Timestamp.toString("yyyy-MM-dd hh:mm:ss"));
	this->ui.label_info_duration->setToolTip(QString("%1Ãë").arg(QString::number(_model.Duration / 1000, 'f', 2)));
	this->ui.label_info_pname->setToolTip(_model.Process.Name);
	this->ui.label_info_ppath->setToolTip(_model.Process.FilePath);
	this->ui.label_info_pmd5->setToolTip(_model.Process.Md5);
	this->ui.label_info_cevent->setToolTip(QString::number(Reader.total_count()));

	auto _list_values = this->ui.widget_info->findChildren<QLabel*>();
	for (auto _value : _list_values)
	{
		if (_value->objectName().endsWith("_tip"))
			continue;
		_value->setText(QFontMetrics(_value->font()).elidedText(_value->toolTip(), Qt::ElideRight, 185));
	}

	for (auto& _focus : _model.Focus)
	{
		EventCategory category;
		if (!EventCategory::FindEventCategory(_focus, category))
			return;
		EventTag* tag = new EventTag(category, this->ui.scrollAreaWidgetContents_info_fevent);
		auto _size = tag->size();
		tag->setMaximumSize(_size.width() - 10, _size.height() - 10);
		tag->setMinimumSize(_size.width() - 10, _size.height() - 10);
		this->ui.scrollAreaWidgetContents_info_fevent->layout()->addWidget(tag);
	}

	auto _list_categories = EventCategory::GetEventCategoryMap();
	this->_map_event_count = Reader.event_count();

	QMultiHash<qulonglong, QString> _mhash_event_count;
	for (auto _event : _list_categories)
	{
		_mhash_event_count.insertMulti(
			this->_map_event_count.contains(_event.Name) ?
			this->_map_event_count.value(_event.Name) : 0,
			_event.Name
		);
	}	
	auto _set_order_keys = _mhash_event_count.keys().toSet().toList();
	qSort(_set_order_keys.begin(), _set_order_keys.end());
	int _int_order_index = 0;
	QHash<QString, int> _hash_event_order;
	for (auto _key : _set_order_keys)
	{
		++_int_order_index;
		auto _list_names = _mhash_event_count.values(_key);
		for (auto _name : _list_names)
			_hash_event_order.insert(_name, _int_order_index);
	}
	for (int i = 0, c = _list_categories.size(); i < c; i ++)
	{
		auto _name = _list_categories.at(i).Name;
		auto _order_index = _hash_event_order.value(_name);
		this->_ptr_series_scatter->append(i, _order_index);
		this->_ptr_series_line->append(i, _order_index);
	}
	this->_ptr_series_line->append(_list_categories.size(), this->_ptr_series_line->points().first().y());
}

bool SummaryView::eventFilter(QObject* object, QEvent* e)
{
	if (this->ui.widget_info == object)
	{
		QWidget* _ptr_obj = qobject_cast<QWidget*>(object);
		switch (e->type())
		{
		case QEvent::Paint:
		{
			QLinearGradient linear(0, _ptr_obj->height() / 2.0, _ptr_obj->width(), _ptr_obj->height() / 2.0);
			linear.setColorAt(0, QColor("#eef1f5"));
			linear.setColorAt(1, QColor("#e6e9f0"));
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
	else if (this->ui.widget_proc == object)
	{
		QWidget* _ptr_obj = qobject_cast<QWidget*>(object);
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
	else if (this->ui.widget_tooltip == object)
	{
		QWidget* _ptr_obj = qobject_cast<QWidget*>(object);
		switch (e->type())
		{
		case QEvent::Paint:
		{
			QLinearGradient linear(0, _ptr_obj->height() / 2.0, _ptr_obj->width(), _ptr_obj->height() / 2.0);
			linear.setColorAt(0, QColor("#eef1f5"));
			linear.setColorAt(1, QColor("#e6e9f0"));
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
	return QWidget::eventFilter(object, e);
}
