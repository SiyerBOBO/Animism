#pragma once
#include <windows.h>
#include <QObject>
#include <QProcess>
#include <QFileInfo>
#include <QVariantMap>
#include "Model.h"

namespace Utils
{
	bool SingleApplication();
	void ActiveUi(QWidget* widget);
	void CenterUi(QWidget* widget);
	void KillMe();
	bool IsWow64();

	//bool compressData(QByteArray src, QByteArray& dst);
	//bool uncompressData(QByteArray src, QByteArray& dst);

	bool GetFileMd5ByPath(QString filepath, QString& result);
	bool GetPathByProcessID(int pid, QString& path);
	bool GetIconByPath(QString filepath, QIcon& icon);
	bool GetCurrentProcessMap(QMap<qint64, QString>& app_pid);

	std::wstring GetDosNameFromNtName(PCWSTR name);

	bool getFinalPathNameByHandle(const HANDLE handle, std::wstring& path);
	std::wstring formatError(unsigned long errorCode);
	bool getModuleName(HANDLE process, HMODULE handle, std::wstring& name);

	bool getInformationByWindow(ProcessInfo& info);
	void declareWindow();

	bool CheckFileRelation(const QString& strExt, const QString& strAppKey);
	void RegisterFileRelation(const QString& strExt, const QString& strAppName, const QString& strAppKey, const QString& strDefaultIcon, const QString& strDescribe);
}