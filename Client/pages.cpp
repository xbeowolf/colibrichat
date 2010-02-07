
//-----------------------------------------------------------------------------

//
// Includes
//

#pragma region Includes

#include "stdafx.h"

// Common
#include "stylepr.h"
//#include "CRC.h"
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

std::tstring JClient::JPage::getSafeName(DWORD idUser) const
{
	return pSource->getSafeName(idUser);
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
	if ((a > m_alert || a == eGreen)
		&& (pSource->jpOnline != this || !pSource->m_mUser[pSource->m_idOwn].isOnline)
		&& pSource->hwndPage)
	{
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

	pSource->EvLinkStart += MakeDelegate(this, &JClient::JPage::OnLinkStart);
	pSource->EvLinkClose += MakeDelegate(this, &JClient::JPage::OnLinkClose);
}

void JClient::JPage::OnUnhook(JEventable* src)
{
	ASSERT(pSource);
	using namespace fastdelegate;

	pSource->EvLinkStart -= MakeDelegate(this, &JClient::JPage::OnLinkStart);
	pSource->EvLinkClose -= MakeDelegate(this, &JClient::JPage::OnLinkClose);

	__super::OnUnhook(src);

	SetSource(0);
}

void JClient::JPage::OnLinkStart(SOCKET sock)
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
	m_Groups.insert(eMessage);
	m_Groups.insert(eDescription);
	m_Groups.insert(eInformation);
	m_Groups.insert(eWarning);
	m_Groups.insert(eError);
	m_Priority = eNormal;
	etimeFormat = etimeHHMMSS;
}

void CALLBACK JClient::JPageLog::AppendRtf(std::string& content, bool toascii) const
{
	CHARRANGE crMark, crIns; // Selection position
	int nTextLen; // Length of text in control

	SendMessage(m_hwndLog, EM_HIDESELECTION, TRUE, 0);

	SendMessage(m_hwndLog, EM_EXGETSEL, 0, (LPARAM)&crMark);
	nTextLen = GetWindowTextLength(m_hwndLog);

	SendMessage(m_hwndLog, EM_SETSEL, -1, -1);
	SendMessage(m_hwndLog, EM_EXGETSEL, 0, (LPARAM)&crIns);
	SendMessage(m_hwndLog, EM_REPLACESEL, FALSE, (LPARAM)ANSIToTstr(content).c_str());

	if (toascii) {
		crIns.cpMax = -1;
		SendMessage(m_hwndLog, EM_EXSETSEL, 0, (LPARAM)&crIns);
		SendMessage(m_hwndLog, EM_EXGETSEL, 0, (LPARAM)&crIns);
		content.resize(crIns.cpMax - crIns.cpMin);
		SendMessageA(m_hwndLog, EM_GETSELTEXT, 0, (LPARAM)content.data());
	}

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
			_stprintf_s(time, _countof(time), TEXT("[style=time][%02u:%02u][/style] "), st.wHour, st.wMinute);
			break;
		case etimeHHMMSS:
			_stprintf_s(time, _countof(time), TEXT("[style=time][%02u:%02u:%02u][/style] "), st.wHour, st.wMinute, st.wSecond);
			break;
		default:
			_stprintf_s(time, _countof(time), TEXT(""));
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

void CALLBACK JClient::JPageLog::Say(DWORD idWho, std::string& content)
{
	AppendScript(tformat(TEXT("[color=%s]%s[/color]:"),
		idWho != pSource->m_idOwn ? TEXT("red") : TEXT("blue"),
		getSafeName(idWho).c_str()));
	AppendRtf(content, true);

	// Lua response
	if (pSource->m_luaEvents) {
		lua_getglobal(pSource->m_luaEvents, "onSay");
		lua_pushstring(pSource->m_luaEvents, tstrToANSI(getSafeName(idWho)).c_str());
		lua_pushstring(pSource->m_luaEvents, tstrToANSI(getname()).c_str());
		lua_pushstring(pSource->m_luaEvents, content.c_str());
		ASSERT(lua_gettop(pSource->m_luaEvents) == 4);
		lua_call(pSource->m_luaEvents, 3, 0);
	}
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

			case IDC_LOGSAVEAS:
				{
					static TCHAR szAcceptFile[4096], szTitleBuffer[MAX_PATH];
					static TCHAR szCustomFilter[256] = TEXT("");
					static TCHAR szTxtFilter[] =
						TEXT("Rich text format (*.rtf)\000*.rtf\000")
						TEXT("Text document (*.txt)\000*.txt\000")
						TEXT("Unicode text document (*.txt)\000*.txt\000")
						;
					OPENFILENAME ofn =
					{
						sizeof(OPENFILENAME), // lStructSize
						0, // hwndOwner
						0, // hInstance
						szTxtFilter, // lpstrFilter
						szCustomFilter, // lpstrCustomFilter
						_countof(szCustomFilter), // nMaxCustFilter
						1, // nFilterIndex
						szAcceptFile, // lpstrFile
						_countof(szAcceptFile), // nMaxFile
						szTitleBuffer, // lpstrFileTitle
						_countof(szTitleBuffer), // nMaxFileTitle
						0, // lpstrInitialDir
						0, // lpstrTitle
						0, // Flags
						0, // nFileOffset
						0, // nFileExtension
						TEXT("RTF"), // lpstrDefExt
						0, // lCustData
						0, // lpfnHook
						0 // lpTemplateName
					};
					ofn.hwndOwner = hWnd;
					ofn.Flags =
						OFN_ENABLESIZING | OFN_EXPLORER |
						OFN_EXTENSIONDIFFERENT | OFN_FILEMUSTEXIST |
						OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST;
					if (GetSaveFileName(&ofn)) {
						HANDLE hFile = CreateFile(szAcceptFile, GENERIC_WRITE, FILE_SHARE_READ, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
						if (hFile != INVALID_HANDLE_VALUE) {
							UINT mode = SF_RTF;
							switch (ofn.nFilterIndex)
							{
							case 1:
								mode = SF_RTF;
								break;
							case 2:
								mode = SF_TEXTIZED;
								break;
							case 3:
								{
									WORD prefix = 0xFEFF;
									DWORD cb;
									VERIFY(WriteFile(hFile, &prefix, 2, &cb, 0));
									mode = SF_TEXTIZED | SF_UNICODE;
									break;
								}
							}
							EDITSTREAM es;
							es.dwCookie = (DWORD_PTR)hFile;
							es.dwError = 0;
							es.pfnCallback = rtf::StreamToHandle;
							SendMessage(m_hwndLog, EM_STREAMOUT, mode, (LPARAM)&es);
							VERIFY(CloseHandle(hFile));
						}
					}
					break;
				}

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
				retval = __super::DlgProc(hWnd, message, wParam, lParam);
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
	EnableWindow(m_hwndNick, TRUE);
	m_fEnabled = true;
}

void CALLBACK JClient::JPageServer::Disable()
{
	EnableWindow(m_hwndHost, TRUE);
	EnableWindow(m_hwndPort, TRUE);
	EnableWindow(m_hwndPass, TRUE);
	EnableWindow(m_hwndNick, TRUE);
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
			SetWindowText(m_hwndPass, pSource->m_passwordNet.c_str());
			// Init Nick control
			SetWindowText(m_hwndNick, pSource->m_mUser[pSource->m_idOwn].name.c_str());
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

			OnMetrics(pSource->m_metrics);

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
					// Lua response
					if (pSource->m_luaEvents) {
						lua_getglobal(pSource->m_luaEvents, "idcConnect");
						ASSERT(lua_gettop(pSource->m_luaEvents) == 1);
						lua_call(pSource->m_luaEvents, 0, 0);
					} else if (pSource->m_clientsock) {
						pSource->DeleteLink(pSource->m_clientsock);
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
							pSource->PushTrn(pSource->m_clientsock, pSource->Make_Cmd_STATUS_Mode(stat, pSource->m_mAlert[stat]));
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
							pSource->PushTrn(pSource->m_clientsock, pSource->Make_Cmd_STATUS_Img(img));
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
	pSource->EvMetrics += MakeDelegate(this, &JClient::JPageServer::OnMetrics);
}

void JClient::JPageServer::OnUnhook(JEventable* src)
{
	ASSERT(pSource);
	using namespace fastdelegate;

	pSource->EvLog -= MakeDelegate(this, &JClient::JPageServer::OnLog);
	pSource->EvReport -= MakeDelegate(this, &JClient::JPageServer::OnReport);
	pSource->EvMetrics -= MakeDelegate(this, &JClient::JPageServer::OnMetrics);

	__super::OnUnhook(src);
}

void JClient::JPageServer::OnLog(const std::tstring& str, bool withtime)
{
	ASSERT(pSource);
	AppendScript(str, withtime);
}

void JClient::JPageServer::OnReport(const std::tstring& str, EGroup gr, EPriority prior)
{
	ASSERT(pSource);
	if (!m_hwndPage) return; // ignore if window closed
	if (m_Groups.find(gr) != m_Groups.end() && prior < m_Priority) return;

	std::tstring msg;
	switch (gr)
	{
	case eMessage:
		msg = TEXT("[style=Msg]") + str + TEXT("[/style]");
		setAlert(eBlue);
		break;
	case eDescription:
		msg = TEXT("[style=Descr]") + str + TEXT("[/style]");
		setAlert(eBlue);
		break;
	case eInformation:
		msg = TEXT("[style=Info]") + str + TEXT("[/style]");
		setAlert(eBlue);
		break;
	case eWarning:
		msg = TEXT("[style=Warning]") + str + TEXT("[/style]");
		setAlert(eYellow);
		break;
	case eError:
		msg = TEXT("[style=Error]") + str + TEXT("[/style]");
		setAlert(eRed);
		break;
	default:
		msg = TEXT("[style=Default]") + str + TEXT("[/style]");
		break;
	}
	AppendScript(msg);
}

void JClient::JPageServer::OnMetrics(const Metrics& metrics)
{
	ASSERT(pSource);
	if (!m_hwndPage) return; // ignore if window closed

	SendMessage(m_hwndPass, EM_LIMITTEXT, metrics.uPassMaxLength, 0);
	SendMessage(m_hwndNick, EM_LIMITTEXT, metrics.uNameMaxLength, 0);
	SendMessage(m_hwndStatusMsg, EM_LIMITTEXT, metrics.uStatusMsgMaxLength, 0);
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

MapChannel::const_iterator JClient::JPageList::getSelChannel() const
{
	int index = ListView_GetNextItem(m_hwndList, -1, LVNI_SELECTED);
	if (index >= 0) {
		LVITEM lvi;
		lvi.mask = LVIF_PARAM;
		lvi.iItem = index;
		lvi.iSubItem = 0;
		if (ListView_GetItem(m_hwndList, &lvi)) {
			return m_mChannel.find((DWORD)lvi.lParam);
		}
	}
	return m_mChannel.end();
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

			OnMetrics(pSource->m_metrics);

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
					ASSERT(pSource->m_clientsock);
					std::tstring chanbuf(pSource->m_metrics.uNameMaxLength, 0), passbuf(pSource->m_metrics.uPassMaxLength, 0);
					std::tstring chan, pass;
					GetWindowText(m_hwndChan, &chanbuf[0], (int)chanbuf.size()+1);
					GetWindowText(m_hwndPass, &passbuf[0], (int)passbuf.size()+1);
					const TCHAR* msg;
					chan = chanbuf.c_str(), pass = passbuf.c_str();
					if (JClient::CheckNick(chan, msg)) { // check content
						// send only c-strings, not buffer!
						pSource->PushTrn(pSource->m_clientsock, pSource->Make_Quest_JOIN(chan, pass));
					} else {
						pSource->DisplayMessage(m_hwndChan, msg, MAKEINTRESOURCE(IDS_MSG_NICKERROR), 2);
					}
					break;
				}

			case IDC_RENAME:
				{
					ASSERT(pSource->m_clientsock);
					int index = ListView_GetNextItem(m_hwndList, -1, LVNI_SELECTED);
					if (index >= 0) {
						ListView_EditLabel(m_hwndList, index);
					}
					break;
				}

			case IDC_TOPIC:
				{
					ASSERT(pSource->m_clientsock);
					MapChannel::const_iterator ic = getSelChannel();
					ASSERT(ic != m_mChannel.end());
					if (ic->second.getStatus(pSource->m_idOwn) >= (ic->second.isOpened(pSource->m_idOwn) ? eMember : eAdmin) || pSource->isGod())
						CreateDialogParam(JClientApp::jpApp->hinstApp, MAKEINTRESOURCE(IDD_TOPIC), pSource->hwndPage, (DLGPROC)JDialog::DlgProcStub, (LPARAM)(JDialog*)new JTopic(pSource, ic->first, ic->second.name, ic->second.topic));
					break;
				}

			case IDC_REFRESHLIST:
				{
					ASSERT(pSource->m_clientsock);
					pSource->PushTrn(pSource->m_clientsock, Make_Quest_LIST());
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
							ASSERT(iter != m_mChannel.end());
							switch (pnmv->item.iSubItem)
							{
							case 0:
								pnmv->item.pszText = (TCHAR*)iter->second.name.c_str();
								break;

							case 1:
								_stprintf_s(buffer, _countof(buffer), TEXT("%u"), iter->second.opened.size());
								pnmv->item.pszText = buffer;
								break;

							case 2:
								pnmv->item.pszText = (TCHAR*)iter->second.topic.c_str();
								break;

							case 3:
								pnmv->item.pszText = (TCHAR*)s_mapChanStatName[iter->second.nAutoStatus].c_str();
								break;

							case 4:
								pnmv->item.pszText = iter->second.isPrivate() ? TEXT("+") : TEXT("-");
								break;

							case 5:
								pnmv->item.pszText = iter->second.isAnonymous ? TEXT("+") : TEXT("-");
								break;

							case 6:
								{
									SYSTEMTIME st;
									FileTimeToLocalTime(iter->second.ftCreation, st);

									_stprintf_s(buffer, _countof(buffer),
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
						ASSERT(ic != m_mChannel.end());
						if (pnmv->iItem >= 0 && (pnmv->uNewState & LVIS_SELECTED) != 0 && (pnmv->uOldState & LVIS_SELECTED) == 0)
						{
							SetWindowText(m_hwndChan, ic->second.name.c_str());
						}
					}
					break;
				}

			case LVN_BEGINLABELEDIT:
				{
					NMLVDISPINFO* pdi = (NMLVDISPINFO*)lParam;
					if (!pSource->m_clientsock) break;
					if (pnmh->idFrom == IDC_CHANLIST) {
						MapChannel::const_iterator ic = m_mChannel.find((DWORD)pdi->item.lParam);
						ASSERT(ic != m_mChannel.end());
						if (ic->second.getStatus(pSource->m_idOwn) == eFounder || pSource->isGod()) {
							JClientApp::jpApp->haccelCurrent = 0; // disable accelerators
							retval = FALSE;
						}
					}
					break;
				}

			case LVN_ENDLABELEDIT:
				{
					NMLVDISPINFO* pdi = (NMLVDISPINFO*)lParam;
					JClientApp::jpApp->haccelCurrent = JClientApp::jpApp->haccelMain; // enable accelerators
					if (pnmh->idFrom == IDC_CHANLIST) {
						if (pdi->item.pszText && pSource->m_clientsock) {
							std::tstring nick = pdi->item.pszText;
							const TCHAR* msg;
							if (JClient::CheckNick(nick, msg)) {
								pSource->PushTrn(pSource->m_clientsock, pSource->Make_Cmd_NICK((DWORD)pdi->item.lParam, nick));
								SetWindowText(m_hwndChan, nick.c_str());
								retval = TRUE;
								break;
							} else {
								pSource->DisplayMessage(ListView_GetEditControl(m_hwndList), msg, MAKEINTRESOURCE(IDS_MSG_NICKERROR), 2);
							}
						}
					}
					retval = FALSE;
					break;
				}

			case NM_DBLCLK:
				{
					if (pnmh->idFrom == IDC_CHANLIST) {
						LPNMITEMACTIVATE lpnmitem = (LPNMITEMACTIVATE)lParam;
						if (lpnmitem->iItem >= 0 && pSource->m_clientsock) {
							SendMessage(hWnd, WM_COMMAND, IDC_JOIN, 0);
						}
					}
					break;
				}

			default: retval = __super::DlgProc(hWnd, message, wParam, lParam);
			}
			break;
		}

	case WM_CONTEXTMENU:
		{
			if ((HWND)wParam == m_hwndList
				&& pSource->m_clientsock && ListView_GetSelectedCount(m_hwndList)) {
				RECT r;
				GetWindowRect((HWND)wParam, &r);
				TrackPopupMenu(GetSubMenu(JClientApp::jpApp->hmenuList, 0), TPM_LEFTALIGN | TPM_RIGHTBUTTON,
					min(max(GET_X_LPARAM(lParam), r.left), r.right),
					min(max(GET_Y_LPARAM(lParam), r.top), r.bottom), 0, hWnd, 0);
			} else {
				retval = __super::DlgProc(hWnd, message, wParam, lParam);
			}
			break;
		}

	case WM_INITMENUPOPUP:
		{
			if ((HMENU)wParam == GetSubMenu(JClientApp::jpApp->hmenuList, 0)) {
				int index = -1;
				MapChannel::const_iterator ic = getSelChannel();
				bool valid = ic != m_mChannel.end();

				VERIFY(SetMenuDefaultItem((HMENU)wParam, IDC_JOIN, FALSE));
				EnableMenuItem((HMENU)wParam, IDC_RENAME,
					MF_BYCOMMAND | (ic->second.getStatus(pSource->m_idOwn) == eFounder || pSource->isGod() ? MF_ENABLED : MF_GRAYED));
				EnableMenuItem((HMENU)wParam, IDC_TOPIC,
					MF_BYCOMMAND | (ic->second.getStatus(pSource->m_idOwn) >= (ic->second.isOpened(pSource->m_idOwn) ? eMember : eAdmin) || pSource->isGod() ? MF_ENABLED : MF_GRAYED));
			} else {
				__super::DlgProc(hWnd, message, wParam, lParam);
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

	pSource->EvLinkStart += MakeDelegate(this, &JClient::JPageList::OnLinkStart);
	pSource->EvMetrics += MakeDelegate(this, &JClient::JPageList::OnMetrics);
	pSource->EvTopic += MakeDelegate(this, &JClient::JPageList::OnTopic);
	pSource->EvNick += MakeDelegate(this, &JClient::JPageList::OnNick);

	// Transactions parsers
	pSource->m_mTrnReply[CCPM_LIST] = fastdelegate::MakeDelegate(this, &JClient::JPageList::Recv_Reply_LIST);
}

void JClient::JPageList::OnUnhook(JEventable* src)
{
	ASSERT(pSource);
	using namespace fastdelegate;

	// Transactions parsers
	pSource->m_mTrnReply.erase(CCPM_LIST);

	pSource->EvLinkStart -= MakeDelegate(this, &JClient::JPageList::OnLinkStart);
	pSource->EvMetrics -= MakeDelegate(this, &JClient::JPageList::OnMetrics);
	pSource->EvTopic -= MakeDelegate(this, &JClient::JPageList::OnTopic);
	pSource->EvNick -= MakeDelegate(this, &JClient::JPageList::OnNick);

	__super::OnUnhook(src);
}

void JClient::JPageList::OnLinkStart(SOCKET sock)
{
	ASSERT(pSource);

	pSource->PushTrn(sock, Make_Quest_LIST());
}

void JClient::JPageList::OnMetrics(const Metrics& metrics)
{
	ASSERT(pSource);
	if (!m_hwndPage) return; // ignore if window closed

	SendMessage(m_hwndChan, EM_LIMITTEXT, metrics.uNameMaxLength, 0);
	SendMessage(m_hwndPass, EM_LIMITTEXT, metrics.uPassMaxLength, 0);
}

void JClient::JPageList::OnTopic(DWORD idWho, DWORD idWhere, const std::tstring& topic)
{
	ASSERT(pSource);
	MapChannel::iterator ic = m_mChannel.find(idWhere);
	if (ic == m_mChannel.end()) return;
	ic->second.idTopicWriter = idWho;
	ic->second.topic = topic;
	if (m_hwndPage && IsWindowVisible(m_hwndPage)) {
		LVFINDINFO lvfi;
		lvfi.flags = LVFI_PARAM;
		lvfi.lParam = (LPARAM)idWhere;
		int index = ListView_FindItem(m_hwndList, -1, &lvfi);
		if (index != -1) VERIFY(ListView_RedrawItems(m_hwndList, index, index));
		else AddLine(idWhere);
	}
}

void JClient::JPageList::OnNick(DWORD idOld, const std::tstring& oldname, DWORD idNew, const std::tstring& newname)
{
	ASSERT(pSource);
	MapChannel::iterator ic = m_mChannel.find(idOld);
	if (ic != m_mChannel.end()) {
		ic->second.name = newname;
		m_mChannel[idNew] = ic->second;
		m_mChannel.erase(idOld);

		if (m_hwndPage) {
			LVFINDINFO lvfi;
			lvfi.flags = LVFI_PARAM;
			lvfi.lParam = (LPARAM)idOld;
			int index = ListView_FindItem(m_hwndList, -1, &lvfi);
			if (index != -1) {
				LVITEM lvi;
				lvi.mask = LVIF_PARAM;
				lvi.iItem = index;
				lvi.iSubItem = 0;
				lvi.lParam = (LPARAM)idNew;
				VERIFY(ListView_SetItem(m_hwndList, &lvi));
				VERIFY(ListView_RedrawItems(m_hwndList, index, index));
			} else AddLine(idNew);
		}
	}
}

void JClient::JPageList::Recv_Reply_LIST(SOCKET sock, WORD trnid, io::mem& is)
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
			pSource->EvReport(SZ_BADTRN, eWarning, eLow);
			return;
		}
	}

	if (m_hwndPage) {
		ClearView();
		BuildView();
	}
	pSource->EvReport(tformat(TEXT("listed [b]%u[/b] channels"), m_mChannel.size()), eInformation, eNormal);
}

JPtr<JTransaction> JClient::JPageList::Make_Quest_LIST() const
{
	std::ostringstream os;
	return pSource->MakeTrn(QUEST(CCPM_LIST), 0, os.str());
}

//-----------------------------------------------------------------------------

//
// JPageChat
//

std::map<UINT, const TCHAR*> JClient::JPageChat::s_mapButTips;

void JClient::JPageChat::initclass()
{
	// Editor toolbar buttons tips
	{
		using namespace rtf;
		s_mapButTips[idcRtfCmd] = MAKEINTRESOURCE(idsRtfCmd);
		s_mapButTips[idcBold] = MAKEINTRESOURCE(idsBold);
		s_mapButTips[idcItalic] = MAKEINTRESOURCE(idsItalic);
		s_mapButTips[idcUnderline] = MAKEINTRESOURCE(idsUnderline);
		s_mapButTips[idcSubscript] = MAKEINTRESOURCE(idsSubscript);
		s_mapButTips[idcSuperscript] = MAKEINTRESOURCE(idsSuperscript);
		s_mapButTips[idcFont] = MAKEINTRESOURCE(idsFont);
		s_mapButTips[idcFgColor] = MAKEINTRESOURCE(idsFgColor);
		s_mapButTips[idcBgColor] = MAKEINTRESOURCE(idsBgColor);
		s_mapButTips[idcSheetColor] = MAKEINTRESOURCE(idsSheetColor);
		s_mapButTips[idcAlignLeft] = MAKEINTRESOURCE(idsAlignLeft);
		s_mapButTips[idcAlignRight] = MAKEINTRESOURCE(idsAlignRight);
		s_mapButTips[idcAlignCenter] = MAKEINTRESOURCE(idsAlignCenter);
		s_mapButTips[idcAlignJustify] = MAKEINTRESOURCE(idsAlignJustify);
		s_mapButTips[idcMarksBullet] = MAKEINTRESOURCE(idsMarksBullet);
		s_mapButTips[idcMarksArabic] = MAKEINTRESOURCE(idsMarksArabic);
		s_mapButTips[idcStartIndentInc] = MAKEINTRESOURCE(idsStartIndentInc);
		s_mapButTips[idcStartIndentDec] = MAKEINTRESOURCE(idsStartIndentDec);
		s_mapButTips[idcBkMode] = MAKEINTRESOURCE(idsBkMode);
	}
}

void JClient::JPageChat::doneclass()
{
	s_mapButTips.clear();
}

CALLBACK JClient::JPageChat::JPageChat(DWORD id)
: JPageLog(), rtf::Editor()
{
	m_ID = id;
}

void CALLBACK JClient::JPageChat::Say(DWORD idWho, std::string& content)
{
	__super::Say(idWho, content);

	if (idWho == pSource->m_idOwn) { // Blue spin
		vecMsgSpinBlue.insert(vecMsgSpinBlue.begin(), content);
		if (vecMsgSpinBlue.size() > pSource->m_metrics.nMsgSpinMaxCount)
			vecMsgSpinBlue.erase(vecMsgSpinBlue.begin() + pSource->m_metrics.nMsgSpinMaxCount, vecMsgSpinBlue.end());
		SendMessage(m_hwndMsgSpinBlue, UDM_SETRANGE, 0, MAKELONG(vecMsgSpinBlue.size(), 0));
		InvalidateRect(m_hwndMsgSpinBlue, 0, TRUE);
		LPARAM pos = SendMessage(m_hwndMsgSpinBlue, UDM_GETPOS, 0, 0);
		if (LOWORD(pos) && !HIWORD(pos)) {
			pos = min(pos + 1, (LPARAM)vecMsgSpinBlue.size());
			SendMessage(m_hwndMsgSpinBlue, UDM_SETPOS, 0, MAKELONG(pos, 0));
		}
	} else { // Red spin
		vecMsgSpinRed.insert(vecMsgSpinRed.begin(), content);
		if (vecMsgSpinRed.size() > pSource->m_metrics.nMsgSpinMaxCount)
			vecMsgSpinRed.erase(vecMsgSpinRed.begin(), vecMsgSpinRed.begin() + vecMsgSpinRed.size() - pSource->m_metrics.nMsgSpinMaxCount);
		SendMessage(m_hwndMsgSpinRed, UDM_SETRANGE, 0, MAKELONG(vecMsgSpinRed.size(), 0));
		InvalidateRect(m_hwndMsgSpinRed, 0, TRUE);
		LPARAM pos = SendMessage(m_hwndMsgSpinRed, UDM_GETPOS, 0, 0);
		if (LOWORD(pos) && !HIWORD(pos)) {
			pos = min(pos + 1, (LPARAM)vecMsgSpinRed.size());
			SendMessage(m_hwndMsgSpinRed, UDM_SETPOS, 0, MAKELONG(pos, 0));
		}
	}
}

LRESULT WINAPI JClient::JPageChat::DlgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
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
				{IML_FRCOL, rtf::idcFgColor,
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
			m_hwndMsgSpinBlue = GetDlgItem(hWnd, IDC_MSGSPINBLUE);
			m_hwndMsgSpinRed = GetDlgItem(hWnd, IDC_MSGSPINRED);
			m_hwndSend = GetDlgItem(hWnd, IDC_SEND);

			// Get initial windows sizes
			MapControl(m_hwndEdit, rcEdit);
			MapControl(m_hwndMsgSpinBlue, rcMsgSpinBlue);
			MapControl(m_hwndMsgSpinRed, rcMsgSpinRed);
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
			// Init Log control
			SendMessage(m_hwndLog, EM_SETEVENTMASK, 0, ENM_MOUSEEVENTS);

			// Init up-doun control
			vecMsgSpinBlue.clear();
			SendMessage(m_hwndMsgSpinBlue, UDM_SETRANGE, 0, MAKELONG(vecMsgSpinBlue.size(), 0));
			vecMsgSpinRed.clear();
			SendMessage(m_hwndMsgSpinRed, UDM_SETRANGE, 0, MAKELONG(vecMsgSpinRed.size(), 0));

			// Inits Send button
			SendMessage(m_hwndSend, BM_SETIMAGE, IMAGE_BITMAP, (LPARAM)JClientApp::jpApp->himgSend);

			retval = TRUE;
			break;
		}

	case WM_DESTROY:
		{
			pSource->EvPageClose.Invoke(m_ID);

			vecMsgSpinBlue.clear();
			vecMsgSpinRed.clear();

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
							if (content.size() < pSource->m_metrics.uChatLineMaxVolume) {
								if (CanSend()) {
									pSource->PushTrn(pSource->m_clientsock, pSource->Make_Cmd_SAY(getID(), SF_RTF, content));
									SetWindowText(m_hwndEdit, TEXT(""));
									SendMessage(m_hwndMsgSpinBlue, UDM_SETPOS, 0, MAKELONG(0, 0));
									SendMessage(m_hwndMsgSpinRed, UDM_SETPOS, 0, MAKELONG(0, 0));
								} else pSource->DisplayMessage(m_hwndEdit, MAKEINTRESOURCE(IDS_MSG_READER), MAKEINTRESOURCE(IDS_MSG_EDITOR), 1);
							} else pSource->DisplayMessage(m_hwndEdit, MAKEINTRESOURCE(IDS_MSG_LIMITCHATLINE), MAKEINTRESOURCE(IDS_MSG_EDITOR), 2);
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
			case TTN_GETDISPINFO:
				{
					TOOLTIPTEXT* lpttt = (TOOLTIPTEXT*)lParam;
					lpttt->hinst = JClientApp::jpApp->hinstApp;
					lpttt->lpszText = (LPTSTR)s_mapButTips[lpttt->hdr.idFrom];
					break;
				}

			case UDN_DELTAPOS:
				{
					LPNMUPDOWN lpnmud = (LPNMUPDOWN)lParam;
					if (pnmh->idFrom == IDC_MSGSPINBLUE) {
						if (lpnmud->iPos) {
							if (Profile::GetInt(RF_CLIENT, RK_QUOTATIONBLUE, FALSE)) {
								SetWindowTextA(m_hwndEdit, "");
								CHARRANGE cr;
								SendMessage(m_hwndEdit, EM_SETSEL, 0, 0);
								SendMessage(m_hwndEdit, EM_REPLACESEL, FALSE, (LPARAM)ANSIToTstr(vecMsgSpinBlue[lpnmud->iPos - 1]).c_str());
								cr.cpMin = 0, cr.cpMax = -1;
								SendMessage(m_hwndEdit, EM_EXSETSEL, 0, (LPARAM)&cr);
								SendMessage(m_hwndEdit, EM_EXGETSEL, 0, (LPARAM)&cr);
								cr.cpMax -= 2;
								SendMessage(m_hwndEdit, EM_EXSETSEL, 0, (LPARAM)&cr);
								SendMessage(m_hwndPage, WM_COMMAND, rtf::idcStartIndentInc, 0);
								cr.cpMin = -1, cr.cpMax = -1;
								SendMessage(m_hwndEdit, EM_EXSETSEL, 0, (LPARAM)&cr);
								SendMessage(m_hwndEdit, WM_VSCROLL, SB_BOTTOM, 0);
							} else {
								SetWindowTextA(m_hwndEdit, vecMsgSpinBlue[lpnmud->iPos - 1].c_str());
							}
						} else {
							SetWindowTextA(m_hwndEdit, "");
						}
					} else if (pnmh->idFrom == IDC_MSGSPINRED) {
						if (lpnmud->iPos) {
							if (Profile::GetInt(RF_CLIENT, RK_QUOTATIONRED, TRUE)) {
								SetWindowTextA(m_hwndEdit, "");
								CHARRANGE cr;
								SendMessage(m_hwndEdit, EM_SETSEL, 0, 0);
								SendMessage(m_hwndEdit, EM_REPLACESEL, FALSE, (LPARAM)ANSIToTstr(vecMsgSpinRed[lpnmud->iPos - 1]).c_str());
								cr.cpMin = 0, cr.cpMax = -1;
								SendMessage(m_hwndEdit, EM_EXSETSEL, 0, (LPARAM)&cr);
								SendMessage(m_hwndEdit, EM_EXGETSEL, 0, (LPARAM)&cr);
								cr.cpMax -= 2;
								SendMessage(m_hwndEdit, EM_EXSETSEL, 0, (LPARAM)&cr);
								SendMessage(m_hwndPage, WM_COMMAND, rtf::idcStartIndentInc, 0);
								cr.cpMin = -1, cr.cpMax = -1;
								SendMessage(m_hwndEdit, EM_EXSETSEL, 0, (LPARAM)&cr);
								SendMessage(m_hwndEdit, WM_VSCROLL, SB_BOTTOM, 0);
							} else {
								SetWindowTextA(m_hwndEdit, vecMsgSpinRed[lpnmud->iPos - 1].c_str());
							}
						} else {
							SetWindowTextA(m_hwndEdit, "");
						}
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
				retval =
					JPageLog::DlgProc(hWnd, message, wParam, lParam) ||
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

//-----------------------------------------------------------------------------

//
// JPageUser
//

CALLBACK JClient::JPageUser::JPageUser(DWORD id, const std::tstring& nick)
: JPageChat(id)
{
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
	pSource->PushTrn(pSource->m_clientsock, pSource->Make_Cmd_CHANOPTIONS(m_ID, CHANOP_BACKGROUND, cr));
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
			__super::DlgProc(hWnd, message, wParam, lParam);

			retval = TRUE;
			break;
		}

	case WM_DESTROY:
		{
			pSource->PushTrn(pSource->clientsock, pSource->Make_Cmd_PART(pSource->m_idOwn, m_ID));
			pSource->EvReport(tformat(TEXT("parts from [b]%s[/b] private talk"), m_user.name.c_str()), eInformation, eHigher);

			__super::DlgProc(hWnd, message, wParam, lParam);
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
				cx - rcPage.right + rcMsgSpinBlue.left,
				cy - rcPage.bottom + rcMsgSpinBlue.top,
				cx - rcPage.right + rcMsgSpinBlue.right,
				cy - rcPage.bottom + rcMsgSpinBlue.bottom);
			DeferWindowPos(hdwp, m_hwndMsgSpinBlue, 0, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, SWP_NOSIZE | SWP_NOREPOSITION | SWP_NOZORDER);
			SetRect(&rc,
				cx - rcPage.right + rcMsgSpinRed.left,
				cy - rcPage.bottom + rcMsgSpinRed.top,
				cx - rcPage.right + rcMsgSpinRed.right,
				cy - rcPage.bottom + rcMsgSpinRed.bottom);
			DeferWindowPos(hdwp, m_hwndMsgSpinRed, 0, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, SWP_NOSIZE | SWP_NOREPOSITION | SWP_NOZORDER);
			SetRect(&rc,
				cx - rcPage.right + rcSend.left,
				cy - rcPage.bottom + rcSend.top,
				cx - rcPage.right + rcSend.right,
				cy - rcPage.bottom + rcSend.bottom);
			DeferWindowPos(hdwp, m_hwndSend, 0, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, SWP_NOSIZE | SWP_NOREPOSITION | SWP_NOZORDER);
			EndDeferWindowPos(hdwp);
			break;
		}

	default: retval = __super::DlgProc(hWnd, message, wParam, lParam);
	}
	return retval;
}

void JClient::JPageUser::OnHook(JEventable* src)
{
	ASSERT(pSource);
	using namespace fastdelegate;

	__super::OnHook(src);

	pSource->EvLinkStart += MakeDelegate(this, &JClient::JPageUser::OnLinkStart);
	pSource->EvLinkClose += MakeDelegate(this, &JClient::JPageUser::OnLinkClose);
}

void JClient::JPageUser::OnUnhook(JEventable* src)
{
	ASSERT(pSource);
	using namespace fastdelegate;

	pSource->EvLinkStart -= MakeDelegate(this, &JClient::JPageUser::OnLinkStart);
	pSource->EvLinkClose -= MakeDelegate(this, &JClient::JPageUser::OnLinkClose);

	__super::OnUnhook(src);
}

void JClient::JPageUser::OnLinkStart(SOCKET sock)
{
	ASSERT(pSource);
	if (m_hwndPage) {
		pSource->PushTrn(pSource->m_clientsock, pSource->Make_Quest_JOIN(m_user.name, m_user.password, gettype()));
	}
}

void JClient::JPageUser::OnLinkClose(SOCKET sock, UINT err)
{
	ASSERT(pSource);
	if (m_hwndPage) {
		AppendScript(TEXT("[style=Info]Disconnected[/style]"));
	}
}

//-----------------------------------------------------------------------------

//
// JPageChannel
//

CALLBACK JClient::JPageChannel::JPageChannel(DWORD id, const std::tstring& nick)
: JPageChat(id)
{
	m_channel.name = nick;
	m_channel.crBackground = GetSysColor(COLOR_WINDOW);
}

std::tstring JClient::JPageChannel::getSafeName(DWORD idUser) const
{
	return pSource->getSafeName(pSource->m_bCheatAnonymous || !m_channel.isAnonymous || pSource->isGod(idUser) ? idUser : CRC_ANONYMOUS);
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
	} else if (m_channel.isAnonymous || pSource->mUser.find(m_channel.idTopicWriter) == pSource->mUser.end()) {
		return tformat(TEXT("#%s: %s"), m_channel.name.c_str(), m_channel.topic.c_str());
	} else {
		return tformat(TEXT("#%s: %s (%s)"), m_channel.name.c_str(), m_channel.topic.c_str(), getSafeName(m_channel.idTopicWriter).c_str());
	}
}

void CALLBACK JClient::JPageChannel::DisplayMessage(DWORD idUser, const TCHAR* msg, HICON hicon, COLORREF cr)
{
	POINT p;
	VERIFY(GetCursorPos(&p));
	RECT r0, r;
	GetWindowRect(m_hwndList, &r0);
	LVFINDINFO lvfi;
	lvfi.flags = LVFI_PARAM;
	lvfi.lParam = (LPARAM)idUser;
	LVITEM lvi;
	lvi.mask = LVIF_PARAM;
	int index = ListView_FindItem(m_hwndList, -1, &lvfi);
	if (index >= 0 && ListView_GetItemRect(m_hwndList, index, &r, LVIR_SELECTBOUNDS)) {
		MapWindowPoints(m_hwndList, 0, (LPPOINT)&r, sizeof(RECT)/sizeof(POINT));
		r.left = r0.left, r.right = r0.right;
		if (r.top < r0.top) r.top = r0.top;
		if (r.bottom > r0.bottom) r.bottom = r0.bottom;
	} else r = r0;
	if (p.x < r.left || p.x > r.right) p.x = (r.left + r.right)/2;
	if (p.y < r.top || p.y > r.bottom) p.y = (r.top + r.bottom)/2;
	MapUser::const_iterator iu = pSource->m_mUser.find(idUser);
	pSource->ShowBaloon(
		p,
		m_hwndList,
		msg,
		iu != pSource->m_mUser.end() ? iu->second.name.c_str() : 0,
		hicon,
		cr);
}

void CALLBACK JClient::JPageChannel::OnSheetColor(COLORREF cr)
{
	__super::OnSheetColor(cr);

	ASSERT(pSource);
	if (m_channel.getStatus(pSource->m_idOwn) >= eAdmin || pSource->isGod()) {
		pSource->PushTrn(pSource->m_clientsock, pSource->Make_Cmd_CHANOPTIONS(m_ID, CHANOP_BACKGROUND, cr));
	}
}

bool CALLBACK JClient::JPageChannel::CanSend() const
{
	return m_channel.getStatus(pSource->m_idOwn) > eReader || pSource->isCheats();
}

void CALLBACK JClient::JPageChannel::setchannel(const Channel& val)
{
	m_channel = val;

	if (m_hwndPage) {
		ListView_DeleteAllItems(m_hwndList);
		BuildView();
	}
}

void CALLBACK JClient::JPageChannel::rename(DWORD idNew, const std::tstring& newname)
{
	m_ID = idNew;
	m_channel.name = newname;
}

bool CALLBACK JClient::JPageChannel::replace(DWORD idOld, DWORD idNew)
{
	bool retval = false;

	if (m_channel.isOpened(idOld)) {
		m_channel.opened.insert(idNew);
		m_channel.opened.erase(idOld);
		retval = true;

		if (m_hwndPage) {
			LVFINDINFO lvfi;
			lvfi.flags = LVFI_PARAM;
			lvfi.lParam = (LPARAM)idOld;
			int index = ListView_FindItem(m_hwndList, -1, &lvfi);
			if (index >= 0) {
				LVITEM lvi;
				lvi.mask = LVIF_PARAM;
				lvi.iItem = index;
				lvi.iSubItem = 0;
				lvi.lParam = (LPARAM)idNew;
				VERIFY(ListView_SetItem(m_hwndList, &lvi));
				VERIFY(ListView_RedrawItems(m_hwndList, index, index));
			} else AddLine(idNew);
		}
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
	case eReader:
		m_channel.reader.insert(idNew);
		m_channel.reader.erase(idOld);
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

void CALLBACK JClient::JPageChannel::Join(DWORD idWho)
{
	m_channel.opened.insert(idWho);
	if (m_channel.getStatus(idWho) == eOutsider)
		m_channel.setStatus(idWho, m_channel.nAutoStatus > eOutsider ? m_channel.nAutoStatus : eWriter);

	if (m_hwndPage) AddLine(idWho);

	// Introduction messages only fo finded user here - user can be unknown or hidden
	MapUser::const_iterator iu = pSource->m_mUser.find(idWho);
	if (iu != pSource->m_mUser.end() && iu->second.name.length()) {
		// Lua response
		if (pSource->m_luaEvents) {
			lua_getglobal(pSource->m_luaEvents, "onJoinChannel");
			lua_pushstring(pSource->m_luaEvents, tstrToANSI(getSafeName(idWho)).c_str());
			lua_pushstring(pSource->m_luaEvents, tstrToANSI(m_channel.name).c_str());
			ASSERT(lua_gettop(pSource->m_luaEvents) == 3);
			lua_call(pSource->m_luaEvents, 2, 0);
		} else {
			AppendScript(tformat(TEXT("[style=Info]joins: [b]%s[/b][/style]"), getSafeName(idWho).c_str()));
		}
	}
}

void CALLBACK JClient::JPageChannel::Part(DWORD idWho, DWORD idBy)
{
	m_channel.opened.erase(idWho);
	if (m_hwndPage) DelLine(idWho);

	// Lua response
	if (pSource->m_luaEvents) {
		lua_getglobal(pSource->m_luaEvents, "onPartChannel");
		lua_pushstring(pSource->m_luaEvents, tstrToANSI(getSafeName(idWho)).c_str());
		lua_pushstring(pSource->m_luaEvents, tstrToANSI(getSafeName(idBy)).c_str());
		lua_pushstring(pSource->m_luaEvents, tstrToANSI(m_channel.name).c_str());
		ASSERT(lua_gettop(pSource->m_luaEvents) == 4);
		lua_call(pSource->m_luaEvents, 3, 0);
	} else {
		// Parting message
		std::tstring msg;
		std::tstring who = getSafeName(idWho);
		std::tstring by = getSafeName(idBy);
		if (idWho == idBy) {
			msg = tformat(TEXT("[style=Info]parts: [b]%s[/b][/style]"), who.c_str());
		} else if (idBy == CRC_SERVER) {
			msg = tformat(TEXT("[style=Info]quits: [b]%s[/b][/style]"), who.c_str());
		} else {
			msg = tformat(TEXT("[style=Info][b]%s[/b] was kicked by [b]%s[/b][/style]"), who.c_str(), by.c_str());
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
			__super::DlgProc(hWnd, message, wParam, lParam);

			m_hwndList = GetDlgItem(hWnd, IDC_USERLIST);

			// Get initial windows sizes
			MapControl(m_hwndList, rcList);

			TOOLINFO ti;
			ti.cbSize = sizeof(ti);
			ti.uFlags = TTF_ABSOLUTE | TTF_IDISHWND | TTF_TRACK;
			ti.hwnd = pSource->hwndPage;
			ti.uId = (UINT_PTR)m_hwndList;
			ti.hinst = JClientApp::jpApp->hinstApp;
			ti.lpszText = 0;
			VERIFY(SendMessage(pSource->m_hwndBaloon, TTM_ADDTOOL, 0, (LPARAM)&ti));

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

			pSource->PushTrn(pSource->clientsock, pSource->Make_Cmd_PART(pSource->m_idOwn, m_ID));
			for each (SetId::value_type const& v in m_channel.opened) {
				pSource->UnlinkUser(v, m_ID);
			}
			pSource->EvReport(tformat(TEXT("parts from [b]#%s[/b] channel"), m_channel.name.c_str()), eInformation, eHigher);

			__super::DlgProc(hWnd, message, wParam, lParam);
			break;
		}

	case BEM_ADJUSTSIZE:
		{
			RECT rc;
			int cx = LOWORD(lParam), cy = HIWORD(lParam);
			HDWP hdwp = BeginDeferWindowPos(6);
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
				cx - rcPage.right + rcMsgSpinBlue.left,
				cy - rcPage.bottom + rcMsgSpinBlue.top,
				cx - rcPage.right + rcMsgSpinBlue.right,
				cy - rcPage.bottom + rcMsgSpinBlue.bottom);
			DeferWindowPos(hdwp, m_hwndMsgSpinBlue, 0, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, SWP_NOSIZE | SWP_NOREPOSITION | SWP_NOZORDER);
			SetRect(&rc,
				cx - rcPage.right + rcMsgSpinRed.left,
				cy - rcPage.bottom + rcMsgSpinRed.top,
				cx - rcPage.right + rcMsgSpinRed.right,
				cy - rcPage.bottom + rcMsgSpinRed.bottom);
			DeferWindowPos(hdwp, m_hwndMsgSpinRed, 0, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, SWP_NOSIZE | SWP_NOREPOSITION | SWP_NOZORDER);
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
			case IDC_CHANTOPIC:
				if (m_channel.getStatus(pSource->m_idOwn) >= (m_channel.isOpened(pSource->m_idOwn) ? eMember : eAdmin) || pSource->isGod())
					CreateDialogParam(JClientApp::jpApp->hinstApp, MAKEINTRESOURCE(IDD_TOPIC), pSource->hwndPage, (DLGPROC)JDialog::DlgProcStub, (LPARAM)(JDialog*)new JTopic(pSource, m_ID, m_channel.name, m_channel.topic));
				break;

			case IDC_CHANFOUNDER:
			case IDC_CHANADMIN:
			case IDC_CHANMODERATOR:
			case IDC_CHANMEMBER:
			case IDC_CHANWRITER:
			case IDC_CHANREADER:
			case IDC_CHANPRIVATE:
				if (m_channel.getStatus(pSource->m_idOwn) >= eAdmin || pSource->isGod()) {
					ASSERT(pSource->m_clientsock);

					static const struct {UINT idc; EChanStatus stat;} cmp[] = {
						{IDC_CHANPRIVATE, eOutsider},
						{IDC_CHANREADER, eReader},
						{IDC_CHANWRITER, eWriter},
						{IDC_CHANMEMBER, eMember},
						{IDC_CHANMODERATOR, eModerator},
						{IDC_CHANADMIN, eAdmin},
						{IDC_CHANFOUNDER, eFounder},
					};
					int i;
					for (i = 0; cmp[i].idc != LOWORD(wParam); i++) {}
					ASSERT(i < _countof(cmp));
					pSource->PushTrn(pSource->m_clientsock, pSource->Make_Cmd_CHANOPTIONS(m_ID, CHANOP_AUTOSTATUS, cmp[i].stat));
				}
				break;

			case IDC_CHANHIDDEN:
				if (m_channel.getStatus(pSource->m_idOwn) >= eAdmin || pSource->isGod()) {
					ASSERT(pSource->m_clientsock);
					pSource->PushTrn(pSource->m_clientsock, pSource->Make_Cmd_CHANOPTIONS(m_ID, CHANOP_HIDDEN, !m_channel.isHidden));
				}
				break;

			case IDC_CHANANONYMOUS:
				if (m_channel.getStatus(pSource->m_idOwn) >= eAdmin || pSource->isGod()) {
					ASSERT(pSource->m_clientsock);
					pSource->PushTrn(pSource->m_clientsock, pSource->Make_Cmd_CHANOPTIONS(m_ID, CHANOP_ANONYMOUS, !m_channel.isAnonymous));
				}
				break;

			case IDC_PRIVATETALK:
				{
					MapUser::const_iterator iu = getSelUser();
					ASSERT(iu != pSource->m_mUser.end());
					if ((!m_channel.isAnonymous && iu->second.accessibility.fCanOpenPrivate) || pSource->isGod()) {
						ASSERT(pSource->m_clientsock);
						pSource->PushTrn(pSource->m_clientsock, pSource->Make_Quest_JOIN(iu->second.name));
					} else DisplayMessage(iu->first, MAKEINTRESOURCE(IDS_MSG_PRIVATETALK), (HICON)1);
					break;
				}

			case IDC_PRIVATEMESSAGE:
				{
					MapUser::const_iterator iu = getSelUser();
					ASSERT(iu != pSource->m_mUser.end());
					if ((!m_channel.isAnonymous && iu->second.accessibility.fCanMessage) || pSource->isGod()) {
						ASSERT(pSource->m_clientsock);
						CreateDialogParam(JClientApp::jpApp->hinstApp, MAKEINTRESOURCE(IDD_MSGSEND), pSource->hwndPage, JClient::JSplashRtfEditor::DlgProcStub, (LPARAM)(JDialog*)new JClient::JMessageEditor(pSource, iu->second.name, false));
					} else DisplayMessage(iu->first, MAKEINTRESOURCE(IDS_MSG_PRIVATEMESSAGE), (HICON)1);
					break;
				}

			case IDC_ALERT:
				{
					MapUser::const_iterator iu = getSelUser();
					ASSERT(iu != pSource->m_mUser.end());
					if ((!m_channel.isAnonymous && iu->second.accessibility.fCanAlert) || pSource->isGod()) {
						ASSERT(pSource->m_clientsock);
						CreateDialogParam(JClientApp::jpApp->hinstApp, MAKEINTRESOURCE(IDD_MSGSEND), pSource->hwndPage, JClient::JSplashRtfEditor::DlgProcStub, (LPARAM)(JDialog*)new JClient::JMessageEditor(pSource, iu->second.name, true));
					} else DisplayMessage(iu->first, MAKEINTRESOURCE(IDS_MSG_ALERT), (HICON)1);
					break;
				}

			case IDS_SOUNDSIGNAL:
				{
					MapUser::const_iterator iu = getSelUser();
					ASSERT(iu != pSource->m_mUser.end());
					if (iu->second.accessibility.fCanSignal || pSource->isGod()) {
						ASSERT(pSource->m_clientsock);
						pSource->PushTrn(pSource->m_clientsock, pSource->Make_Cmd_BEEP(iu->first));
					} else DisplayMessage(iu->first, MAKEINTRESOURCE(IDS_MSG_SOUNDSIGNAL), (HICON)1);
					break;
				}

			case IDC_CLIPBOARD:
				{
					MapUser::const_iterator iu = getSelUser();
					ASSERT(iu != pSource->m_mUser.end());
					if ((pSource->m_metrics.flags.bTransmitClipboard && iu->second.accessibility.fCanRecvClipboard) || pSource->isGod()) {
						ASSERT(pSource->m_clientsock);
						pSource->PushTrn(pSource->m_clientsock, pSource->Make_Cmd_CLIPBOARD(iu->first));
					} else DisplayMessage(iu->first, MAKEINTRESOURCE(IDS_MSG_CLIPBOARD), (HICON)1);
					break;
				}

			case IDC_SPLASHRTF:
				{
					MapUser::const_iterator iu = getSelUser();
					ASSERT(iu != pSource->m_mUser.end());
					if (iu->second.accessibility.fCanSplash || pSource->isGod()) {
						ASSERT(pSource->m_clientsock);
						CreateDialogParam(JClientApp::jpApp->hinstApp, MAKEINTRESOURCE(IDD_SPLASHRTFEDITOR), pSource->hwndPage, JClient::JSplashRtfEditor::DlgProcStub, (LPARAM)(JDialog*)new JClient::JSplashRtfEditor(pSource, iu->first));
					} else DisplayMessage(iu->first, MAKEINTRESOURCE(IDS_MSG_SPLASHRTF), (HICON)1);
					break;
				}

			case IDC_RENAME:
				{
					ASSERT(pSource->m_clientsock);
					int index = ListView_GetNextItem(m_hwndList, -1, LVNI_SELECTED);
					if (index >= 0) {
						ListView_EditLabel(m_hwndList, index);
					}
					break;
				}

			case IDC_KICK:
				{
					MapUser::const_iterator iu = getSelUser();
					ASSERT(iu != pSource->m_mUser.end());

					bool isModer = m_channel.getStatus(pSource->m_idOwn) >= eModerator;
					bool canKick = m_channel.getStatus(pSource->m_idOwn) >= m_channel.getStatus(iu->first);
					if (pSource->m_idOwn == iu->first || (isModer && canKick && !pSource->isDevil(iu->first)) || pSource->isDevil()) {
						ASSERT(pSource->m_clientsock);
						pSource->PushTrn(pSource->clientsock, pSource->Make_Cmd_PART(iu->first, m_ID));
					} else DisplayMessage(iu->first, MAKEINTRESOURCE(IDS_MSG_KICK), (HICON)1);
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
					ASSERT(iu != pSource->m_mUser.end());

					EChanStatus statOwn = m_channel.getStatus(pSource->m_idOwn), statUser = m_channel.getStatus(iu->first);
					static const struct {UINT idc; EChanStatus stat;} cmp[] = {
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
					if (canModer || pSource->isGod()) {
						ASSERT(pSource->m_clientsock);
						pSource->PushTrn(pSource->m_clientsock, pSource->Make_Cmd_ACCESS(iu->first, m_ID, cmp[i].stat));
					}
					break;
				}

			default: retval = __super::DlgProc(hWnd, message, wParam, lParam);
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
					if (valid && iter->second.cheat.isGod) {
						ImageList_Draw(JClientApp::jpApp->himlMan, IML_MANGOD, lpDrawItem->hDC, rc.left, rc.top, 0);
						rc.left += 16 + sep;
					}
					if (valid && iter->second.cheat.isDevil) {
						ImageList_Draw(JClientApp::jpApp->himlMan, IML_MANDEVIL, lpDrawItem->hDC, rc.left, rc.top, 0);
						rc.left += 16 + sep;
					}
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
					DrawText(lpDrawItem->hDC, getSafeName((DWORD)lpDrawItem->itemData).c_str(), -1, &rc, DT_LEFT | DT_NOCLIP | DT_VCENTER);
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
					DrawText(lpDrawItem->hDC, valid && !m_channel.isAnonymous
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
			case LVN_BEGINLABELEDIT:
				{
					NMLVDISPINFO* pdi = (NMLVDISPINFO*)lParam;
					if (!pSource->m_clientsock) break;
					if (pnmh->idFrom == IDC_USERLIST) {
						if (pSource->isGod()) {
							JClientApp::jpApp->haccelCurrent = 0; // disable accelerators
							retval = FALSE;
						}
					}
					break;
				}

			case LVN_ENDLABELEDIT:
				{
					NMLVDISPINFO* pdi = (NMLVDISPINFO*)lParam;
					JClientApp::jpApp->haccelCurrent = JClientApp::jpApp->haccelMain; // enable accelerators
					if (pnmh->idFrom == IDC_USERLIST) {
						if (pdi->item.pszText && pSource->m_clientsock) {
							std::tstring nick = pdi->item.pszText;
							const TCHAR* msg;
							if (JClient::CheckNick(nick, msg)) {
								pSource->PushTrn(pSource->m_clientsock, pSource->Make_Cmd_NICK((DWORD)pdi->item.lParam, nick));
								retval = TRUE;
								break;
							} else {
								pSource->DisplayMessage(ListView_GetEditControl(m_hwndList), msg, MAKEINTRESOURCE(IDS_MSG_NICKERROR), 2);
							}
						}
					}
					retval = FALSE;
					break;
				}

			case NM_DBLCLK:
				{
					if (pnmh->idFrom == IDC_USERLIST) {
						if (pSource->m_clientsock) {
							SendMessage(hWnd, WM_COMMAND, IDC_PRIVATETALK, 0);
						}
					}
					break;
				}

			case LVN_ITEMCHANGED:
				{
					LPNMLISTVIEW pnmv = (LPNMLISTVIEW)lParam;
					if (pnmh->idFrom == IDC_USERLIST && pnmv->uChanged == LVIF_STATE) {
						MapUser::const_iterator iu = pSource->m_mUser.find((DWORD)pnmv->lParam);
						if (iu != pSource->m_mUser.end() && pnmv->iItem >= 0
							&& (pnmv->uNewState & LVIS_SELECTED) != 0 && (pnmv->uOldState & LVIS_SELECTED) == 0
							&& !m_channel.isAnonymous)
						{
							ASSERT(pSource->m_clientsock);

							SYSTEMTIME st;
							FileTimeToLocalTime(iu->second.ftCreation, st);
							HICON hicon = ImageList_GetIcon(JClientApp::jpApp->himlStatusImg, iu->second.nStatusImg, ILD_TRANSPARENT);
							DisplayMessage(
								iu->first,
								tformat(
								TEXT("%s%s%s\n")
								TEXT("Status text:\t\"%s\"\n")
								TEXT("IP-address:\t%i.%i.%i.%i\n")
								TEXT("Connected:\t%02i:%02i:%02i, %02i.%02i.%04i"),
								iu->second.isOnline ? TEXT("User online") : TEXT("User offline"),
								iu->second.cheat.isGod ? TEXT(", god mode") : TEXT(""),
								iu->second.cheat.isDevil ? TEXT(", devil mode") : TEXT(""),
								iu->second.strStatus.c_str(),
								iu->second.IP.S_un.S_un_b.s_b1,
								iu->second.IP.S_un.S_un_b.s_b2,
								iu->second.IP.S_un.S_un_b.s_b3,
								iu->second.IP.S_un.S_un_b.s_b4,
								st.wHour, st.wMinute, st.wSecond,
								st.wDay, st.wMonth, st.wYear).c_str(),
								hicon,
								(DWORD)pnmv->lParam != pSource->m_idOwn
								? RGB(0xC0, 0x00, 0x00)
								: RGB(0x00, 0x00, 0xC0));
								DestroyIcon(hicon);
						}
					}
					break;
				}

			case EN_MSGFILTER:
				{
					MSGFILTER* pmf = (MSGFILTER*)lParam;
					if (pnmh->idFrom == IDC_LOG) {
						if (pmf->msg == WM_LBUTTONDBLCLK) {
							if (m_channel.getStatus(pSource->m_idOwn) >= (m_channel.isOpened(pSource->m_idOwn) ? eMember : eAdmin))
								SendMessage(hWnd, WM_COMMAND, IDC_CHANTOPIC, 0);
						}
					}
					break;
				}

			default: retval = __super::DlgProc(hWnd, message, wParam, lParam);
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
			} else if ((HWND)wParam == m_hwndList
				&& ListView_GetSelectedCount(m_hwndList)) {
				RECT r;
				GetWindowRect((HWND)wParam, &r);
				TrackPopupMenu(GetSubMenu(pSource->isGod() ? JClientApp::jpApp->hmenuUserGod : JClientApp::jpApp->hmenuUser, 0), TPM_LEFTALIGN | TPM_RIGHTBUTTON,
					min(max(GET_X_LPARAM(lParam), r.left), r.right),
					min(max(GET_Y_LPARAM(lParam), r.top), r.bottom), 0, hWnd, 0);
			} else {
				retval = __super::DlgProc(hWnd, message, wParam, lParam);
			}
			break;
		}

	case WM_INITMENUPOPUP:
		{
			if ((HMENU)wParam == GetSubMenu(JClientApp::jpApp->hmenuChannel, 0)) {
				bool canAdmin = m_channel.getStatus(pSource->m_idOwn) >= eAdmin || pSource->isGod();
				bool canMember = m_channel.getStatus(pSource->m_idOwn) >= (m_channel.isOpened(pSource->m_idOwn) ? eMember : eAdmin) || pSource->isGod();
				VERIFY(SetMenuDefaultItem((HMENU)wParam, IDC_CHANTOPIC, FALSE));
				EnableMenuItem((HMENU)wParam, IDC_CHANTOPIC,
					MF_BYCOMMAND | (canMember ? MF_ENABLED : MF_GRAYED));
				EnableMenuItem((HMENU)wParam, IDC_CHANHIDDEN,
					MF_BYCOMMAND | (canAdmin ? MF_ENABLED : MF_GRAYED));
				EnableMenuItem((HMENU)wParam, IDC_CHANANONYMOUS,
					MF_BYCOMMAND | (canAdmin ? MF_ENABLED : MF_GRAYED));
				CheckMenuItem((HMENU)wParam, IDC_CHANHIDDEN,
					MF_BYCOMMAND | (m_channel.isHidden ? MF_CHECKED : MF_UNCHECKED));
				CheckMenuItem((HMENU)wParam, IDC_CHANANONYMOUS,
					MF_BYCOMMAND | (m_channel.isAnonymous ? MF_CHECKED : MF_UNCHECKED));

				CHARRANGE cr;
				SendMessage(m_hwndLog, EM_EXGETSEL, 0, (LPARAM)&cr);
				bool cancopy = cr.cpMin != cr.cpMax;
				EnableMenuItem((HMENU)wParam, IDC_LOGCOPY,
					MF_BYCOMMAND | (cancopy ? MF_ENABLED : MF_GRAYED));
				break;
			} else if ((HMENU)wParam == GetSubMenu(GetSubMenu(JClientApp::jpApp->hmenuChannel, 0), 1)) {
				bool canAdmin = m_channel.getStatus(pSource->m_idOwn) >= eAdmin || pSource->isGod();
				EnableMenuItem((HMENU)wParam, IDC_CHANFOUNDER,
					MF_BYCOMMAND | (canAdmin ? MF_ENABLED : MF_GRAYED));
				EnableMenuItem((HMENU)wParam, IDC_CHANADMIN,
					MF_BYCOMMAND | (canAdmin ? MF_ENABLED : MF_GRAYED));
				EnableMenuItem((HMENU)wParam, IDC_CHANMODERATOR,
					MF_BYCOMMAND | (canAdmin ? MF_ENABLED : MF_GRAYED));
				EnableMenuItem((HMENU)wParam, IDC_CHANMEMBER,
					MF_BYCOMMAND | (canAdmin ? MF_ENABLED : MF_GRAYED));
				EnableMenuItem((HMENU)wParam, IDC_CHANWRITER,
					MF_BYCOMMAND | (canAdmin ? MF_ENABLED : MF_GRAYED));
				EnableMenuItem((HMENU)wParam, IDC_CHANREADER,
					MF_BYCOMMAND | (canAdmin ? MF_ENABLED : MF_GRAYED));
				EnableMenuItem((HMENU)wParam, IDC_CHANPRIVATE,
					MF_BYCOMMAND | (canAdmin ? MF_ENABLED : MF_GRAYED));
				CheckMenuRadioItem((HMENU)wParam, eOutsider, eFounder, eFounder - m_channel.nAutoStatus, MF_BYPOSITION);
			} else if ((HMENU)wParam == GetSubMenu(JClientApp::jpApp->hmenuUser, 0)
				|| (HMENU)wParam == GetSubMenu(JClientApp::jpApp->hmenuUserGod, 0)) {
				MapUser::const_iterator iu = getSelUser();
				bool valid = iu != pSource->m_mUser.end();

				pSource->HideBaloon(m_hwndList);

				VERIFY(SetMenuDefaultItem((HMENU)wParam, IDC_PRIVATETALK, FALSE));
				EnableMenuItem((HMENU)wParam, IDC_PRIVATETALK,
					MF_BYCOMMAND | (valid && ((!m_channel.isAnonymous && iu->second.accessibility.fCanOpenPrivate) || pSource->isGod()) ? MF_ENABLED : MF_GRAYED));
				EnableMenuItem((HMENU)wParam, IDC_PRIVATEMESSAGE,
					MF_BYCOMMAND | (valid && ((!m_channel.isAnonymous && iu->second.accessibility.fCanMessage) || pSource->isGod()) ? MF_ENABLED : MF_GRAYED));
				EnableMenuItem((HMENU)wParam, IDC_ALERT,
					MF_BYCOMMAND | (valid && ((!m_channel.isAnonymous && iu->second.accessibility.fCanAlert) || pSource->isGod()) ? MF_ENABLED : MF_GRAYED));
				EnableMenuItem((HMENU)wParam, IDS_SOUNDSIGNAL,
					MF_BYCOMMAND | (valid && (iu->second.accessibility.fCanSignal || pSource->isGod()) ? MF_ENABLED : MF_GRAYED));
				EnableMenuItem((HMENU)wParam, IDC_CLIPBOARD,
					MF_BYCOMMAND | (valid && ((iu->second.accessibility.fCanRecvClipboard && pSource->m_metrics.flags.bTransmitClipboard) || pSource->isGod()) ? MF_ENABLED : MF_GRAYED));
				EnableMenuItem((HMENU)wParam, IDC_SPLASHRTF,
					MF_BYCOMMAND | (valid && (iu->second.accessibility.fCanSplash || pSource->isGod()) ? MF_ENABLED : MF_GRAYED));

				EnableMenuItem((HMENU)wParam, IDC_RENAME,
					MF_BYCOMMAND | (valid && pSource->isGod() ? MF_ENABLED : MF_GRAYED));

				bool isModer = m_channel.getStatus(pSource->m_idOwn) >= eModerator;
				bool canKick = m_channel.getStatus(pSource->m_idOwn) >= m_channel.getStatus(iu->first);
				EnableMenuItem((HMENU)wParam, IDC_KICK,
					MF_BYCOMMAND | (valid && (iu->first == pSource->m_idOwn || (isModer && canKick && !pSource->isDevil(iu->first)) || pSource->isDevil()) ? MF_ENABLED : MF_GRAYED));
			} else if ((HMENU)wParam == GetSubMenu(GetSubMenu(JClientApp::jpApp->hmenuUser, 0), 8)) {
				MapUser::const_iterator iu = getSelUser();
				bool valid = iu != pSource->m_mUser.end();

				if (valid) {
					for (int i = eOutsider; i <= eFounder; i++) {
						EChanStatus statOwn = m_channel.getStatus(pSource->m_idOwn), statUser = m_channel.getStatus(iu->first);
						bool canModer =
							(statOwn > statUser || statOwn >= eModerator || iu->first == pSource->m_idOwn) &&
							(statOwn > eFounder - i || statOwn == eFounder);
						CheckMenuRadioItem((HMENU)wParam, eOutsider, eFounder, eFounder - statUser, MF_BYPOSITION);
						EnableMenuItem((HMENU)wParam, IDC_FOUNDER + i,
							MF_BYCOMMAND | (canModer || pSource->isGod()? MF_ENABLED : MF_GRAYED));
					}
				} else {
					for (int i = eOutsider; i <= eFounder; i++) {
						EnableMenuItem((HMENU)wParam, IDC_FOUNDER + eFounder - i, MF_BYCOMMAND | MF_GRAYED);
					}
				}
			} else {
				__super::DlgProc(hWnd, message, wParam, lParam);
			}
			break;
		}

	default: retval = __super::DlgProc(hWnd, message, wParam, lParam);
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

	pSource->EvLinkStart += MakeDelegate(this, &JClient::JPageChannel::OnLinkStart);
	pSource->EvLinkClose += MakeDelegate(this, &JClient::JPageChannel::OnLinkClose);
	pSource->EvTopic += MakeDelegate(this, &JClient::JPageChannel::OnTopic);
	pSource->EvNick += MakeDelegate(this, &JClient::JPageChannel::OnNick);
}

void JClient::JPageChannel::OnUnhook(JEventable* src)
{
	ASSERT(pSource);
	using namespace fastdelegate;

	pSource->EvLinkStart -= MakeDelegate(this, &JClient::JPageChannel::OnLinkStart);
	pSource->EvLinkClose -= MakeDelegate(this, &JClient::JPageChannel::OnLinkClose);
	pSource->EvTopic -= MakeDelegate(this, &JClient::JPageChannel::OnTopic);
	pSource->EvNick -= MakeDelegate(this, &JClient::JPageChannel::OnNick);

	__super::OnUnhook(src);
}

void JClient::JPageChannel::OnLinkStart(SOCKET sock)
{
	ASSERT(pSource);
	if (m_hwndPage) {
		pSource->PushTrn(pSource->m_clientsock, pSource->Make_Quest_JOIN(m_channel.name, m_channel.password, gettype()));
	}
}

void JClient::JPageChannel::OnLinkClose(SOCKET sock, UINT err)
{
	ASSERT(pSource);
	if (m_hwndPage) {
		AppendScript(TEXT("[style=Info]Disconnected[/style]"));
	}
	m_channel.opened.clear(); // no users on disconnected channel
}

void JClient::JPageChannel::OnNick(DWORD idOld, const std::tstring& oldname, DWORD idNew, const std::tstring& newname)
{
	ASSERT(pSource);
}

void JClient::JPageChannel::OnTopic(DWORD idWho, DWORD idWhere, const std::tstring& topic)
{
	ASSERT(pSource);
	if (idWhere == m_ID) {
		m_channel.topic = topic;
		m_channel.idTopicWriter = idWho;
		if (m_hwndPage) {
			setAlert(eRed);
		}
	}
}

//-----------------------------------------------------------------------------

// The End.