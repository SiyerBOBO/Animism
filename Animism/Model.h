#pragma once
#include "./animism_engine/kernel_guids.hpp"
#include <QString>
#include <QDateTime>
#include <QMultiMap>
#include <QVariantMap>
#include <QMetaType>
#include <QUuid>
#include <QSet>
#include <WTypesbase.h>
#include <QString>
#include <QIcon>

struct EventInfo
{
	QDateTime Timestamp;
	QString TaskName;
	QString OpcodeName;
	uint32_t Opcode = 0;
	uint32_t ProcessId = 0;
	uint32_t ThreadId = 0;
	QVariantMap Properties;
};
Q_DECLARE_METATYPE(EventInfo);

struct ProcessInfo
{
	uint32_t Pid = 0;
	QString Name;
	QString FilePath;
	QPixmap Icon;
	QString Md5;
};
Q_DECLARE_METATYPE(ProcessInfo);

struct Model
{
	QUuid Uuid;
	QDateTime Timestamp;
	uint32_t Duration = 0;
	QSet<QUuid> Focus;
	ProcessInfo Process;
};
Q_DECLARE_METATYPE(Model);

struct EventCategory
{
	QString Name;
	QUuid Guid;
	QString Description;
	static bool FindEventCategory(const QUuid& uuid, EventCategory& category);
	static bool FindEventCategory(const QString& name, EventCategory& category);
	static const QList<EventCategory>& GetEventCategoryMap();
};
Q_DECLARE_METATYPE(EventCategory);

struct Usage
{
	QDateTime Timestamp;
	double Cpu = 0;
	quint64 Ram = 0;
};
Q_DECLARE_METATYPE(Usage);

struct EventCounter
{
	QString TaskName;
	QHash<QString, uint32_t> OpcodeNameToCount;
	QHash<QString, QPair<QDateTime, QDateTime>> OpcodeNameToTimeStamp;
	uint32_t TotalCount = 0;
	void add(const QString& opcode_name, const QDateTime& opcode_timestamp)
	{
		OpcodeNameToCount[opcode_name]++;
		if (!OpcodeNameToTimeStamp.contains(opcode_name))
			OpcodeNameToTimeStamp[opcode_name].first = opcode_timestamp;
		OpcodeNameToTimeStamp[opcode_name].second = opcode_timestamp;
		++TotalCount;
	}
	uint32_t get(const QString& opcode_name = "")
	{
		if (opcode_name.isEmpty())
			return TotalCount;
		return OpcodeNameToCount.value(opcode_name);
	}
};
Q_DECLARE_METATYPE(EventCounter);