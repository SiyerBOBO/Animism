#pragma once

#include <QObject>
#include <QUuid>
#include <QHash>
#include <QFile>
#include <QSharedPointer>
#include <QTemporaryFile>
#include <QSqlDatabase>
#include <QtConcurrent>
#include "Model.h"

#define Pool (*ModelPool::Instance())

class ModelPool : public QObject
{
	Q_OBJECT

public:
	static ModelPool* Instance();
	~ModelPool();
	bool create(const Model& _model, const QString& _filepath);
	void save(const QUuid& _uuid);
	bool release(const QUuid& _uuid);
	bool contains(const QUuid& _uuid);

	QTemporaryFile* file_info(const QUuid& _uuid);
	QTemporaryFile* file_usage(const QUuid& _uuid);
	QTemporaryFile* file_event(const QUuid& _uuid);
	QString file_path(const QUuid& _uuid) const;
	bool is_over(const QUuid& _uuid);

	Q_SIGNAL void progress_of_save(const QUuid& uuid, qreal value);
	Q_SIGNAL void result_of_save(const QUuid& uuid, bool isComplete);

	void cleanUnfinished();

protected:
	ModelPool(QObject *parent = nullptr);
	void initData();
	void saveTask(const QUuid& _uuid);

private:
	struct Package
	{
		QString FilePath;
		bool Over = false;
		QFuture<void> Task;
		QTemporaryFile* Info;
		QTemporaryFile* Usage;
		QTemporaryFile* Event;
		QSqlDatabase Database;
	};
	static std::unique_ptr<ModelPool> _spointer_instance;
	QHash<QUuid, Package> _hash_uuid2package;
};
