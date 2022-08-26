#pragma once

#include <QObject>
#include <QPointer>
#include <QThreadPool>
#include <QtConcurrent>
#include <QSqlDatabase>
#include <QMap>
#include "Model.h"

#define Reader (*ModelReaderV2::Instance())

class ModelReaderV2 : public QObject
{
	Q_OBJECT

public:
	static ModelReaderV2* Instance();
	~ModelReaderV2();

	QFuture<bool> load_model(const QString& filepath);
	Model model() const;
	QList<QVariantMap> cache() const;
	QPair<QDateTime, qint64> duration() const;
	QPair<QDateTime, QDateTime> duration(const QString& task_name) const;
	QPair<QDateTime, QDateTime> duration(const QString& task_name, const QString& opcode_name) const;

	qulonglong total_count();
	QMap<QString, qulonglong> event_count() const;
	qulonglong event_count(const QString& task_name) const;
	QMap<QString, qulonglong> opcode_count(const QString& task_name) const;
	QFuture<QMap<QString, qulonglong>> opcode_count(const QString& task_name, const QDateTime& from, const QDateTime& to) const;
	qulonglong opcode_count(const QString& task_name, const QString& opcode_name) const;

	QFuture<QList<qint64>> event_timestamp(const QString& task_name) const;
	QFuture<QList<qint64>> opcode_timestamp(const QString& task_name, const QString& opcode_name) const;

	QFuture<QList<EventInfo>> find_event(const QString& task_name, qlonglong offset = -1, qlonglong count = -1) const;
	QFuture<QList<EventInfo>> find_event(const QString& task_name, const QString& opcode_name, qlonglong offset = -1, qlonglong count = -1) const;
	QFuture<QList<EventInfo>> find_event(const QString& task_name, const QDateTime& from, const QDateTime& to, qlonglong offset = -1, qlonglong count = -1) const;

protected:
	ModelReaderV2(QObject *parent = nullptr);

private:
	static std::unique_ptr<ModelReaderV2> _spointer_instance;
	QSqlDatabase _sqldb_model;
	QPointer<QThreadPool> _ptr_threadpool;

	Model _model;
	QList<QVariantMap> _cache;
};
