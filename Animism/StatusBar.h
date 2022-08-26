#pragma once

#include <QWidget>
#include <QHash>
#include <QPointer>

class QPaintEvent;

class StatusBar : public QWidget
{
	Q_OBJECT

public:
	StatusBar(QWidget* parent);
	~StatusBar();

	void append(const QString& item);
	void append(const QStringList& items);

	bool set(int index, const QString& item);

	void removeAt(int index);
	void clear();

	QStringList items() const;
	QString itemAt(int index) const;

protected:
	void paintEvent(QPaintEvent* e) override;

private:
	QStringList _strlist_tags;
};
