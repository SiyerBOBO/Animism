#pragma execution_character_set("utf-8")
#include "Animism.h"
#include <QtWidgets/QApplication>
#include <QLocalSocket>
#include <QLocalServer>
#include <QJsonObject>
#include <QJsonValue>
#include <QJsonDocument>
#include <QProcess>
#include <QSysInfo>
#include "Utils.h"
#include "Model.h"
#include "MessageDialog.h"
#include "Analysis.h"

using namespace std;

int main(int argc, char* argv[])
{
	QApplication::setAttribute(Qt::AA_EnableHighDpiScaling, true);
	QApplication::setAttribute(Qt::AA_UseDesktopOpenGL, true);
	QApplication app(argc, argv);
	app.setWindowIcon(QIcon(":/Animism/Resource/icon.png"));
	app.setApplicationVersion("v1.5.4 release");
	app.setOrganizationName("org.nchu.lime.studio");
	QFont font("Microsoft YaHei");
	font.setPixelSize(13);
	app.setFont(font);

	qRegisterMetaType<EventInfo>("EventInfo");
	qRegisterMetaType<ProcessInfo>("ProcessInfo");
	qRegisterMetaType<Model>("Model");
	qRegisterMetaType<EventCategory>("EventCategory");
	qRegisterMetaType<Usage>("Usage");
	qRegisterMetaType<EventCounter>("EventCounter");

	if (argc <= 1) {
		if (!Utils::SingleApplication()) {
			QLocalSocket socket;
			socket.connectToServer("animism-framework:desktop-app-request");
			if (!socket.waitForConnected(1000)) {
				MessageDialog::MessageDialog(Q_NULLPTR, QObject::tr("错误"), QObject::tr("通道被占用或系统异常")).exec();
				return 0;
			}
			QJsonObject _json_obj;
			_json_obj.insert("request", "active");
			socket.write(QJsonDocument(_json_obj).toJson());
			socket.waitForBytesWritten(1000);
			return 0;
		}
		else {
			app.setApplicationName("Animism Engine");
			app.setApplicationDisplayName(QObject::tr("Animism 进程行为感知引擎"));

			auto _sys_productVersion = QSysInfo::productVersion();

			if (_sys_productVersion != "10" || !Utils::IsWow64())
			{
				MessageDialog::MessageDialog(nullptr, QObject::tr("提示"), QObject::tr("最低要求Windows10 64位")).exec();
				return 0;
			}

			Animism* ui = new Animism();
			ui->show();

			QLocalServer* _ptr_server = new QLocalServer();
			_ptr_server->setMaxPendingConnections(65534);
			_ptr_server->listen("animism-framework:desktop-app-request");
			QObject::connect(_ptr_server, &QLocalServer::newConnection, [=]() {
				while (_ptr_server->hasPendingConnections()) {
					auto _socket = _ptr_server->nextPendingConnection();
					QObject::connect(_socket, &QLocalSocket::readyRead, [=]() {
						while (_socket->bytesAvailable()) {
							auto datagram = _socket->readAll();
							QJsonParseError error;
							auto document = QJsonDocument::fromJson(datagram, &error);
							if (QJsonParseError::NoError != error.error)
								continue;
							if (document.isNull() || document.isEmpty())
								continue;
							if (!document.isObject())
								continue;
							auto object = document.object();
							if (!object.contains("request"))
								continue;
							if (object.value("request").toString() == "active")
								Utils::ActiveUi(ui);
							else
								continue;
						}
						});
					QObject::connect(_socket, &QLocalSocket::disconnected, [=]() {
						_socket->deleteLater();
						});
				}
				});
		}
	}
	else {
		app.setApplicationName("Animism Analysis");
		app.setApplicationDisplayName(QObject::tr("Animism 进程行为分析"));
		Analysis* ui = new Analysis(QString::fromLocal8Bit(argv[1]));
		ui->show();
		if (argc > 2) {
			QStringList _strlist_files;
			for (int i = 2; i < argc; i++)
				_strlist_files << argv[i];
			QProcess::startDetached(argv[0], _strlist_files);
		}
	}
	return app.exec();
}