#pragma once
#include "./animism_engine/animism_engine.hpp"
#include <thread>
#include <QObject>
#include <QPointer>
#include <QTimer>
#include "Model.h"

#define Trapper (*EventTrapper::Instance())

class EventTrapper : public QObject
{
	Q_OBJECT

public:
	static EventTrapper* Instance();
	~EventTrapper();
	qulonglong eventCount();

	Q_SIGNAL void speedPerSecond(qulonglong value);
	Q_SIGNAL void errorOccurred(const QString& what);

	Q_SIGNAL void processCaptured(const EventInfo& info);
	Q_SIGNAL void threadCaptured(const EventInfo& info);
	Q_SIGNAL void imageloadCaptured(const EventInfo& info);
	Q_SIGNAL void fileioCaptured(const EventInfo& info);
	Q_SIGNAL void registryCaptured(const EventInfo& info);
	Q_SIGNAL void tcpipCaptured(const EventInfo& info);
	Q_SIGNAL void udpipCaptured(const EventInfo& info);

protected:
	EventTrapper(QObject* parent = Q_NULLPTR);
	void initData();

	static void process_callback(const EVENT_RECORD& record, const animism_engine::trace_context& trace_context);
	static void thread_callback(const EVENT_RECORD& record, const animism_engine::trace_context& trace_context);
	static void image_load_callback(const EVENT_RECORD& record, const animism_engine::trace_context& trace_context);
	static void registry_callback(const EVENT_RECORD& record, const animism_engine::trace_context& trace_context);
	static void file_io_callback(const EVENT_RECORD& record, const animism_engine::trace_context& trace_context);
	static void tcpip_callback(const EVENT_RECORD& record, const animism_engine::trace_context& trace_context);
	static void udpip_callback(const EVENT_RECORD& record, const animism_engine::trace_context& trace_context);

private:
	QPointer<QTimer> _ptr_timer_counter;
	static std::unique_ptr<EventTrapper> _spointer_instance;

	animism_engine::kernel::process_provider _provider_process;
	animism_engine::kernel::thread_provider _provider_thread;
	animism_engine::kernel::image_load_provider _provider_image_load;
	animism_engine::kernel::registry_provider _provider_registry;
	animism_engine::kernel::file_io_provider _provider_file_io;
	animism_engine::kernel::file_init_io_provider _provider_file_init_io;
	animism_engine::kernel::network_tcpip_provider _provider_tcpip;
	animism_engine::kernel::network_udpip_provider _provider_udpip;
	
	std::unique_ptr<animism_engine::kernel_trace> tracer;
	std::thread _thread_handle;
	std::atomic_flag _flag_state = ATOMIC_FLAG_INIT;

	friend static void process_callback(const EVENT_RECORD& record, const animism_engine::trace_context& trace_context);
	friend static void thread_callback(const EVENT_RECORD& record, const animism_engine::trace_context& trace_context);
	friend static void image_load_callback(const EVENT_RECORD& record, const animism_engine::trace_context& trace_context);
	friend static void registry_callback(const EVENT_RECORD& record, const animism_engine::trace_context& trace_context);
	friend static void file_io_callback(const EVENT_RECORD& record, const animism_engine::trace_context& trace_context);
	friend static void tcpip_callback(const EVENT_RECORD& record, const animism_engine::trace_context& trace_context);
	friend static void udpip_callback(const EVENT_RECORD& record, const animism_engine::trace_context& trace_context);
};