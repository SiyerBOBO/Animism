#pragma execution_character_set("utf-8")
#include "Settings.h"
#include <QDesktopServices>
#include <QUrl>
#include <QTimer>
#include "Utils.h"

Settings::Settings(QWidget *parent)
	: QWidget(parent)
{
	this->initUi();
}

Settings::~Settings()
{
}

void Settings::initUi()
{
	ui.setupUi(this);

	this->ui.label_appVersion->setText(qApp->applicationVersion());
	QObject::connect(this->ui.pushButton_qt, &QPushButton::clicked, &QApplication::aboutQt);
	QObject::connect(this->ui.pushButton_nchu, &QPushButton::clicked, this, [=]() {QDesktopServices::openUrl(QUrl("https://www.nchu.edu.cn/")); });

	QTimer::singleShot(1000, this, &Settings::initData);
}

void Settings::initData()
{
	this->_hash_model2detail.insert(tr("Animism Engine Framework %1").arg(qApp->applicationVersion()), tr("NanChang HangKong University.\nWangBo.2022.LimeStudio\n著作权所有\nQQ: 1162472111"));
	this->_hash_model2detail.insert(tr("Qt for Open Source Development 5.15.2"), tr("Qt is a cross-platform application development framework for desktop, embedded and mobile. Supported Platforms include Linux, OS X, Windows, VxWorks, QNX, Android, iOS, BlackBerry, Sailfish OS and others.\nQt is available under various licenses: The Qt Company sells commercial licenses, but Qt is also available as free software under several versions of the GPL and the LGPL."));
	this->_hash_model2detail.insert(tr("Microsoft.Windows.ImplementationLibrary 1.0.220201.1"), tr("The Windows Implementation Libraries (wil) were created to improve productivity and solve problems commonly seen by Windows developers."));
	this->_hash_model2detail.insert(tr("SQLite 3.33.0"), tr("SQLite is a small C library that implements a self-contained, embeddable, zero-configuration SQL database engine."));
	this->_hash_model2detail.insert(tr("QtCipherSqlitePlugin 1.3.0"), tr("QtCipherSqlitePlugin is a Qt plugin for ciphered SQLite which is based on SQLite source and wxSQLite3. This plugin is released with LGPLv2.1, LGPLv3 or GPL v3 the same as Qt open source license."));

	auto _keys = this->_hash_model2detail.keys();
	this->ui.listWidget_module->addItems(_keys);

	QObject::connect(this->ui.listWidget_module, &QListWidget::currentItemChanged, this, [=](QListWidgetItem* current, QListWidgetItem* previous) {
		QString _text = current->text();
		this->ui.label_module_detail->setText(this->_hash_model2detail.value(_text));
		});
}

void Settings::showEvent(QShowEvent* event)
{
	this->ui.listWidget_module->clearSelection();
	this->ui.label_module_detail->clear();
	return QWidget::showEvent(event);
}
