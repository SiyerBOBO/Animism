#pragma execution_character_set("utf-8")
#include "Model.h"

static const QList<EventCategory> EventCategoryMap =
{
	{
		"Process",
		animism_engine::guids::process,
		QObject::tr("To enable process events in an NT Kernel logging session.\n") +
		QObject::tr("Include =")
		.append(" Start(1)")
		.append(" | End(2)")
		.append(" | DCStart(3)")
		.append(" | DCEnd(4)")
		.append(" | Defunct(39)")
	},
	{
		"Thread",
		animism_engine::guids::thread,
		QObject::tr("To enable thread events in an NT Kernel logging session.\n") +
		QObject::tr("Include =")
		.append(" Start(1)")
		.append(" | End(2)")
		.append(" | DCStart(3)")
		.append(" | DCEnd(4)")
	},
	{
		"Image",
		animism_engine::guids::image_load,
		QObject::tr("To enable image events in an NT Kernel logging session.\n") +
		QObject::tr("Include =")
		.append(" Load(10)")
		.append(" | Unload(2)")
		.append(" | DCStart(3)")
		.append(" | DCEnd(4)")
	},
	{
		"FileIo",
		animism_engine::guids::file_io,
		QObject::tr("To enable the File IO events in an NT Kernel logging session.\n") +
		QObject::tr("Include =")
		.append(" Name(0)")
		.append(" | FileCreate(32)")
		.append(" | FileDelete(35)")
		.append(" | FileRundown(36)")
		.append(" | Create(64)")
		.append(" | Cleanup(65)")
		.append(" | Close(66)")
		.append(" | Read(67)")
		.append(" | Write(68)")
		.append(" | SetInfo(69)")
		.append(" | Delete(70)")
		.append(" | Rename(71)")
		.append(" | DirEnum(72)")
		.append(" | Flush(73)")
		.append(" | QueryInfo(74)")
		.append(" | FSControl(75)")
		.append(" | OperationEnd(76)")
		.append(" | DirNotify(77)")
	},
	{
		"TcpIp",
		animism_engine::guids::tcp_ip,
		QObject::tr("To enable TCP/IP events in an NT Kernel logging session.\n")+
		QObject::tr("Include =")
		.append(" Send(10)")
		.append(" | SendIPV4(10)")
		.append(" | Recv(11)")
		.append(" | RecvIPV4(11)")
		.append(" | Connect(12)")
		.append(" | ConnectIPV4(12)")
		.append(" | Disconnect(13)")
		.append(" | DisconnectIPV4(13)")
		.append(" | Retransmit(14)")
		.append(" | RetransmitIPV4(14)")
		.append(" | Accept(15)")
		.append(" | AcceptIPV4(15)")
		.append(" | Reconnect(16)")
		.append(" | ReconnectIPV4(16)")
		.append(" | Fail(17)")
		.append(" | TCPCopyIPV4(18)")
		.append(" | SendIPV6(26)")
		.append(" | RecvIPV6(27)")
		.append(" | ConnectIPV6(28)")
		.append(" | DisconnectIPV6(29)")
		.append(" | RetransmitIPV6(30)")
		.append(" | AcceptIPV6(31)")
		.append(" | ReconnectIPV6(32)")
		.append(" | TCPCopyIPV6(34)")
	},
	{
		"UdpIp",
		animism_engine::guids::udp_ip,
		QObject::tr("To enable UDP/IP events in an NT Kernel logging session.\n") + 
		QObject::tr("Include =")
		.append(" Send(10)")
		.append(" | SendIPV4(10)")
		.append(" | Recv(11)")
		.append(" | RecvIPV4(11)")
		.append(" | Fail(17)")
		.append(" | SendIPV6(26)")
		.append(" | RecvIPV6(27)")
	},
	{
		"Registry",
		animism_engine::guids::registry,
		QObject::tr("To enable registry events in an NT Kernel logging session.\n") +
		QObject::tr("Include =")
		.append(" Create(10)")
		.append(" | Open(11)")
		.append(" | Delete(12)")
		.append(" | Query(13)")
		.append(" | SetValue(14)")
		.append(" | DeleteValue(15)")
		.append(" | QueryValue(16)")
		.append(" | EnumerateKey(17)")
		.append(" | EnumerateValueKey(18)")
		.append(" | QueryMultipleValue(19)")
		.append(" | SetInformation(20)")
		.append(" | Flush(21)")
		.append(" | KCBCreate(22)")
		.append(" | KCBDelete(23)")
		.append(" | KCBRundownBegin(24)")
		.append(" | KCBRundownEnd(25)")
		.append(" | Virtualize(26)")
		.append(" | Close(27)")
	}
};

bool EventCategory::FindEventCategory(const QUuid& uuid, EventCategory& category)
{
	for (auto cat : EventCategoryMap)
		if (cat.Guid == uuid)
		{
			category = cat;
			return true;
		}
	return false;
}

bool EventCategory::FindEventCategory(const QString& name, EventCategory& category)
{
	for (auto cat : EventCategoryMap)
		if (cat.Name == name)
		{
			category = cat;
			return true;
		}
	return false;
}

const QList<EventCategory>& EventCategory::GetEventCategoryMap()
{
	return EventCategoryMap;
}