#pragma execution_character_set("utf-8")
#include "MiniOpcodeView.h"
#include <QPainter>
#include <QPainterPath>
#include <QClipBoard>
#include <QtMath>
#include <QScrollBar>
#include "ModelReaderV2.h"
#include "Utils.h"
#include "MessageDialog.h"

MiniOpcodeView::MiniOpcodeView(QWidget *parent)
	: QWidget(parent)
{
	this->initUi();
}

void MiniOpcodeView::initUi()
{
	ui.setupUi(this);
	this->setAttribute(Qt::WA_TranslucentBackground);
	// this->setAttribute(Qt::WA_TransparentForMouseEvents);
	this->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::Tool);

	this->ui.tableWidget_data->setAttribute(Qt::WA_Hover);
	this->ui.tableWidget_data->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeMode::Fixed);
	this->ui.tableWidget_data->horizontalHeader()->setHighlightSections(false);
	this->ui.tableWidget_data->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeMode::Fixed);
	this->ui.tableWidget_data->verticalHeader()->setHighlightSections(false);
	this->ui.tableWidget_data->setShowGrid(false);
	this->ui.tableWidget_data->setAlternatingRowColors(true);
	this->ui.tableWidget_data->verticalHeader()->setDefaultSectionSize(25);

	auto _ptr_scrollbar = this->ui.tableWidget_data->verticalScrollBar();
	QObject::connect(_ptr_scrollbar, &QScrollBar::valueChanged, this, [=](int value) {
		if (value == _ptr_scrollbar->maximum())
			this->loadData();
		});

	QObject::connect(this->ui.tableWidget_data, &QTableWidget::itemSelectionChanged, this, [=]() {
		int index = this->ui.tableWidget_data->currentRow();
		if (index < 0)
			return;
		this->ui.plainTextEdit_param->setPlainText(this->_list_param.at(index).simplified());
		});

	QObject::connect(this->ui.tableWidget_data, &QTableWidget::itemClicked, this, [=](QTableWidgetItem* item) {
		this->ui.plainTextEdit_param->setPlainText(this->_list_param.at(item->row()).simplified());
		});

	QObject::connect(this->ui.pushButton_data_copy, &QPushButton::clicked, this, [=]() {
		auto _items = this->ui.tableWidget_data->selectedItems();
		if (_items.isEmpty())
			return;
		QStringList _strlist_text;
		_strlist_text
			<< "Timestamp = " + _items.at(0)->text()
			<< "Task = " + _items.at(1)->text()
			<< "Opcode = " + _items.at(2)->text()
			<< "Thread = " + _items.at(3)->text()
			<< "Properties = " + this->_list_param.at(_items.at(0)->row());
		QApplication::clipboard()->setText(_strlist_text.join('\n'));
		this->ui.plainTextEdit_param->setPlainText(tr("信息已复制\n") + this->_list_param.at(_items.at(0)->row()).simplified());
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
}

bool MiniOpcodeView::event(QEvent* e)
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
	case QEvent::Show:
	{
		Utils::CenterUi(this);
		Utils::ActiveUi(this);
		this->ui.tableWidget_data->setColumnWidth(0, this->ui.tableWidget_data->viewport()->width() * 0.37);
		this->ui.tableWidget_data->setColumnWidth(1, this->ui.tableWidget_data->viewport()->width() * 0.16);
		this->ui.tableWidget_data->setColumnWidth(2, this->ui.tableWidget_data->viewport()->width() * 0.25);
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
		// this->update();
		this->hide();
		break;
	}
	default:
		break;
	}
	return QWidget::event(e);
}

void MiniOpcodeView::loadData()
{
	if (this->_ptr_watcher_event.isNull() || this->_ptr_watcher_event->isRunning())
		return;
	this->_ptr_watcher_event->setFuture(
		Reader.find_event(
			this->_str_task_name, 
			this->_str_opcode_name,
			this->ui.tableWidget_data->rowCount(),
			256
		));
}

void MiniOpcodeView::reset()
{
	if (this->_ptr_watcher_event)
		this->_ptr_watcher_event->deleteLater();
	this->_ptr_watcher_event = new QFutureWatcher<QList<EventInfo>>(_spointer_instance.get());
	QObject::connect(this->_ptr_watcher_event, &QFutureWatcher<QList<EventInfo>>::finished, this, [=]() {
		auto _data = this->_ptr_watcher_event->result();
		if (_data.isEmpty())
			return;
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

			this->_list_param.append(QJsonDocument(QJsonObject::fromVariantMap(_value.Properties)).toJson());

			QApplication::processEvents();
		}
		});
	this->ui.tableWidget_data->clearContents();
	this->ui.tableWidget_data->setRowCount(0);
	this->ui.plainTextEdit_param->clear();
	this->_list_param.clear();
	this->ui.label_data_count->setText(tr("共计%1条事件").arg(Reader.opcode_count(this->_str_task_name, this->_str_opcode_name)));
	this->ui.label_data_duration->setText(tr("%1/%2类").arg(this->_str_task_name).arg(this->_str_opcode_name));
	this->loadData();
}

std::unique_ptr<MiniOpcodeView> MiniOpcodeView::_spointer_instance;
MiniOpcodeView* MiniOpcodeView::Instance(const QString& task_name, const QString& opcode_name)
{
	static std::once_flag oc;
	std::call_once(oc, [&] {  _spointer_instance.reset(new MiniOpcodeView()); });
	_spointer_instance->_str_task_name = task_name;
	_spointer_instance->_str_opcode_name = opcode_name;
	_spointer_instance->reset();
	return _spointer_instance.get();
}

MiniOpcodeView::~MiniOpcodeView()
{
}
