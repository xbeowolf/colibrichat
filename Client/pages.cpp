
//-----------------------------------------------------------------------------

//
// Includes
//

#pragma region Includes

#include "stdafx.h"

// Windows API
#include <strsafe.h>
#include <richedit.h>

// Common
#include "stylepr.h"
//#include "dCRC.h"
#include "Profile.h"

// Project
#include "..\ColibriProtocol.h"
#include "resource.h"
#include "client.h"

#pragma endregion

using namespace colibrichat;

//-----------------------------------------------------------------------------

CHARFORMAT cfDefault =
{
	sizeof(cfDefault), // cbSize
	CFM_BOLD | CFM_COLOR | CFM_FACE | CFM_ITALIC | CFM_OFFSET | CFM_SIZE | CFM_UNDERLINE, // dwMask
	0, // dwEffects
	10*20, // yHeight
	0, // yOffset
	RGB(0, 0, 128), // crTextColor
	DEFAULT_CHARSET, // bCharSet
	DEFAULT_PITCH | FF_ROMAN, // bPitchAndFamily
	TEXT("Tahoma") // szFaceName
};

//-----------------------------------------------------------------------------

// Time funstions
static void FileTimeToLocalTime(const FILETIME &ft, SYSTEMTIME &st)
{
	FILETIME temp;
	FileTimeToLocalFileTime(&ft, &temp);
	FileTimeToSystemTime(&temp, &st);
}

static void CALLBACK GetSystemFileTime(FILETIME& ft)
{
	SYSTEMTIME st;
	GetSystemTime(&st);
	SystemTimeToFileTime(&st, &ft);
}

//-----------------------------------------------------------------------------

//
// JPage
//

bool CALLBACK JClient::JPage::Write(HWND hwnd, const TCHAR* str)
{
	static STYLEFIELD sf;
	static STYLERANGE sr[32];
	static TCHAR EvBuffer[4096];
	static const TCHAR* StyleName[] =
	{
		TEXT("Default"), TEXT("Default style"), // 0
		TEXT("Time"), TEXT("Style of time"), // 1
		TEXT("Msg"), TEXT("Style of messages"), // 2
		TEXT("Descr"), TEXT("Style of descriptions"), // 3
		TEXT("Info"), TEXT("Style of information"), // 4
		TEXT("Warning"), TEXT("Style of warnings"), // 5
		TEXT("Error"), TEXT("Style of errors"), // 6
		0, 0
	};
	static const CHARFORMAT cf[] =
	{
		// Default: black
		{ // 0
			sizeof(CHARFORMAT), // cbSize
			CFM_BOLD | CFM_COLOR | CFM_FACE | CFM_ITALIC | CFM_SIZE | CFM_STRIKEOUT | CFM_UNDERLINE, // dwMask
			0, // dwEffects
			12*20, // yHeight
			0, // yOffset
			RGB(0, 0, 0), // crTextColor
			DEFAULT_CHARSET, // bCharSet
			DEFAULT_PITCH, // bPitchAndFamily
			TEXT("Times New Roman") // szFaceName
		},
		// Time: brown
		{ // 1
			sizeof(CHARFORMAT), // cbSize
			CFM_BOLD | CFM_COLOR, // dwMask
			0, // dwEffects
			12*20, // yHeight
			0, // yOffset
			RGB(80, 80, 0), // crTextColor
			DEFAULT_CHARSET, // bCharSet
			DEFAULT_PITCH, // bPitchAndFamily
			TEXT("Times New Roman") // szFaceName
		},
		// Msg: navy
		{ // 2
			sizeof(CHARFORMAT), // cbSize
			CFM_BOLD | CFM_COLOR, // dwMask
			0, // dwEffects
			12*20, // yHeight
			0, // yOffset
			RGB(0, 0, 128), // crTextColor
			DEFAULT_CHARSET, // bCharSet
			DEFAULT_PITCH, // bPitchAndFamily
			TEXT("Times New Roman") // szFaceName
		},
		// Descr: green
		{ // 3
			sizeof(CHARFORMAT), // cbSize
			CFM_BOLD | CFM_COLOR, // dwMask
			0, // dwEffects
			12*20, // yHeight
			0, // yOffset
			RGB(0, 128, 0), // crTextColor
			DEFAULT_CHARSET, // bCharSet
			DEFAULT_PITCH, // bPitchAndFamily
			TEXT("Times New Roman") // szFaceName
		},
		// Info: green
		{ // 4
			sizeof(CHARFORMAT), // cbSize
			CFM_BOLD | CFM_COLOR, // dwMask
			0, // dwEffects
			12*20, // yHeight
			0, // yOffset
			RGB(0, 128, 128), // crTextColor
			DEFAULT_CHARSET, // bCharSet
			DEFAULT_PITCH, // bPitchAndFamily
			TEXT("Times New Roman") // szFaceName
		},
		// Warning: cyan
		{ // 5
			sizeof(CHARFORMAT), // cbSize
			CFM_BOLD | CFM_COLOR, // dwMask
			0, // dwEffects
			12*20, // yHeight
			0, // yOffset
			RGB(128, 0, 128), // crTextColor
			DEFAULT_CHARSET, // bCharSet
			DEFAULT_PITCH, // bPitchAndFamily
			TEXT("Times New Roman") // szFaceName
		},
		// Error: red
		{ // 6
			sizeof(CHARFORMAT), // cbSize
			CFM_BOLD | CFM_COLOR, // dwMask
			0, // dwEffects
			12*20, // yHeight
			0, // yOffset
			RGB(128, 0, 0), // crTextColor
			DEFAULT_CHARSET, // bCharSet
			DEFAULT_PITCH, // bPitchAndFamily
			TEXT("Times New Roman") // szFaceName
		}
	};
	sf.sr = sr;
	sf.sr_num = _countof(sr);
	sf.str = EvBuffer;
	sf.len = _countof(EvBuffer);
	return StylePrint_Append(hwnd, str, &sf, StyleName, cf, _countof(cf)) != FALSE;
}

CALLBACK JClient::JPage::JPage()
: JAttachedDialog<JClient>()
{
	m_alert = eGreen;
}

void CALLBACK JClient::JPage::activate()
{
	if (m_alert > eGreen) {
		setAlert(eGreen);
	}
}

void CALLBACK JClient::JPage::setAlert(EAlert a)
{
	ASSERT(pSource);
	if ((a > m_alert || a == eGreen) && (pSource->jpOnline != this || !pSource->m_mUser[pSource->m_idOwn].isOnline) && pSource->hwndPage) {
		m_alert = a;

		TCITEM tci;
		tci.mask = TCIF_IMAGE;
		tci.iImage = ImageIndex();
		VERIFY(TabCtrl_SetItem(pSource->m_hwndTab, pSource->getTabIndex(getID()), &tci));
	}
}

void JClient::JPage::OnHook(JEventable* src)
{
	ASSERT(pSource);
	using namespace fastdelegate;

	__super::OnHook(src);

	pSource->EvLinkIdentify += MakeDelegate(this, &JClient::JPage::OnLinkIdentify);
	pSource->EvLinkClose += MakeDelegate(this, &JClient::JPage::OnLinkClose);
}

void JClient::JPage::OnUnhook(JEventable* src)
{
	ASSERT(pSource);
	using namespace fastdelegate;

	pSource->EvLinkIdentify -= MakeDelegate(this, &JClient::JPage::OnLinkIdentify);
	pSource->EvLinkClose -= MakeDelegate(this, &JClient::JPage::OnLinkClose);

	__super::OnUnhook(src);

	SetSource(0);
}

void JClient::JPage::OnLinkIdentify(SOCKET sock, const netengine::SetAccess& access)
{
	ASSERT(pSource);
	if (m_hwndPage) {
		Enable();
	}
}

void JClient::JPage::OnLinkClose(SOCKET sock, UINT err)
{
	ASSERT(pSource);
	if (m_hwndPage) {
		Disable();
	}
}

//-----------------------------------------------------------------------------

//
// JPageLog
//

CALLBACK JClient::JPageLog::JPageLog()
: JPage()
{
	m_Groups.insert(netengine::eMessage);
	m_Groups.insert(netengine::eDescription);
	m_Groups.insert(netengine::eInformation);
	m_Groups.insert(netengine::eWarning);
	m_Groups.insert(netengine::eError);
	m_Priority = netengine::eNormal;
	etimeFormat = etimeHHMMSS;
}

void CALLBACK JClient::JPageLog::AppendRtf(const std::string& content) const
{
	CHARRANGE crMark; // Selection position
	int nTextLen; // Length of text in control

	SendMessage(m_hwndLog, EM_HIDESELECTION, TRUE, 0);

	SendMessage(m_hwndLog, EM_EXGETSEL, 0, (LPARAM)&crMark);
	nTextLen = GetWindowTextLength(m_hwndLog);

	SendMessage(m_hwndLog, EM_SETSEL, -1, -1);
	SendMessage(m_hwndLog, EM_REPLACESEL, FALSE, (LPARAM)ANSIToTstr(content).c_str());

	// Restore caret position
	if (crMark.cpMin == crMark.cpMax && crMark.cpMin == nTextLen) SendMessage(m_hwndLog, EM_SETSEL, -1, -1);
	else SendMessage(m_hwndLog, EM_EXSETSEL, 0, (LPARAM)&crMark);

	SendMessage(m_hwndLog, EM_HIDESELECTION, FALSE, 0);

	if (GetFocus() != m_hwndLog) SendMessage(m_hwndLog, WM_VSCROLL, SB_BOTTOM, 0);
}

void CALLBACK JClient::JPageLog::AppendScript(const std::tstring& content, bool withtime) const
{
	CHARRANGE crMark; // Selection position
	int nTextLen; // Length of text in control

	// Save caret position
	SendMessage(m_hwndLog, EM_EXGETSEL, 0, (LPARAM)&crMark);
	nTextLen = GetWindowTextLength(m_hwndLog);
	// Write time
	static TCHAR time[32];
	time[0] = 0;
	if (withtime)
	{
		static SYSTEMTIME st;
		GetLocalTime(&st);
		switch (etimeFormat)
		{
		case etimeHHMM:
			StringCchPrintf(time, _countof(time), TEXT("[style=time][%02u:%02u][/style] "), st.wHour, st.wMinute);
			break;
		case etimeHHMMSS:
			StringCchPrintf(time, _countof(time), TEXT("[style=time][%02u:%02u:%02u][/style] "), st.wHour, st.wMinute, st.wSecond);
			break;
		default:
			StringCchPrintf(time, _countof(time), TEXT(""));
			break;
		}
	}
	// Write whole string to log
	Write(m_hwndLog, time) && Write(m_hwndLog, content.c_str()) && Write(m_hwndLog, TEXT("\r\n"));
	// Restore caret position
	if (crMark.cpMin == crMark.cpMax && crMark.cpMin == nTextLen) SendMessage(m_hwndLog, EM_SETSEL, -1, -1);
	else SendMessage(m_hwndLog, EM_EXSETSEL, 0, (LPARAM)&crMark);

	if (GetFocus() != m_hwndLog) SendMessage(m_hwndLog, WM_VSCROLL, SB_BOTTOM, 0);
}

LRESULT WINAPI JClient::JPageLog::DlgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	LRESULT retval = TRUE;
	switch (message)
	{
	case WM_INITDIALOG:
		{
			ASSERT(pSource);
			__super::DlgProc(hWnd, message, wParam, lParam);

			m_hwndLog = GetDlgItem(hWnd, IDC_LOG);

			// Get initial windows sizes
			MapControl(m_hwndLog, rcLog);

			SendMessage(m_hwndLog, EM_SETCHARFORMAT, SCF_ALL, (LPARAM)&cfDefault);

			retval = TRUE;
			break;
		}

	case WM_DESTROY:
		{
			m_hwndLog = 0;

			__super::DlgProc(hWnd, message, wParam, lParam);
			break;
		}

	case WM_COMMAND:
		{
			switch (LOWORD(wParam))
			{
			case IDC_LOGCOPY:
				SendMessage(m_hwndLog, WM_COPY, 0, 0);
				break;

			case IDC_LOGCLEAR:
				SetWindowText(m_hwndLog, TEXT(""));
				break;

			default:
				retval = __super::DlgProc(hWnd, message, wParam, lParam);
			}
			break;
		}

	case WM_CONTEXTMENU:
		{
			if ((HWND)wParam == m_hwndLog) {
				RECT r;
				GetWindowRect((HWND)wParam, &r);
				TrackPopupMenu(GetSubMenu(JClientApp::jpApp->hmenuLog, 0), TPM_LEFTALIGN | TPM_RIGHTBUTTON,
					min(max(GET_X_LPARAM(lParam), r.left), r.right),
					min(max(GET_Y_LPARAM(lParam), r.top), r.bottom), 0, hWnd, 0);
			} else {
				__super::DlgProc(hWnd, message, wParam, lParam);
			}
			break;
		}

	case WM_INITMENUPOPUP:
		{
			if ((HMENU)wParam == GetSubMenu(JClientApp::jpApp->hmenuLog, 0)) {
				CHARRANGE cr;
				SendMessage(m_hwndLog, EM_EXGETSEL, 0, (LPARAM)&cr);
				bool cancopy = cr.cpMin != cr.cpMax;
				EnableMenuItem((HMENU)wParam, IDC_LOGCOPY,
					MF_BYCOMMAND | (cancopy ? MF_ENABLED : MF_GRAYED));
				break;
			} else {
				__super::DlgProc(hWnd, message, wParam, lParam);
			}
			break;
		}

	default: retval = __super::DlgProc(hWnd, message, wParam, lParam);
	}
	return retval;
}

//-----------------------------------------------------------------------------

//
// JPageServer
//

CALLBACK JClient::JPageServer::JPageServer()
: JPageLog()
{
}

void CALLBACK JClient::JPageServer::Enable()
{
	EnableWindow(m_hwndHost, FALSE);
	EnableWindow(m_hwndPort, FALSE);
	EnableWindow(m_hwndPass, FALSE);
	m_fEnabled = true;
}

void CALLBACK JClient::JPageServer::Disable()
{
	EnableWindow(m_hwndHost, TRUE);
	EnableWindow(m_hwndPort, TRUE);
	EnableWindow(m_hwndPass, TRUE);
	m_fEnabled = false;
}

int CALLBACK JClient::JPageServer::ImageIndex() const
{
	switch (m_alert)
	{
	case eGreen:
		return IML_SERVERGREEN;
	case eBlue:
		return IML_SERVERBLUE;
	case eYellow:
		return IML_SERVERYELLOW;
	case eRed:
	default:
		return IML_SERVERRED;
	}
}

LRESULT WINAPI JClient::JPageServer::DlgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	LRESULT retval = TRUE;
	switch (message)
	{
	case WM_INITDIALOG:
		{
			__super::DlgProc(hWnd, message, wParam, lParam);

			m_hwndHost = GetDlgItem(hWnd, IDC_HOST);
			m_hwndPort = GetDlgItem(hWnd, IDC_PORT);
			m_hwndPass = GetDlgItem(hWnd, IDC_PASS);
			m_hwndNick = GetDlgItem(hWnd, IDC_NICK);
			m_hwndStatus = GetDlgItem(hWnd, IDC_STATUS);
			m_hwndStatusImg = GetDlgItem(hWnd, IDC_STATUSIMG);
			m_hwndStatusMsg = GetDlgItem(hWnd, IDC_STATUSMSG);

			// Get initial windows sizes
			MapControl(m_hwndHost, rcHost);
			MapControl(m_hwndPort, rcPort);
			MapControl(m_hwndPass, rcPass);
			MapControl(m_hwndNick, rcNick);
			MapControl(m_hwndStatus, rcStatus);
			MapControl(m_hwndStatusImg, rcStatusImg);
			MapControl(m_hwndStatusMsg, rcStatusMsg);
			MapControl(IDC_STATIC1, rcStatic1);
			MapControl(IDC_STATIC2, rcStatic2);
			MapControl(IDC_STATIC3, rcStatic3);
			MapControl(IDC_STATIC4, rcStatic4);
			MapControl(IDC_STATIC5, rcStatic5);
			MapControl(IDC_CONNECT, rcConnect);

			// Init Host control
			SetWindowTextA(m_hwndHost, pSource->m_hostname.c_str());
			SendMessage(m_hwndHost, EM_LIMITTEXT, 100, 0);
			// Init Port control
			SetWindowText(m_hwndPort, tformat(TEXT("%u"), pSource->m_port).c_str());
			SendMessage(m_hwndPort, EM_LIMITTEXT, 5, 0);
			// Init Pass control
			SetWindowText(m_hwndPass, pSource->m_password.c_str());
			SendMessage(m_hwndPass, EM_LIMITTEXT, pSource->m_metrics.uPassMaxLength, 0);
			// Init Nick control
			SetWindowText(m_hwndNick, pSource->m_mUser[pSource->m_idOwn].name.c_str());
			SendMessage(m_hwndNick, EM_LIMITTEXT, pSource->m_metrics.uNickMaxLength, 0);
			// Init status combobox
			COMBOBOXEXITEM cbei[] = {
				{CBEIF_IMAGE | CBEIF_SELECTEDIMAGE | CBEIF_TEXT, 0, TEXT("Ready"), -1, 0, 0, 0, 0, 0},
				{CBEIF_IMAGE | CBEIF_SELECTEDIMAGE | CBEIF_TEXT, 1, TEXT("D'N'D"), -1, 1, 1, 0, 0, 0},
				{CBEIF_IMAGE | CBEIF_SELECTEDIMAGE | CBEIF_TEXT, 2, TEXT("Busy"), -1, 2, 2, 0, 0, 0},
				{CBEIF_IMAGE | CBEIF_SELECTEDIMAGE | CBEIF_TEXT, 3, TEXT("N/A"), -1, 3, 3, 0, 0, 0},
				{CBEIF_IMAGE | CBEIF_SELECTEDIMAGE | CBEIF_TEXT, 4, TEXT("Away"), -1, 4, 4, 0, 0, 0},
				{CBEIF_IMAGE | CBEIF_SELECTEDIMAGE | CBEIF_TEXT, 5, TEXT("Glass"), -1, 5, 5, 0, 0, 0},
			};
			for (int i = 0; i < _countof(cbei); i++)
				SendMessage(m_hwndStatus, CBEM_INSERTITEM, 0, (LPARAM)&cbei[i]);
			SendMessage(m_hwndStatus, CBEM_SETIMAGELIST, 0, (LPARAM)JClientApp::jpApp->himlStatus);
			SendMessage(m_hwndStatus, CB_SETCURSEL, pSource->m_mUser[pSource->m_idOwn].nStatus, 0);
			// Init status images combobox
			COMBOBOXEXITEM cbeiImg;
			cbeiImg.mask = CBEIF_IMAGE | CBEIF_SELECTEDIMAGE;
			for (int i = 0; i < IML_STATUSIMG_COUNT; i++) {
				cbeiImg.iItem = i;
				cbeiImg.iImage = i = cbeiImg.iSelectedImage = i;
				SendMessage(m_hwndStatusImg, CBEM_INSERTITEM, 0, (LPARAM)&cbeiImg);
			}
			SendMessage(m_hwndStatusImg, CBEM_SETIMAGELIST, 0, (LPARAM)JClientApp::jpApp->himlStatusImg);
			SendMessage(m_hwndStatusImg, CB_SETCURSEL, pSource->m_mUser[pSource->m_idOwn].nStatusImg, 0);
			// Init Status message control
			SetWindowText(m_hwndStatusMsg, pSource->m_mUser[pSource->m_idOwn].strStatus.c_str());
			SendMessage(m_hwndStatusMsg, EM_LIMITTEXT, pSource->m_metrics.uStatusMsgMaxLength, 0);

			// Set introduction comments
			static const std::tstring szIntro =
				TEXT("[color=red,size=16,b]Colibri Chat[/color] [color=purple]v") APPVER TEXT("[/color,/size,/b]\n")
				TEXT("[b]© Podobashev Dmitry / BEOWOLF[/b], ") APPDATE TEXT(".\n");
			AppendScript(szIntro, false);

			retval = TRUE;
			break;
		}

	case WM_DESTROY:
		{
			__super::DlgProc(hWnd, message, wParam, lParam);
			break;
		}

	case WM_SIZE:
		{
			if (wParam == SIZE_MINIMIZED) break;
			SendMessage(hWnd, BEM_ADJUSTSIZE, wParam, lParam);
			break;
		}

	case BEM_ADJUSTSIZE:
		{
			RECT rc;
			int cx = LOWORD(lParam), cy = HIWORD(lParam);
			HDWP hdwp = BeginDeferWindowPos(14);
			SetRect(&rc,
				rcLog.left,
				rcLog.top,
				cx - rcPage.right + rcLog.right,
				cy - rcPage.bottom + rcLog.bottom);
			DeferWindowPos(hdwp, m_hwndLog, 0, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, SWP_NOMOVE | SWP_NOREPOSITION | SWP_NOZORDER);
			SetRect(&rc,
				rcStatic1.left,
				cy - rcPage.bottom + rcStatic1.top,
				rcStatic1.right,
				cy - rcPage.bottom + rcStatic1.bottom);
			DeferWindowPos(hdwp, GetDlgItem(hWnd, IDC_STATIC1), 0, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, SWP_NOSIZE | SWP_NOREPOSITION | SWP_NOZORDER);
			SetRect(&rc,
				rcHost.left,
				cy - rcPage.bottom + rcHost.top,
				cx - rcPage.right + rcHost.right,
				cy - rcPage.bottom + rcHost.bottom);
			DeferWindowPos(hdwp, m_hwndHost, 0, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, SWP_NOREPOSITION | SWP_NOZORDER);
			SetRect(&rc,
				cx - rcPage.right + rcStatic2.left,
				cy - rcPage.bottom + rcStatic2.top,
				cx - rcPage.right + rcStatic2.right,
				cy - rcPage.bottom + rcStatic2.bottom);
			DeferWindowPos(hdwp, GetDlgItem(hWnd, IDC_STATIC2), 0, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, SWP_NOSIZE | SWP_NOREPOSITION | SWP_NOZORDER);
			SetRect(&rc,
				cx - rcPage.right + rcPort.left,
				cy - rcPage.bottom + rcPort.top,
				cx - rcPage.right + rcPort.right,
				cy - rcPage.bottom + rcPort.bottom);
			DeferWindowPos(hdwp, m_hwndPort, 0, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, SWP_NOSIZE | SWP_NOREPOSITION | SWP_NOZORDER);
			SetRect(&rc,
				cx - rcPage.right + rcStatic3.left,
				cy - rcPage.bottom + rcStatic3.top,
				cx - rcPage.right + rcStatic3.right,
				cy - rcPage.bottom + rcStatic3.bottom);
			DeferWindowPos(hdwp, GetDlgItem(hWnd, IDC_STATIC3), 0, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, SWP_NOSIZE | SWP_NOREPOSITION | SWP_NOZORDER);
			SetRect(&rc,
				cx - rcPage.right + rcPass.left,
				cy - rcPage.bottom + rcPass.top,
				cx - rcPage.right + rcPass.right,
				cy - rcPage.bottom + rcPass.bottom);
			DeferWindowPos(hdwp, m_hwndPass, 0, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, SWP_NOSIZE | SWP_NOREPOSITION | SWP_NOZORDER);
			SetRect(&rc,
				rcConnect.left,
				cy - rcPage.bottom + rcConnect.top,
				cx - rcPage.right + rcConnect.right,
				cy - rcPage.bottom + rcConnect.bottom);
			DeferWindowPos(hdwp, GetDlgItem(hWnd, IDC_CONNECT), 0, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, SWP_NOREPOSITION | SWP_NOZORDER);
			int dx = (cx - rcPage.right + rcPage.left)/2;
			SetRect(&rc,
				rcStatic4.left + dx,
				cy - rcPage.bottom + rcStatic4.top,
				rcStatic4.right + dx,
				cy - rcPage.bottom + rcStatic4.bottom);
			DeferWindowPos(hdwp, GetDlgItem(hWnd, IDC_STATIC4), 0, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, SWP_NOSIZE | SWP_NOREPOSITION | SWP_NOZORDER);
			SetRect(&rc,
				rcNick.left + dx,
				cy - rcPage.bottom + rcNick.top,
				rcNick.right + dx,
				cy - rcPage.bottom + rcNick.bottom);
			DeferWindowPos(hdwp, m_hwndNick, 0, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, SWP_NOREPOSITION | SWP_NOZORDER);
			SetRect(&rc,
				rcStatus.left + dx,
				cy - rcPage.bottom + rcStatus.top,
				rcStatus.right + dx,
				cy - rcPage.bottom + rcStatus.bottom);
			DeferWindowPos(hdwp, m_hwndStatus, 0, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, SWP_NOREPOSITION | SWP_NOZORDER);
			SetRect(&rc,
				rcStatic5.left + dx,
				cy - rcPage.bottom + rcStatic5.top,
				rcStatic5.right + dx,
				cy - rcPage.bottom + rcStatic5.bottom);
			DeferWindowPos(hdwp, GetDlgItem(hWnd, IDC_STATIC5), 0, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, SWP_NOSIZE | SWP_NOREPOSITION | SWP_NOZORDER);
			SetRect(&rc,
				rcStatusImg.left + dx,
				cy - rcPage.bottom + rcStatusImg.top,
				rcStatusImg.right + dx,
				cy - rcPage.bottom + rcStatusImg.bottom);
			DeferWindowPos(hdwp, m_hwndStatusImg, 0, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, SWP_NOREPOSITION | SWP_NOZORDER);
			SetRect(&rc,
				rcStatusMsg.left + dx,
				cy - rcPage.bottom + rcStatusMsg.top,
				rcStatusMsg.right + dx,
				cy - rcPage.bottom + rcStatusMsg.bottom);
			DeferWindowPos(hdwp, m_hwndStatusMsg, 0, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, SWP_NOREPOSITION | SWP_NOZORDER);
			EndDeferWindowPos(hdwp);
			break;
		}

	case WM_COMMAND:
		{
			switch (LOWORD(wParam))
			{
			case IDC_CONNECT:
				{
					if (pSource->m_clientsock) {
						pSource->Disconnect();
					} else if (pSource->m_nConnectCount) {
						pSource->m_nConnectCount = 0;
						KillTimer(pSource->hwndPage, IDT_CONNECT);
						// Update interface
						CheckDlgButton(m_hwndPage, IDC_CONNECT, FALSE);
						Disable();
						pSource->EvLog(TEXT("[style=msg]Canceled.[/style]"), true);
					} else {
						pSource->Connect(true);
					}
					break;
				}

			case IDC_STATUS:
				{
					switch (HIWORD(wParam))
					{
					case CBN_SELENDOK:
						{
							EUserStatus stat = (EUserStatus)SendMessage((HWND)lParam, CB_GETCURSEL, 0, 0);
							pSource->Send_Cmd_STATUS_Mode(pSource->m_clientsock, stat);
							break;
						}
					}
					break;
				}

			case IDC_STATUSIMG:
				{
					switch (HIWORD(wParam))
					{
					case CBN_SELENDOK:
						{
							int img = (int)SendMessage((HWND)lParam, CB_GETCURSEL, 0, 0);
							pSource->Send_Cmd_STATUS_Img(pSource->m_clientsock, img);
							break;
						}
					}
					break;
				}

			default: retval = __super::DlgProc(hWnd, message, wParam, lParam);
			}
			break;
		}

	default: retval = __super::DlgProc(hWnd, message, wParam, lParam);
	}
	return retval;
}

void JClient::JPageServer::OnHook(JEventable* src)
{
	ASSERT(pSource);
	using namespace fastdelegate;

	__super::OnHook(src);

	pSource->EvLog += MakeDelegate(this, &JClient::JPageServer::OnLog);
	pSource->EvReport += MakeDelegate(this, &JClient::JPageServer::OnReport);
}

void JClient::JPageServer::OnUnhook(JEventable* src)
{
	ASSERT(pSource);
	using namespace fastdelegate;

	pSource->EvLog -= MakeDelegate(this, &JClient::JPageServer::OnLog);
	pSource->EvReport -= MakeDelegate(this, &JClient::JPageServer::OnReport);

	__super::OnUnhook(src);
}

void JClient::JPageServer::OnLog(const std::tstring& str, bool withtime)
{
	ASSERT(pSource);
	AppendScript(str, withtime);
}

void JClient::JPageServer::OnReport(const std::tstring& str, netengine::EGroup gr, netengine::EPriority prior)
{
	ASSERT(pSource);
	if (!m_hwndPage) return; // ignore if window closed
	if (m_Groups.find(gr) != m_Groups.end() && prior < m_Priority) return;

	std::tstring msg;
	switch (gr)
	{
	case netengine::eMessage:
		msg = TEXT("[style=Msg]") + str + TEXT("[/style]");
		setAlert(eBlue);
		break;
	case netengine::eDescription:
		msg = TEXT("[style=Descr]") + str + TEXT("[/style]");
		setAlert(eBlue);
		break;
	case netengine::eInformation:
		msg = TEXT("[style=Info]") + str + TEXT("[/style]");
		setAlert(eBlue);
		break;
	case netengine::eWarning:
		msg = TEXT("[style=Warning]") + str + TEXT("[/style]");
		setAlert(eYellow);
		break;
	case netengine::eError:
		msg = TEXT("[style=Error]") + str + TEXT("[/style]");
		setAlert(eRed);
		break;
	default:
		msg = TEXT("[style=Default]") + str + TEXT("[/style]");
		break;
	}
	AppendScript(msg);
}

//-----------------------------------------------------------------------------

//
// JPageList
//

CALLBACK JClient::JPageList::JPageList()
: JPage()
{
}

void CALLBACK JClient::JPageList::Enable()
{
	EnableWindow(m_hwndChan, TRUE);
	EnableWindow(m_hwndPass, TRUE);
	EnableWindow(GetDlgItem(m_hwndPage, IDC_JOIN), TRUE);
	EnableWindow(GetDlgItem(m_hwndPage, IDC_REFRESHLIST), TRUE);
	m_fEnabled = true;
}

void CALLBACK JClient::JPageList::Disable()
{
	EnableWindow(m_hwndChan, FALSE);
	EnableWindow(m_hwndPass, FALSE);
	EnableWindow(GetDlgItem(m_hwndPage, IDC_JOIN), FALSE);
	EnableWindow(GetDlgItem(m_hwndPage, IDC_REFRESHLIST), FALSE);
	m_fEnabled = false;
}

LRESULT WINAPI JClient::JPageList::DlgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	LRESULT retval = TRUE;
	switch (message)
	{
	case WM_INITDIALOG:
		{
			ASSERT(pSource);
			__super::DlgProc(hWnd, message, wParam, lParam);

			m_hwndList = GetDlgItem(hWnd, IDC_CHANLIST);
			m_hwndChan = GetDlgItem(hWnd, IDC_JOINCHAN);
			m_hwndPass = GetDlgItem(hWnd, IDC_JOINPASS);

			// Get initial windows sizes
			MapControl(m_hwndList, rcList);
			MapControl(m_hwndChan, rcChan);
			MapControl(m_hwndPass, rcPass);
			MapControl(IDC_STATIC1, rcStatic1);
			MapControl(IDC_STATIC2, rcStatic2);
			MapControl(IDC_JOIN, rcJoin);
			MapControl(IDC_REFRESHLIST, rcRefresh);

			// Inits Channels list
			ListView_SetExtendedListViewStyle(m_hwndList,
				LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_HEADERDRAGDROP | LVS_EX_ONECLICKACTIVATE | LVS_EX_SUBITEMIMAGES);
			ListView_SetImageList(m_hwndList, JClientApp::jpApp->himlTab, LVSIL_SMALL);
			ListView_SetItemCount(m_hwndList, 64);
			static LV_COLUMN lvc[] = {
				{LVCF_FMT | LVCF_SUBITEM | LVCF_TEXT | LVCF_WIDTH, LVCFMT_LEFT,
				120, TEXT("Channel name"), -1, 0},
				{LVCF_FMT | LVCF_SUBITEM | LVCF_TEXT | LVCF_WIDTH, LVCFMT_LEFT,
				50, TEXT("Users"), -1, 0},
				{LVCF_FMT | LVCF_SUBITEM | LVCF_TEXT | LVCF_WIDTH, LVCFMT_LEFT,
				300, TEXT("Title"), -1, 0},
				{LVCF_FMT | LVCF_SUBITEM | LVCF_TEXT | LVCF_WIDTH, LVCFMT_LEFT,
				50, TEXT("Income status"), -1, 0},
				{LVCF_FMT | LVCF_SUBITEM | LVCF_TEXT | LVCF_WIDTH, LVCFMT_LEFT,
				25, TEXT("Private"), -1, 0},
				{LVCF_FMT | LVCF_SUBITEM | LVCF_TEXT | LVCF_WIDTH, LVCFMT_LEFT,
				25, TEXT("Anonymous"), -1, 0},
				{LVCF_FMT | LVCF_SUBITEM | LVCF_TEXT | LVCF_WIDTH, LVCFMT_LEFT,
				120, TEXT("Creation time"), -1, 0},
			};
			for (int i = 0; i < _countof(lvc); ++i)
				ListView_InsertColumn(m_hwndList, i, &lvc[i]);

			SendMessage(m_hwndChan, EM_LIMITTEXT, pSource->m_metrics.uChanMaxLength, 0);
			SendMessage(m_hwndPass, EM_LIMITTEXT, pSource->m_metrics.uPassMaxLength, 0);

			BuildView();

			retval = TRUE;
			break;
		}

	case WM_DESTROY:
		{
			m_hwndList = 0;

			__super::DlgProc(hWnd, message, wParam, lParam);
			break;
		}

	case WM_SIZE:
		{
			if (wParam == SIZE_MINIMIZED) break;
			SendMessage(hWnd, BEM_ADJUSTSIZE, wParam, lParam);
			break;
		}

	case BEM_ADJUSTSIZE:
		{
			RECT rc;
			int cx = LOWORD(lParam), cy = HIWORD(lParam);
			HDWP hdwp = BeginDeferWindowPos(7);
			SetRect(&rc,
				rcList.left,
				rcList.top,
				cx - rcPage.right + rcList.right,
				cy - rcPage.bottom + rcList.bottom);
			DeferWindowPos(hdwp, m_hwndList, 0, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, SWP_NOMOVE | SWP_NOREPOSITION | SWP_NOZORDER);
			SetRect(&rc,
				rcStatic1.left,
				cy - rcPage.bottom + rcStatic1.top,
				rcStatic1.right,
				cy - rcPage.bottom + rcStatic1.bottom);
			DeferWindowPos(hdwp, GetDlgItem(hWnd, IDC_STATIC1), 0, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, SWP_NOSIZE | SWP_NOREPOSITION | SWP_NOZORDER);
			SetRect(&rc,
				rcChan.left,
				cy - rcPage.bottom + rcChan.top,
				cx - rcPage.right + rcChan.right,
				cy - rcPage.bottom + rcChan.bottom);
			DeferWindowPos(hdwp, m_hwndChan, 0, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, SWP_NOREPOSITION | SWP_NOZORDER);
			SetRect(&rc,
				cx - rcPage.right + rcStatic2.left,
				cy - rcPage.bottom + rcStatic2.top,
				cx - rcPage.right + rcStatic2.right,
				cy - rcPage.bottom + rcStatic2.bottom);
			DeferWindowPos(hdwp, GetDlgItem(hWnd, IDC_STATIC2), 0, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, SWP_NOSIZE | SWP_NOREPOSITION | SWP_NOZORDER);
			SetRect(&rc,
				cx - rcPage.right + rcPass.left,
				cy - rcPage.bottom + rcPass.top,
				cx - rcPage.right + rcPass.right,
				cy - rcPage.bottom + rcPass.bottom);
			DeferWindowPos(hdwp, m_hwndPass, 0, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, SWP_NOSIZE | SWP_NOREPOSITION | SWP_NOZORDER);
			SetRect(&rc,
				cx - rcPage.right + rcJoin.left,
				cy - rcPage.bottom + rcJoin.top,
				cx - rcPage.right + rcJoin.right,
				cy - rcPage.bottom + rcJoin.bottom);
			DeferWindowPos(hdwp, GetDlgItem(hWnd, IDC_JOIN), 0, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, SWP_NOSIZE | SWP_NOREPOSITION | SWP_NOZORDER);
			SetRect(&rc,
				cx - rcPage.right + rcRefresh.left,
				cy - rcPage.bottom + rcRefresh.top,
				cx - rcPage.right + rcRefresh.right,
				cy - rcPage.bottom + rcRefresh.bottom);
			DeferWindowPos(hdwp, GetDlgItem(hWnd, IDC_REFRESHLIST), 0, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, SWP_NOSIZE | SWP_NOREPOSITION | SWP_NOZORDER);
			EndDeferWindowPos(hdwp);
			break;
		}

	case WM_COMMAND:
		{
			switch (LOWORD(wParam))
			{
			case IDC_JOIN:
				{
					if (pSource->m_clientsock) {
						std::tstring chanbuf(pSource->m_metrics.uChanMaxLength, 0), passbuf(pSource->m_metrics.uPassMaxLength, 0);
						GetWindowText(m_hwndChan, &chanbuf[0], (int)chanbuf.size()+1);
						GetWindowText(m_hwndPass, &passbuf[0], (int)passbuf.size()+1);
						std::tstring msg;
						if (pSource->CheckNick(chanbuf, msg)) { // check content
							// send only c-strings, not buffer!
							pSource->Send_Quest_JOIN(pSource->m_clientsock, chanbuf.c_str(), passbuf.c_str());
						} else {
							pSource->DisplayMessage(m_hwndChan, msg);
						}
					}
					break;
				}

			case IDC_REFRESHLIST:
				{
					if (pSource->m_clientsock) {
						Send_Quest_LIST(pSource->m_clientsock);
					}
					break;
				}

			default: retval = __super::DlgProc(hWnd, message, wParam, lParam);
			}
			break;
		}

	case WM_NOTIFY:
		{
			if (!pSource) break;

			NMHDR* pnmh = (NMHDR*)lParam;
			switch (pnmh->code)
			{
			case LVN_GETDISPINFO:
				{
					static TCHAR buffer[32];
					LV_DISPINFO* pnmv = (LV_DISPINFO*)lParam;
					if (pnmh->idFrom == IDC_CHANLIST)
					{
						if (pnmv->item.mask & LVIF_TEXT)
						{
							MapChannel::const_iterator iter = m_mChannel.find((DWORD)pnmv->item.lParam);
							if (iter == m_mChannel.end()) break;
							switch (pnmv->item.iSubItem)
							{
							case 0:
								pnmv->item.pszText = (TCHAR*)iter->second.name.c_str();
								break;

							case 1:
								StringCchPrintf(buffer, _countof(buffer), TEXT("%u"), iter->second.opened.size());
								pnmv->item.pszText = buffer;
								break;

							case 2:
								pnmv->item.pszText = (TCHAR*)iter->second.topic.c_str();
								break;

							case 3:
								pnmv->item.pszText = (TCHAR*)s_mapChanStatName[iter->second.nAutoStatus].c_str();
								break;

							case 4:
								pnmv->item.pszText = iter->second.isHidden ? TEXT("+") : TEXT("-");
								break;

							case 5:
								pnmv->item.pszText = iter->second.isAnonymous ? TEXT("+") : TEXT("-");
								break;

							case 6:
								{
									SYSTEMTIME st;
									FileTimeToLocalTime(iter->second.ftCreation, st);

									StringCchPrintf(buffer, _countof(buffer),
										TEXT("%02i:%02i:%02i, %02i.%02i.%04i"),
										st.wHour, st.wMinute, st.wSecond,
										st.wDay, st.wMonth, st.wYear);
									pnmv->item.pszText = buffer;
									break;
								}
							}
							pnmv->item.cchTextMax = lstrlen(pnmv->item.pszText);
						}
					}
					break;
				}

			case LVN_ITEMCHANGED:
				{
					LPNMLISTVIEW pnmv = (LPNMLISTVIEW)lParam;
					if (pnmh->idFrom == IDC_CHANLIST && pnmv->uChanged == LVIF_STATE) {
						MapChannel::const_iterator ic = m_mChannel.find((DWORD)pnmv->lParam);
						if (ic != m_mChannel.end() && pnmv->iItem >= 0
							&& (pnmv->uNewState & LVIS_SELECTED) != 0 && (pnmv->uOldState & LVIS_SELECTED) == 0) {
								SetWindowText(m_hwndChan, ic->second.name.c_str());
						}
					}
					break;
				}

			case NM_DBLCLK:
				{
					if (pnmh->idFrom == IDC_CHANLIST) {
						LPNMITEMACTIVATE lpnmitem = (LPNMITEMACTIVATE)lParam;
						if (lpnmitem->iItem >= 0) {
							SendMessage(hWnd, WM_COMMAND, IDC_JOIN, 0);
						}
					}
					break;
				}

			default: retval = __super::DlgProc(hWnd, message, wParam, lParam);
			}
			break;
		}

	default: retval = __super::DlgProc(hWnd, message, wParam, lParam);
	}
	return retval;
}

void CALLBACK JClient::JPageList::BuildView()
{
	for each (MapChannel::value_type const& v in m_mChannel)
		AddLine(v.first);
}

void CALLBACK JClient::JPageList::ClearView()
{
	ListView_DeleteAllItems(m_hwndList);
}

int CALLBACK JClient::JPageList::AddLine(DWORD id)
{
	LVITEM lvi;
	int index = INT_MAX;
	// Put into process log window
	lvi.mask = LVIF_PARAM | LVIF_TEXT | LVIF_IMAGE;
	lvi.iItem = index;
	lvi.iSubItem = 0;
	lvi.pszText = LPSTR_TEXTCALLBACK;
	lvi.cchTextMax = 0;
	MapChannel::const_iterator iter = m_mChannel.find(id);
	if (iter != m_mChannel.end()) {
		switch (iter->second.getStatus(pSource->m_idOwn))
		{
		case eFounder:
			lvi.iImage = IML_CHANNELMAGENTA;
			break;
		case eAdmin:
			lvi.iImage = IML_CHANNELRED;
			break;
		case eModerator:
			lvi.iImage = IML_CHANNELBLUE;
			break;
		case eMember:
			lvi.iImage = IML_CHANNELGREEN;
			break;
		case eWriter:
			lvi.iImage = IML_CHANNELCYAN;
			break;
		case eReader:
			lvi.iImage = IML_CHANNELYELLOW;
			break;
		default:
			lvi.iImage = IML_CHANNELVOID;
			break;
		}
	} else {
		lvi.iImage = IML_CHANNELVOID;
	}
	lvi.lParam = (LPARAM)id;
	index = ListView_InsertItem(m_hwndList, &lvi);
	ListView_SetItemText(m_hwndList, index, 1, LPSTR_TEXTCALLBACK);
	ListView_SetItemText(m_hwndList, index, 2, LPSTR_TEXTCALLBACK);
	ListView_SetItemText(m_hwndList, index, 3, LPSTR_TEXTCALLBACK);
	ListView_SetItemText(m_hwndList, index, 4, LPSTR_TEXTCALLBACK);
	ListView_SetItemText(m_hwndList, index, 5, LPSTR_TEXTCALLBACK);
	return index;
}

void JClient::JPageList::OnHook(JEventable* src)
{
	ASSERT(pSource);
	using namespace fastdelegate;

	__super::OnHook(src);

	pSource->EvLinkIdentify += MakeDelegate(this, &JClient::JPageList::OnLinkIdentify);
	pSource->EvTransactionProcess += MakeDelegate(this, &JClient::JPageList::OnTransactionProcess);
}

void JClient::JPageList::OnUnhook(JEventable* src)
{
	ASSERT(pSource);
	using namespace fastdelegate;

	pSource->EvLinkIdentify -= MakeDelegate(this, &JClient::JPageList::OnLinkIdentify);
	pSource->EvTransactionProcess -= MakeDelegate(this, &JClient::JPageList::OnTransactionProcess);

	__super::OnUnhook(src);
}

void JClient::JPageList::OnLinkIdentify(SOCKET sock, const netengine::SetAccess& access)
{
	ASSERT(pSource);

	Send_Quest_LIST(sock);
}

void JClient::JPageList::OnTransactionProcess(SOCKET sock, WORD message, WORD trnid, io::mem is)
{
	typedef void (CALLBACK JClient::JPageList::*TrnParser)(SOCKET, WORD, io::mem&);
	struct {
		WORD message;
		TrnParser parser;
	} responseTable[] =
	{
		{REPLY(CCPM_LIST), &JClient::JPageList::Recv_Reply_LIST},
	};
	for (int i = 0; i < _countof(responseTable); i++)
	{
		if (responseTable[i].message == message)
		{
			(this->*responseTable[i].parser)(sock, trnid, is);
			return;
		}
	}
}

void CALLBACK JClient::JPageList::Recv_Reply_LIST(SOCKET sock, WORD trnid, io::mem& is)
{
	size_t size;

	m_mChannel.clear();
	try
	{
		io::unpack(is, size);
		for (size_t i = 0; i < size; i++)
		{
			DWORD ID;
			Channel chan;

			io::unpack(is, ID);
			io::unpack(is, chan);

			m_mChannel[ID] = chan;
		}
	}
	catch (io::exception e)
	{
		switch (e.count)
		{
		case 0:
			// Report about message
			pSource->EvReport(SZ_BADTRN, netengine::eWarning, netengine::eLow);
			return;
		}
	}

	if (m_hwndPage) {
		ClearView();
		BuildView();
	}
	pSource->EvReport(tformat(TEXT("listed [b]%u[/b] channels"), m_mChannel.size()), netengine::eInformation, netengine::eNormal);
}

void CALLBACK JClient::JPageList::Send_Quest_LIST(SOCKET sock)
{
	std::ostringstream os;
	pSource->PushTrn(sock, QUEST(CCPM_LIST), 0, os.str());
}

//-----------------------------------------------------------------------------

//
// JPageUser
//

CALLBACK JClient::JPageUser::JPageUser(DWORD id, const std::tstring& nick)
: JPageLog(), rtf::Editor()
{
	m_ID = id;
	m_user.name = nick;
}

void CALLBACK JClient::JPageUser::Enable()
{
	EnableWindow(m_hwndEdit, TRUE);
	m_fEnabled = true;
}

void CALLBACK JClient::JPageUser::Disable()
{
	EnableWindow(m_hwndEdit, FALSE);
	m_fEnabled = false;
}

int CALLBACK JClient::JPageUser::ImageIndex() const
{
	switch (m_alert)
	{
	case eGreen:
		return IML_PRIVATEGREEN;
	case eYellow:
		return IML_PRIVATEYELLOW;
	case eRed:
	default:
		return IML_PRIVATERED;
	}
}

void CALLBACK JClient::JPageUser::OnSheetColor(COLORREF cr)
{
	__super::OnSheetColor(cr);

	ASSERT(pSource);
	pSource->Send_Cmd_BACKGROUND(pSource->m_clientsock, m_ID, cr);
}

void CALLBACK JClient::JPageUser::rename(DWORD idNew, const std::tstring& newname)
{
	m_ID = idNew;
	m_user.name = newname;
}

LRESULT WINAPI JClient::JPageUser::DlgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	LRESULT retval = TRUE;
	switch (message)
	{
	case WM_INITDIALOG:
		{
			static const TBBUTTON tbButtons[] =
			{
				{IML_BOLD, rtf::idcBold,
				TBSTATE_ENABLED, TBSTYLE_CHECK,
				{0, 0}, 0, 0},
				{IML_ITALIC, rtf::idcItalic,
				TBSTATE_ENABLED, TBSTYLE_CHECK,
				{0, 0}, 0, 0},
				{IML_ULINE, rtf::idcUnderline,
				TBSTATE_ENABLED, TBSTYLE_CHECK,
				{0, 0}, 0, 0},
				{IML_SUBSCRIPT, rtf::idcSubscript,
				TBSTATE_ENABLED, TBSTYLE_CHECK,
				{0, 0}, 0, 0},
				{IML_SUPERSCRIPT, rtf::idcSuperscript,
				TBSTATE_ENABLED, TBSTYLE_CHECK,
				{0, 0}, 0, 0},
				{IML_FONT, rtf::idcFont,
				TBSTATE_ENABLED, TBSTYLE_BUTTON,
				{0, 0}, 0, 0},
				{0, 0,
				0, TBSTYLE_SEP,
				{0, 0}, 0, 0},
				{IML_FRCOL, rtf::idcFrColor,
				TBSTATE_ENABLED, TBSTYLE_BUTTON,
				{0, 0}, 0, 0},
				{IML_BGCOL, rtf::idcBgColor,
				TBSTATE_ENABLED, TBSTYLE_BUTTON,
				{0, 0}, 0, 0},
				{IML_SHEETCOL, rtf::idcSheetColor,
				TBSTATE_ENABLED, TBSTYLE_BUTTON,
				{0, 0}, 0, 0},
				{0, 0,
				0, TBSTYLE_SEP,
				{0, 0}, 0, 0},
				{IML_LEFT, rtf::idcAlignLeft,
				TBSTATE_ENABLED, TBSTYLE_CHECK,
				{0, 0}, 0, 0},
				{IML_CENTER, rtf::idcAlignCenter,
				TBSTATE_ENABLED, TBSTYLE_CHECK,
				{0, 0}, 0, 0},
				{IML_RIGHT, rtf::idcAlignRight,
				TBSTATE_ENABLED, TBSTYLE_CHECK,
				{0, 0}, 0, 0},
				{IML_JUSTIFY, rtf::idcAlignJustify,
				TBSTATE_ENABLED, TBSTYLE_CHECK,
				{0, 0}, 0, 0},
				{IML_MARKS_BULLET, rtf::idcMarksBullet,
				TBSTATE_ENABLED, TBSTYLE_CHECK,
				{0, 0}, 0, 0},
				{IML_MARKS_ARABIC, rtf::idcMarksArabic,
				TBSTATE_ENABLED, TBSTYLE_CHECK,
				{0, 0}, 0, 0},
				{IML_STARTINDENTINC, rtf::idcStartIndentInc,
				TBSTATE_ENABLED, TBSTYLE_BUTTON,
				{0, 0}, 0, 0},
				{IML_STARTINDENTDEC, rtf::idcStartIndentDec,
				TBSTATE_ENABLED, TBSTYLE_BUTTON,
				{0, 0}, 0, 0},
				/*{IML_BKMODE, rtf::idcBkMode,
				TBSTATE_ENABLED, TBSTYLE_CHECK,
				{0, 0}, 0, 0},*/
			};

			JPageLog::DlgProc(hWnd, message, wParam, lParam);
			rtf::Editor::DlgProc(hWnd, message, wParam, lParam);

			m_hwndTB = GetDlgItem(hWnd, IDC_TOOLBAR);
			m_hwndEdit = GetDlgItem(hWnd, IDC_RICHEDIT);
			m_hwndSend = GetDlgItem(hWnd, IDC_SEND);
			m_hwndMsgSpin = GetDlgItem(hWnd, IDC_MSGSPIN);

			// Get initial windows sizes
			MapControl(m_hwndEdit, rcEdit);
			MapControl(m_hwndMsgSpin, rcMsgSpin);
			MapControl(m_hwndSend, rcSend);

			m_fTransparent = true;
			wCharFormatting = SCF_SELECTION;

			// Inits tool bar
			SendMessage(m_hwndTB, TB_BUTTONSTRUCTSIZE, (WPARAM)sizeof(TBBUTTON), 0);
			SendMessage(m_hwndTB, TB_SETBITMAPSIZE, 0, MAKELONG(16, 16));
			SendMessage(m_hwndTB, TB_SETIMAGELIST, 0, (LPARAM)JClientApp::jpApp->himlEdit);
			SendMessage(m_hwndTB, TB_ADDBUTTONS, _countof(tbButtons), (LPARAM)&tbButtons);
			// Setup font and checks buttons
			SendMessage(m_hwndEdit, EM_SETCHARFORMAT, SCF_ALL, (LPARAM)&cfDefault);
			UpdateCharacterButtons();
			UpdateParagraphButtons();
			SendMessage(m_hwndTB, TB_CHECKBUTTON, rtf::idcBkMode, MAKELONG(m_fTransparent, 0));

			// Inits Edit control
			m_crSheet = GetSysColor(COLOR_WINDOW);
			SendMessage(m_hwndEdit, EM_SETEVENTMASK, 0, EN_DRAGDROPDONE | ENM_SELCHANGE);
			SendMessage(m_hwndEdit, EM_SETBKGNDCOLOR, FALSE, (LPARAM)m_crSheet);

			// Init up-doun control
			vecMsgSpin.clear();
			vecMsgSpin.push_back("");
			EnableWindow(m_hwndMsgSpin, FALSE);

			// Inits Send button
			SendMessage(m_hwndSend, BM_SETIMAGE, IMAGE_BITMAP, (LPARAM)JClientApp::jpApp->himgSend);

			retval = TRUE;
			break;
		}

	case WM_DESTROY:
		{
			pSource->EvPageClose.Invoke(m_ID);

			pSource->Send_Cmd_PART(pSource->clientsock, pSource->m_idOwn, m_ID);
			pSource->EvReport(tformat(TEXT("parts from [b]%s[/b] private talk"), m_user.name.c_str()), netengine::eInformation, netengine::eHigher);

			JPageLog::DlgProc(hWnd, message, wParam, lParam);
			rtf::Editor::DlgProc(hWnd, message, wParam, lParam);
			break;
		}

	case WM_SIZE:
		{
			if (wParam == SIZE_MINIMIZED) break;
			SendDlgItemMessage(hWnd, IDC_TOOLBAR, TB_AUTOSIZE, 0, 0);
			SendMessage(hWnd, BEM_ADJUSTSIZE, wParam, lParam);
			break;
		}

	case BEM_ADJUSTSIZE:
		{
			RECT rc;
			int cx = LOWORD(lParam), cy = HIWORD(lParam);
			HDWP hdwp = BeginDeferWindowPos(4);
			SetRect(&rc,
				rcLog.left,
				rcLog.top,
				cx - rcPage.right + rcLog.right,
				cy - rcPage.bottom + rcLog.bottom);
			DeferWindowPos(hdwp, m_hwndLog, 0, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, SWP_NOMOVE | SWP_NOREPOSITION | SWP_NOZORDER);
			SetRect(&rc,
				rcEdit.left,
				cy - rcPage.bottom + rcEdit.top,
				cx - rcPage.right + rcEdit.right,
				cy - rcPage.bottom + rcEdit.bottom);
			DeferWindowPos(hdwp, m_hwndEdit, 0, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, SWP_NOREPOSITION | SWP_NOZORDER);
			SetRect(&rc,
				cx - rcPage.right + rcMsgSpin.left,
				cy - rcPage.bottom + rcMsgSpin.top,
				cx - rcPage.right + rcMsgSpin.right,
				cy - rcPage.bottom + rcMsgSpin.bottom);
			DeferWindowPos(hdwp, m_hwndMsgSpin, 0, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, SWP_NOSIZE | SWP_NOREPOSITION | SWP_NOZORDER);
			SetRect(&rc,
				cx - rcPage.right + rcSend.left,
				cy - rcPage.bottom + rcSend.top,
				cx - rcPage.right + rcSend.right,
				cy - rcPage.bottom + rcSend.bottom);
			DeferWindowPos(hdwp, m_hwndSend, 0, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, SWP_NOSIZE | SWP_NOREPOSITION | SWP_NOZORDER);
			EndDeferWindowPos(hdwp);
			break;
		}

	case WM_COMMAND:
		{
			switch (LOWORD(wParam))
			{
			case IDC_SEND:
				{
					if (pSource->m_clientsock) { // send content
						if (GetWindowTextLength(m_hwndEdit)) {
							std::string content;
							getContent(content, SF_RTF);
							pSource->Send_Cmd_SAY(pSource->m_clientsock, getID(), SF_RTF, content);
							SetWindowText(m_hwndEdit, TEXT(""));

							ASSERT(vecMsgSpin.size() > 0);
							vecMsgSpin.back() = content;
							vecMsgSpin.push_back("");
							if (vecMsgSpin.size() > pSource->m_metrics.nMsgSpinMaxCount)
								vecMsgSpin.erase(vecMsgSpin.begin(), vecMsgSpin.begin() + vecMsgSpin.size() - pSource->m_metrics.nMsgSpinMaxCount);
							EnableWindow(m_hwndMsgSpin, TRUE);
							InvalidateRect(m_hwndMsgSpin, 0, TRUE);
							SendMessage(m_hwndMsgSpin, UDM_SETRANGE, 0, MAKELONG(vecMsgSpin.size()-1, 0));
							SendMessage(m_hwndMsgSpin, UDM_SETPOS, 0, MAKELONG(0, 0));
						}
					} else { // connect
						pSource->Connect(true);
					}
					break;
				}

			case IDC_SENDBYENTER:
				pSource->m_bSendByEnter = true;
				break;

			case IDC_SENDBYCTRLENTER:
				pSource->m_bSendByEnter = false;
				break;

			case IDC_COPY:
				SendMessage(m_hwndEdit, WM_COPY, 0, 0);
				break;

			case IDC_CUT:
				SendMessage(m_hwndEdit, WM_CUT, 0, 0);
				break;

			case IDC_PASTE:
				SendMessage(m_hwndEdit, WM_PASTE, 0, 0);
				break;

			case IDC_DELETE:
				SendMessage(m_hwndEdit, WM_CLEAR, 0, 0);
				break;

			case IDC_SELECTALL:
				SendMessage(m_hwndEdit, EM_SETSEL, 0, -1);
				break;

			default:
				retval =
					JPageLog::DlgProc(hWnd, message, wParam, lParam) ||
					rtf::Editor::DlgProc(hWnd, message, wParam, lParam);
			}
			break;
		}


	case WM_NOTIFY:
		{
			if (!pSource) break;

			NMHDR* pnmh = (NMHDR*)lParam;
			switch (pnmh->code)
			{
			case UDN_DELTAPOS:
				{
					LPNMUPDOWN lpnmud = (LPNMUPDOWN)lParam;
					if (pnmh->idFrom == IDC_MSGSPIN) {
						SetWindowText(m_hwndEdit, ANSIToTstr(vecMsgSpin[vecMsgSpin.size() - 1 - lpnmud->iPos]).c_str());
					}
					break;
				}

			default:
				retval =
					JPageLog::DlgProc(hWnd, message, wParam, lParam) ||
					rtf::Editor::DlgProc(hWnd, message, wParam, lParam);
			}
			break;
		}

	case WM_CONTEXTMENU:
		{
			if ((HWND)wParam == m_hwndEdit) {
				RECT r;
				GetWindowRect((HWND)wParam, &r);
				TrackPopupMenu(GetSubMenu(JClientApp::jpApp->hmenuRichEdit, 0), TPM_LEFTALIGN | TPM_RIGHTBUTTON,
					min(max(GET_X_LPARAM(lParam), r.left), r.right),
					min(max(GET_Y_LPARAM(lParam), r.top), r.bottom), 0, hWnd, 0);
			} else {
				JPageLog::DlgProc(hWnd, message, wParam, lParam);
				rtf::Editor::DlgProc(hWnd, message, wParam, lParam);
			}
			break;
		}

	case WM_INITMENUPOPUP:
		{
			if ((HMENU)wParam == GetSubMenu(JClientApp::jpApp->hmenuRichEdit, 0))
			{
				VERIFY(SetMenuDefaultItem((HMENU)wParam, IDC_SEND, FALSE));
				CheckMenuRadioItem((HMENU)wParam,
					IDC_SENDBYENTER, IDC_SENDBYCTRLENTER,
					pSource->bSendByEnter ? IDC_SENDBYENTER : IDC_SENDBYCTRLENTER,
					MF_BYCOMMAND);

				CHARRANGE cr;
				SendMessage(m_hwndEdit, EM_EXGETSEL, 0, (LPARAM)&cr);
				bool cancopy = cr.cpMin != cr.cpMax;
				bool canpaste = SendMessage(m_hwndEdit, EM_CANPASTE, CF_TEXT, 0) != 0;
				bool candelete = cancopy && (GetWindowLong(m_hwndEdit, GWL_STYLE) & ES_READONLY) == 0;
				bool canselect = GetWindowTextLength(m_hwndEdit) > 0;
				EnableMenuItem((HMENU)wParam, IDC_COPY,
					MF_BYCOMMAND | (cancopy ? MF_ENABLED : MF_GRAYED));
				EnableMenuItem((HMENU)wParam, IDC_CUT,
					MF_BYCOMMAND | (candelete ? MF_ENABLED : MF_GRAYED));
				EnableMenuItem((HMENU)wParam, IDC_PASTE,
					MF_BYCOMMAND | (canpaste ? MF_ENABLED : MF_GRAYED));
				EnableMenuItem((HMENU)wParam, IDC_DELETE,
					MF_BYCOMMAND | (candelete ? MF_ENABLED : MF_GRAYED));
				EnableMenuItem((HMENU)wParam, IDC_SELECTALL,
					MF_BYCOMMAND | (canselect ? MF_ENABLED : MF_GRAYED));
				break;
			} else {
				JPageLog::DlgProc(hWnd, message, wParam, lParam);
				rtf::Editor::DlgProc(hWnd, message, wParam, lParam);
			}
			break;
		}

	default:
		retval =
			JPageLog::DlgProc(hWnd, message, wParam, lParam) ||
			rtf::Editor::DlgProc(hWnd, message, wParam, lParam);
	}
	return retval;
}

void JClient::JPageUser::OnHook(JEventable* src)
{
	ASSERT(pSource);
	using namespace fastdelegate;

	__super::OnHook(src);

	pSource->EvLinkIdentify += MakeDelegate(this, &JClient::JPageUser::OnLinkIdentify);
	pSource->EvLinkClose += MakeDelegate(this, &JClient::JPageUser::OnLinkClose);
}

void JClient::JPageUser::OnUnhook(JEventable* src)
{
	ASSERT(pSource);
	using namespace fastdelegate;

	pSource->EvLinkIdentify -= MakeDelegate(this, &JClient::JPageUser::OnLinkIdentify);
	pSource->EvLinkClose -= MakeDelegate(this, &JClient::JPageUser::OnLinkClose);

	__super::OnUnhook(src);
}

void JClient::JPageUser::OnLinkIdentify(SOCKET sock, const netengine::SetAccess& access)
{
	ASSERT(pSource);
	if (m_hwndPage) {
		pSource->Send_Quest_JOIN(pSource->m_clientsock, m_user.name, m_user.password, gettype());
	}
}

void JClient::JPageUser::OnLinkClose(SOCKET sock, UINT err)
{
	ASSERT(pSource);
	if (m_hwndPage) {
		if (m_user.idOnline) // only one disconnect message during connecting
			AppendScript(TEXT("[style=Info]Disconnected[/style]"));
	}
	m_user.idOnline = 0; // no active page on disconnected user
}

//-----------------------------------------------------------------------------

//
// JPageChannel
//

CALLBACK JClient::JPageChannel::JPageChannel(DWORD id, const std::tstring& nick)
: JPageLog(), rtf::Editor()
{
	m_ID = id;
	m_channel.name = nick;
	m_channel.crBackground = GetSysColor(COLOR_WINDOW);
}

void CALLBACK JClient::JPageChannel::Enable()
{
	EnableWindow(m_hwndEdit, TRUE);
	m_fEnabled = true;
}

void CALLBACK JClient::JPageChannel::Disable()
{
	ListView_DeleteAllItems(m_hwndList);
	EnableWindow(m_hwndEdit, FALSE);
	m_fEnabled = false;
}

int CALLBACK JClient::JPageChannel::ImageIndex() const
{
	switch (m_alert)
	{
	case eGreen:
		return IML_CHANNELGREEN;
	case eYellow:
		return IML_CHANNELYELLOW;
	case eRed:
	default:
		return IML_CHANNELRED;
	}
}

std::tstring JClient::JPageChannel::gettopic() const
{
	ASSERT(pSource);

	if (m_channel.topic.empty()) {
		return tformat(TEXT("#%s"), m_channel.name.c_str());
	} else if (pSource->mUser.find(m_channel.idTopicWriter) == pSource->mUser.end()) {
		return tformat(TEXT("#%s: %s"), m_channel.name.c_str(), m_channel.topic.c_str());
	} else {
		return tformat(TEXT("#%s: %s (%s)"), m_channel.name.c_str(), m_channel.topic.c_str(), pSource->getSafeName(m_channel.idTopicWriter).c_str());
	}
}

void CALLBACK JClient::JPageChannel::OnSheetColor(COLORREF cr)
{
	__super::OnSheetColor(cr);

	ASSERT(pSource);
	if (m_channel.getStatus(pSource->m_idOwn) >= eMember) {
		pSource->Send_Cmd_BACKGROUND(pSource->m_clientsock, m_ID, cr);
	}
}

void CALLBACK JClient::JPageChannel::setchannel(const Channel& val)
{
	m_channel = val;

	if (m_hwndPage) {
		ListView_DeleteAllItems(m_hwndList);
		BuildView();
	}

	AppendScript(tformat(TEXT("[style=Info]now talking in [b]#%s[/b][/style]"), m_channel.name.c_str()));
}

void CALLBACK JClient::JPageChannel::rename(DWORD idNew, const std::tstring& newname)
{
	m_ID = idNew;
	m_channel.name = newname;
}

bool CALLBACK JClient::JPageChannel::replace(DWORD idOld, DWORD idNew)
{
	bool retval = false;

	if (m_channel.opened.find(idOld) != m_channel.opened.end()) {
		m_channel.opened.insert(idNew);
		m_channel.opened.erase(idOld);
		retval = true;

		LVFINDINFO lvfi;
		lvfi.flags = LVFI_PARAM;
		lvfi.lParam = idOld;
		int index = ListView_FindItem(m_hwndList, -1, &lvfi);
		if (index >= 0) {
			LVITEM lvi;
			lvi.mask = LVIF_PARAM;
			lvi.iItem = index;
			lvi.iSubItem = 0;
			lvi.lParam = idNew;
			VERIFY(ListView_SetItem(m_hwndList, &lvi));
			VERIFY(ListView_RedrawItems(m_hwndList, index, index));
		} else AddLine(idNew);
	}

	// replace content of channels access rights
	switch (m_channel.getStatus(idOld))
	{
	case eFounder:
		m_channel.founder.insert(idNew);
		m_channel.founder.erase(idOld);
		retval = true;
		break;
	case eAdmin:
		m_channel.admin.insert(idNew);
		m_channel.admin.erase(idOld);
		retval = true;
		break;
	case eModerator:
		m_channel.moderator.insert(idNew);
		m_channel.moderator.erase(idOld);
		retval = true;
		break;
	case eMember:
		m_channel.member.insert(idNew);
		m_channel.member.erase(idOld);
		retval = true;
		break;
	case eWriter:
		m_channel.writer.insert(idNew);
		m_channel.writer.erase(idOld);
		retval = true;
		break;
	}

	return retval;
}

void CALLBACK JClient::JPageChannel::redrawUser(DWORD idUser)
{
	LVFINDINFO lvfi;
	lvfi.flags = LVFI_PARAM;
	lvfi.lParam = (LPARAM)idUser;
	int index = ListView_FindItem(m_hwndList, -1, &lvfi);
	if (index != -1) VERIFY(ListView_RedrawItems(m_hwndList, index, index));
}

void CALLBACK JClient::JPageChannel::Join(DWORD idUser)
{
	m_channel.opened.insert(idUser);
	if (m_channel.getStatus(idUser) <= eReader)
		m_channel.setStatus(idUser, m_channel.nAutoStatus);

	if (m_hwndPage) AddLine(idUser);

	// Introduction messages only fo finded user here - user can be unknown or hidden
	MapUser::const_iterator iu = pSource->m_mUser.find(idUser);
	if (iu != pSource->m_mUser.end() && iu->second.name.length()) {
		AppendScript(tformat(TEXT("[style=Info]joins: [b]%s[/b][/style]"), iu->second.name.c_str()));
	}
}

void CALLBACK JClient::JPageChannel::Part(DWORD idUser, DWORD idBy)
{
	m_channel.opened.erase(idUser);
	if (m_hwndPage) DelLine(idUser);

	// Parting message
	MapUser::const_iterator iu = pSource->m_mUser.find(idUser);
	if (iu != pSource->m_mUser.end() && iu->second.name.length()) {
		std::tstring msg;
		if (idUser == idBy) {
			msg = tformat(TEXT("[style=Info]parts: [b]%s[/b][/style]"), iu->second.name.c_str());
		} else if (idBy == CRC_SERVER) {
			msg = tformat(TEXT("[style=Info]quits: [b]%s[/b][/style]"), iu->second.name.c_str());
		} else {
			MapUser::const_iterator iu2 = pSource->m_mUser.find(idBy);
			std::tstring by = iu2 != pSource->m_mUser.end() ? iu2->second.name : TEXT("unknown");
			msg = tformat(TEXT("[style=Info][b]%s[/b] was kicked by [b]%s[/b][/style]"), iu->second.name.c_str(), by.c_str());
		}
		AppendScript(msg);
	}
}

int  CALLBACK JClient::JPageChannel::indexIcon(DWORD idUser) const
{
	MapUser::const_iterator iu = pSource->m_mUser.find(idUser);
	if (iu == pSource->m_mUser.end()) return IML_MANVOID;
	int offset = iu->second.isOnline ? 0 : 6;
	switch (m_channel.getStatus(idUser))
	{
	case eFounder:
		return IML_MANMAGENTAON + offset;
	case eAdmin:
		return IML_MANREDON + offset;
	case eModerator:
		return IML_MANBLUEON + offset;
	case eMember:
		return IML_MANGREENON + offset;
	case eWriter:
		return IML_MANCYANON + offset;
	case eReader:
		return IML_MANYELLOWON + offset;
	default:
		return IML_MANVOID;
	}
}

MapUser::const_iterator JClient::JPageChannel::getSelUser() const
{
	int index = ListView_GetNextItem(m_hwndList, -1, LVNI_SELECTED);
	if (index >= 0) {
		LVITEM lvi;
		lvi.mask = LVIF_PARAM;
		lvi.iItem = index;
		lvi.iSubItem = 0;
		if (ListView_GetItem(m_hwndList, &lvi)) {
			return pSource->m_mUser.find((DWORD)lvi.lParam);
		}
	}
	return pSource->m_mUser.end();
}

LRESULT WINAPI JClient::JPageChannel::DlgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	LRESULT retval = TRUE;
	switch (message)
	{
	case WM_INITDIALOG:
		{
			static const TBBUTTON tbButtons[] =
			{
				{IML_BOLD, rtf::idcBold,
				TBSTATE_ENABLED, TBSTYLE_CHECK,
				{0, 0}, 0, 0},
				{IML_ITALIC, rtf::idcItalic,
				TBSTATE_ENABLED, TBSTYLE_CHECK,
				{0, 0}, 0, 0},
				{IML_ULINE, rtf::idcUnderline,
				TBSTATE_ENABLED, TBSTYLE_CHECK,
				{0, 0}, 0, 0},
				{IML_SUBSCRIPT, rtf::idcSubscript,
				TBSTATE_ENABLED, TBSTYLE_CHECK,
				{0, 0}, 0, 0},
				{IML_SUPERSCRIPT, rtf::idcSuperscript,
				TBSTATE_ENABLED, TBSTYLE_CHECK,
				{0, 0}, 0, 0},
				{IML_FONT, rtf::idcFont,
				TBSTATE_ENABLED, TBSTYLE_BUTTON,
				{0, 0}, 0, 0},
				{0, 0,
				0, TBSTYLE_SEP,
				{0, 0}, 0, 0},
				{IML_FRCOL, rtf::idcFrColor,
				TBSTATE_ENABLED, TBSTYLE_BUTTON,
				{0, 0}, 0, 0},
				{IML_BGCOL, rtf::idcBgColor,
				TBSTATE_ENABLED, TBSTYLE_BUTTON,
				{0, 0}, 0, 0},
				{IML_SHEETCOL, rtf::idcSheetColor,
				TBSTATE_ENABLED, TBSTYLE_BUTTON,
				{0, 0}, 0, 0},
				{0, 0,
				0, TBSTYLE_SEP,
				{0, 0}, 0, 0},
				{IML_LEFT, rtf::idcAlignLeft,
				TBSTATE_ENABLED, TBSTYLE_CHECK,
				{0, 0}, 0, 0},
				{IML_CENTER, rtf::idcAlignCenter,
				TBSTATE_ENABLED, TBSTYLE_CHECK,
				{0, 0}, 0, 0},
				{IML_RIGHT, rtf::idcAlignRight,
				TBSTATE_ENABLED, TBSTYLE_CHECK,
				{0, 0}, 0, 0},
				{IML_JUSTIFY, rtf::idcAlignJustify,
				TBSTATE_ENABLED, TBSTYLE_CHECK,
				{0, 0}, 0, 0},
				{IML_MARKS_BULLET, rtf::idcMarksBullet,
				TBSTATE_ENABLED, TBSTYLE_CHECK,
				{0, 0}, 0, 0},
				{IML_MARKS_ARABIC, rtf::idcMarksArabic,
				TBSTATE_ENABLED, TBSTYLE_CHECK,
				{0, 0}, 0, 0},
				{IML_STARTINDENTINC, rtf::idcStartIndentInc,
				TBSTATE_ENABLED, TBSTYLE_BUTTON,
				{0, 0}, 0, 0},
				{IML_STARTINDENTDEC, rtf::idcStartIndentDec,
				TBSTATE_ENABLED, TBSTYLE_BUTTON,
				{0, 0}, 0, 0},
				/*{IML_BKMODE, rtf::idcBkMode,
				TBSTATE_ENABLED, TBSTYLE_CHECK,
				{0, 0}, 0, 0},*/
			};

			JPageLog::DlgProc(hWnd, message, wParam, lParam);
			rtf::Editor::DlgProc(hWnd, message, wParam, lParam);

			m_hwndTB = GetDlgItem(hWnd, IDC_TOOLBAR);
			m_hwndEdit = GetDlgItem(hWnd, IDC_RICHEDIT);
			m_hwndList = GetDlgItem(hWnd, IDC_USERLIST);
			m_hwndMsgSpin = GetDlgItem(hWnd, IDC_MSGSPIN);
			m_hwndSend = GetDlgItem(hWnd, IDC_SEND);

			// Get initial windows sizes
			MapControl(m_hwndEdit, rcEdit);
			MapControl(m_hwndList, rcList);
			MapControl(m_hwndMsgSpin, rcMsgSpin);
			MapControl(m_hwndSend, rcSend);

			m_fTransparent = true;
			wCharFormatting = SCF_SELECTION;

			TOOLINFO ti;
			ti.cbSize = sizeof(ti);
			ti.uFlags = TTF_ABSOLUTE | TTF_IDISHWND | TTF_TRACK;
			ti.hwnd = pSource->hwndPage;
			ti.uId = (UINT_PTR)m_hwndList;
			ti.hinst = JClientApp::jpApp->hinstApp;
			ti.lpszText = 0;
			VERIFY(SendMessage(pSource->m_hwndBaloon, TTM_ADDTOOL, 0, (LPARAM)&ti));

			// Inits tool bar
			SendMessage(m_hwndTB, TB_BUTTONSTRUCTSIZE, (WPARAM)sizeof(TBBUTTON), 0);
			SendMessage(m_hwndTB, TB_SETBITMAPSIZE, 0, MAKELONG(16, 16));
			SendMessage(m_hwndTB, TB_SETIMAGELIST, 0, (LPARAM)JClientApp::jpApp->himlEdit);
			SendMessage(m_hwndTB, TB_ADDBUTTONS, _countof(tbButtons), (LPARAM)&tbButtons);
			// Setup font and checks buttons
			SendMessage(m_hwndEdit, EM_SETCHARFORMAT, SCF_ALL, (LPARAM)&cfDefault);
			UpdateCharacterButtons();
			UpdateParagraphButtons();
			SendMessage(m_hwndTB, TB_CHECKBUTTON, rtf::idcBkMode, MAKELONG(m_fTransparent, 0));

			// Inits Edit control
			m_crSheet = GetSysColor(COLOR_WINDOW);
			SendMessage(m_hwndEdit, EM_SETEVENTMASK, 0, EN_DRAGDROPDONE | ENM_SELCHANGE);
			SendMessage(m_hwndEdit, EM_SETBKGNDCOLOR, FALSE, (LPARAM)m_crSheet);
			// Init Log control
			SendMessage(m_hwndLog, EM_SETEVENTMASK, 0, ENM_MOUSEEVENTS);

			// Inits Users list
			ListView_SetExtendedListViewStyle(m_hwndList,
				LVS_EX_GRIDLINES | LVS_EX_HEADERDRAGDROP | LVS_EX_ONECLICKACTIVATE | LVS_EX_SUBITEMIMAGES | LVS_EX_TRACKSELECT);
			ListView_SetImageList(m_hwndList, JClientApp::jpApp->himlMan, LVSIL_SMALL);
			ListView_SetItemCount(m_hwndList, 64);
			static LV_COLUMN lvc0[] = {
				{LVCF_FMT | LVCF_SUBITEM | LVCF_TEXT | LVCF_WIDTH, LVCFMT_LEFT,
				120, TEXT("Nickname"), -1, 0},
				{LVCF_FMT | LVCF_SUBITEM | LVCF_TEXT | LVCF_WIDTH, LVCFMT_LEFT,
				100, TEXT("Status"), -1, 1},
				{LVCF_FMT | LVCF_SUBITEM | LVCF_TEXT | LVCF_WIDTH, LVCFMT_LEFT,
				80, TEXT("IP-address"), -1, 2},
				{LVCF_FMT | LVCF_SUBITEM | LVCF_TEXT | LVCF_WIDTH, LVCFMT_LEFT,
				120, TEXT("Connected"), -1, 3},
			};
			for (int i = 0; i < _countof(lvc0); ++i)
				ListView_InsertColumn(m_hwndList, i, &lvc0[i]);

			// Init up-down control
			vecMsgSpin.clear();
			vecMsgSpin.push_back("");
			EnableWindow(m_hwndMsgSpin, FALSE);

			// Inits Send button
			SendMessage(m_hwndSend, BM_SETIMAGE, IMAGE_BITMAP, (LPARAM)JClientApp::jpApp->himgSend);

			BuildView();

			retval = TRUE;
			break;
		}

	case WM_DESTROY:
		{
			pSource->HideBaloon(m_hwndList);
			TOOLINFO ti;
			ti.cbSize = sizeof(ti);
			ti.hwnd = pSource->hwndPage;
			ti.uId = (UINT_PTR)m_hwndList;
			SendMessage(pSource->m_hwndBaloon, TTM_DELTOOL, 0, (LPARAM)&ti);

			pSource->EvPageClose.Invoke(m_ID);

			pSource->Send_Cmd_PART(pSource->clientsock, pSource->m_idOwn, m_ID);
			for each (SetId::value_type const& v in m_channel.opened) {
				pSource->UnlinkUser(v, m_ID);
			}
			pSource->EvReport(tformat(TEXT("parts from [b]#%s[/b] channel"), m_channel.name.c_str()), netengine::eInformation, netengine::eHigher);

			JPageLog::DlgProc(hWnd, message, wParam, lParam);
			rtf::Editor::DlgProc(hWnd, message, wParam, lParam);
			break;
		}

	case WM_SIZE:
		{
			if (wParam == SIZE_MINIMIZED) break;
			SendDlgItemMessage(hWnd, IDC_TOOLBAR, TB_AUTOSIZE, 0, 0);
			SendMessage(hWnd, BEM_ADJUSTSIZE, wParam, lParam);
			break;
		}

	case BEM_ADJUSTSIZE:
		{
			RECT rc;
			int cx = LOWORD(lParam), cy = HIWORD(lParam);
			HDWP hdwp = BeginDeferWindowPos(5);
			SetRect(&rc,
				rcLog.left,
				rcLog.top,
				cx - rcPage.right + rcLog.right,
				cy - rcPage.bottom + rcLog.bottom);
			DeferWindowPos(hdwp, m_hwndLog, 0, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, SWP_NOMOVE | SWP_NOREPOSITION | SWP_NOZORDER);
			SetRect(&rc,
				rcEdit.left,
				cy - rcPage.bottom + rcEdit.top,
				cx - rcPage.right + rcEdit.right,
				cy - rcPage.bottom + rcEdit.bottom);
			DeferWindowPos(hdwp, m_hwndEdit, 0, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, SWP_NOREPOSITION | SWP_NOZORDER);
			SetRect(&rc,
				cx - rcPage.right + rcList.left,
				rcList.top,
				cx - rcPage.right + rcList.right,
				cy - rcPage.bottom + rcList.bottom);
			DeferWindowPos(hdwp, m_hwndList, 0, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, SWP_NOREPOSITION | SWP_NOZORDER);
			SetRect(&rc,
				cx - rcPage.right + rcMsgSpin.left,
				cy - rcPage.bottom + rcMsgSpin.top,
				cx - rcPage.right + rcMsgSpin.right,
				cy - rcPage.bottom + rcMsgSpin.bottom);
			DeferWindowPos(hdwp, m_hwndMsgSpin, 0, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, SWP_NOSIZE | SWP_NOREPOSITION | SWP_NOZORDER);
			SetRect(&rc,
				cx - rcPage.right + rcSend.left,
				cy - rcPage.bottom + rcSend.top,
				cx - rcPage.right + rcSend.right,
				cy - rcPage.bottom + rcSend.bottom);
			DeferWindowPos(hdwp, m_hwndSend, 0, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, SWP_NOSIZE | SWP_NOREPOSITION | SWP_NOZORDER);
			EndDeferWindowPos(hdwp);
			break;
		}

	case WM_COMMAND:
		{
			switch (LOWORD(wParam))
			{
			case IDC_SEND:
				{
					if (pSource->m_clientsock) { // send content
						if (GetWindowTextLength(m_hwndEdit)) {
							std::string content;
							getContent(content, SF_RTF);
							if (m_channel.getStatus(pSource->m_idOwn) > eReader)
								pSource->Send_Cmd_SAY(pSource->m_clientsock, getID(), SF_RTF, content);
							SetWindowText(m_hwndEdit, TEXT(""));

							ASSERT(vecMsgSpin.size() > 0);
							vecMsgSpin.back() = content;
							vecMsgSpin.push_back("");
							if (vecMsgSpin.size() > pSource->m_metrics.nMsgSpinMaxCount)
								vecMsgSpin.erase(vecMsgSpin.begin(), vecMsgSpin.begin() + vecMsgSpin.size() - pSource->m_metrics.nMsgSpinMaxCount);
							EnableWindow(m_hwndMsgSpin, TRUE);
							InvalidateRect(m_hwndMsgSpin, 0, TRUE);
							SendMessage(m_hwndMsgSpin, UDM_SETRANGE, 0, MAKELONG(vecMsgSpin.size()-1, 0));
							SendMessage(m_hwndMsgSpin, UDM_SETPOS, 0, MAKELONG(0, 0));
						}
					} else { // connect
						pSource->Connect(true);
					}
					break;
				}

			case IDC_SENDBYENTER:
				pSource->m_bSendByEnter = true;
				break;

			case IDC_SENDBYCTRLENTER:
				pSource->m_bSendByEnter = false;
				break;

			case IDC_COPY:
				SendMessage(m_hwndEdit, WM_COPY, 0, 0);
				break;

			case IDC_CUT:
				SendMessage(m_hwndEdit, WM_CUT, 0, 0);
				break;

			case IDC_PASTE:
				SendMessage(m_hwndEdit, WM_PASTE, 0, 0);
				break;

			case IDC_DELETE:
				SendMessage(m_hwndEdit, WM_CLEAR, 0, 0);
				break;

			case IDC_SELECTALL:
				SendMessage(m_hwndEdit, EM_SETSEL, 0, -1);
				break;

			case IDC_CHANTOPIC:
				if (m_channel.getStatus(pSource->m_idOwn) >= eMember)
					CreateDialogParam(JClientApp::jpApp->hinstApp, MAKEINTRESOURCE(IDD_TOPIC), pSource->hwndPage, (DLGPROC)JDialog::DlgProcStub, (LPARAM)(JDialog*)new JTopic(pSource, this));
				break;

			case IDC_PRIVATETALK:
				{
					MapUser::const_iterator iu = getSelUser();
					if (iu == pSource->m_mUser.end()) break;
					if (JClient::s_mapAlert[iu->second.nStatus].fCanOpenPrivate) {
						ASSERT(pSource->m_clientsock);
						pSource->Send_Quest_JOIN(pSource->m_clientsock, iu->second.name);
					} else pSource->DisplayMessage(m_hwndList, TEXT("User bans to open private talks"));
					break;
				}

			case IDC_PRIVATEMESSAGE:
				{
					MapUser::const_iterator iu = getSelUser();
					if (iu == pSource->m_mUser.end()) break;
					if (JClient::s_mapAlert[iu->second.nStatus].fCanMessage) {
						ASSERT(pSource->m_clientsock);
						CreateDialogParam(JClientApp::jpApp->hinstApp, MAKEINTRESOURCE(IDD_MSGSEND), pSource->hwndPage, JClient::JSplashRtfEditor::DlgProcStub, (LPARAM)(JDialog*)new JClient::JMessageEditor(pSource, iu->second.name, false));
					} else pSource->DisplayMessage(m_hwndList, TEXT("User messages recieving is banned"));
					break;
				}

			case IDC_ALERT:
				{
					MapUser::const_iterator iu = getSelUser();
					if (iu == pSource->m_mUser.end()) break;
					if (JClient::s_mapAlert[iu->second.nStatus].fCanAlert) {
						ASSERT(pSource->m_clientsock);
						CreateDialogParam(JClientApp::jpApp->hinstApp, MAKEINTRESOURCE(IDD_MSGSEND), pSource->hwndPage, JClient::JSplashRtfEditor::DlgProcStub, (LPARAM)(JDialog*)new JClient::JMessageEditor(pSource, iu->second.name, true));
					} else pSource->DisplayMessage(m_hwndList, TEXT("User alerts recieving is banned"));
					break;
				}

			case IDS_SOUNDSIGNAL:
				{
					MapUser::const_iterator iu = getSelUser();
					if (iu == pSource->m_mUser.end()) break;
					if (JClient::s_mapAlert[iu->second.nStatus].fCanSignal) {
						ASSERT(pSource->m_clientsock);
						pSource->Send_Cmd_BEEP(pSource->m_clientsock, iu->first);
					} else pSource->DisplayMessage(m_hwndList, TEXT("Sound signal disabled now for this user"));
					break;
				}

			case IDC_CLIPBOARD:
				{
					MapUser::const_iterator iu = getSelUser();
					if (iu == pSource->m_mUser.end()) break;
					if (JClient::s_mapAlert[iu->second.nStatus].fCanRecvClipboard) {
						ASSERT(pSource->m_clientsock);
						pSource->Send_Cmd_CLIPBOARD(pSource->m_clientsock, iu->first);
					} else pSource->DisplayMessage(m_hwndList, TEXT("User bans recieving windows clipboard content"));
					break;
				}

			case IDC_SPLASHRTF:
				{
					MapUser::const_iterator iu = getSelUser();
					if (iu == pSource->m_mUser.end()) break;
					if (JClient::s_mapAlert[iu->second.nStatus].fCanAlert) {
						ASSERT(pSource->m_clientsock);
						CreateDialogParam(JClientApp::jpApp->hinstApp, MAKEINTRESOURCE(IDD_SPLASHRTFEDITOR), pSource->hwndPage, JClient::JSplashRtfEditor::DlgProcStub, (LPARAM)(JDialog*)new JClient::JSplashRtfEditor(pSource, iu->first));
					} else pSource->DisplayMessage(m_hwndList, TEXT("User bans recieving splashes"));
					break;
				}

			case IDC_KICK:
				{
					MapUser::const_iterator iu = getSelUser();
					if (iu == pSource->m_mUser.end()) break;
					ASSERT(pSource->m_clientsock);

					bool isModer = m_channel.getStatus(pSource->m_idOwn) >= eModerator;
					bool canKick = m_channel.getStatus(pSource->m_idOwn) >= m_channel.getStatus(iu->first);
					if (iu->first == pSource->m_idOwn || (isModer && canKick)) {
						pSource->Send_Cmd_PART(pSource->m_clientsock, iu->first, m_ID);
					}
					break;
				}

			case IDC_FOUNDER:
			case IDC_ADMIN:
			case IDC_MODERATOR:
			case IDC_MEMBER:
			case IDC_WRITER:
			case IDC_READER:
			case IDC_OUTSIDER:
				{
					MapUser::const_iterator iu = getSelUser();
					if (iu == pSource->m_mUser.end()) break;
					ASSERT(pSource->m_clientsock);

					EChanStatus statOwn = m_channel.getStatus(pSource->m_idOwn), statUser = m_channel.getStatus(iu->first);
					const struct {UINT idc; EChanStatus stat;} cmp[] = {
						{IDC_OUTSIDER, eOutsider},
						{IDC_READER, eReader},
						{IDC_WRITER, eWriter},
						{IDC_MEMBER, eMember},
						{IDC_MODERATOR, eModerator},
						{IDC_ADMIN, eAdmin},
						{IDC_FOUNDER, eFounder},
					};
					int i;
					for (i = 0; cmp[i].idc != LOWORD(wParam); i++) {}
					ASSERT(i < _countof(cmp));
					bool canModer =
						(statOwn > statUser || statOwn >= eModerator || iu->first == pSource->m_idOwn) &&
						(statOwn > cmp[i].stat || statOwn == eFounder);
					if (canModer) {
						pSource->Send_Cmd_ACCESS(pSource->m_clientsock, iu->first, m_ID, cmp[i].stat);
					}
					break;
				}

			default:
				retval =
					JPageLog::DlgProc(hWnd, message, wParam, lParam) ||
					rtf::Editor::DlgProc(hWnd, message, wParam, lParam);
			}
			break;
		}

	case WM_DRAWITEM:
		{
			LPDRAWITEMSTRUCT lpDrawItem = (LPDRAWITEMSTRUCT)lParam;
			switch (wParam)
			{
			case IDC_USERLIST:
				{
					MapUser::const_iterator iter = pSource->m_mUser.find((DWORD)lpDrawItem->itemData);
					bool valid = iter != pSource->m_mUser.end() && iter->second.name.length();
					// Get columns width and order
					int nColOrder[4], nColWidth[4], nColPos[4];
					ListView_GetColumnOrderArray(m_hwndList, _countof(nColOrder), nColOrder);
					for (int i = 0; i < _countof(nColWidth); i++) {
						nColWidth[i] = ListView_GetColumnWidth(m_hwndList, i);
					}
					for (int i = 0, pos = 0; i < _countof(nColPos); i++) {
						nColPos[nColOrder[i]] = pos;
						pos += nColWidth[nColOrder[i]];
					}
					RECT rc;
					const int sep = 5;
					// Draw background
					HDC hdcCompatible = CreateCompatibleDC(lpDrawItem->hDC);
					HBITMAP himgOld;
					int w, h;
					if (lpDrawItem->itemState & ODS_HOTLIGHT) {
						himgOld = (HBITMAP)JClientApp::jpApp->himgULHot;
						w = 16, h = 256;
					} else if (lpDrawItem->itemState & (ODS_FOCUS | ODS_SELECTED)) {
						himgOld = /*lpDrawItem->itemState & ODS_FOCUS
							? (HBITMAP)JClientApp::jpApp->himgULFoc
							: */(HBITMAP)JClientApp::jpApp->himgULSel;
						w = 128, h = 64;
					} else {
						himgOld = (HBITMAP)JClientApp::jpApp->himgULBG;
						w = 256, h = 16;
					}
					himgOld = (HBITMAP)SelectObject(hdcCompatible, himgOld);
					StretchBlt(lpDrawItem->hDC,
						lpDrawItem->rcItem.left, lpDrawItem->rcItem.top,
						lpDrawItem->rcItem.right - lpDrawItem->rcItem.left,
						lpDrawItem->rcItem.bottom - lpDrawItem->rcItem.top,
						hdcCompatible, 0, 0, w, h, SRCCOPY);
					himgOld = (HBITMAP)SelectObject(hdcCompatible, himgOld);
					DeleteDC(hdcCompatible);
					// Draw "Nickname" colunm
					rc = lpDrawItem->rcItem;
					rc.left += nColPos[0] + sep;
					rc.right = rc.left + nColWidth[0];
					ImageList_Draw(JClientApp::jpApp->himlMan, indexIcon((DWORD)lpDrawItem->itemData), lpDrawItem->hDC, rc.left, rc.top, 0);
					ImageList_Draw(JClientApp::jpApp->himlStatus, valid ? (int)iter->second.nStatus : 0, lpDrawItem->hDC, rc.left, rc.top, 0);
					rc.left += 16 + sep;
					if (valid && iter->second.nStatusImg) {
						ImageList_Draw(JClientApp::jpApp->himlStatusImg, iter->second.nStatusImg, lpDrawItem->hDC, rc.left, rc.top, 0);
						rc.left += 16 + sep;
					}
					COLORREF cr = SetTextColor(lpDrawItem->hDC, (DWORD)lpDrawItem->itemData != pSource->m_idOwn
						? RGB(0xFF, 0x00, 0x00)
						: RGB(0x00, 0x00, 0xFF));
					DrawText(lpDrawItem->hDC, pSource->getSafeName((DWORD)lpDrawItem->itemData).c_str(), -1, &rc, DT_LEFT | DT_NOCLIP | DT_VCENTER);
					SetTextColor(lpDrawItem->hDC, cr);
					// Draw "Status" column
					rc = lpDrawItem->rcItem;
					rc.left += nColPos[1] + sep;
					rc.right = rc.left + nColWidth[1];
					DrawText(lpDrawItem->hDC, valid ? iter->second.strStatus.c_str() : TEXT(""), -1, &rc, DT_LEFT | DT_NOCLIP | DT_VCENTER);
					// Draw "IP-address" column
					rc = lpDrawItem->rcItem;
					rc.left += nColPos[2] + sep;
					rc.right = rc.left + nColWidth[2];
					DrawText(lpDrawItem->hDC, valid
						? tformat(TEXT("%i.%i.%i.%i"),
						iter->second.IP.S_un.S_un_b.s_b1,
						iter->second.IP.S_un.S_un_b.s_b2,
						iter->second.IP.S_un.S_un_b.s_b3,
						iter->second.IP.S_un.S_un_b.s_b4).c_str()
						: TEXT("N/A"), -1, &rc, DT_LEFT | DT_NOCLIP | DT_VCENTER);
					// Draw "Connected" column
					rc = lpDrawItem->rcItem;
					rc.left += nColPos[3] + sep;
					rc.right = rc.left + nColWidth[3];
					if (valid) {
						SYSTEMTIME st;
						FileTimeToLocalTime(iter->second.ftCreation, st);
						DrawText(lpDrawItem->hDC,
							tformat(TEXT("%02i:%02i:%02i, %02i.%02i.%04i"),
							st.wHour, st.wMinute, st.wSecond,
							st.wDay, st.wMonth, st.wYear).c_str(),
							-1, &rc, DT_LEFT | DT_NOCLIP | DT_VCENTER);
					} else DrawText(lpDrawItem->hDC, TEXT("N/A"), -1, &rc, DT_LEFT | DT_NOCLIP | DT_VCENTER);
					break;
				}
			}
			break;
		}

	case WM_NOTIFY:
		{
			if (!pSource) break;

			NMHDR* pnmh = (NMHDR*)lParam;
			switch (pnmh->code)
			{
			case NM_DBLCLK:
				{
					if (pnmh->idFrom == IDC_USERLIST) {
						SendMessage(hWnd, WM_COMMAND, IDC_PRIVATETALK, 0);
					}
					break;
				}

			case LVN_ITEMCHANGED:
				{
					LPNMLISTVIEW pnmv = (LPNMLISTVIEW)lParam;
					if (pnmh->idFrom == IDC_USERLIST && pnmv->uChanged == LVIF_STATE) {
						MapUser::const_iterator iu = pSource->m_mUser.find((DWORD)pnmv->lParam);
						if (iu != pSource->m_mUser.end() && pnmv->iItem >= 0)
						{
							ASSERT(pSource->m_clientsock);

							SendMessage(pSource->m_hwndBaloon, TTM_SETTIPTEXTCOLOR, (DWORD)pnmv->lParam != pSource->m_idOwn
								? RGB(0xFF, 0x00, 0x00)
								: RGB(0x00, 0x00, 0xFF), 0);

							HICON hicon = ImageList_GetIcon(JClientApp::jpApp->himlStatusImg, iu->second.nStatusImg, ILD_TRANSPARENT);
							VERIFY(SendMessage(pSource->m_hwndBaloon, TTM_SETTITLE, (WPARAM)hicon, (LPARAM)iu->second.name.c_str()));
							DestroyIcon(hicon);

							static TCHAR  bttbuf[256];
							SYSTEMTIME st;
							FileTimeToLocalTime(iu->second.ftCreation, st);
							TOOLINFO ti;
							_stprintf_s(bttbuf, _countof(bttbuf),
								TEXT("%s\n")
								TEXT("Status text:\t\"%s\"\n")
								TEXT("IP-address:\t%i.%i.%i.%i\n")
								TEXT("Connected:\t%02i:%02i:%02i, %02i.%02i.%04i"),
								iu->second.isOnline ? TEXT("User online") : TEXT("User offline"),
								iu->second.strStatus.c_str(),
								iu->second.IP.S_un.S_un_b.s_b1,
								iu->second.IP.S_un.S_un_b.s_b2,
								iu->second.IP.S_un.S_un_b.s_b3,
								iu->second.IP.S_un.S_un_b.s_b4,
								st.wHour, st.wMinute, st.wSecond,
								st.wDay, st.wMonth, st.wYear);
							ti.cbSize = sizeof(ti);
							ti.hwnd = pSource->hwndPage;
							ti.uId = (UINT_PTR)m_hwndList;
							ti.hinst = JClientApp::jpApp->hinstApp;
							ti.lpszText = bttbuf;
							SendMessage(pSource->m_hwndBaloon, TTM_UPDATETIPTEXT, 0, (LPARAM)&ti);

							POINT p;
							VERIFY(GetCursorPos(&p));
							SendMessage(pSource->m_hwndBaloon, TTM_TRACKPOSITION, 0, MAKELPARAM(p.x, p.y));

							SendMessage(pSource->m_hwndBaloon, TTM_TRACKACTIVATE, TRUE, (LPARAM)&ti);
							pSource->m_isBaloon = m_hwndList;
							SetTimer(pSource->hwndPage, IDT_BALOONPOP, TIMER_BALOONPOP, 0);
						}
					}
					break;
				}

			case EN_MSGFILTER:
				{
					MSGFILTER* pmf = (MSGFILTER*)lParam;
					if (pnmh->idFrom == IDC_LOG) {
						if (pmf->msg == WM_LBUTTONDBLCLK && m_channel.getStatus(pSource->m_idOwn) >= eMember) {
							SendMessage(hWnd, WM_COMMAND, IDC_CHANTOPIC, 0);
						}
					}
					break;
				}

			case UDN_DELTAPOS:
				{
					LPNMUPDOWN lpnmud = (LPNMUPDOWN)lParam;
					if (pnmh->idFrom == IDC_MSGSPIN) {
						SetWindowText(m_hwndEdit, ANSIToTstr(vecMsgSpin[vecMsgSpin.size() - 1 - lpnmud->iPos]).c_str());
					}
					break;
				}

			default:
				retval =
					JPageLog::DlgProc(hWnd, message, wParam, lParam) ||
					rtf::Editor::DlgProc(hWnd, message, wParam, lParam);
			}
			break;
		}

	case WM_CONTEXTMENU:
		{
			if ((HWND)wParam == m_hwndLog) {
				RECT r;
				GetWindowRect((HWND)wParam, &r);
				TrackPopupMenu(GetSubMenu(JClientApp::jpApp->hmenuChannel, 0), TPM_LEFTALIGN | TPM_RIGHTBUTTON,
					min(max(GET_X_LPARAM(lParam), r.left), r.right),
					min(max(GET_Y_LPARAM(lParam), r.top), r.bottom), 0, hWnd, 0);
			} else if ((HWND)wParam == m_hwndEdit) {
				RECT r;
				GetWindowRect((HWND)wParam, &r);
				TrackPopupMenu(GetSubMenu(JClientApp::jpApp->hmenuRichEdit, 0), TPM_LEFTALIGN | TPM_RIGHTBUTTON,
					min(max(GET_X_LPARAM(lParam), r.left), r.right),
					min(max(GET_Y_LPARAM(lParam), r.top), r.bottom), 0, hWnd, 0);
			} else if ((HWND)wParam == hwndList
				&& ListView_GetSelectedCount(hwndList)) {
				RECT r;
				GetWindowRect((HWND)wParam, &r);
				TrackPopupMenu(GetSubMenu(JClientApp::jpApp->hmenuUser, 0), TPM_LEFTALIGN | TPM_RIGHTBUTTON,
					min(max(GET_X_LPARAM(lParam), r.left), r.right),
					min(max(GET_Y_LPARAM(lParam), r.top), r.bottom), 0, hWnd, 0);
			} else {
				JPageLog::DlgProc(hWnd, message, wParam, lParam);
				rtf::Editor::DlgProc(hWnd, message, wParam, lParam);
			}
			break;
		}

	case WM_INITMENUPOPUP:
		{
			if ((HMENU)wParam == GetSubMenu(JClientApp::jpApp->hmenuChannel, 0)) {
				VERIFY(SetMenuDefaultItem((HMENU)wParam, IDC_CHANTOPIC, FALSE));
				EnableMenuItem((HMENU)wParam, IDC_CHANTOPIC,
					MF_BYCOMMAND | (m_channel.getStatus(pSource->m_idOwn) >= eMember ? MF_ENABLED : MF_GRAYED));

				CHARRANGE cr;
				SendMessage(m_hwndLog, EM_EXGETSEL, 0, (LPARAM)&cr);
				bool cancopy = cr.cpMin != cr.cpMax;
				EnableMenuItem((HMENU)wParam, IDC_LOGCOPY,
					MF_BYCOMMAND | (cancopy ? MF_ENABLED : MF_GRAYED));
				break;
			} else if ((HMENU)wParam == GetSubMenu(JClientApp::jpApp->hmenuRichEdit, 0))
			{
				VERIFY(SetMenuDefaultItem((HMENU)wParam, IDC_SEND, FALSE));
				CheckMenuRadioItem((HMENU)wParam,
					IDC_SENDBYENTER, IDC_SENDBYCTRLENTER,
					pSource->bSendByEnter ? IDC_SENDBYENTER : IDC_SENDBYCTRLENTER,
					MF_BYCOMMAND);

				CHARRANGE cr;
				SendMessage(m_hwndEdit, EM_EXGETSEL, 0, (LPARAM)&cr);
				bool cancopy = cr.cpMin != cr.cpMax;
				bool canpaste = SendMessage(m_hwndEdit, EM_CANPASTE, CF_TEXT, 0) != 0;
				bool candelete = cancopy && (GetWindowLong(m_hwndEdit, GWL_STYLE) & ES_READONLY) == 0;
				bool canselect = GetWindowTextLength(m_hwndEdit) > 0;
				EnableMenuItem((HMENU)wParam, IDC_COPY,
					MF_BYCOMMAND | (cancopy ? MF_ENABLED : MF_GRAYED));
				EnableMenuItem((HMENU)wParam, IDC_CUT,
					MF_BYCOMMAND | (candelete ? MF_ENABLED : MF_GRAYED));
				EnableMenuItem((HMENU)wParam, IDC_PASTE,
					MF_BYCOMMAND | (canpaste ? MF_ENABLED : MF_GRAYED));
				EnableMenuItem((HMENU)wParam, IDC_DELETE,
					MF_BYCOMMAND | (candelete ? MF_ENABLED : MF_GRAYED));
				EnableMenuItem((HMENU)wParam, IDC_SELECTALL,
					MF_BYCOMMAND | (canselect ? MF_ENABLED : MF_GRAYED));
				break;
			} else if ((HMENU)wParam == GetSubMenu(JClientApp::jpApp->hmenuUser, 0)) {
				MapUser::const_iterator iu = getSelUser();
				bool valid = iu != pSource->m_mUser.end();

				VERIFY(SetMenuDefaultItem((HMENU)wParam, IDC_PRIVATETALK, FALSE));
				EnableMenuItem((HMENU)wParam, IDC_PRIVATETALK,
					MF_BYCOMMAND | (valid && JClient::s_mapAlert[iu->second.nStatus].fCanOpenPrivate ? MF_ENABLED : MF_GRAYED));
				EnableMenuItem((HMENU)wParam, IDC_PRIVATEMESSAGE,
					MF_BYCOMMAND | (valid && JClient::s_mapAlert[iu->second.nStatus].fCanMessage ? MF_ENABLED : MF_GRAYED));
				EnableMenuItem((HMENU)wParam, IDS_SOUNDSIGNAL,
					MF_BYCOMMAND | (valid && JClient::s_mapAlert[iu->second.nStatus].fCanSignal ? MF_ENABLED : MF_GRAYED));
				EnableMenuItem((HMENU)wParam, IDC_ALERT,
					MF_BYCOMMAND | (valid && JClient::s_mapAlert[iu->second.nStatus].fCanAlert ? MF_ENABLED : MF_GRAYED));
				EnableMenuItem((HMENU)wParam, IDC_SPLASHRTF,
					MF_BYCOMMAND | (valid && JClient::s_mapAlert[iu->second.nStatus].fCanAlert ? MF_ENABLED : MF_GRAYED));

				bool isModer = m_channel.getStatus(pSource->m_idOwn) >= eModerator;
				bool canKick = m_channel.getStatus(pSource->m_idOwn) >= m_channel.getStatus(iu->first);
				EnableMenuItem((HMENU)wParam, IDC_KICK,
					MF_BYCOMMAND | (valid && (pSource->m_idOwn == iu->first || (isModer && canKick)) ? MF_ENABLED : MF_GRAYED));
			} else if ((HMENU)wParam == GetSubMenu(GetSubMenu(JClientApp::jpApp->hmenuUser, 0), 8)) {
				MapUser::const_iterator iu = getSelUser();
				bool valid = iu != pSource->m_mUser.end();

				if (valid) {
					EChanStatus statOwn = m_channel.getStatus(pSource->m_idOwn), statUser = m_channel.getStatus(iu->first);
					CheckMenuRadioItem((HMENU)wParam, eOutsider, eFounder, eFounder - statUser, MF_BYPOSITION);
					for (int i = eOutsider; i <= eFounder; i++) {
						EnableMenuItem((HMENU)wParam, IDC_FOUNDER + eFounder - i,
							MF_BYCOMMAND | (
							(statOwn > statUser || statOwn >= eModerator || iu->first == pSource->m_idOwn) &&
							(statOwn > i || statOwn == eFounder)
							? MF_ENABLED : MF_GRAYED));
					}
				} else {
					for (int i = eOutsider; i <= eFounder; i++) {
						EnableMenuItem((HMENU)wParam, IDC_FOUNDER + eFounder - i, MF_BYCOMMAND | MF_GRAYED);
					}
				}
			} else {
				JPageLog::DlgProc(hWnd, message, wParam, lParam);
				rtf::Editor::DlgProc(hWnd, message, wParam, lParam);
			}
			break;
		}

	default:
		retval =
			JPageLog::DlgProc(hWnd, message, wParam, lParam) ||
			rtf::Editor::DlgProc(hWnd, message, wParam, lParam);
	}
	return retval;
}

void CALLBACK JClient::JPageChannel::BuildView()
{
	SendMessage(m_hwndList, WM_SETREDRAW, FALSE, 0);

	// Set background color
	SendMessage(m_hwndEdit, EM_SETBKGNDCOLOR, FALSE, (LPARAM)m_channel.crBackground);
	SendMessage(m_hwndLog, EM_SETBKGNDCOLOR, FALSE, (LPARAM)m_channel.crBackground);
	ListView_SetBkColor(m_hwndList, m_channel.crBackground);

	for each (SetId::value_type const& v in m_channel.opened)
		AddLine(v);

	SendMessage(m_hwndList, WM_SETREDRAW, TRUE, 0);
	InvalidateRect(m_hwndList, 0, TRUE);
}

void CALLBACK JClient::JPageChannel::ClearView()
{
	ListView_DeleteAllItems(m_hwndList);
	SetWindowText(m_hwndLog, TEXT(""));
	SetWindowText(m_hwndEdit, TEXT(""));
}

int  CALLBACK JClient::JPageChannel::AddLine(DWORD id)
{
	LVITEM lvi;
	int index = INT_MAX;
	// Put into process log window
	lvi.mask = LVIF_PARAM | LVIF_TEXT | LVIF_IMAGE;
	lvi.iItem = index;
	lvi.iSubItem = 0;
	lvi.pszText = LPSTR_TEXTCALLBACK;
	lvi.cchTextMax = 0;
	lvi.iImage = I_IMAGECALLBACK;
	lvi.lParam = (LPARAM)id;
	index = ListView_InsertItem(m_hwndList, &lvi);
	ListView_SetItemText(m_hwndList, index, 1, LPSTR_TEXTCALLBACK);
	ListView_SetItemText(m_hwndList, index, 2, LPSTR_TEXTCALLBACK);
	return index;
}

void CALLBACK JClient::JPageChannel::DelLine(DWORD id)
{
	LVFINDINFO lvfi;
	lvfi.flags = LVFI_PARAM;
	lvfi.lParam = id;
	ListView_DeleteItem(m_hwndList, ListView_FindItem(m_hwndList, -1, &lvfi));
}

void JClient::JPageChannel::OnHook(JEventable* src)
{
	ASSERT(pSource);
	using namespace fastdelegate;

	__super::OnHook(src);

	pSource->EvLinkIdentify += MakeDelegate(this, &JClient::JPageChannel::OnLinkIdentify);
	pSource->EvLinkClose += MakeDelegate(this, &JClient::JPageChannel::OnLinkClose);
}

void JClient::JPageChannel::OnUnhook(JEventable* src)
{
	ASSERT(pSource);
	using namespace fastdelegate;

	pSource->EvLinkIdentify -= MakeDelegate(this, &JClient::JPageChannel::OnLinkIdentify);
	pSource->EvLinkClose -= MakeDelegate(this, &JClient::JPageChannel::OnLinkClose);

	__super::OnUnhook(src);
}

void JClient::JPageChannel::OnLinkIdentify(SOCKET sock, const netengine::SetAccess& access)
{
	ASSERT(pSource);
	if (m_hwndPage) {
		pSource->Send_Quest_JOIN(pSource->m_clientsock, m_channel.name, m_channel.password, gettype());
	}
}

void JClient::JPageChannel::OnLinkClose(SOCKET sock, UINT err)
{
	ASSERT(pSource);
	if (m_hwndPage) {
		if (m_channel.opened.size()) // only one disconnect message during connecting
			AppendScript(TEXT("[style=Info]Disconnected[/style]"));
	}
	m_channel.opened.clear(); // no users on disconnected channel
}

//-----------------------------------------------------------------------------

// The End.