#include "ModelReaderV2.h"
#include <QFile>
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlQueryModel>

std::unique_ptr<ModelReaderV2> ModelReaderV2::_spointer_instance;

ModelReaderV2* ModelReaderV2::Instance() {
	static std::once_flag oc;
	std::call_once(oc, [&] {  _spointer_instance.reset(new ModelReaderV2); });
	return _spointer_instance.get();
}

ModelReaderV2::ModelReaderV2(QObject* parent)
	: QObject(parent)
{
	this->_ptr_threadpool = new QThreadPool(this);
}

ModelReaderV2::~ModelReaderV2()
{
	
}

QFuture<bool> ModelReaderV2::load_model(const QString& filepath)
{
	return QtConcurrent::run(this->_ptr_threadpool, [=]() -> bool {
		QFile file(filepath);

		if (!file.open(QFile::OpenModeFlag::ReadOnly))
			return false;
		if (0 == file.size())
			return false;

		this->_sqldb_model = QSqlDatabase::addDatabase("SQLITECIPHER", QUuid::createUuid().toString());
		this->_sqldb_model.setDatabaseName(filepath);
		this->_sqldb_model.setConnectOptions("QSQLITE_OPEN_READONLY");
		this->_sqldb_model.setPassword(QCryptographicHash::hash(qApp->organizationName().toLatin1().toBase64(), QCryptographicHash::Algorithm::Md5));
		if (this->_sqldb_model.open())
		{
			Model _model;
			QSqlQuery _query(this->_sqldb_model);
			_query.exec("select * from model");
			if (_query.first())
			{
				_model.Process.Pid = _query.value("_pid").toInt();
				_model.Process.Name = _query.value("_pname").toString();
				_model.Process.FilePath = _query.value("_ppath").toString();
				_model.Process.Md5 = _query.value("_pmd5").toString();
				_model.Process.Icon.loadFromData(_query.value("_picon").toByteArray(), "png");

				_model.Timestamp = _query.value("_timestamp").toDateTime();
				_model.Duration = _query.value("_duration").toLongLong();
				_model.Uuid = _query.value("_uuid").toUuid();

				auto _strlist_uuid = _query.value("_focus").toString().split('&');
				for (auto _str_uuid : _strlist_uuid)
					_model.Focus.insert(QUuid::fromString(_str_uuid));
			}
			this->_model = _model;

			QList<QVariantMap> _predata;
			_query.clear();
			_query.exec("select * from pre_data");
			while (_query.next())
			{
				QVariantMap _map;
				_map["_task_name"] = _query.value("_task_name").toString();
				_map["_opcode_name"] = _query.value("_opcode_name").toString();
				_map["_count"] = _query.value("_count").toULongLong();
				_map["_timestamp_start"] = _query.value("_timestamp_start").toDateTime();
				_map["_timestamp_finish"] = _query.value("_timestamp_finish").toDateTime();
				_predata.append(_map);
			}
			this->_cache = _predata;

			return true;
		}
		return false;
		});
}

Model ModelReaderV2::model() const
{
	return this->_model;
}

QList<QVariantMap> ModelReaderV2::cache() const
{
	return this->_cache;
}

QPair<QDateTime, qint64> ModelReaderV2::duration() const
{
	return qMakePair<QDateTime, qint64>(this->_model.Timestamp, this->_model.Duration);
}

QPair<QDateTime, QDateTime> ModelReaderV2::duration(const QString& task_name) const
{
	QPair<QDateTime, QDateTime> _duration;
	bool flag = true;
	for (auto item : this->_cache)
	{
		if (task_name != item.value("_task_name").toString())
			continue;
		QDateTime _timestamp_start = item.value("_timestamp_start").toDateTime();
		QDateTime _timestamp_finish = item.value("_timestamp_finish").toDateTime();

		if (flag)
		{
			_duration.first = _timestamp_start;
			_duration.second = _timestamp_finish;
			flag = false;
			continue;
		}

		if (_duration.first > _timestamp_start)
			_duration.first = _timestamp_start;
		if (_duration.second < _timestamp_finish)
			_duration.second = _timestamp_finish;
	}
	return _duration;
}

QPair<QDateTime, QDateTime> ModelReaderV2::duration(const QString& task_name, const QString& opcode_name) const
{
	QPair<QDateTime, QDateTime> _duration;
	bool flag = true;
	for (auto item : this->_cache)
	{
		if (task_name != item.value("_task_name").toString())
			continue;
		if (opcode_name != item.value("_opcode_name").toString())
			continue;

		_duration.first = item.value("_timestamp_start").toDateTime();
		_duration.second = item.value("_timestamp_finish").toDateTime();

		break;
	}
	return _duration;
}

qulonglong ModelReaderV2::total_count()
{
	qulonglong _count = 0;
	for (auto item : this->_cache)
		_count += item.value("_count").toULongLong();
	return _count;
}

QMap<QString, qulonglong> ModelReaderV2::event_count() const
{
	QMap<QString, qulonglong> _count;
	for (auto item : this->_cache)
		_count[item.value("_task_name").toString()] += item.value("_count").toULongLong();
	return _count;
}

qulonglong ModelReaderV2::event_count(const QString& task_name) const
{
	qulonglong _count = 0;
	for (auto item : this->_cache)
		if (task_name == item.value("_task_name").toString())
			_count += item.value("_count").toULongLong();
	return _count;
}

QMap<QString, qulonglong> ModelReaderV2::opcode_count(const QString& task_name) const
{
	QMap<QString, qulonglong> _count;
	for (auto item : this->_cache)
		if (task_name == item.value("_task_name").toString())
			_count[item.value("_opcode_name").toString()] += item.value("_count").toULongLong();
	return _count;
}

QFuture<QMap<QString, qulonglong>> ModelReaderV2::opcode_count(const QString& task_name, const QDateTime& from, const QDateTime& to) const
{
	return QtConcurrent::run(this->_ptr_threadpool, [&](const QString& task_name, const QDateTime& from, const QDateTime& to) -> QMap<QString, qulonglong> {
		QMap<QString, qulonglong> _result;
		QSqlDatabase _db = QSqlDatabase::cloneDatabase(this->_sqldb_model, QUuid::createUuid().toString());
		if (_db.open())
		{
			QSqlQuery _query(_db);
			_query.prepare("select _opcode_name, count(*) from proc_data where _task_name = :_task_name and _timestamp between :_from and :_to group by _opcode_name");
			_query.bindValue(":_task_name", task_name);
			_query.bindValue(":_from", from);
			_query.bindValue(":_to", to);
			_query.exec();
			while (_query.next())
				_result.insert(_query.value("_opcode_name").toString(), _query.value("count(*)").toULongLong());
		}
		return _result;
		}, task_name, from, to);
}

qulonglong ModelReaderV2::opcode_count(const QString& task_name, const QString& opcode_name) const
{
	qulonglong _count = 0;
	for (auto item : this->_cache)
	{
		if (task_name != item.value("_task_name").toString())
			continue;
		if (opcode_name != item.value("_opcode_name").toString())
			continue;
		_count += item.value("_count").toULongLong();
	}
	return _count;
}

QFuture<QList<qint64>> ModelReaderV2::event_timestamp(const QString& task_name) const
{
	return QtConcurrent::run(this->_ptr_threadpool, [&](const QString& task_name) -> QList<qint64> {
		QList<qint64> _result;
		QSqlDatabase _db = QSqlDatabase::cloneDatabase(this->_sqldb_model, QUuid::createUuid().toString());
		if (_db.open())
		{
			QSqlQuery _query(_db);
			_query.prepare("select _timestamp from proc_data where _task_name = :_task_name");
			_query.bindValue(":_task_name", task_name);
			_query.exec();
			while (_query.next())
				_result.append(_query.value("_timestamp").toDateTime().toSecsSinceEpoch());
		}
		return _result;
		}, task_name);
}

QFuture<QList<qint64>> ModelReaderV2::opcode_timestamp(const QString& task_name, const QString& opcode_name) const
{
	return QtConcurrent::run(this->_ptr_threadpool, [&](const QString& task_name, const QString& opcode_name) -> QList<qint64> {
		QList<qint64> _result;
		QSqlDatabase _db = QSqlDatabase::cloneDatabase(this->_sqldb_model, QUuid::createUuid().toString());
		if (_db.open())
		{
			QSqlQuery _query(_db);
			_query.prepare("select * from proc_data where _task_name = :_task_name and _timestamp between :_from and :_to group by _opcode_name");
			_query.bindValue(":_task_name", task_name);
			_query.bindValue(":_opcode_name", opcode_name);
			_query.exec();
			while (_query.next())
				_result.append(_query.value("_timestamp").toDateTime().toMSecsSinceEpoch());
		}
		return _result;
		}, task_name, opcode_name);
}

QFuture<QList<EventInfo>> ModelReaderV2::find_event(const QString& task_name, qlonglong offset, qlonglong count) const
{
	return QtConcurrent::run(this->_ptr_threadpool, [&](const QString& task_name, qlonglong offset, qlonglong count) -> QList<EventInfo> {
		QList<EventInfo> _result;
		QSqlDatabase _db = QSqlDatabase::cloneDatabase(this->_sqldb_model, QUuid::createUuid().toString());
		if (_db.open())
		{
			QSqlQuery _query(_db);
			_query.prepare("select * from proc_data where _task_name = :_task_name limit :_limit offset :_offset");
			_query.bindValue(":_task_name", task_name);
			_query.bindValue(":_limit", count);
			_query.bindValue(":_offset", offset);
			_query.exec();
			while (_query.next())
			{
				EventInfo _einfo;
				_einfo.Timestamp = _query.value("_timestamp").toDateTime();
				_einfo.ThreadId = _query.value("_tid").toInt();
				_einfo.TaskName = _query.value("_task_name").toString();
				_einfo.Properties = QJsonDocument::fromJson(_query.value("_properties").toByteArray()).object().toVariantMap();
				_einfo.ProcessId = _query.value("_pid").toInt();
				_einfo.OpcodeName = _query.value("_opcode_name").toString();
				_einfo.Opcode = _query.value("_opcode").toInt();
				_result.append(_einfo);
			}
		}
		return _result;
		}, task_name, offset, count);
}

QFuture<QList<EventInfo>> ModelReaderV2::find_event(const QString& task_name, const QString& opcode_name, qlonglong offset, qlonglong count) const
{
	return QtConcurrent::run(this->_ptr_threadpool, [&](const QString& task_name, qlonglong offset, qlonglong count) -> QList<EventInfo> {
		QList<EventInfo> _result;
		QSqlDatabase _db = QSqlDatabase::cloneDatabase(this->_sqldb_model, QUuid::createUuid().toString());
		if (_db.open())
		{
			QSqlQuery _query(_db);
			_query.prepare("select * from proc_data where _task_name = :_task_name and _opcode_name = :_opcode_name limit :_limit offset :_offset");
			_query.bindValue(":_task_name", task_name);
			_query.bindValue(":_opcode_name", opcode_name);
			_query.bindValue(":_limit", count);
			_query.bindValue(":_offset", offset);
			_query.exec();
			while (_query.next())
			{
				EventInfo _einfo;
				_einfo.Timestamp = _query.value("_timestamp").toDateTime();
				_einfo.ThreadId = _query.value("_tid").toInt();
				_einfo.TaskName = _query.value("_task_name").toString();
				_einfo.Properties = QJsonDocument::fromJson(_query.value("_properties").toByteArray()).object().toVariantMap();
				_einfo.ProcessId = _query.value("_pid").toInt();
				_einfo.OpcodeName = _query.value("_opcode_name").toString();
				_einfo.Opcode = _query.value("_opcode").toInt();
				_result.append(_einfo);
			}
		}
		return _result;
		}, task_name, offset, count);
}

QFuture<QList<EventInfo>> ModelReaderV2::find_event(const QString& task_name, const QDateTime& from, const QDateTime& to, qlonglong offset, qlonglong count) const
{
	return QtConcurrent::run(this->_ptr_threadpool, [&](const QString& task_name, const QDateTime& from, const QDateTime& to, qlonglong offset, qlonglong count) -> QList<EventInfo> {
		QList<EventInfo> _result;
		QSqlDatabase _db = QSqlDatabase::cloneDatabase(this->_sqldb_model, QUuid::createUuid().toString());
		if (_db.open())
		{
			QSqlQuery _query(_db);
			_query.prepare("select * from proc_data where _task_name = :_task_name and _timestamp between :_from and :_to limit :_limit offset :_offset");
			_query.bindValue(":_task_name", task_name);
			_query.bindValue(":_from", from);
			_query.bindValue(":_to", to);
			_query.bindValue(":_limit", count);
			_query.bindValue(":_offset", offset);
			_query.exec();
			while (_query.next())
			{
				EventInfo _einfo;
				_einfo.Timestamp = _query.value("_timestamp").toDateTime();
				_einfo.ThreadId = _query.value("_tid").toInt();
				_einfo.TaskName = _query.value("_task_name").toString();
				_einfo.Properties = QJsonDocument::fromJson(_query.value("_properties").toByteArray()).object().toVariantMap();
				_einfo.ProcessId = _query.value("_pid").toInt();
				_einfo.OpcodeName = _query.value("_opcode_name").toString();
				_einfo.Opcode = _query.value("_opcode").toInt();
				_result.append(_einfo);
			}
		}
		return _result;
		}, task_name, from, to, offset, count);
}