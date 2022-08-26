#pragma once

#include <QWidget>
#include <QPointer>
#include "ui_Animism.h"

class Welcome;
class Explorer;
class Settings;

class Animism : public QWidget
{
	Q_OBJECT

public:
	Animism(QWidget* parent = Q_NULLPTR);
	~Animism();

protected:
	void initUi();
	bool event(QEvent* e) override;
	bool eventFilter(QObject* obj, QEvent* e) override;
#if (QT_VERSION >= QT_VERSION_CHECK(6,0,0))
	bool nativeEvent(const QByteArray& eventType, void* message, qintptr* result);
#else
	bool nativeEvent(const QByteArray& eventType, void* message, long* result);
#endif

private:
	Ui::Animism ui;
	QVector<QPair<QRect, bool>> _vect_resize_area;

	QPointer<Welcome> _ptr_welcome;
	QPointer<Explorer> _ptr_explorer;
	QPointer<Settings> _ptr_settings;
};
