#pragma execution_character_set("utf-8")
#include "DataView.h"
#include "ModelReaderV2.h"
#include "MessageDialog.h"
#include "ToolTip.h"
#include "MiniDurationView.h"
#include "MiniOpcodeView.h"
#include <QKeyEvent>
#include <QDebug>

DataView::DataView(const QUuid& uuid, QWidget* parent)
	: QWidget(parent)
{
	EventCategory::FindEventCategory(uuid, this->_category);
	this->initUi();
}

DataView::~DataView()
{
}

void DataView::initUi()
{
	ui.setupUi(this);

	this->_buttonGroup.addButton(this->ui.pushButton_chart, 0);
	this->_buttonGroup.addButton(this->ui.pushButton_data, 1);
	QObject::connect(&this->_buttonGroup, &QButtonGroup::idClicked, this->ui.stackedWidget_container, &QStackedWidget::setCurrentIndex);

	this->ui.label_name->setText(tr("关于 %1 事件").arg(this->_category.Name));
	this->ui.label_overview_name->setText(this->_category.Name);
	this->ui.label_overview_icon->setPixmap(QPixmap(QString(":/Animism/%1").arg(this->_category.Name)));
	this->ui.label_overview_name->setText(this->_category.Name);
	this->ui.label_overview_guid->setText(this->_category.Guid.toString(QUuid::Id128));
	this->ui.plainTextEdit_overview_description->setPlainText(this->_category.Description);

	this->_ptr_chart_opcode = new QChart();
	this->_ptr_chart_opcode->setTheme(QChart::ChartTheme::ChartThemeLight);
	this->_ptr_chart_opcode->setAnimationOptions(QChart::AnimationOption::SeriesAnimations);
	this->_ptr_chart_opcode->setContentsMargins(0, 0, 0, 0);
	this->_ptr_chart_opcode->layout()->setContentsMargins(0, 0, 0, 0);
	this->_ptr_chart_opcode->setBackgroundRoundness(0);
	this->_ptr_chart_opcode->legend()->hide();

	this->_ptr_series_opcode = new QPieSeries(this->_ptr_chart_opcode);
	this->_ptr_series_opcode->setPieSize(0.8);

	this->ui.graphicsView_opcode_frequency->setRenderHint(QPainter::Antialiasing, true);
	this->ui.graphicsView_opcode_frequency->setChart(this->_ptr_chart_opcode);
	this->ui.graphicsView_opcode_frequency->setContentsMargins(0, 0, 0, 0);

	this->ui.tableWidget_data->setAttribute(Qt::WA_Hover);
	this->ui.tableWidget_data->horizontalHeader()->setVisible(true);
	this->ui.tableWidget_data->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeMode::Fixed);
	this->ui.tableWidget_data->horizontalHeader()->setHighlightSections(false);
	this->ui.tableWidget_data->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeMode::Fixed);
	this->ui.tableWidget_data->verticalHeader()->setHighlightSections(false);
	this->ui.tableWidget_data->setShowGrid(false);
	this->ui.tableWidget_data->setAlternatingRowColors(true);
	this->ui.tableWidget_data->verticalHeader()->setDefaultSectionSize(25);

	this->ui.tableWidget_data->setColumnWidth(0, this->ui.tableWidget_data->viewport()->width() * 0.37);
	this->ui.tableWidget_data->setColumnWidth(1, this->ui.tableWidget_data->viewport()->width() * 0.16);
	this->ui.tableWidget_data->setColumnWidth(2, this->ui.tableWidget_data->viewport()->width() * 0.25);

	this->ui.widget_overview->installEventFilter(this);
	this->ui.tableWidget_data->installEventFilter(this);
	this->ui.page_empty->installEventFilter(this);

	this->initData();
}

void DataView::initData()
{
	this->_viewlock.store(false);

	auto _opcode_count = Reader.opcode_count(this->_category.Name);
	if (_opcode_count.isEmpty())
	{
		this->ui.stackedWidget_container->setCurrentIndex(2);
		this->ui.widget_navi->hide();
		return;
	}

	for (auto it = _opcode_count.begin(); it != _opcode_count.end(); it++)
	{
		QPieSlice* slice = this->_ptr_series_opcode->append(it.key(), it.value());
		_list_slice_opcode << slice;

		QObject::connect(slice, &QPieSlice::hovered, [=](bool state) {
			slice->setExploded(state);
			auto _time_pair = Reader.duration(this->_category.Name, slice->label());
			auto _ptr_tooltip = ToolTip::Instance(
				slice->label() + tr("事件"),
				tr("该事件于%1被首次捕捉，最后一次发生在%2，模型周期内共发生%3次。")
				.arg(_time_pair.first.toString("yyyy年MM月dd日hh时mm分ss秒"))
				.arg(_time_pair.second.toString("yyyy年MM月dd日hh时mm分ss秒"))
				.arg(slice->value())
			);
			_ptr_tooltip->setHidden(!state);
			});
		QObject::connect(slice, &QPieSlice::clicked, [=]() {
			auto _ptr_miniview = MiniOpcodeView::Instance(this->_category.Name, slice->label());
			_ptr_miniview->show();
			});
	}
	this->_ptr_chart_opcode->addSeries(this->_ptr_series_opcode);

	QFutureWatcher<QList<qint64>>* watcher = new QFutureWatcher<QList<qint64>>(this);
	QObject::connect(watcher, &QFutureWatcher<QList<qint64>>::finished, this, [=]() {
		auto _event_duration = Reader.duration();
		qint64 _event_start_secs = _event_duration.first.toSecsSinceEpoch();
		this->ui.widget_time_frequency->setRange(_event_start_secs, _event_start_secs + _event_duration.second / 1000);
		this->ui.widget_time_frequency->insert(watcher->result());
		QObject::connect(this->ui.widget_time_frequency, &ScatterDiagram::drag, this, [=](qreal min, qreal max) {
			auto _duration = Reader.duration().second;
			this->ui.label_time_frequency_area->setText(tr("从%1秒至%2秒")
				.arg(QString::number(_duration / 1000.0 * min / 100.0, 'f', 0))
				.arg(QString::number(_duration / 1000.0 * max / 100.0, 'f', 0))
			);
			});
		QObject::connect(this->ui.widget_time_frequency, &ScatterDiagram::drop, this, [=](qreal min, qreal max) {
			this->ui.label_time_frequency_area->clear();
			if (min == max)
				return;
			auto _duration = Reader.duration();
			QDateTime _timestamp_from = _duration.first.addMSecs(_duration.second * min / 100.0);
			QDateTime _timestamp_to = _duration.first.addMSecs(_duration.second * max / 100.0);
			auto _ptr_miniview = MiniDurationView::Instance(this->_category.Name, _timestamp_from, _timestamp_to);
			_ptr_miniview->show();
			});
		watcher->deleteLater();
		});
	watcher->setFuture(Reader.event_timestamp(this->_category.Name));

	this->ui.label_data_count->setText(tr("共计%1条").arg(Reader.event_count(this->_category.Name)));

	this->_ptr_watcher_data = new QFutureWatcher<QList<EventInfo>>(this);
	QObject::connect(this->_ptr_watcher_data, &QFutureWatcher<QList<EventInfo>>::finished, this, [=]() {
		this->_viewlock.store(true);
		auto _data = this->_ptr_watcher_data->result();
		if (_data.isEmpty())
		{
			this->_viewlock.store(false);
			return;
		}
		int _current_row = this->ui.tableWidget_data->rowCount();
		this->ui.tableWidget_data->setRowCount(_current_row + _data.size());
		for (auto _value : _data)
		{
			QTableWidgetItem* _item_timestamp = new QTableWidgetItem(_value.Timestamp.toString(Qt::DateFormat::ISODateWithMs));
			QTableWidgetItem* _item_task = new QTableWidgetItem(_value.TaskName);
			QTableWidgetItem* _item_opcode = new QTableWidgetItem(_value.OpcodeName);
			QTableWidgetItem* _item_thread = new QTableWidgetItem(QString::number(_value.ThreadId));

			_item_timestamp->setTextAlignment(Qt::AlignCenter);
			_item_task->setTextAlignment(Qt::AlignCenter);
			_item_opcode->setTextAlignment(Qt::AlignCenter);
			_item_thread->setTextAlignment(Qt::AlignCenter);

			this->ui.tableWidget_data->setItem(_current_row, 0, _item_timestamp);
			this->ui.tableWidget_data->setItem(_current_row, 1, _item_task);
			this->ui.tableWidget_data->setItem(_current_row, 2, _item_opcode);
			this->ui.tableWidget_data->setItem(_current_row++, 3, _item_thread);

			this->_list_data_param.append(QJsonDocument(QJsonObject::fromVariantMap(_value.Properties)).toJson());

			QApplication::processEvents();
		}
		this->ui.tableWidget_data->setColumnWidth(0, this->ui.tableWidget_data->viewport()->width() * 0.37);
		this->ui.tableWidget_data->setColumnWidth(1, this->ui.tableWidget_data->viewport()->width() * 0.16);
		this->ui.tableWidget_data->setColumnWidth(2, this->ui.tableWidget_data->viewport()->width() * 0.25);
		this->_viewlock.store(false);
		});

	auto _ptr_scrollbar = this->ui.tableWidget_data->verticalScrollBar();
	QObject::connect(_ptr_scrollbar, &QScrollBar::valueChanged, this, [=](int value) {
		if (value == _ptr_scrollbar->maximum())
			this->loadData();
		});

	QObject::connect(this->ui.tableWidget_data, &QTableWidget::itemSelectionChanged, this, [=]() {
		int index = this->ui.tableWidget_data->currentRow();
		if (index < 0)
			return;
		this->ui.plainTextEdit_param->setPlainText(this->_list_data_param.at(index));
		});

	QObject::connect(this->ui.tableWidget_data, &QTableWidget::itemClicked, this, [=](QTableWidgetItem* item) {
		this->ui.plainTextEdit_param->setPlainText(this->_list_data_param.at(item->row()));
		});

	QObject::connect(this->ui.pushButton_data_copy, &QPushButton::clicked, this, [=]() {
		auto _items = this->ui.tableWidget_data->selectedItems();
		if (_items.isEmpty())
		{
			MessageDialog::MessageDialog(this, tr("提示"), tr("未选择或空数据")).exec();
			return;
		}
		QStringList _strlist_text;
		_strlist_text
			<< "Timestamp = " + _items.at(0)->text()
			<< "Task = " + _items.at(1)->text()
			<< "Opcode = " + _items.at(2)->text()
			<< "Thread = " + _items.at(3)->text()
			<< "Properties = " + this->_list_data_param.at(_items.at(0)->row());
		QApplication::clipboard()->setText(_strlist_text.join('\n'));
		MessageDialog::MessageDialog(this, tr("提示"), tr("信息已复制")).exec();
		});

	QObject::connect(this->ui.pushButton_data_output, &QPushButton::clicked, [=]() {
		MessageDialog::MessageDialog(this, tr("提示"), tr("功能开发中")).exec();
		// this->_ptr_reader->event_output(this->_category.Name, "C:\\Users\\SiyerBOBO\\Desktop\\123.txt");
		});

	QObject::connect(this->ui.pushButton_data_top, &QPushButton::clicked, this, [=]() {
		this->ui.tableWidget_data->selectRow(0);
		this->ui.tableWidget_data->scrollTo(this->ui.tableWidget_data->selectionModel()->currentIndex());
		});

	QObject::connect(this->ui.pushButton_data_current, &QPushButton::clicked, this, [=]() {
		this->ui.tableWidget_data->scrollTo(this->ui.tableWidget_data->selectionModel()->currentIndex());
		});

	QObject::connect(this->ui.pushButton_data_previous, &QPushButton::clicked, this, [=]() {
		this->ui.tableWidget_data->selectRow(this->ui.tableWidget_data->selectionModel()->currentIndex().row() - 1);
		this->ui.tableWidget_data->scrollTo(this->ui.tableWidget_data->selectionModel()->currentIndex());
		});

	QObject::connect(this->ui.pushButton_data_next, &QPushButton::clicked, this, [=]() {
		this->ui.tableWidget_data->selectRow(this->ui.tableWidget_data->selectionModel()->currentIndex().row() + 1);
		this->ui.tableWidget_data->scrollTo(this->ui.tableWidget_data->selectionModel()->currentIndex());
		});

	this->loadData();
}

void DataView::loadData()
{
	if (this->_ptr_watcher_data->isRunning())
		return;
	if (this->_list_data_param.count() == Reader.event_count(this->_category.Name))
		return;
	this->_ptr_watcher_data->setFuture(
		Reader.find_event(
			this->_category.Name,
			this->ui.tableWidget_data->rowCount(),
			256
		));
}

bool DataView::event(QEvent* e)
{
	switch (e->type())
	{
	case QEvent::KeyPress:
	{
		QKeyEvent* _key_event = (QKeyEvent*)e;
		if (1 == this->ui.stackedWidget_container->currentIndex())
		{
			switch (_key_event->key())
			{
			case Qt::Key::Key_Up:
			{
				int _row = this->ui.tableWidget_data->selectionModel()->currentIndex().row() - 1;
				_row = _row < 0 ? 0 : _row;
				this->ui.tableWidget_data->selectRow(_row);
				this->ui.tableWidget_data->scrollTo(this->ui.tableWidget_data->selectionModel()->currentIndex());
				break;
			}
			case Qt::Key::Key_Down:
			{
				if (this->_viewlock.load())
					break;
				int _row = this->ui.tableWidget_data->selectionModel()->currentIndex().row() + 1;
				_row = _row >= this->ui.tableWidget_data->rowCount() ? this->ui.tableWidget_data->rowCount() - 1 : _row;
				this->ui.tableWidget_data->selectRow(_row);
				this->ui.tableWidget_data->scrollTo(this->ui.tableWidget_data->selectionModel()->currentIndex());
				break;
			}
			case Qt::Key::Key_PageUp:
			{
				int _row = this->ui.tableWidget_data->selectionModel()->currentIndex().row() - 10;
				_row = _row < 0 ? 0 : _row;
				this->ui.tableWidget_data->selectRow(_row);
				this->ui.tableWidget_data->scrollTo(this->ui.tableWidget_data->selectionModel()->currentIndex());
				break;
			}
			case Qt::Key::Key_PageDown:
			{
				if (this->_viewlock.load())
					break;
				int _row = this->ui.tableWidget_data->selectionModel()->currentIndex().row() + 10;
				_row = _row >= this->ui.tableWidget_data->rowCount() ? this->ui.tableWidget_data->rowCount() - 1 : _row;
				this->ui.tableWidget_data->selectRow(_row);
				this->ui.tableWidget_data->scrollTo(this->ui.tableWidget_data->selectionModel()->currentIndex());
				break;
			}
			default:
				break;
			}
		}
		break;
	}
	default:
		break;
	}
	return QWidget::event(e);
}

bool DataView::eventFilter(QObject* obj, QEvent* event)
{
	if (this->ui.widget_overview == obj)
	{
		QWidget* _ptr_obj = qobject_cast<QWidget*>(obj);
		switch (event->type())
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
	else if (this->ui.tableWidget_data == obj)
	{
		QTableView* _ptr_obj = qobject_cast<QTableView*>(obj);
		switch (event->type())
		{
		case QEvent::Show:
		case QEvent::Resize:
		{
			this->ui.tableWidget_data->setColumnWidth(0, this->ui.tableWidget_data->viewport()->width() * 0.37);
			this->ui.tableWidget_data->setColumnWidth(1, this->ui.tableWidget_data->viewport()->width() * 0.16);
			this->ui.tableWidget_data->setColumnWidth(2, this->ui.tableWidget_data->viewport()->width() * 0.25);
			break;
		}
		default:
			break;
		}
	}
	else if (this->ui.page_empty == obj)
	{
		QWidget* _ptr_obj = qobject_cast<QWidget*>(obj);
		switch (event->type())
		{
		case QEvent::Paint:
		{
			QPainter painter(_ptr_obj);
			painter.setRenderHints(QPainter::Antialiasing);
			painter.setPen(Qt::NoPen);
			painter.setBrush(Qt::lightGray);
			painter.drawRect(_ptr_obj->rect());

			QFont font("Microsoft YaHei");
			font.setPixelSize(23);
			painter.setFont(font);
			painter.setPen(Qt::white);
			painter.setBrush(Qt::NoBrush);
			painter.drawText(_ptr_obj->rect(), Qt::AlignCenter, tr("无事件"));
			break;
		}
		default:
			break;
		}
	}
	return QWidget::eventFilter(obj, event);
}
