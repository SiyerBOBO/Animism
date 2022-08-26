#pragma comment (lib,"Dwmapi.lib")
#pragma comment (lib,"user32.lib")
#include "WinEffectHelper.h"

void setAcrylicEffect(HWND hWnd, DWORD accentFlags, DWORD gradientColor, DWORD animationId)
{

	//�����ǿ���Ч��
	HMODULE hUser = GetModuleHandle(L"user32.dll");
	if (hUser)
	{
		pfnSetWindowCompositionAttribute setWindowCompositionAttribute = (pfnSetWindowCompositionAttribute)GetProcAddress(hUser, "SetWindowCompositionAttribute");
		if (setWindowCompositionAttribute)
		{
			ACCENT_POLICY accent = { ACCENT_ENABLE_ACRYLICBLURBEHIND, accentFlags, gradientColor, animationId };
			WINDOWCOMPOSITIONATTRIBDATA data;
			data.Attrib = WCA_ACCENT_POLICY;
			data.pvData = &accent;
			data.cbData = sizeof(accent);
			setWindowCompositionAttribute(hWnd, &data);
		}
	}
}

void setAeroEffect(HWND hWnd)
{
	//����AeroЧ��
	HMODULE hUser = GetModuleHandle(L"user32.dll");
	if (hUser)
	{
		pfnSetWindowCompositionAttribute setWindowCompositionAttribute = (pfnSetWindowCompositionAttribute)GetProcAddress(hUser, "SetWindowCompositionAttribute");
		if (setWindowCompositionAttribute)
		{
			ACCENT_POLICY accent = { ACCENT_ENABLE_BLURBEHIND, 0,0,0 };
			WINDOWCOMPOSITIONATTRIBDATA data;
			data.Attrib = WCA_ACCENT_POLICY;
			data.pvData = &accent;
			data.cbData = sizeof(accent);
			setWindowCompositionAttribute(hWnd, &data);
		}
	}
}

bool setShadowEffect(bool class_amended, HWND hWnd, bool newShadow)
{
	//���û��ȥ���ɵ���Ӱ����ȥ��

	if (class_amended == false)
	{
		class_amended = true;
		DWORD class_style = ::GetClassLong(hWnd, GCL_STYLE);
		class_style &= ~CS_DROPSHADOW;
		::SetClassLong(hWnd, GCL_STYLE, class_style);
	}

	//����newShadow�����Ƿ�����µ���Ӱ
	DWMNCRENDERINGPOLICY val = DWMNCRP_ENABLED;
	::DwmSetWindowAttribute(hWnd, DWMWA_NCRENDERING_POLICY, &val, sizeof(DWMNCRENDERINGPOLICY));

	MARGINS m = { -1 };
	if (!newShadow) m = { 0 };
	HRESULT hr = ::DwmExtendFrameIntoClientArea(hWnd, &m);
	if (SUCCEEDED(hr))
	{

	}

	return class_amended;

}

//ֱ�������Ӱ
void addShadowEffect(bool shadowEnable, HWND hWnd)
{
	if (shadowEnable)
	{
		BOOL bEnable = false;
		::DwmIsCompositionEnabled(&bEnable);
		if (bEnable)
		{
			DWMNCRENDERINGPOLICY ncrp = DWMNCRP_ENABLED;
			::DwmSetWindowAttribute(hWnd, DWMWA_NCRENDERING_POLICY, &ncrp, sizeof(ncrp));
			MARGINS margins = { -1 };
			::DwmExtendFrameIntoClientArea(hWnd, &margins);
		}
	}
}

void setWindowFrame(HWND hWnd, int left, int top, int right, int buttom)
{
	SetWindowLongPtr(hWnd, GWL_STYLE, WS_POPUP | WS_CAPTION | WS_THICKFRAME | WS_MAXIMIZEBOX | WS_MINIMIZEBOX);
	//��client������չ���������Ϻͱ߿��ϣ�ֻ��1�����ص�����
	const MARGINS shadow = { left,top,right,buttom };
	DwmExtendFrameIntoClientArea(hWnd, &shadow);
}

void setWindowAnimation(HWND hWnd)
{
	DWORD style = ::GetWindowLong(hWnd, GWL_STYLE);
	SetWindowLong(hWnd, GWL_STYLE, style | WS_MAXIMIZEBOX | WS_THICKFRAME | WS_CAPTION | CS_DBLCLKS);
}

void adjustMaximizedClientRect(HWND hWnd, LPARAM lParam)
{
	auto monitor = ::MonitorFromWindow(hWnd, MONITOR_DEFAULTTONULL);
	if (!monitor) { return; }
	MONITORINFO monitor_info{};
	monitor_info.cbSize = sizeof(monitor_info);
	if (!::GetMonitorInfoW(monitor, &monitor_info)) { return; }
	auto& params = *reinterpret_cast<NCCALCSIZE_PARAMS*>(lParam);
	params.rgrc[0] = monitor_info.rcWork;
}

void moveWindow(HWND hWnd)
{
	if (ReleaseCapture())
		SendMessage(hWnd, WM_SYSCOMMAND, SC_MOVE + HTCAPTION, 0);
}