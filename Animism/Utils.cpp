#include "Utils.h"
#include <QFile>
#include <QFileInfo>
#include <QFileIconProvider>
#include <Windows.h>
#include <QCryptographicHash>
#include <QApplication>
#include <QDesktopWidget>
#include <QWidget>

#include <Psapi.h>
#include <tlhelp32.h>
#include <comdef.h>
#include <fileapi.h>
#include <sstream>
#include <string>
#include <tchar.h>

bool Utils::GetFileMd5ByPath(QString filepath, QString& result)
{
	QFile sourceFile(filepath);
	qint64 fileSize = sourceFile.size();
	const qint64 bufferSize = 4096;

	if (!sourceFile.open(QIODevice::ReadOnly))
		return false;
	char buffer[bufferSize];
	int bytesRead;
	int readSize = qMin(fileSize, bufferSize);
	QCryptographicHash hash(QCryptographicHash::Md5);
	while (readSize > 0 && (bytesRead = sourceFile.read(buffer, readSize)) > 0) {
		fileSize -= bytesRead;
		hash.addData(buffer, bytesRead);
		readSize = qMin(fileSize, bufferSize);
	}
	sourceFile.close();
	result = hash.result().toHex();
	return true;
}

bool Utils::SingleApplication() {
	HANDLE hMutex = NULL;
	hMutex = CreateMutex(NULL, FALSE, qApp->applicationName().toStdWString().c_str());
	if (hMutex != NULL)
	{
		if (ERROR_ALREADY_EXISTS == GetLastError())
		{
			ReleaseMutex(hMutex);
			return false;
		}
	}
	return true;
}

void Utils::ActiveUi(QWidget* widget) {
	if (Q_NULLPTR == widget)
		return;
	if (widget->isHidden())
		widget->setHidden(false);
	WId wid = widget->winId();
	widget->activateWindow();
	widget->raise();
	SetForegroundWindow((HWND)wid);
	SwitchToThisWindow((HWND)wid, TRUE);
}

void Utils::CenterUi(QWidget* widget)
{
	QDesktopWidget* desktop = QApplication::desktop();
	widget->move((desktop->width() - widget->width()) / 2, (desktop->height() - widget->height()) / 2);
}

void Utils::KillMe()
{
	HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, qApp->applicationPid());
	if (hProcess)
		TerminateProcess(hProcess, -1);
}

bool Utils::IsWow64()
{
	typedef void (WINAPI* PGNSI)(LPSYSTEM_INFO);
	PGNSI pGNSI;
	SYSTEM_INFO si;
	ZeroMemory(&si, sizeof(SYSTEM_INFO));
	pGNSI = (PGNSI)GetProcAddress(GetModuleHandle(TEXT("kernel32.dll")), "GetNativeSystemInfo");
	if (pGNSI)
	{
		pGNSI(&si);
		if (si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64 ||
			si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_IA64 ||
			si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_ARM64)
			return TRUE;
	}
	return FALSE;
}

//extern "C"
//{
//	__declspec(dllexport) int compress(unsigned char* dest, unsigned long* destLen, const unsigned char* source, unsigned long sourceLen);
//	__declspec(dllexport) int uncompress(unsigned char* dest, unsigned long* destLen, const unsigned char* source, unsigned long sourceLen);
//}

//bool Utils::compressData(QByteArray src, QByteArray& dst)
//{
//	unsigned long len_compress;
//	unsigned char* buffer_compress = new unsigned char[40960];
//	int _isfail = compress(buffer_compress, &len_compress, (unsigned char*)src.data(), src.length());
//	// if success, result == 0
//	if (!_isfail) {
//		QByteArray data = QByteArray((const char*)buffer_compress, len_compress);
//		dst.clear();
//		for (QByteArray::iterator it = data.begin(); it != data.end(); it++)
//			dst.append(*it);
//	}
//	delete[] buffer_compress;
//	return !_isfail;
//}
//
//bool Utils::uncompressData(QByteArray src, QByteArray& dst)
//{
//	unsigned long len_uncompress = 40960;
//	unsigned char* buffer_uncompress = new unsigned char[40960];
//	int _isfail = uncompress(buffer_uncompress, &len_uncompress, (unsigned char*)src.data(), src.length());
//	// if success, result == 0
//	if (!_isfail) {
//		QByteArray data = QByteArray((const char*)buffer_uncompress, len_uncompress);
//		dst.clear();
//		for (QByteArray::iterator it = data.begin(); it != data.end(); it++)
//			dst.append(*it);
//	}
//	delete[] buffer_uncompress;
//	return !_isfail;
//}

bool Utils::GetPathByProcessID(int pid, QString& path)
{
	path.clear();
	HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
	if (!hProcess) {
		return false;
	}
	WCHAR filePath[MAX_PATH];
	DWORD ret = GetModuleFileNameEx(hProcess, NULL, filePath, MAX_PATH);
	CloseHandle(hProcess);
	if (!ret)
		return false;
	path = QString::fromStdWString(filePath);
	return true;
}

bool Utils::GetIconByPath(QString filepath, QIcon& icon)
{
	QFileInfo info(filepath);
	if (!info.exists())
		return false;
	QFileIconProvider iconpro;
	icon = iconpro.icon(info);
	return true;
}

bool Utils::GetCurrentProcessMap(QMap<qint64, QString>& app_pid)
{
	app_pid.clear();
	PROCESSENTRY32 pe32;
	pe32.dwSize = sizeof(pe32);
	HANDLE hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (hProcessSnap == INVALID_HANDLE_VALUE)
		return false;
	BOOL bMore = Process32First(hProcessSnap, &pe32);
	while (bMore)
	{
		QString exeName = QString::fromUtf16(reinterpret_cast<const unsigned short*>(pe32.szExeFile));
		app_pid[pe32.th32ProcessID] = exeName;
		bMore = Process32Next(hProcessSnap, &pe32);
	}
	CloseHandle(hProcessSnap);
	return true;
}

std::wstring Utils::GetDosNameFromNtName(PCWSTR name) {
	static std::vector<std::pair<std::wstring, std::wstring>> deviceNames;
	static bool first = true;
	if (first) {
		auto drives = ::GetLogicalDrives();
		int drive = 0;
		while (drives) {
			if (drives & 1) {
				// drive exists
				WCHAR driveName[] = { L"X:" };
				driveName[0] = (WCHAR)(drive + 'A');
				WCHAR path[MAX_PATH];
				if (::QueryDosDevice(driveName, path, MAX_PATH)) {
					deviceNames.push_back({ path, driveName });
				}
			}
			drive++;
			drives >>= 1;
		}
		first = false;
	}

	for (auto& [ntName, dosName] : deviceNames) {
		if (::_wcsnicmp(name, ntName.c_str(), ntName.size()) == 0)
			return dosName + (name + ntName.size());
	}
	return L"";
}

bool Utils::getFinalPathNameByHandle(const HANDLE handle, std::wstring& path)
{
	DWORD size = GetFinalPathNameByHandleW(handle, nullptr, 0, FILE_NAME_NORMALIZED);
	if (size) {
		path.resize(size);
		if (!GetFinalPathNameByHandleW(handle, path.data(), size, FILE_NAME_NORMALIZED))
			return false;
		return true;
	}
	return false;
}

std::wstring Utils::formatError(unsigned long errorCode)
{
	return _com_error(errorCode).ErrorMessage();
}

bool Utils::getModuleName(HANDLE process, HMODULE handle, std::wstring& name)
{
	size_t size = 0;
	do {
		name.resize(1024);
		size = GetModuleFileNameExW(process, handle, name.data(), name.size());
	} while (GetLastError() == ERROR_INSUFFICIENT_BUFFER);

	if (!size)
		return false;
	name.resize(size);
	return true;
}

bool Utils::getInformationByWindow(ProcessInfo& info)
{
	POINT pt;
	::GetCursorPos(&pt);
	auto g_hWnd = ::WindowFromPoint(pt);
	if (!g_hWnd)
		return false;

	HWND grayHwnd = ::GetWindow(g_hWnd, GW_CHILD);
	RECT tempRc;
	BOOL bFind = FALSE;
	while (grayHwnd)
	{
		::GetWindowRect(grayHwnd, &tempRc);
		if (::PtInRect(&tempRc, pt))
		{
			bFind = TRUE;
			break;
		}
		else
			grayHwnd = ::GetWindow(grayHwnd, GW_HWNDNEXT);

	}//while
	if (bFind == TRUE)
	{
		bFind = FALSE;
		g_hWnd = grayHwnd;
	}
	unsigned long iPID = 0;
	GetWindowThreadProcessId(g_hWnd, &iPID);

	QMap<qint64, QString> app_pid;
	GetCurrentProcessMap(app_pid);

	info.Pid = iPID;
	info.Name = app_pid.value(iPID);
	GetPathByProcessID(iPID, info.FilePath);
	QIcon icon;
	if (GetIconByPath(info.FilePath, icon))
		info.Icon = icon.pixmap(256, 256);
	else
		info.Icon = QIcon(":/AnimismAI/Resource/filetype_exe.png").pixmap(256, 256);
	GetFileMd5ByPath(info.FilePath, info.Md5);

	return true;
}

void Utils::declareWindow()
{
	POINT pnt;
	RECT rc;

	HWND DeskHwnd = ::GetDesktopWindow();
	HDC DeskDC = ::GetWindowDC(DeskHwnd);

	int oldRop2 = SetROP2(DeskDC, 10);

	::GetCursorPos(&pnt);
	HWND UnHwnd = ::WindowFromPoint(pnt);

	HWND grayHwnd = ::GetWindow(UnHwnd, GW_CHILD);

	RECT tempRc;
	BOOL bFind = FALSE;
	//找到标记
	while (grayHwnd)
	{
		::GetWindowRect(grayHwnd, &tempRc);
		if (::PtInRect(&tempRc, pnt))//判断点pnt是否在RECT对象tempRc 如果在，则返回非零
		{
			bFind = TRUE;
			break;
		}
		else
			grayHwnd = ::GetWindow(grayHwnd, GW_HWNDNEXT);
	}
	if (bFind == TRUE)
	{
		bFind = FALSE;
		UnHwnd = grayHwnd;
	}

	::GetWindowRect(UnHwnd, &rc);

	if (rc.left < 0) rc.left = 0;
	if (rc.top < 0) rc.top = 0;

	HPEN newPen = ::CreatePen(0, 3, RGB(125, 0, 125));
	HGDIOBJ oldPen = ::SelectObject(DeskDC, newPen);
	::Rectangle(DeskDC, rc.left, rc.top, rc.right, rc.bottom);

	Sleep(100);
	::Rectangle(DeskDC, rc.left, rc.top, rc.right, rc.bottom);

	::SetROP2(DeskDC, oldRop2);
	::SelectObject(DeskDC, oldPen);
	::DeleteObject(newPen);
	::ReleaseDC(DeskHwnd, DeskDC);

	DeskDC = NULL;
}


bool Utils::CheckFileRelation(const QString& strExt, const QString& strAppKey)
{
	int nRet = FALSE;
	HKEY hExtKey;
	TCHAR szPath[_MAX_PATH];
	DWORD dwSize = sizeof(szPath);
	if (RegOpenKey(HKEY_CLASSES_ROOT, strExt.toStdWString().c_str(), &hExtKey) == ERROR_SUCCESS)
	{
		RegQueryValueEx(hExtKey, NULL, NULL, NULL, (LPBYTE)szPath, &dwSize);
		//if(_stricmp(szPath,strAppKey)==0)
		if (wcsicmp(szPath, strAppKey.toStdWString().c_str()) == 0)
		{
			nRet = TRUE;
		}
		RegCloseKey(hExtKey);
		return nRet;
	}
	return nRet;
}

/*
* 注册文件关联
* strExe: 要检测的扩展名(例如: ".txt")
* strAppName: 要关联的应用程序名(例如: "C:/MyApp/MyApp.exe")
* strAppKey: ExeName扩展名在注册表中的键值(例如: "txtfile")
* strDefaultIcon: 扩展名为strAppName的图标文件(例如: *"C:/MyApp/MyApp.exe,0")
* strDescribe: 文件类型描述
*/
void Utils::RegisterFileRelation(const QString& strExt, const QString& strAppName, const QString& strAppKey, const QString& strDefaultIcon, const QString& strDescribe)
{
	TCHAR strTemp[_MAX_PATH];
	HKEY hKey;

	RegCreateKey(HKEY_CLASSES_ROOT, strExt.toStdWString().c_str(), &hKey);
	RegSetValue(hKey, _T(""), REG_SZ, strAppKey.toStdWString().c_str(), lstrlen(strAppKey.toStdWString().c_str()) + 1);
	RegCloseKey(hKey);

	HKEY RoothKey;
	int lxx = RegCreateKey(HKEY_CLASSES_ROOT, strAppKey.toStdWString().c_str(), &hKey);
	RegSetValueW(hKey, _T(""), REG_SZ, strDescribe.toStdWString().c_str(), lstrlen(strDescribe.toStdWString().c_str()) + 1);
	RegCloseKey(hKey);

	_stprintf(strTemp, _T("%s\\DefaultIcon"), strAppKey.toStdWString().c_str());
	RegCreateKey(HKEY_CLASSES_ROOT, strTemp, &hKey);
	RegSetValue(hKey, _T(""), REG_SZ, strDefaultIcon.toStdWString().c_str(), lstrlen(strDefaultIcon.toStdWString().c_str()) + 1);
	RegCloseKey(hKey);

	_stprintf(strTemp, _T("%s\\Shell"), strAppKey.toStdWString().c_str());
	RegCreateKey(HKEY_CLASSES_ROOT, strTemp, &hKey);
	_stprintf(strTemp, _T("%s"), _T("Open"));
	lxx = RegSetValueW(hKey, _T(""), REG_SZ, strTemp, lstrlen(strTemp) + 1);
	RegCloseKey(hKey);

	_stprintf(strTemp, _T("%s\\Shell\\Open\\Command"), strAppKey.toStdWString().c_str());
	RegCreateKey(HKEY_CLASSES_ROOT, strTemp, &hKey);
	_stprintf(strTemp, _T("\"%s\" \"%%1\""), strAppName.toStdWString().c_str());
	RegSetValue(hKey, _T(""), REG_SZ, strTemp, lstrlen(strTemp) + 1);
	RegCloseKey(hKey);
}