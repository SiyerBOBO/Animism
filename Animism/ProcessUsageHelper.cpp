#include "ProcessUsageHelper.h"

ProcessUsageHelper::ProcessUsageHelper(quint64 pid, QObject* parent)
	:QObject(parent), m_currentPid(pid)
{
	m_hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, (DWORD)(m_currentPid));

	if (!m_hProcess)
		return;

	// get init data for cpu query
	SYSTEM_INFO sysInfo;
	FILETIME ftime, fsys, fuser;

	GetSystemInfo(&sysInfo);
	numProcessors = sysInfo.dwNumberOfProcessors;

	GetSystemTimeAsFileTime(&ftime);
	memcpy(&lastCPU, &ftime, sizeof(FILETIME));

	GetProcessTimes(m_hProcess, &ftime, &ftime, &fsys, &fuser);
	memcpy(&lastSysCPU, &fsys, sizeof(FILETIME));
	memcpy(&lastUserCPU, &fuser, sizeof(FILETIME));
}

ProcessUsageHelper::~ProcessUsageHelper()
{
	if (m_hProcess != NULL) {
		CloseHandle(m_hProcess);
		m_hProcess = NULL;
		numProcessors = 0;
	}
}

QPair<double, quint64> ProcessUsageHelper::getData()
{
	if (m_hProcess == NULL) 
		return QPair<double, qint64>();

	// cpu query
	FILETIME ftime, fsys, fuser;
	ULARGE_INTEGER now, sys, user;
	double percent;

	GetSystemTimeAsFileTime(&ftime);
	memcpy(&now, &ftime, sizeof(FILETIME));

	GetProcessTimes(m_hProcess, &ftime, &ftime, &fsys, &fuser);
	memcpy(&sys, &fsys, sizeof(FILETIME));
	memcpy(&user, &fuser, sizeof(FILETIME));
	percent = (sys.QuadPart - lastSysCPU.QuadPart) +
		(user.QuadPart - lastUserCPU.QuadPart);
	percent /= (now.QuadPart - lastCPU.QuadPart);
	percent /= numProcessors;
	lastCPU = now;
	lastUserCPU = user;
	lastSysCPU = sys;

	// ram query
	PROCESS_MEMORY_COUNTERS pmc;
	GetProcessMemoryInfo(m_hProcess, &pmc, sizeof(pmc));
	return qMakePair<double, quint64>(percent, pmc.WorkingSetSize);
}