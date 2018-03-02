// app.cpp
//

//-----------------------------------------------------------------------------

//
// Includes
//

#pragma region Includes

#include "stdafx.h"

// Common
#include "app.h"

// Windows
#include <commdlg.h>

#pragma endregion

//-----------------------------------------------------------------------------

using namespace attachment;

//-----------------------------------------------------------------------------

JApplication::JApplication(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR szcl, int ncs)
{
	hinstApp = hInstance;
	hinstPrev = hPrevInstance;
	lpszCmdLine = szcl ? szcl : GetCommandLine();
	nCmdShow = ncs;

	hiMain16 = hiMain32 = 0;

	hdlgCurrent = 0;
	haccelCurrent = 0;
}

int JApplication::Run()
{
	while (true) {
		std::vector<HANDLE> handles;
		getWaitObjects(handles);
		DWORD result = handles.empty()
			? MsgWaitForMultipleObjects(0, 0, FALSE, IdleTimeout(), QS_ALLINPUT)
			: MsgWaitForMultipleObjects(handles.size(), &handles[0], FALSE, IdleTimeout(), QS_ALLINPUT);
		_ASSERT(result != WAIT_FAILED);
		if (result >= WAIT_OBJECT_0 && result < WAIT_OBJECT_0 + handles.size()) {
			onSignaled(handles[WAIT_OBJECT_0 + handles.size() - result - 1]);
		} else if (result == WAIT_OBJECT_0 + handles.size()) {
			MSG msg;
			while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
				if (msg.message == WM_QUIT)
					return (int)msg.wParam;

				HWND hwnd = 0;
				for each (HWND const& v in m_setPropertySheet) {
					if (PropSheet_IsDialogMessage(v, &msg)) {
						hwnd = v;
						if (!PropSheet_GetCurrentPageHwnd(v)) {
							DelPropertySheet(v);
							DestroyWindow(v);
						}
						break;
					}
				}
				if (!hwnd &&
					!TranslateAccelerator(hdlgCurrent, haccelCurrent, &msg) &&
					!IsDialogMessage(hdlgCurrent, &msg))
				{
					TranslateMessage(&msg);
					DispatchMessage(&msg);
				}
				if (m_setWindow.empty()) {
					PostQuitMessage(0);
				}
			}
			onInput();
		} else if (result == WAIT_TIMEOUT) {
			onIdleTimeout();
		}
	}
}

void JApplication::Clear()
{
	std::set<HWND> setHwnd;
	setHwnd = m_setPropertySheet;
	for each (HWND const& v in setHwnd) {
		if (IsWindow(v)) DestroyWindow(v);
	}
	m_setPropertySheet.clear();
	setHwnd = m_setWindow;
	for each (HWND const& v in setHwnd) {
		if (IsWindow(v)) DestroyWindow(v);
	}
	m_setWindow.clear();

	hiMain16 = hiMain32 = 0;

	hdlgCurrent = 0;
	haccelCurrent = 0;
}

int JApplication::Iteration()
{
	int retval = 0;
	Init();
	if (InitInstance()) retval = Run();
	Clear();
	Done();
	return retval;
}

void JApplication::AddWindow(HWND hwnd)
{
	m_setWindow.insert(hwnd);
}

void JApplication::DelWindow(HWND hwnd)
{
	m_setWindow.erase(hwnd);
}

void JApplication::AddPropertySheet(HWND hwnd)
{
	m_setPropertySheet.insert(hwnd);
}

void JApplication::DelPropertySheet(HWND hwnd)
{
	m_setPropertySheet.erase(hwnd);
}

std::tstring JApplication::LoadString(UINT uID)
{
	static TCHAR buffer[1024];
	::LoadString(hinstApp, uID, buffer, _countof(buffer));
	return buffer;
}

//-----------------------------------------------------------------------------

// The End.