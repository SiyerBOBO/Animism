#pragma once

#include <QObject>
#include <QPointer>
#include "windows.h"
#include "TCHAR.h"
#include "pdh.h"
#include "psapi.h"

class ProcessUsageHelper : public QObject
{
	Q_OBJECT
public:
	explicit ProcessUsageHelper(quint64 pid, QObject* parent = 0);
	~ProcessUsageHelper();

	QPair<double, quint64> getData();

private: // fields
	quint64 m_currentPid = 0;
	// cpu
	ULARGE_INTEGER lastCPU;
	ULARGE_INTEGER lastSysCPU;
	ULARGE_INTEGER lastUserCPU;
	int numProcessors = 0;
	HANDLE m_hProcess = NULL;
};