#pragma execution_character_set("utf-8")
#include "ModelPool.h"
#include <mutex>
#include <QFileInfo>
#include <QStandardPaths>
#include <QDataStream>
#include <QSqlError>
#include <QSqlQuery>
#include <QDebug>

std::unique_ptr<ModelPool> ModelPool::_spointer_instance;
ModelPool* ModelPool::Instance() {
	static std::once_flag oc;
	std::call_once(oc, [&] {  _spointer_instance.reset(new ModelPool); });
	return _spointer_instance.get();
}

ModelPool::ModelPool(QObject *parent)
	: QObject(parent)
{
	this->initData();
}

void ModelPool::initData()
{

}

void ModelPool::saveTask(const QUuid& _uuid)
{
	try
	{
		if (!this->_hash_uuid2package.contains(_uuid))
		{
			emit this->result_of_save(_uuid, false);
			return;
		}
		auto& _pkg = this->_hash_uuid2package[_uuid];
		if (_pkg.Over || !_pkg.Info->isOpen() || !_pkg.Usage->isOpen() || !_pkg.Event->isOpen() || !_pkg.Database.isOpen())
		{
			emit this->result_of_save(_uuid, false);
			return;
		}
		if (0 == _pkg.Info->size())
		{
			emit this->result_of_save(_uuid, false);
			return;
		}

		if (!_pkg.Database.transaction())
		{
			emit this->result_of_save(_uuid, false);
			return;
		}

		qulonglong _ull_size_total = _pkg.Info->size() + _pkg.Usage->size() + _pkg.Event->size();

		QSqlQuery _query(_pkg.Database);
		_query.exec("PRAGMA synchronous = OFF; ");
		_pkg.Info->seek(0);
		_pkg.Usage->seek(0);
		_pkg.Event->seek(0);

		// Load Info
		QDataStream r1(_pkg.Info);
		r1.setVersion(1);
		Model _model;
		QByteArray _data_icon;
		QString _data_focus;
		r1
			>> _model.Uuid
			>> _model.Timestamp
			>> _model.Duration
			>> _data_focus
			>> _model.Process.Pid
			>> _model.Process.Name
			>> _model.Process.FilePath
			>> _data_icon
			>> _model.Process.Md5;
		_query.prepare("replace into \"model\" values(:_uuid, :_timestamp, :_duration, :_focus, :_pid, :_pname, :_ppath, :_picon, :_pmd5);");
		_query.bindValue(":_uuid", _model.Uuid);
		_query.bindValue(":_timestamp", _model.Timestamp);
		_query.bindValue(":_duration", _model.Duration);
		_query.bindValue(":_focus", _data_focus);
		_query.bindValue(":_pid", _model.Process.Pid);
		_query.bindValue(":_pname", _model.Process.Name);
		_query.bindValue(":_ppath", _model.Process.FilePath);
		_query.bindValue(":_picon", _data_icon);
		_query.bindValue(":_pmd5", _model.Process.Md5);
		if (!_query.exec())
		{
			emit this->result_of_save(_uuid, false);
			return;
		}
		emit this->progress_of_save(_uuid, ((_pkg.Info->pos() + _pkg.Usage->pos() + _pkg.Event->pos()) * 100.0) / _ull_size_total);

		// Load Usage
		QDataStream r2(_pkg.Usage);
		r2.setVersion(1);
		while (!_pkg.Usage->atEnd())
		{
			Usage _usage;
			r2
				>> _usage.Timestamp
				>> _usage.Cpu
				>> _usage.Ram;

			_query.prepare("replace into sys_data (_timestamp, _cpu_usage, _ram_usage) values(:_timestamp, :_cpu_usage, :_ram_usage)");
			_query.bindValue(":_timestamp", _usage.Timestamp);
			_query.bindValue(":_cpu_usage", _usage.Cpu);
			_query.bindValue(":_ram_usage", _usage.Ram);
			if (!_query.exec())
			{
				emit this->result_of_save(_uuid, false);
				return;
			}
			emit this->progress_of_save(_uuid, ((_pkg.Info->pos() + _pkg.Usage->pos() + _pkg.Event->pos()) * 100.0) / _ull_size_total);
		}

		QHash<QString, EventCounter> _hash_task2counter;
		auto _categories = EventCategory::GetEventCategoryMap();
		for (auto _category : _categories)
			_hash_task2counter.insert(_category.Name, { _category.Name });

		// Load Event
		QDataStream r3(_pkg.Event);
		r3.setVersion(1);
		while (!_pkg.Event->atEnd())
		{
			EventInfo _event;
			r3
				>> _event.Timestamp
				>> _event.TaskName
				>> _event.OpcodeName
				>> _event.Opcode
				>> _event.ProcessId
				>> _event.ThreadId
				>> _event.Properties;

			_query.prepare("replace into proc_data (_timestamp, _task_name, _opcode_name, _opcode, _pid, _tid, _properties) values(:_timestamp, :_task_name, :_opcode_name, :_opcode, :_pid, :_tid, :_properties);");
			_query.bindValue(":_timestamp", _event.Timestamp);
			_query.bindValue(":_task_name", _event.TaskName);
			_query.bindValue(":_opcode_name", _event.OpcodeName);
			_query.bindValue(":_opcode", _event.Opcode);
			_query.bindValue(":_pid", _event.ProcessId);
			_query.bindValue(":_tid", _event.ThreadId);
			_query.bindValue(":_properties", QJsonDocument(QJsonObject::fromVariantMap(_event.Properties)).toJson());
			if (!_query.exec())
			{
				emit this->result_of_save(_uuid, false);
				return;
			}

			_hash_task2counter[_event.TaskName].add(_event.OpcodeName, _event.Timestamp);
			emit this->progress_of_save(_uuid, ((_pkg.Info->pos() + _pkg.Usage->pos() + _pkg.Event->pos()) * 100.0) / _ull_size_total);
		}

		for (auto it = _hash_task2counter.begin(); it != _hash_task2counter.end(); it++)
		{
			auto opcode_names = it->OpcodeNameToCount.keys();
			for (auto& opcode_name : opcode_names)
			{
				_query.prepare("replace into pre_data values(:_task_name, :_opcode_name, :_count, :_timestamp_start, :_timestamp_finish);");
				_query.bindValue(":_task_name", it->TaskName);
				_query.bindValue(":_opcode_name", opcode_name);
				_query.bindValue(":_count", it->OpcodeNameToCount[opcode_name]);
				_query.bindValue(":_timestamp_start", it->OpcodeNameToTimeStamp[opcode_name].first);
				_query.bindValue(":_timestamp_finish", it->OpcodeNameToTimeStamp[opcode_name].second);
				if (!_query.exec())
				{
					emit this->result_of_save(_uuid, false);
					return;
				}
			}
		}

		if (!_pkg.Database.commit())
		{
			_pkg.Database.rollback();
			emit this->result_of_save(_uuid, false);
			return;
		}

		_pkg.Over = true;
		emit this->result_of_save(_uuid, true);
	}
	catch (...)
	{
		emit this->result_of_save(_uuid, false);
		return;
	}
}

void ModelPool::cleanUnfinished()
{
	if (this->_hash_uuid2package.isEmpty())
		return;
	for (auto& _pkg : this->_hash_uuid2package)
	{
		_pkg.Database.close();
		_pkg.Info->close();
		_pkg.Info->deleteLater();
		_pkg.Usage->close();
		_pkg.Usage->deleteLater();
		_pkg.Event->close();
		_pkg.Event->deleteLater();
		QFile::remove(_pkg.FilePath);
	}
	this->_hash_uuid2package.clear();
}

ModelPool::~ModelPool()
{
	
}

bool ModelPool::create(const Model& _model, const QString& _filepath)
{
	if (this->_hash_uuid2package.contains(_model.Uuid))
		return false;

	QFileInfo _fileinfo(_filepath);

	for (auto& _pkg : this->_hash_uuid2package)
	{
		if (_pkg.FilePath == _fileinfo.absoluteFilePath())
			return false;
	}

	if (_fileinfo.exists() && _fileinfo.isFile())
		if (!QFile::remove(_filepath))
			return false;

	Package _pkg;

	_pkg.FilePath = _fileinfo.absoluteFilePath();
	_pkg.Database = QSqlDatabase::addDatabase("SQLITECIPHER", _model.Uuid.toString(QUuid::Id128));
	// _pkg.Database = QSqlDatabase::addDatabase("QSQLITE", _model.Uuid.toString(QUuid::Id128));
	_pkg.Database.setDatabaseName(_fileinfo.absoluteFilePath());
	_pkg.Database.setConnectOptions("QSQLITE_CREATE_KEY");
	_pkg.Database.setPassword(QCryptographicHash::hash(qApp->organizationName().toLatin1().toBase64(), QCryptographicHash::Algorithm::Md5));
	if (!_pkg.Database.open())
		return false;
	
	QSqlQuery _query(_pkg.Database);
	bool _isSuc = true;
	_isSuc &= _query.exec("CREATE TABLE IF NOT EXISTS \"model\"( \"_uuid\" TEXT NOT NULL, \"_timestamp\" DATETIME NOT NULL, \"_duration\" INTEGER NOT NULL, \"_focus\" TEXT NOT NULL, \"_pid\" INTEGER NOT NULL, \"_pname\" TEXT NOT NULL, \"_ppath\" TEXT, \"_picon\" BLOB, \"_pmd5\" TEXT NOT NULL, PRIMARY KEY(\"_uuid\"))");
	_isSuc &= _query.exec("CREATE TABLE IF NOT EXISTS \"proc_data\"( \"_index\" INTEGER UNIQUE, \"_timestamp\" DATETIME NOT NULL, \"_task_name\" TEXT NOT NULL, \"_opcode_name\" TEXT NOT NULL, \"_opcode\" INTEGER NOT NULL, \"_pid\" INTEGER NOT NULL, \"_tid\" INTEGER NOT NULL, \"_properties\" JSON NOT NULL, PRIMARY KEY(\"_index\" AUTOINCREMENT))");
	_isSuc &= _query.exec("CREATE TABLE IF NOT EXISTS \"sys_data\"( \"_index\" INTEGER UNIQUE,  \"_timestamp\" DATETIME NOT NULL, \"_cpu_usage\" DOUBLE NOT NULL, \"_ram_usage\" INTEGER NOT NULL, PRIMARY KEY(\"_index\" AUTOINCREMENT))");
	_isSuc &= _query.exec("CREATE TABLE IF NOT EXISTS \"pre_data\" ( \"_task_name\" TEXT NOT NULL, \"_opcode_name\" TEXT NOT NULL, \"_count\" INTEGER NOT NULL, \"_timestamp_start\" DATETIME NOT NULL, \"_timestamp_finish\" DATETIME NOT NULL)");
	if (!_isSuc)
		return false;

	_pkg.Info = new QTemporaryFile();
	_pkg.Info->setAutoRemove(true);
	_pkg.Info->open();
	
	_pkg.Usage = new QTemporaryFile();
	_pkg.Usage->setAutoRemove(true);
	_pkg.Usage->open();
	
	_pkg.Event = new QTemporaryFile();
	_pkg.Event->setAutoRemove(true);
	_pkg.Event->open();

	this->_hash_uuid2package.insert(_model.Uuid, _pkg);
	return true;
}

void ModelPool::save(const QUuid& _uuid)
{
	if (!this->_hash_uuid2package.contains(_uuid))
		return;
	this->_hash_uuid2package[_uuid].Task = QtConcurrent::run(this, &ModelPool::saveTask, _uuid);
}

bool ModelPool::release(const QUuid& _uuid)
{
	if (!this->_hash_uuid2package.contains(_uuid))
		return false;
	auto& _pkg = this->_hash_uuid2package[_uuid];
	_pkg.Task.cancel();
	_pkg.Database.close();
	_pkg.Info->close();
	_pkg.Info->deleteLater();
	_pkg.Usage->close();
	_pkg.Usage->deleteLater();
	_pkg.Event->close();
	_pkg.Event->deleteLater();
	if (!_pkg.Over)
		QFile::remove(_pkg.FilePath);
	this->_hash_uuid2package.remove(_uuid);
	return true;
}

bool ModelPool::contains(const QUuid& _uuid)
{
	return this->_hash_uuid2package.contains(_uuid);
}

bool ModelPool::is_over(const QUuid& _uuid)
{
	if (!this->_hash_uuid2package.contains(_uuid))
		return false;
	return this->_hash_uuid2package.value(_uuid).Over;
}

QTemporaryFile* ModelPool::file_info(const QUuid& _uuid)
{
	if (!this->_hash_uuid2package.contains(_uuid))
		return nullptr;
	return this->_hash_uuid2package.value(_uuid).Info;
}

QTemporaryFile* ModelPool::file_usage(const QUuid& _uuid)
{
	if (!this->_hash_uuid2package.contains(_uuid))
		return nullptr;
	return this->_hash_uuid2package.value(_uuid).Usage;
}

QTemporaryFile* ModelPool::file_event(const QUuid& _uuid)
{
	if (!this->_hash_uuid2package.contains(_uuid))
		return nullptr;
	return this->_hash_uuid2package.value(_uuid).Event;
}

QString ModelPool::file_path(const QUuid& _uuid) const
{
	if (!this->_hash_uuid2package.contains(_uuid))
		return "";
	return this->_hash_uuid2package.value(_uuid).FilePath;
}
