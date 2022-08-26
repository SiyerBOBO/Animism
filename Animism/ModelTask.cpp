#include "EventTrapper.h"
#include "Utils.h"
#include "./animism_engine/parser.hpp"
#include "./animism_engine/kernel_guids.hpp"
#include "ModelTask.h"
#include "ModelPool.h"
#include <QMap>
#include <QMultiMap>
#include <QDebug>
#include <QTimer>
#include <QMutexLocker>
#include <QVariant>
#include <QApplication>
#include <QStandardPaths>
#include <QDir>

#include <QSql>
#include <QSqlDriver>
#include <QSqlError>
#include <QSqlQuery>

#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonValue>
#include <QJsonObject>

#include <QBuffer>
#include <QCryptographicHash>
#include <QtConcurrent>
#include <QDebug>

ModelTask::ModelTask(const Model& model, QObject* parent)
	: QThread(parent), _model(model)
{
	this->_ull_process_count.store(0);
	this->_ull_thread_count.store(0);
	this->_ull_imageload_count.store(0);
	this->_ull_fileio_count.store(0);
	this->_ull_registry_count.store(0);
	this->_ull_tcpip_count.store(0);
	this->_ull_udpip_count.store(0);
}

ModelTask::~ModelTask()
{
	this->callStop();
}

void ModelTask::save_event()
{
	if (this->_list_record[this->_bool_point].isEmpty())
		return;

	this->_bool_point = !this->_bool_point;

	QList<EventInfo> _list_buffer;
	_list_buffer.append(this->_list_record[!this->_bool_point]);
	this->_list_record[!this->_bool_point].clear();

	QDataStream io(this->file_event);
	io.setVersion(1);

	for (QList<EventInfo>::iterator it = _list_buffer.begin(); it != _list_buffer.end(); it++)
	{
		io
			<< it->Timestamp
			<< it->TaskName
			<< it->OpcodeName
			<< it->Opcode
			<< it->ProcessId
			<< it->ThreadId
			<< it->Properties;
	}
	this->file_event->flush();
}

void ModelTask::save_usage()
{
	if (_bool_pause)
		return;

	auto sys_data = this->_ptr_usage_helper->getData();

	QDataStream io(this->file_usage);
	io.setVersion(1);
	io 
		<< QDateTime::currentDateTime()
		<< sys_data.first
		<< sys_data.second;
	this->file_usage->flush();

	emit this->monitor(sys_data.first * 100, sys_data.second);
	emit this->counter(
		this->_ull_process_count,
		this->_ull_thread_count,
		this->_ull_imageload_count,
		this->_ull_fileio_count,
		this->_ull_tcpip_count,
		this->_ull_udpip_count,
		this->_ull_registry_count
	);
}

void ModelTask::save_model()
{
	_model.Duration = _model.Timestamp.msecsTo(QDateTime::currentDateTime());

	QDataStream io(file_info);
	io.setVersion(1);

	QByteArray _data_icon;
	QBuffer _buffer_icon(&_data_icon);
	_buffer_icon.open(QIODevice::WriteOnly);
	_model.Process.Icon.save(&_buffer_icon, "png");

	QString _data_focus;
	QStringList _buffer_focus;
	for (auto& uuid : _model.Focus)
		_buffer_focus.append(uuid.toString(QUuid::StringFormat::WithoutBraces));
	_data_focus = _buffer_focus.join('&');
	io
		<< _model.Uuid
		<< _model.Timestamp
		<< _model.Duration
		<< _data_focus
		<< _model.Process.Pid
		<< _model.Process.Name
		<< _model.Process.FilePath
		<< _data_icon
		<< _model.Process.Md5;

	this->file_info->flush();
}

Model ModelTask::model() const
{
	return this->_model;
}

void ModelTask::callStop(unsigned long time)
{
	if (!this->isRunning())
		return;
	emit this->stop();
	if (this->wait(time))
		return;
	this->terminate();
}

ModelTask::ErrorType ModelTask::lastError()
{
	return this->_error;
}

void ModelTask::run()
{
	this->_bool_pause = false;
	if (!this->initFile())
	{
		this->_error = ErrorType::FileSysError;
		return;
	}
	if (!this->initTask())
	{
		this->_error = ErrorType::TrapperError;
		return;
	}
	QEventLoop loop;
	QObject::connect(this, &ModelTask::stop, &loop, &QEventLoop::quit);
	QTimer _usage_timer, _event_timer;
	QObject::connect(&_usage_timer, &QTimer::timeout, this, &ModelTask::save_usage);
	QObject::connect(&_event_timer, &QTimer::timeout, this, &ModelTask::save_event, Qt::DirectConnection);
	_usage_timer.start(1000);
	_event_timer.start(500);
	loop.exec();
	_bool_pause = true;
	_usage_timer.stop();
	_event_timer.stop();
	this->beforeFinish();
	this->save_event();
	this->save_model();
}

bool ModelTask::initFile()
{
	if (!Pool.contains(this->_model.Uuid))
		return false;

	this->file_info = Pool.file_info(this->_model.Uuid);
	this->file_usage = Pool.file_usage(this->_model.Uuid);
	this->file_event = Pool.file_event(this->_model.Uuid);

	if (!this->file_info || !this->file_usage || !this->file_event)
		return false;
	return true;
}

bool ModelTask::initTask()
{
	_ptr_usage_helper.reset(new ProcessUsageHelper(_model.Process.Pid));
	for (auto& e : this->_model.Focus)
	{
		if (e == animism_engine::guids::process)
			QObject::connect(&Trapper, &EventTrapper::processCaptured, this, &ModelTask::onProcessCaptured);
		else if (e == animism_engine::guids::thread)
			QObject::connect(&Trapper, &EventTrapper::threadCaptured, this, &ModelTask::onThreadCaptured);
		else if (e == animism_engine::guids::image_load)
			QObject::connect(&Trapper, &EventTrapper::imageloadCaptured, this, &ModelTask::onImageloadCaptured);
		else if (e == animism_engine::guids::registry)
			QObject::connect(&Trapper, &EventTrapper::registryCaptured, this, &ModelTask::onRegistryCaptured);
		else if (e == animism_engine::guids::file_io)
			QObject::connect(&Trapper, &EventTrapper::fileioCaptured, this, &ModelTask::onFileioCaptured);
		else if (e == animism_engine::guids::tcp_ip)
			QObject::connect(&Trapper, &EventTrapper::tcpipCaptured, this, &ModelTask::onTcpipCaptured);
		else if (e == animism_engine::guids::udp_ip)
			QObject::connect(&Trapper, &EventTrapper::udpipCaptured, this, &ModelTask::onUdpipCaptured);
	}
	return true;
}

void ModelTask::beforeFinish()
{
	QObject::disconnect(&Trapper, &EventTrapper::processCaptured, this, &ModelTask::onProcessCaptured);
	QObject::disconnect(&Trapper, &EventTrapper::threadCaptured, this, &ModelTask::onThreadCaptured);
	QObject::disconnect(&Trapper, &EventTrapper::imageloadCaptured, this, &ModelTask::onImageloadCaptured);
	QObject::disconnect(&Trapper, &EventTrapper::registryCaptured, this, &ModelTask::onRegistryCaptured);
	QObject::disconnect(&Trapper, &EventTrapper::fileioCaptured, this, &ModelTask::onFileioCaptured);
	QObject::disconnect(&Trapper, &EventTrapper::tcpipCaptured, this, &ModelTask::onTcpipCaptured);
	QObject::disconnect(&Trapper, &EventTrapper::udpipCaptured, this, &ModelTask::onUdpipCaptured);
}

void ModelTask::onProcessCaptured(const EventInfo& info)
{
	if (this->_bool_pause || info.ProcessId != this->_model.Process.Pid)
		return;
	this->_list_record[_bool_point].append(info);
	this->_ull_process_count.fetch_add(1);
}

void ModelTask::onThreadCaptured(const EventInfo& info)
{
	if (this->_bool_pause || info.ProcessId != this->_model.Process.Pid)
		return;
	this->_list_record[_bool_point].append(info);
	this->_ull_thread_count.fetch_add(1);
}

void ModelTask::onImageloadCaptured(const EventInfo& info)
{
	if (this->_bool_pause || info.ProcessId != this->_model.Process.Pid)
		return;
	this->_list_record[_bool_point].append(info);
	this->_ull_imageload_count.fetch_add(1);
}

void ModelTask::onFileioCaptured(const EventInfo& info)
{
	if (this->_bool_pause || info.ProcessId != this->_model.Process.Pid)
		return;
	this->_list_record[_bool_point].append(info);
	this->_ull_fileio_count.fetch_add(1);
}

void ModelTask::onRegistryCaptured(const EventInfo& info)
{
	if (this->_bool_pause || info.ProcessId != this->_model.Process.Pid)
		return;
	this->_list_record[_bool_point].append(info);
	this->_ull_registry_count.fetch_add(1);
}

void ModelTask::onTcpipCaptured(const EventInfo& info)
{
	if (this->_bool_pause || info.ProcessId != this->_model.Process.Pid)
		return;
	this->_list_record[_bool_point].append(info);
	this->_ull_tcpip_count.fetch_add(1);
}

void ModelTask::onUdpipCaptured(const EventInfo& info)
{
	if (this->_bool_pause || info.ProcessId != this->_model.Process.Pid)
		return;
	this->_list_record[_bool_point].append(info);
	this->_ull_udpip_count.fetch_add(1);
}
