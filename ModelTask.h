#include "./animism_engine/schema.hpp"
#pragma once
#include <QMutex>
#include <QThread>
#include <QPointer>
#include <QEventLoop>
#include <QMap>
#include <QSqlDataBase>
#include <QFile>
#include "Model.h"
#include "ProcessUsageHelper.h"

class QTimer;

class ModelTask : public QThread
{
	Q_OBJECT

public:
	ModelTask(const Model& model, QObject* parent);
	~ModelTask();

	Model model() const;
	void callStop(unsigned long time = 15000);
	enum ErrorType
	{
		None,
		FileSysError,
		TrapperError,
		UnknownError
	};
	ErrorType lastError();

	Q_SIGNAL void monitor(double cpu, quint64 ram);
	Q_SIGNAL void counter(
		unsigned long long, 
		unsigned long long,
		unsigned long long,
		unsigned long long,
		unsigned long long,
		unsigned long long,
		unsigned long long
		);

protected:
	Q_SIGNAL void stop();
	void run() override;

	bool initDatabase();
	bool initFile();
	bool initTask();
	void beforeFinish();

	Q_SLOT void onProcessCaptured(const EventInfo& info);
	Q_SLOT void onThreadCaptured(const EventInfo& info);
	Q_SLOT void onImageloadCaptured(const EventInfo& info);
	Q_SLOT void onFileioCaptured(const EventInfo& info);
	Q_SLOT void onRegistryCaptured(const EventInfo& info);
	Q_SLOT void onTcpipCaptured(const EventInfo& info);
	Q_SLOT void onUdpipCaptured(const EventInfo& info);
	Q_SLOT void save_event();
	Q_SLOT void save_usage();
	Q_SLOT void save_model();

private:
	Model _model;
	ErrorType _error = ErrorType::None;
	QList<EventInfo> _list_record[2];
	QScopedPointer<ProcessUsageHelper> _ptr_usage_helper;
	bool _bool_point = false;
	bool _bool_pause = false;
	QTemporaryFile* file_info, * file_usage, * file_event;
	std::atomic<unsigned long long> _ull_process_count;
	std::atomic<unsigned long long> _ull_thread_count;
	std::atomic<unsigned long long> _ull_imageload_count;
	std::atomic<unsigned long long> _ull_fileio_count;
	std::atomic<unsigned long long> _ull_registry_count;
	std::atomic<unsigned long long> _ull_tcpip_count;
	std::atomic<unsigned long long> _ull_udpip_count;

	friend class ModelView;
};
