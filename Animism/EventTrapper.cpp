#include "EventTrapper.h"
#include <wil\resource.h>
#include <mutex>
#include "Utils.h"
#include <QDebug>
#include <iostream>
#include <QHostAddress>

std::unique_ptr<EventTrapper> EventTrapper::_spointer_instance;
EventTrapper* EventTrapper::Instance() {
	static std::once_flag oc;
	std::call_once(oc, [&] {  _spointer_instance.reset(new EventTrapper); });
	return _spointer_instance.get();
}
static std::atomic_ullong _ull_counter(0);

static inline QDateTime GetTimestamp(LARGE_INTEGER ts)
{
	FILETIME filetime
	{
		ts.LowPart,
		ts.HighPart
	};
	SYSTEMTIME systime;
	FileTimeToSystemTime(&filetime, &systime);

	QDateTime timestamp;
	timestamp.setDate(QDate(systime.wYear, systime.wMonth, systime.wDay));
	timestamp.setTime(QTime(systime.wHour, systime.wMinute, systime.wSecond, systime.wMilliseconds));
	timestamp.setTimeSpec(Qt::TimeSpec::UTC);
	return timestamp.toLocalTime();
}

inline void ParseProperties(animism_engine::schema& schema, QVariantMap& properties);
inline void HandleNoProcessId(EventInfo& info);

EventTrapper::EventTrapper(QObject* parent)
	: QObject(parent)
{
	this->initData();
}

EventTrapper::~EventTrapper()
{
	this->tracer->stop();
	this->_thread_handle.join();
}

qulonglong EventTrapper::eventCount()
{
	return _ull_counter;
}

static bool EnablePrivilege(PCWSTR privilege) {
	HANDLE hToken;
	if (!::OpenProcessToken(::GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES, &hToken))
		return false;

	TOKEN_PRIVILEGES tp;
	tp.PrivilegeCount = 1;
	tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
	if (!::LookupPrivilegeValue(nullptr, privilege, &tp.Privileges[0].Luid))
		return false;

	BOOL success = ::AdjustTokenPrivileges(hToken, FALSE, &tp, sizeof(tp), nullptr, nullptr);
	::CloseHandle(hToken);

	return success && ::GetLastError() == ERROR_SUCCESS;
}

void EventTrapper::initData()
{
	EnablePrivilege(SE_DEBUG_NAME);
	EnablePrivilege(SE_SYSTEM_PROFILE_NAME);

	this->_ptr_timer_counter = new QTimer(this);
	QObject::connect(this->_ptr_timer_counter, &QTimer::timeout, this, [=]() {
		static unsigned long long _ull_last_count = 0;
		unsigned long long _ull_current_count = _ull_counter.load();
		emit speedPerSecond(_ull_current_count - _ull_last_count);
		_ull_last_count = _ull_current_count;
		}, Qt::DirectConnection);
	this->_ptr_timer_counter->start(1000);

	this->tracer.reset(new animism_engine::kernel_trace(L"animism-framework-event-tracking-engine"));

	this->_provider_process.add_on_event_callback(process_callback);
	this->_provider_thread.add_on_event_callback(thread_callback);
	this->_provider_image_load.add_on_event_callback(image_load_callback);
	this->_provider_registry.add_on_event_callback(registry_callback);
	this->_provider_file_io.add_on_event_callback(file_io_callback);
	this->_provider_file_init_io.add_on_event_callback(file_io_callback);
	this->_provider_tcpip.add_on_event_callback(tcpip_callback);
	this->_provider_udpip.add_on_event_callback(udpip_callback);

	this->tracer->enable(this->_provider_process);
	this->tracer->enable(this->_provider_thread);
	this->tracer->enable(this->_provider_image_load);
	this->tracer->enable(this->_provider_registry);
	this->tracer->enable(this->_provider_file_io);
	this->tracer->enable(this->_provider_file_init_io);
	this->tracer->enable(this->_provider_tcpip);
	this->tracer->enable(this->_provider_udpip);

	this->_thread_handle = std::thread([=]() { tracer->start(); });
}

void EventTrapper::process_callback(const EVENT_RECORD& record, const animism_engine::trace_context& trace_context)
{
	_ull_counter.fetch_add(1);
	try
	{
		auto schema = animism_engine::schema(record, trace_context.schema_locator);
		EventInfo info;
		info.Timestamp = GetTimestamp(schema.timestamp());
		info.Opcode = schema.event_opcode();
		info.OpcodeName = QString::fromStdWString(schema.opcode_name());
		info.ProcessId = schema.process_id();
		info.ThreadId = schema.thread_id();
		info.TaskName = QString::fromStdWString(schema.task_name());
		ParseProperties(schema, info.Properties);
		HandleNoProcessId(info);
		emit Trapper.processCaptured(info);
	}
	catch (const std::exception& e)
	{
		emit Trapper.errorOccurred(e.what());
	}
}

void EventTrapper::thread_callback(const EVENT_RECORD& record, const animism_engine::trace_context& trace_context)
{
	_ull_counter.fetch_add(1);
	try
	{
		auto schema = animism_engine::schema(record, trace_context.schema_locator);
		EventInfo info;
		info.Timestamp = GetTimestamp(schema.timestamp());
		info.Opcode = schema.event_opcode();
		info.OpcodeName = QString::fromStdWString(schema.opcode_name());
		info.ProcessId = schema.process_id();
		info.ThreadId = schema.thread_id();
		info.TaskName = QString::fromStdWString(schema.task_name());
		ParseProperties(schema, info.Properties);
		HandleNoProcessId(info);
		emit Trapper.threadCaptured(info);
	}
	catch (const std::exception& e)
	{
		emit Trapper.errorOccurred(e.what());
	}
}

void EventTrapper::image_load_callback(const EVENT_RECORD& record, const animism_engine::trace_context& trace_context)
{
	_ull_counter.fetch_add(1);
	try
	{
		auto schema = animism_engine::schema(record, trace_context.schema_locator);
		EventInfo info;
		info.Timestamp = GetTimestamp(schema.timestamp());
		info.Opcode = schema.event_opcode();
		info.OpcodeName = QString::fromStdWString(schema.opcode_name());
		info.ProcessId = schema.process_id();
		info.ThreadId = schema.thread_id();
		info.TaskName = QString::fromStdWString(schema.task_name());
		ParseProperties(schema, info.Properties);
		HandleNoProcessId(info);
		emit Trapper.imageloadCaptured(info);
	}
	catch (const std::exception& e)
	{
		emit Trapper.errorOccurred(e.what());
	}
}

void EventTrapper::registry_callback(const EVENT_RECORD& record, const animism_engine::trace_context& trace_context)
{
	_ull_counter.fetch_add(1);
	try
	{
		auto schema = animism_engine::schema(record, trace_context.schema_locator);
		EventInfo info;
		info.Timestamp = GetTimestamp(schema.timestamp());
		info.Opcode = schema.event_opcode();
		info.OpcodeName = QString::fromStdWString(schema.opcode_name());
		info.ProcessId = schema.process_id();
		info.ThreadId = schema.thread_id();
		info.TaskName = QString::fromStdWString(schema.task_name());
		ParseProperties(schema, info.Properties);
		HandleNoProcessId(info);
		emit Trapper.registryCaptured(info);
	}
	catch (const std::exception& e)
	{
		emit Trapper.errorOccurred(e.what());
	}
}

void EventTrapper::file_io_callback(const EVENT_RECORD& record, const animism_engine::trace_context& trace_context)
{
	_ull_counter.fetch_add(1);
	try
	{
		auto schema = animism_engine::schema(record, trace_context.schema_locator);
		EventInfo info;
		info.Timestamp = GetTimestamp(schema.timestamp());
		info.Opcode = schema.event_opcode();
		info.OpcodeName = QString::fromStdWString(schema.opcode_name());
		info.ProcessId = schema.process_id();
		info.ThreadId = schema.thread_id();
		info.TaskName = QString::fromStdWString(schema.task_name());
		ParseProperties(schema, info.Properties);
		HandleNoProcessId(info);
		emit Trapper.fileioCaptured(info);
	}
	catch (const std::exception& e)
	{
		emit Trapper.errorOccurred(e.what());
	}
}

void EventTrapper::tcpip_callback(const EVENT_RECORD& record, const animism_engine::trace_context& trace_context)
{
	_ull_counter.fetch_add(1);
	try
	{
		auto schema = animism_engine::schema(record, trace_context.schema_locator);
		EventInfo info;
		info.Timestamp = GetTimestamp(schema.timestamp());
		info.Opcode = schema.event_opcode();
		info.OpcodeName = QString::fromStdWString(schema.opcode_name());
		info.ProcessId = schema.process_id();
		info.ThreadId = schema.thread_id();
		info.TaskName = QString::fromStdWString(schema.task_name());
		ParseProperties(schema, info.Properties);
		if (info.Properties.contains("daddr"))
			info.Properties["daddr"] = QHostAddress(info.Properties["daddr"].toULongLong()).toString();
		if (info.Properties.contains("saddr"))
			info.Properties["saddr"] = QHostAddress(info.Properties["saddr"].toULongLong()).toString();
		HandleNoProcessId(info);
		emit Trapper.tcpipCaptured(info);
	}
	catch (const std::exception& e)
	{
		emit Trapper.errorOccurred(e.what());
	}
}

void EventTrapper::udpip_callback(const EVENT_RECORD& record, const animism_engine::trace_context& trace_context)
{
	_ull_counter.fetch_add(1);
	try
	{
		auto schema = animism_engine::schema(record, trace_context.schema_locator);
		EventInfo info;
		info.Timestamp = GetTimestamp(schema.timestamp());
		info.Opcode = schema.event_opcode();
		info.OpcodeName = QString::fromStdWString(schema.opcode_name());
		info.ProcessId = schema.process_id();
		info.ThreadId = schema.thread_id();
		info.TaskName = QString::fromStdWString(schema.task_name());
		ParseProperties(schema, info.Properties);
		if (info.Properties.contains("daddr"))
			info.Properties["daddr"] = QHostAddress(info.Properties["daddr"].toULongLong()).toString();
		if (info.Properties.contains("saddr"))
			info.Properties["saddr"] = QHostAddress(info.Properties["saddr"].toULongLong()).toString();
		HandleNoProcessId(info);
		emit Trapper.udpipCaptured(info);
	}
	catch (const std::exception& e)
	{
		emit Trapper.errorOccurred(e.what());
	}
}

inline void ParseProperties(animism_engine::schema& schema, QVariantMap& properties)
{
	properties.clear();
	animism_engine::parser parser(schema);
	for (auto& prop : parser.properties()) {
		const std::wstring name = prop.name();
		const _TDH_IN_TYPE type = prop.type();
		QVariant value;
		switch (type)
		{
		case TDH_INTYPE_BOOLEAN: {
			bool _data;
			if (parser.try_parse<bool>(name, _data))
				value = _data;
			break;
		}
		case TDH_INTYPE_UINT64: {
			std::uint64_t _data;
			if (parser.try_parse<std::uint64_t>(name, _data))
				value = _data;
			break;
		}
		case TDH_INTYPE_UINT32: {
			std::uint32_t _data;
			if (parser.try_parse<std::uint32_t>(name, _data))
				value = _data;
			break;
		}
		case TDH_INTYPE_UINT16: {
			std::uint16_t _data;
			if (parser.try_parse<std::uint16_t>(name, _data))
				value = _data;
			break;
		}
		case TDH_INTYPE_UINT8: {
			std::uint8_t _data;
			if (parser.try_parse<std::uint8_t>(name, _data))
				value = _data;
			break;
		}
		case TDH_INTYPE_INT64: {
			std::int64_t _data;
			if (parser.try_parse<std::int64_t>(name, _data))
				value = _data;
			break;
		}
		case TDH_INTYPE_HEXINT64: {
			animism_engine::hexint64 _data(0);
			if (parser.try_parse<animism_engine::hexint64>(name, _data))
				value = _data.value;
			break;
		}
		case TDH_INTYPE_INT32: {
			std::int32_t _data;
			if (parser.try_parse<std::int32_t>(name, _data))
				value = _data;
			break;
		}
		case TDH_INTYPE_HEXINT32: {
			animism_engine::hexint32 _data(0);
			if (parser.try_parse<animism_engine::hexint32>(name, _data))
				value = _data.value;
			break;
		}
		case TDH_INTYPE_INT16: {
			std::int16_t _data;
			if (parser.try_parse<std::int16_t>(name, _data))
				value = _data;
			break;
		}
		case TDH_INTYPE_INT8: {
			std::int8_t _data;
			if (parser.try_parse<std::int8_t>(name, _data))
				value = _data;
			break;
		}
		case TDH_INTYPE_FLOAT: {
			std::float_t _data;
			if (parser.try_parse<std::float_t>(name, _data))
				value = _data;
			break;
		}
		case TDH_INTYPE_DOUBLE: {
			std::double_t _data;
			if (parser.try_parse<std::double_t>(name, _data))
				value = _data;
			break;
		}
		case TDH_INTYPE_UNICODESTRING:
		case TDH_INTYPE_COUNTEDSTRING:
		case TDH_INTYPE_REVERSEDCOUNTEDSTRING:
		case TDH_INTYPE_NONNULLTERMINATEDSTRING: {
			std::wstring _data;
			if (parser.try_parse<std::wstring>(name, _data))
				value = QString::fromStdWString(_data);
			break;
		}
		case TDH_INTYPE_ANSISTRING:
		case TDH_INTYPE_COUNTEDANSISTRING:
		case TDH_INTYPE_REVERSEDCOUNTEDANSISTRING:
		case TDH_INTYPE_NONNULLTERMINATEDANSISTRING: {
			std::string _data;
			if (parser.try_parse<std::string>(name, _data))
				value = QString::fromStdString(_data);
			break;
		}
		case TDH_INTYPE_UNICODECHAR: {
			wchar_t _data;
			if (parser.try_parse<wchar_t>(name, _data))
				value = QString::fromWCharArray(&_data, 1).at(0);
			break;
		}
		case TDH_INTYPE_ANSICHAR: {
			char _data;
			if (parser.try_parse<char>(name, _data))
				value = _data;
			break;
		}
		case TDH_INTYPE_BINARY: {
			animism_engine::binary _data;
			if (parser.try_parse<animism_engine::binary>(name, _data))
				value = QByteArray(reinterpret_cast<unsigned char>(_data.bytes().data()), _data.bytes().size());
			break;
		}
		case TDH_INTYPE_NULL: {
			value = 0;
			break;
		}
		case TDH_INTYPE_POINTER: {
			animism_engine::pointer _data;
			if (parser.try_parse<animism_engine::pointer>(name, _data))
				value = _data.address;
			break;
		}
		case TDH_INTYPE_SIZET: {
			std::size_t _data;
			if (parser.try_parse<std::size_t>(name, _data))
				value = _data;
			break;
		}
		case TDH_INTYPE_SYSTEMTIME:
		case TDH_INTYPE_FILETIME: {
			std::chrono::system_clock::time_point _data;
			if (parser.try_parse<std::chrono::system_clock::time_point>(name, _data)) {
				auto _data2 = std::chrono::system_clock::to_time_t(_data);
				std::tm tm_value;
				gmtime_s(&tm_value, &_data2);
				value = QDateTime(
					QDate(tm_value.tm_year, tm_value.tm_mon, tm_value.tm_mday),
					QTime(tm_value.tm_hour, tm_value.tm_min, tm_value.tm_sec),
					Qt::UTC);
			}
			break;
		}
		case TDH_INTYPE_GUID: {
			GUID _data;
			if (parser.try_parse<GUID>(name, _data))
				value = QUuid(_data);
			break;
		}
		case TDH_INTYPE_WBEMSID:
		case TDH_INTYPE_SID: {
			animism_engine::sid _data;
			if (parser.try_parse<animism_engine::sid>(name, _data))
				value = QString::fromStdString(_data.sid_string);
			break;
		}
		default:
			value = "[unsupported type]";
			break;
		}
		properties.insert(QString::fromStdWString(name), value);
	}
}

inline void HandleNoProcessId(EventInfo& info) {
	if (info.ThreadId == 0 || info.ThreadId == (DWORD)-1) {
		if (info.Properties.contains("ThreadId"))
			info.ThreadId = info.Properties.value("ThreadId").toUInt();
		else if (info.Properties.contains("TThreadId"))
			info.ThreadId = info.Properties.value("TThreadId").toUInt();
	}
	if (info.ProcessId == 0 || info.ProcessId == (DWORD)-1) {
		if (info.Properties.contains("PID"))
			info.ProcessId = info.Properties.value("PID").toUInt();
		else if (info.Properties.contains("ProcessId"))
			info.ProcessId = info.Properties.value("ProcessId").toUInt();
		else if (info.ThreadId != 0 && info.ThreadId != (DWORD)-1) {
			wil::unique_handle hThread(::OpenThread(THREAD_QUERY_LIMITED_INFORMATION, FALSE, info.ThreadId));
			if (hThread)
				info.ProcessId = ::GetProcessIdOfThread(hThread.get());
		}
	}
}