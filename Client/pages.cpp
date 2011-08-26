
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

//-----------------------------------------------------------------------------

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

JClient::JPage::JPage()
: JDialog()
{
	m_alert = eGreen;
}

std::tstring JClient::JPage::getSafeName(DWORD idUser) const
{
	return pNode->getSafeName(idUser);
}

void JClient::JPage::activate()
{
	if (m_alert > eGreen) {
		setAlert(eGreen);
	}
}

void JClient::JPage::setAlert(EAlert a)
{
	if ((a > m_alert || a == eGreen)
		&& (pNode->jpOnline != this || !pNode->m_mUser[pNode->m_idOwn].isOnline)
		&& pNode->hwndPage)
	{
		m_alert = a;

		TCITEM tci;
		tci.mask = TCIF_IMAGE;
		tci.iImage = ImageIndex();
		VERIFY(TabCtrl_SetItem(pNode->m_hwndTab, pNode->getTabIndex(getID()), &tci));
	}
}

void JClient::JPage::OnHook(JNode* src)
{
	using namespace fastdelegate;

	__super::OnHook(src);

	JNODE(JClient, node, src);
	if (node) {
		jpLuaVM = node->jpLuaVM;
		node->EvLinkStart += MakeDelegate(this, &JClient::JPage::OnLinkStart);
		node->EvLinkClose += MakeDelegate(this, &JClient::JPage::OnLinkClose);
	}
}

void JClient::JPage::OnUnhook(JNode* src)
{
	using namespace fastdelegate;

	JNODE(JClient, node, src);
	if (node) {
		node->EvLinkStart -= MakeDelegate(this, &JClient::JPage::OnLinkStart);
		node->EvLinkClose -= MakeDelegate(this, &JClient::JPage::OnLinkClose);
		jpLuaVM = 0;
	}

	__super::OnUnhook(src);
}

void JClient::JPage::OnLinkStart(SOCKET sock)
{
	if (m_hwndPage) {
		Enable();
	}
}

void JClient::JPage::OnLinkClose(SOCKET sock, UINT err)
{
	if (m_hwndPage) {
		Disable();
	}
}

//-----------------------------------------------------------------------------

//
// JPageLog
//

JClient::JPageLog::JPageLog()
: JPage()
{
}

std::string JClient::JPageLog::AppendRtf(const std::string& content) const
{
	CHARRANGE crMark, crIns; // Selection position
	int nTextLen; // Length of text in control
	std::string plain;

	SendMessage(m_hwndLog, EM_HIDESELECTION, TRUE, 0);

	SendMessage(m_hwndLog, EM_EXGETSEL, 0, (LPARAM)&crMark);
	nTextLen = GetWindowTextLength(m_hwndLog);

	SendMessage(m_hwndLog, EM_SETSEL, -1, -1);
	SendMessage(m_hwndLog, EM_EXGETSEL, 0, (LPARAM)&crIns);
	SendMessage(m_hwndLog, EM_REPLACESEL, FALSE, (LPARAM)ANSIToTstr(content).c_str());

	crIns.cpMax = -1;
	SendMessage(m_hwndLog, EM_EXSETSEL, 0, (LPARAM)&crIns);
	SendMessage(m_hwndLog, EM_EXGETSEL, 0, (LPARAM)&crIns);
	plain.resize(crIns.cpMax - crIns.cpMin);
	SendMessageA(m_hwndLog, EM_GETSELTEXT, 0, (LPARAM)plain.data());

	// Restore caret position
	if (crMark.cpMin == crMark.cpMax && crMark.cpMin == nTextLen) SendMessage(m_hwndLog, EM_SETSEL, -1, -1);
	else SendMessage(m_hwndLog, EM_EXSETSEL, 0, (LPARAM)&crMark);

	SendMessage(m_hwndLog, EM_HIDESELECTION, FALSE, 0);

	if (GetFocus() != m_hwndLog) SendMessage(m_hwndLog, WM_VSCROLL, SB_BOTTOM, 0);
	return plain;
}

void JClient::JPageLog::AppendScript(const std::tstring& content) const
{
	CHARRANGE crMark; // Selection position
	int nTextLen; // Length of text in control

	// Save caret position
	SendMessage(m_hwndLog, EM_EXGETSEL, 0, (LPARAM)&crMark);
	nTextLen = GetWindowTextLength(m_hwndLog);
	// Write whole string to log
	Write(m_hwndLog, content.c_str()) && Write(m_hwndLog, TEXT("\r\n"));
	// Restore caret position
	if (crMark.cpMin == crMark.cpMax && crMark.cpMin == nTextLen) SendMessage(m_hwndLog, EM_SETSEL, -1, -1);
	else SendMessage(m_hwndLog, EM_EXSETSEL, 0, (LPARAM)&crMark);

	if (GetFocus() != m_hwndLog) SendMessage(m_hwndLog, WM_VSCROLL, SB_BOTTOM, 0);
}

void JClient::JPageLog::Say(DWORD idWho, std::string& content)
{
	// Lua response
	DOLUACS;
	pNode->lua_getmethod(L, "onSay");
	if (lua_isfunction(L, -1)) {
		lua_insert(L, -2);
		lua_pushstring(L, tstrToANSI(getSafeName(idWho)).c_str());
		lua_pushstring(L, tstrToANSI(getname()).c_str());
		lua_pushstring(L, content.c_str());
		lua_call(L, 4, 0);
	} else lua_pop(L, 2);
}

LRESULT WINAPI JClient::JPageLog::DlgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	LRESULT retval = TRUE;
	switch (message)
	{
	case WM_INITDIALOG:
		{
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

JClient::JPageServer::JPageServer()
: JPageLog()
{
}

void JClient::JPageServer::Enable()
{
	EnableWindow(m_hwndHost, FALSE);
	EnableWindow(m_hwndPort, FALSE);
	EnableWindow(m_hwndPass, FALSE);
	EnableWindow(m_hwndNick, TRUE);
	m_fEnabled = true;
}

void JClient::JPageServer::Disable()
{
	EnableWindow(m_hwndHost, TRUE);
	EnableWindow(m_hwndPort, TRUE);
	EnableWindow(m_hwndPass, TRUE);
	EnableWindow(m_hwndNick, TRUE);
	m_fEnabled = false;
}

int JClient::JPageServer::ImageIndex() const
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
			SetWindowTextA(m_hwndHost, pNode->m_hostname.c_str());
			SendMessage(m_hwndHost, CB_LIMITTEXT, 128, 0);
			m_hostlist.clear();
			int count = profile::getInt(RF_HOSTLIST, RK_HOSTCOUNT, 0);
			for ( int i = 0; i < count; i++) {
				std::string str = tstrToANSI(profile::getString(RF_HOSTLIST, tformat(TEXT("%02i"), i)));
				m_hostlist.insert(str);
				SendMessageA(m_hwndHost, CB_ADDSTRING, 0, (LPARAM)str.c_str());
			}
			// Init Port control
			SetWindowText(m_hwndPort, tformat(TEXT("%u"), pNode->m_port).c_str());
			SendMessage(m_hwndPort, EM_LIMITTEXT, 5, 0);
			// Init Nick control
			SetWindowText(m_hwndNick, pNode->m_mUser[pNode->m_idOwn].name.c_str());
			// Init status combobox
			COMBOBOXEXITEM cbei[] = {
				{CBEIF_IMAGE | CBEIF_SELECTEDIMAGE | CBEIF_TEXT, eReady, (LPWSTR)s_mapUserStatName[eReady].c_str(), -1, 0, 0, 0, 0, 0},
				{CBEIF_IMAGE | CBEIF_SELECTEDIMAGE | CBEIF_TEXT, eDND, (LPWSTR)s_mapUserStatName[eDND].c_str(), -1, 1, 1, 0, 0, 0},
				{CBEIF_IMAGE | CBEIF_SELECTEDIMAGE | CBEIF_TEXT, eBusy, (LPWSTR)s_mapUserStatName[eBusy].c_str(), -1, 2, 2, 0, 0, 0},
				{CBEIF_IMAGE | CBEIF_SELECTEDIMAGE | CBEIF_TEXT, eNA, (LPWSTR)s_mapUserStatName[eNA].c_str(), -1, 3, 3, 0, 0, 0},
				{CBEIF_IMAGE | CBEIF_SELECTEDIMAGE | CBEIF_TEXT, eAway, (LPWSTR)s_mapUserStatName[eAway].c_str(), -1, 4, 4, 0, 0, 0},
				{CBEIF_IMAGE | CBEIF_SELECTEDIMAGE | CBEIF_TEXT, eInvisible, (LPWSTR)s_mapUserStatName[eInvisible].c_str(), -1, 5, 5, 0, 0, 0},
			};
			for (int i = 0; i < _countof(cbei); i++)
				SendMessage(m_hwndStatus, CBEM_INSERTITEM, 0, (LPARAM)&cbei[i]);
			SendMessage(m_hwndStatus, CBEM_SETIMAGELIST, 0, (LPARAM)JClientApp::jpApp->himlStatus);
			SendMessage(m_hwndStatus, CB_SETCURSEL, pNode->m_mUser[pNode->m_idOwn].nStatus, 0);
			// Init status images combobox
			COMBOBOXEXITEM cbeiImg;
			cbeiImg.mask = CBEIF_IMAGE | CBEIF_SELECTEDIMAGE;
			for (int i = 0; i < IML_STATUSIMG_COUNT; i++) {
				cbeiImg.iItem = i;
				cbeiImg.iImage = i = cbeiImg.iSelectedImage = i;
				SendMessage(m_hwndStatusImg, CBEM_INSERTITEM, 0, (LPARAM)&cbeiImg);
			}
			SendMessage(m_hwndStatusImg, CBEM_SETIMAGELIST, 0, (LPARAM)JClientApp::jpApp->himlStatusImg);
			SendMessage(m_hwndStatusImg, CB_SETCURSEL, pNode->m_mUser[pNode->m_idOwn].nStatusImg, 0);
			// Init Status message control
			SetWindowText(m_hwndStatusMsg, pNode->m_mUser[pNode->m_idOwn].strStatus.c_str());

			OnMetrics(pNode->m_metrics);

			// Set introduction comments
			static const TCHAR* szIntro =
				TEXT("[color=red,size=16,b]Colibri Chat[/color] [color=purple]v") APPVER TEXT("[/color,/size,/b]\n")
				TEXT("[b]© Podobashev Dmitry / BEOWOLF[/b], ") APPDATE TEXT(".\r\n");
			Write(m_hwndLog, szIntro);

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
					DOLUACS;
					pNode->lua_getmethod(L, "idcConnect");
					if (lua_isfunction(L, -1)) {
						lua_insert(L, -2);
						lua_call(L, 1, 0);
					} else lua_pop(L, 2);
					break;
				}

			case IDC_PASS:
				{
					CreateDialogParam(JClientApp::jpApp->hinstApp, MAKEINTRESOURCE(IDD_CLIENTPASS), pNode->hwndPage, (DLGPROC)JDialog::DlgProcStub, (LPARAM)(JDialog*)new JPassword(pNode));
					break;
				}

			case IDC_STATUS:
				{
					switch (HIWORD(wParam))
					{
					case CBN_SELENDOK:
						{
							EUserStatus stat = (EUserStatus)SendMessage((HWND)lParam, CB_GETCURSEL, 0, 0);
							pNode->PushTrn(pNode->m_clientsock, pNode->Make_Cmd_STATUS_Mode(stat, pNode->m_mAlert[stat]));
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
							pNode->PushTrn(pNode->m_clientsock, pNode->Make_Cmd_STATUS_Img(img));
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

void JClient::JPageServer::OnHook(JNode* src)
{
	using namespace fastdelegate;

	__super::OnHook(src);

	JNODE(JClient, node, src);
	if (node) {
		node->EvLinkStart += MakeDelegate(this, &JClient::JPageServer::OnLinkStart);
		node->EvLog += MakeDelegate(this, &JClient::JPageServer::OnLog);
		node->EvMetrics += MakeDelegate(this, &JClient::JPageServer::OnMetrics);
	}
}

void JClient::JPageServer::OnUnhook(JNode* src)
{
	using namespace fastdelegate;

	JNODE(JClient, node, src);
	if (node) {
		node->EvLinkStart -= MakeDelegate(this, &JClient::JPageServer::OnLinkStart);
		node->EvLog -= MakeDelegate(this, &JClient::JPageServer::OnLog);
		node->EvMetrics -= MakeDelegate(this, &JClient::JPageServer::OnMetrics);
	}

	__super::OnUnhook(src);
}

void JClient::JPageServer::OnLinkStart(SOCKET sock)
{
	if (m_hostlist.find(pNode->m_hostname) == m_hostlist.end()) {
		m_hostlist.insert(pNode->m_hostname);
		int count = profile::getInt(RF_HOSTLIST, RK_HOSTCOUNT, 0);
		profile::setInt(RF_HOSTLIST, RK_HOSTCOUNT, count + 1);
		profile::setString(RF_HOSTLIST, tformat(TEXT("%02i"), count), ANSIToTstr(pNode->m_hostname));
		if (m_hwndPage) SendMessageA(m_hwndHost, CB_ADDSTRING, 0, (LPARAM)pNode->m_hostname.c_str());
	}
}

void JClient::JPageServer::OnLog(const std::string& str, ELog elog)
{
	// Call Lua event
	DOLUACS;
	pNode->lua_getmethod(L, "onLog");
	if (lua_isfunction(L, -1)) {
		lua_insert(L, -2);
		lua_pushstring(L, str.c_str());
		lua_pushinteger(L, elog);
		lua_call(L, 3, 0);
	} else lua_pop(L, 2);
}

void JClient::JPageServer::OnMetrics(const Metrics& metrics)
{
	if (!m_hwndPage) return; // ignore if window closed

	SendMessage(m_hwndPass, EM_LIMITTEXT, metrics.uPassMaxLength, 0);
	SendMessage(m_hwndNick, EM_LIMITTEXT, metrics.uNameMaxLength, 0);
	SendMessage(m_hwndStatusMsg, EM_LIMITTEXT, metrics.uStatusMsgMaxLength, 0);
}

//-----------------------------------------------------------------------------

//
// JPageList
//

JClient::JPageList::JPageList()
: JPage()
{
}

void JClient::JPageList::Enable()
{
	EnableWindow(m_hwndChan, TRUE);
	EnableWindow(m_hwndPass, TRUE);
	EnableWindow(GetDlgItem(m_hwndPage, IDC_JOIN), TRUE);
	EnableWindow(GetDlgItem(m_hwndPage, IDC_REFRESHLIST), TRUE);
	m_fEnabled = true;
}

void JClient::JPageList::Disable()
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

			OnMetrics(pNode->m_metrics);

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
					ASSERT(pNode->m_clientsock);
					std::tstring chanbuf(pNode->m_metrics.uNameMaxLength, 0), passbuf(pNode->m_metrics.uPassMaxLength, 0);
					std::tstring chan, pass;
					GetWindowText(m_hwndChan, &chanbuf[0], (int)chanbuf.size()+1);
					GetWindowText(m_hwndPass, &passbuf[0], (int)passbuf.size()+1);
					const TCHAR* msg;
					chan = chanbuf.c_str(), pass = passbuf.c_str();
					if (JClient::CheckNick(chan, msg)) { // check content
						// send only c-strings, not buffer!
						pNode->PushTrn(pNode->m_clientsock, pNode->Make_Quest_JOIN(chan, pass));
					} else {
						pNode->BaloonShow(m_hwndChan, msg, MAKEINTRESOURCE(IDS_MSG_NICKERROR), 2);
					}
					break;
				}

			case IDC_RENAME:
				{
					ASSERT(pNode->m_clientsock);
					int index = ListView_GetNextItem(m_hwndList, -1, LVNI_SELECTED);
					if (index >= 0) {
						ListView_EditLabel(m_hwndList, index);
					}
					break;
				}

			case IDC_TOPIC:
				{
					ASSERT(pNode->m_clientsock);
					MapChannel::const_iterator ic = getSelChannel();
					ASSERT(ic != m_mChannel.end());
					if (ic->second.getStatus(pNode->m_idOwn) >= (ic->second.isOpened(pNode->m_idOwn) ? eMember : eAdmin) || pNode->isGod()) {
						CreateDialogParam(
							JClientApp::jpApp->hinstApp,
							MAKEINTRESOURCE(IDD_TOPIC),
							pNode->hwndPage,
							(DLGPROC)JDialog::DlgProcStub,
							(LPARAM)(JDialog*)new JTopic(pNode, ic->first, ic->second.name, ic->second.topic));
					}
					break;
				}

			case IDC_REFRESHLIST:
				{
					ASSERT(pNode->m_clientsock);
					pNode->PushTrn(pNode->m_clientsock, Make_Quest_LIST());
					break;
				}

			default: retval = __super::DlgProc(hWnd, message, wParam, lParam);
			}
			break;
		}

	case WM_NOTIFY:
		{
			if (!pNode) break;

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
					if (!pNode->m_clientsock) break;
					if (pnmh->idFrom == IDC_CHANLIST) {
						MapChannel::const_iterator ic = m_mChannel.find((DWORD)pdi->item.lParam);
						ASSERT(ic != m_mChannel.end());
						if (ic->second.getStatus(pNode->m_idOwn) == eFounder || pNode->isGod()) {
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
						if (pdi->item.pszText && pNode->m_clientsock) {
							std::tstring nick = pdi->item.pszText;
							const TCHAR* msg;
							if (JClient::CheckNick(nick, msg)) {
								pNode->PushTrn(pNode->m_clientsock, pNode->Make_Cmd_NICK((DWORD)pdi->item.lParam, nick));
								SetWindowText(m_hwndChan, nick.c_str());
								retval = TRUE;
								break;
							} else {
								pNode->BaloonShow(ListView_GetEditControl(m_hwndList), msg, MAKEINTRESOURCE(IDS_MSG_NICKERROR), 2);
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
						if (lpnmitem->iItem >= 0 && pNode->m_clientsock) {
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
				&& pNode->m_clientsock && ListView_GetSelectedCount(m_hwndList)) {
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
					MF_BYCOMMAND | (ic->second.getStatus(pNode->m_idOwn) == eFounder || pNode->isGod() ? MF_ENABLED : MF_GRAYED));
				EnableMenuItem((HMENU)wParam, IDC_TOPIC,
					MF_BYCOMMAND | (ic->second.getStatus(pNode->m_idOwn) >= (ic->second.isOpened(pNode->m_idOwn) ? eMember : eAdmin) || pNode->isGod() ? MF_ENABLED : MF_GRAYED));
			} else {
				__super::DlgProc(hWnd, message, wParam, lParam);
			}
			break;
		}

	default: retval = __super::DlgProc(hWnd, message, wParam, lParam);
	}
	return retval;
}

void JClient::JPageList::BuildView()
{
	for each (MapChannel::value_type const& v in m_mChannel)
		AddLine(v.first);
}

void JClient::JPageList::ClearView()
{
	ListView_DeleteAllItems(m_hwndList);
}

int JClient::JPageList::AddLine(DWORD id)
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
		switch (iter->second.getStatus(pNode->m_idOwn))
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

void JClient::JPageList::OnHook(JNode* src)
{
	using namespace fastdelegate;

	__super::OnHook(src);

	JNODE(JClient, node, src);
	if (node) {
		node->EvLinkStart += MakeDelegate(this, &JClient::JPageList::OnLinkStart);
		node->EvMetrics += MakeDelegate(this, &JClient::JPageList::OnMetrics);
		node->EvTopic += MakeDelegate(this, &JClient::JPageList::OnTopic);
		node->EvNick += MakeDelegate(this, &JClient::JPageList::OnNick);

		// Transactions parsers
		node->m_mTrnReply[CCPM_LIST] += fastdelegate::MakeDelegate(this, &JClient::JPageList::Recv_Reply_LIST);
	}
}

void JClient::JPageList::OnUnhook(JNode* src)
{
	using namespace fastdelegate;

	JNODE(JClient, node, src);
	if (node) {
		// Transactions parsers
		node->m_mTrnReply[CCPM_LIST] -= fastdelegate::MakeDelegate(this, &JClient::JPageList::Recv_Reply_LIST);

		node->EvLinkStart -= MakeDelegate(this, &JClient::JPageList::OnLinkStart);
		node->EvMetrics -= MakeDelegate(this, &JClient::JPageList::OnMetrics);
		node->EvTopic -= MakeDelegate(this, &JClient::JPageList::OnTopic);
		node->EvNick -= MakeDelegate(this, &JClient::JPageList::OnNick);
	}

	__super::OnUnhook(src);
}

void JClient::JPageList::OnLinkStart(SOCKET sock)
{
	pNode->PushTrn(sock, Make_Quest_LIST());
}

void JClient::JPageList::OnMetrics(const Metrics& metrics)
{
	if (!m_hwndPage) return; // ignore if window closed

	SendMessage(m_hwndChan, EM_LIMITTEXT, metrics.uNameMaxLength, 0);
	SendMessage(m_hwndPass, EM_LIMITTEXT, metrics.uPassMaxLength, 0);
}

void JClient::JPageList::OnTopic(DWORD idWho, DWORD idWhere, const std::tstring& topic)
{
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
			pNode->EvLog(SZ_BADTRN, elogItrn);
			return;
		}
	}

	if (m_hwndPage) {
		ClearView();
		BuildView();
	}
	pNode->EvLog(format("listed [b]%u[/b] channels", m_mChannel.size()), elogInfo);
}

JPtr<JBTransaction> JClient::JPageList::Make_Quest_LIST() const
{
	std::ostringstream os;
	return pNode->MakeTrn(QUEST(CCPM_LIST), 0, os.str());
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

JClient::JPageChat::JPageChat(DWORD id)
: JPageLog(), rtf::Editor()
{
	m_ID = id;
}

void JClient::JPageChat::Say(DWORD idWho, std::string& content)
{
	__super::Say(idWho, content);

	if (idWho == pNode->m_idOwn) { // Blue spin
		vecMsgSpinBlue.insert(vecMsgSpinBlue.begin(), content);
		if (vecMsgSpinBlue.size() > pNode->m_metrics.nMsgSpinMaxCount)
			vecMsgSpinBlue.erase(vecMsgSpinBlue.begin() + pNode->m_metrics.nMsgSpinMaxCount, vecMsgSpinBlue.end());
		SendMessage(m_hwndMsgSpinBlue, UDM_SETRANGE, 0, MAKELONG(vecMsgSpinBlue.size(), 0));
		InvalidateRect(m_hwndMsgSpinBlue, 0, TRUE);
		LPARAM pos = SendMessage(m_hwndMsgSpinBlue, UDM_GETPOS, 0, 0);
		if (LOWORD(pos) && !HIWORD(pos)) {
			pos = min(pos + 1, (LPARAM)vecMsgSpinBlue.size());
			SendMessage(m_hwndMsgSpinBlue, UDM_SETPOS, 0, MAKELONG(pos, 0));
		}
	} else { // Red spin
		vecMsgSpinRed.insert(vecMsgSpinRed.begin(), content);
		if (vecMsgSpinRed.size() > pNode->m_metrics.nMsgSpinMaxCount)
			vecMsgSpinRed.erase(vecMsgSpinRed.begin(), vecMsgSpinRed.begin() + vecMsgSpinRed.size() - pNode->m_metrics.nMsgSpinMaxCount);
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
			pNode->EvPageClose.Invoke(m_ID);

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
					if (pNode->m_clientsock) { // send content
						if (GetWindowTextLength(m_hwndEdit)) {
							std::string content;
							getContent(content, SF_RTF);
							if (content.size() < pNode->m_metrics.uChatLineMaxVolume) {
								if (CanSend()) {
									pNode->PushTrn(pNode->m_clientsock, pNode->Make_Cmd_SAY(getID(), SF_RTF, content));
									SetWindowText(m_hwndEdit, TEXT(""));
									SendMessage(m_hwndMsgSpinBlue, UDM_SETPOS, 0, MAKELONG(0, 0));
									SendMessage(m_hwndMsgSpinRed, UDM_SETPOS, 0, MAKELONG(0, 0));
								} else pNode->BaloonShow(m_hwndEdit, MAKEINTRESOURCE(IDS_MSG_READER), MAKEINTRESOURCE(IDS_MSG_EDITOR), 1);
							} else pNode->BaloonShow(m_hwndEdit, MAKEINTRESOURCE(IDS_MSG_LIMITCHATLINE), MAKEINTRESOURCE(IDS_MSG_EDITOR), 2);
						}
					} else { // connect
						pNode->Connect(true);
					}
					break;
				}

			case IDC_SENDBYENTER:
				pNode->m_bSendByEnter = true;
				break;

			case IDC_SENDBYCTRLENTER:
				pNode->m_bSendByEnter = false;
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
			if (!pNode) break;

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
							if (profile::getInt(RF_CLIENT, RK_QUOTATIONBLUE, FALSE)) {
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
							if (profile::getInt(RF_CLIENT, RK_QUOTATIONRED, TRUE)) {
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
					pNode->bSendByEnter ? IDC_SENDBYENTER : IDC_SENDBYCTRLENTER,
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

JClient::JPageUser::JPageUser(DWORD id, const std::tstring& nick)
: JPageChat(id)
{
	m_user.name = nick;
}

void JClient::JPageUser::Enable()
{
	EnableWindow(m_hwndEdit, TRUE);
	m_fEnabled = true;
}

void JClient::JPageUser::Disable()
{
	EnableWindow(m_hwndEdit, FALSE);
	m_fEnabled = false;
}

int JClient::JPageUser::ImageIndex() const
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

void JClient::JPageUser::OnSheetColor(COLORREF cr)
{
	__super::OnSheetColor(cr);

	pNode->PushTrn(pNode->m_clientsock, pNode->Make_Cmd_CHANOPTIONS(m_ID, CHANOP_BACKGROUND, cr));
}

void JClient::JPageUser::rename(DWORD idNew, const std::tstring& newname)
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
			pNode->PushTrn(pNode->clientsock, pNode->Make_Cmd_PART(pNode->m_idOwn, m_ID));
			pNode->EvLog(format("parts from [b]%s[/b] private talk", tstrToANSI(m_user.name).c_str()), elogInfo);

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

void JClient::JPageUser::OnHook(JNode* src)
{
	using namespace fastdelegate;

	__super::OnHook(src);

	JNODE(JClient, node, src);
	if (node) {
		node->EvLinkStart += MakeDelegate(this, &JClient::JPageUser::OnLinkStart);
		node->EvLinkClose += MakeDelegate(this, &JClient::JPageUser::OnLinkClose);
	}
}

void JClient::JPageUser::OnUnhook(JNode* src)
{
	using namespace fastdelegate;

	JNODE(JClient, node, src);
	if (node) {
		node->EvLinkStart -= MakeDelegate(this, &JClient::JPageUser::OnLinkStart);
		node->EvLinkClose -= MakeDelegate(this, &JClient::JPageUser::OnLinkClose);
	}

	__super::OnUnhook(src);
}

void JClient::JPageUser::OnLinkStart(SOCKET sock)
{
	if (m_hwndPage) {
		pNode->PushTrn(pNode->m_clientsock, pNode->Make_Quest_JOIN(m_user.name, m_user.password, gettype()));
	}
}

void JClient::JPageUser::OnLinkClose(SOCKET sock, UINT err)
{
	if (m_hwndPage) {
		AppendScript(TEXT("[style=Info]Disconnected[/style]"));
	}
}

//-----------------------------------------------------------------------------

//
// JPageChannel
//

JClient::JPageChannel::JPageChannel(DWORD id, const std::tstring& nick)
: JPageChat(id)
{
	m_channel.name = nick;
	m_channel.crBackground = GetSysColor(COLOR_WINDOW);
}

std::tstring JClient::JPageChannel::getSafeName(DWORD idUser) const
{
	return pNode->getSafeName(pNode->m_bCheatAnonymous || !m_channel.isAnonymous || pNode->isGod(idUser) ? idUser : CRC_ANONYMOUS);
}

void JClient::JPageChannel::Enable()
{
	EnableWindow(m_hwndEdit, TRUE);
	m_fEnabled = true;
}

void JClient::JPageChannel::Disable()
{
	ListView_DeleteAllItems(m_hwndList);
	EnableWindow(m_hwndEdit, FALSE);
	m_fEnabled = false;
}

int JClient::JPageChannel::ImageIndex() const
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
	if (m_channel.topic.empty()) {
		return tformat(TEXT("#%s"), m_channel.name.c_str());
	} else if (m_channel.isAnonymous || pNode->mUser.find(m_channel.idTopicWriter) == pNode->mUser.end()) {
		return tformat(TEXT("#%s: %s"), m_channel.name.c_str(), m_channel.topic.c_str());
	} else {
		return tformat(TEXT("#%s: %s (%s)"), m_channel.name.c_str(), m_channel.topic.c_str(), getSafeName(m_channel.idTopicWriter).c_str());
	}
}

void CALLBACK JClient::JPageChannel::BaloonShow(DWORD idUser, const TCHAR* msg, HICON hicon, COLORREF cr)
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
	MapUser::const_iterator iu = pNode->m_mUser.find(idUser);
	pNode->BaloonShow(
		pNode->hwndPage,
		p,
		msg,
		iu != pNode->m_mUser.end() ? iu->second.name.c_str() : 0,
		hicon,
		cr);
}

void JClient::JPageChannel::OnSheetColor(COLORREF cr)
{
	__super::OnSheetColor(cr);

	if (m_channel.getStatus(pNode->m_idOwn) >= eAdmin || pNode->isGod()) {
		pNode->PushTrn(pNode->m_clientsock, pNode->Make_Cmd_CHANOPTIONS(m_ID, CHANOP_BACKGROUND, cr));
	}
}

bool JClient::JPageChannel::CanSend() const
{
	return m_channel.getStatus(pNode->m_idOwn) > eReader || pNode->isCheats();
}

void JClient::JPageChannel::setchannel(const Channel& val)
{
	m_channel = val;

	if (m_hwndPage) {
		ListView_DeleteAllItems(m_hwndList);
		BuildView();
	}
}

void JClient::JPageChannel::rename(DWORD idNew, const std::tstring& newname)
{
	m_ID = idNew;
	m_channel.name = newname;
}

bool JClient::JPageChannel::replace(DWORD idOld, DWORD idNew)
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

void JClient::JPageChannel::redrawUser(DWORD idUser)
{
	LVFINDINFO lvfi;
	lvfi.flags = LVFI_PARAM;
	lvfi.lParam = (LPARAM)idUser;
	int index = ListView_FindItem(m_hwndList, -1, &lvfi);
	if (index != -1) VERIFY(ListView_RedrawItems(m_hwndList, index, index));
}

void JClient::JPageChannel::Join(DWORD idWho)
{
	m_channel.opened.insert(idWho);
	if (m_channel.getStatus(idWho) == eOutsider)
		m_channel.setStatus(idWho, m_channel.nAutoStatus > eOutsider ? m_channel.nAutoStatus : eWriter);

	if (m_hwndPage) AddLine(idWho);

	// Introduction messages only fo finded user here - user can be unknown or hidden
	MapUser::const_iterator iu = pNode->m_mUser.find(idWho);
	if (iu != pNode->m_mUser.end() && iu->second.name.length()) {
		// Lua response
		DOLUACS;
		pNode->lua_getmethod(L, "onJoinChannel");
		if (lua_isfunction(L, -1)) {
			lua_insert(L, -2);
			lua_pushstring(L, tstrToANSI(getSafeName(idWho)).c_str());
			lua_pushstring(L, tstrToANSI(m_channel.name).c_str());
			lua_call(L, 3, 0);
		} else lua_pop(L, 2);
	}
}

void JClient::JPageChannel::Part(DWORD idWho, DWORD idBy)
{
	m_channel.opened.erase(idWho);
	if (m_hwndPage) DelLine(idWho);

	// Lua response
	DOLUACS;
	pNode->lua_getmethod(L, "onPartChannel");
	if (lua_isfunction(L, -1)) {
		lua_insert(L, -2);
		lua_pushstring(L, tstrToANSI(getSafeName(idWho)).c_str());
		lua_pushstring(L, tstrToANSI(getSafeName(idBy)).c_str());
		lua_pushstring(L, tstrToANSI(m_channel.name).c_str());
		lua_call(L, 4, 0);
	} else lua_pop(L, 2);
}

int  JClient::JPageChannel::indexIcon(DWORD idUser) const
{
	MapUser::const_iterator iu = pNode->m_mUser.find(idUser);
	if (iu == pNode->m_mUser.end()) return IML_MANVOID;
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
			return pNode->m_mUser.find((DWORD)lvi.lParam);
		}
	}
	return pNode->m_mUser.end();
}

LRESULT WINAPI JClient::JPageChannel::DlgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	LRESULT retval = TRUE;
	switch (message)
	{
	case WM_INITDIALOG:
		{
			__super::DlgProc(hWnd, message, wParam, lParam);

			m_hwndReBar1 = GetDlgItem(hWnd, IDC_REBAR1);
			m_hwndReBar2 = GetDlgItem(hWnd, IDC_REBAR2);
			m_hwndList = GetDlgItem(hWnd, IDC_USERLIST);

			// Get initial windows sizes
			MapControl(m_hwndList, rcList);

			// Inits ReBars
			REBARINFO rbi = {sizeof(REBARINFO), 0, 0};
			int h = rcLog.bottom - rcLog.top;
			/*int l = rcList.right - rcLog.left;

			REBARBANDINFO rbb1[] = {
				{
					sizeof(REBARBANDINFO), // cbSize
						RBBIM_CHILD | RBBIM_CHILDSIZE | RBBIM_COLORS | RBBIM_STYLE | RBBIM_SIZE, // fMask
						RBBS_CHILDEDGE | RBBS_NOGRIPPER | RBBS_VARIABLEHEIGHT, // fStyle
						GetSysColor(COLOR_BTNTEXT), // clrFore
						GetSysColor(COLOR_BTNFACE), // clrBack
						TEXT("Toolbar"), // lpText
						-1, // cch
						0, // iImage
						m_hwndTB, // hwndChild
						16, // cxMinChild
						l, // cyMinChild
						16, // cx
						0, // hbmBack
						0, // wID
#if (_WIN32_IE >= 0x0400)
						l, // cyChild
						l, // cyMaxChild
						8, // cyIntegral
						16, // cxIdeal
						0, // lParam
						0, // cxHeader
#endif
				},
				{
					sizeof(REBARBANDINFO), // cbSize
						RBBIM_CHILD | RBBIM_CHILDSIZE | RBBIM_COLORS | RBBIM_IDEALSIZE | RBBIM_STYLE | RBBIM_SIZE, // fMask
						RBBS_CHILDEDGE | RBBS_VARIABLEHEIGHT, // fStyle
						GetSysColor(COLOR_BTNTEXT), // clrFore
						GetSysColor(COLOR_BTNFACE), // clrBack
						TEXT("Users list"), // lpText
						-1, // cch
						0, // iImage
						m_hwndReBar2, // hwndChild
						250, // cxMinChild
						l, // cyMinChild
						300, // cx
						0, // hbmBack
						1, // wID
#if (_WIN32_IE >= 0x0400)
						l, // cyChild
						l, // cyMaxChild
						8, // cyIntegral
						300, // cxIdeal
						0, // lParam
						0, // cxHeader
#endif
				},
			};
			VERIFY(SendMessage(m_hwndReBar1, RB_SETBARINFO, 0, (LPARAM)&rbi));
			for (int i = 0; i < _countof(rbb1); i++) {
				VERIFY(SendMessage(m_hwndReBar1, RB_INSERTBAND, i, (LPARAM)&rbb1[i]));
			}*/

			REBARBANDINFO rbb2[] = {
				{
					sizeof(REBARBANDINFO), // cbSize
						RBBIM_CHILD | RBBIM_CHILDSIZE | RBBIM_COLORS | RBBIM_STYLE | RBBIM_SIZE, // fMask
						RBBS_CHILDEDGE | RBBS_VARIABLEHEIGHT, // fStyle
						GetSysColor(COLOR_BTNTEXT), // clrFore
						GetSysColor(COLOR_BTNFACE), // clrBack
						TEXT("Log"), // lpText
						-1, // cch
						0, // iImage
						m_hwndLog, // hwndChild
						200, // cxMinChild
						h, // cyMinChild
						200, // cx
						0, // hbmBack
						0, // wID
#if (_WIN32_IE >= 0x0400)
						h, // cyChild
						h, // cyMaxChild
						8, // cyIntegral
						200, // cxIdeal
						0, // lParam
						0, // cxHeader
#endif
				},
				{
					sizeof(REBARBANDINFO), // cbSize
						RBBIM_CHILD | RBBIM_CHILDSIZE | RBBIM_COLORS | RBBIM_IDEALSIZE | RBBIM_STYLE | RBBIM_SIZE, // fMask
						RBBS_CHILDEDGE | RBBS_VARIABLEHEIGHT, // fStyle
						GetSysColor(COLOR_BTNTEXT), // clrFore
						GetSysColor(COLOR_BTNFACE), // clrBack
						TEXT("Users list"), // lpText
						-1, // cch
						0, // iImage
						m_hwndList, // hwndChild
						0, // cxMinChild
						h, // cyMinChild
						120, // cx
						0, // hbmBack
						1, // wID
#if (_WIN32_IE >= 0x0400)
						h, // cyChild
						h, // cyMaxChild
						8, // cyIntegral
						120, // cxIdeal
						0, // lParam
						0, // cxHeader
#endif
				},
			};
			VERIFY(SendMessage(m_hwndReBar2, RB_SETBARINFO, 0, (LPARAM)&rbi));
			for (int i = 0; i < _countof(rbb2); i++) {
				VERIFY(SendMessage(m_hwndReBar2, RB_INSERTBAND, i, (LPARAM)&rbb2[i]));
			}

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
			pNode->BaloonHide(m_hwndList);

			pNode->EvPageClose.Invoke(m_ID);

			pNode->PushTrn(pNode->clientsock, pNode->Make_Cmd_PART(pNode->m_idOwn, m_ID));
			for each (SetId::value_type const& v in m_channel.opened) {
				pNode->UnlinkUser(v, m_ID);
			}
			pNode->EvLog(format("parts from [b]#%s[/b] channel", tstrToANSI(m_channel.name).c_str()), elogInfo);

			__super::DlgProc(hWnd, message, wParam, lParam);
			break;
		}

	case BEM_ADJUSTSIZE:
		{
			RECT rc;
			int cx = LOWORD(lParam), cy = HIWORD(lParam);
			HDWP hdwp = BeginDeferWindowPos(5);
			SetRect(&rc,
				rcEdit.left,
				cy - rcPage.bottom + rcEdit.top,
				cx - rcPage.right + rcEdit.right,
				cy - rcPage.bottom + rcEdit.bottom);
			DeferWindowPos(hdwp, m_hwndEdit, 0, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, SWP_NOREPOSITION | SWP_NOZORDER);
			SetRect(&rc,
				rcLog.left,
				rcLog.top,
				cx - rcPage.right + rcList.right,
				cy - rcPage.bottom + rcLog.bottom);
			DeferWindowPos(hdwp, m_hwndReBar2, 0, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, SWP_NOREPOSITION | SWP_NOZORDER);
			int h = rc.bottom - rc.top;
			REBARBANDINFO rbb2[] = {
				{
					sizeof(REBARBANDINFO), // cbSize
						RBBIM_CHILDSIZE, // fMask
						0, // fStyle
						0, // clrFore
						0, // clrBack
						TEXT("Log"), // lpText
						-1, // cch
						0, // iImage
						0, // hwndChild
						200, // cxMinChild
						h, // cyMinChild
						200, // cx
						0, // hbmBack
						0, // wID
#if (_WIN32_IE >= 0x0400)
						h, // cyChild
						h, // cyMaxChild
						8, // cyIntegral
						200, // cxIdeal
						0, // lParam
						0, // cxHeader
#endif
				},
				{
					sizeof(REBARBANDINFO), // cbSize
						RBBIM_CHILDSIZE, // fMask
						0, // fStyle
						0, // clrFore
						0, // clrBack
						TEXT("Users list"), // lpText
						-1, // cch
						0, // iImage
						0, // hwndChild
						0, // cxMinChild
						h, // cyMinChild
						120, // cx
						0, // hbmBack
						1, // wID
#if (_WIN32_IE >= 0x0400)
						h, // cyChild
						h, // cyMaxChild
						8, // cyIntegral
						120, // cxIdeal
						0, // lParam
						0, // cxHeader
#endif
				},
			};
			for (int i = 0; i < _countof(rbb2); i++) {
				VERIFY(SendMessage(hwndReBar2, RB_SETBANDINFO, i, (LPARAM)&rbb2[i]));
			}
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
				if (m_channel.getStatus(pNode->m_idOwn) >= (m_channel.isOpened(pNode->m_idOwn) ? eMember : eAdmin) || pNode->isGod()) {
					CreateDialogParam(
						JClientApp::jpApp->hinstApp,
						MAKEINTRESOURCE(IDD_TOPIC),
						pNode->hwndPage,
						(DLGPROC)JDialog::DlgProcStub,
						(LPARAM)(JDialog*)new JTopic(pNode, m_ID, m_channel.name, m_channel.topic));
				}
				break;

			case IDC_CHANFOUNDER:
			case IDC_CHANADMIN:
			case IDC_CHANMODERATOR:
			case IDC_CHANMEMBER:
			case IDC_CHANWRITER:
			case IDC_CHANREADER:
			case IDC_CHANPRIVATE:
				if (m_channel.getStatus(pNode->m_idOwn) >= eAdmin || pNode->isGod()) {
					ASSERT(pNode->m_clientsock);

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
					pNode->PushTrn(pNode->m_clientsock, pNode->Make_Cmd_CHANOPTIONS(m_ID, CHANOP_AUTOSTATUS, cmp[i].stat));
				}
				break;

			case IDC_CHANHIDDEN:
				if (m_channel.getStatus(pNode->m_idOwn) >= eAdmin || pNode->isGod()) {
					ASSERT(pNode->m_clientsock);
					pNode->PushTrn(pNode->m_clientsock, pNode->Make_Cmd_CHANOPTIONS(m_ID, CHANOP_HIDDEN, !m_channel.isHidden));
				}
				break;

			case IDC_CHANANONYMOUS:
				if (m_channel.getStatus(pNode->m_idOwn) >= eAdmin || pNode->isGod()) {
					ASSERT(pNode->m_clientsock);
					pNode->PushTrn(pNode->m_clientsock, pNode->Make_Cmd_CHANOPTIONS(m_ID, CHANOP_ANONYMOUS, !m_channel.isAnonymous));
				}
				break;

			case IDC_PRIVATETALK:
				{
					MapUser::const_iterator iu = getSelUser();
					ASSERT(iu != pNode->m_mUser.end());
					if ((!m_channel.isAnonymous && iu->second.accessibility.fCanOpenPrivate) || pNode->isGod()) {
						ASSERT(pNode->m_clientsock);
						pNode->PushTrn(pNode->m_clientsock, pNode->Make_Quest_JOIN(iu->second.name));
					} else BaloonShow(iu->first, MAKEINTRESOURCE(IDS_MSG_PRIVATETALK), (HICON)1);
					break;
				}

			case IDC_PRIVATEMESSAGE:
				{
					MapUser::const_iterator iu = getSelUser();
					ASSERT(iu != pNode->m_mUser.end());
					if ((!m_channel.isAnonymous && iu->second.accessibility.fCanMessage) || pNode->isGod()) {
						ASSERT(pNode->m_clientsock);
						CreateDialogParam(JClientApp::jpApp->hinstApp, MAKEINTRESOURCE(IDD_MSGSEND), pNode->hwndPage, JClient::JSplashRtfEditor::DlgProcStub, (LPARAM)(JDialog*)new JClient::JMessageEditor(pNode, iu->second.name, false));
					} else BaloonShow(iu->first, MAKEINTRESOURCE(IDS_MSG_PRIVATEMESSAGE), (HICON)1);
					break;
				}

			case IDC_ALERT:
				{
					MapUser::const_iterator iu = getSelUser();
					ASSERT(iu != pNode->m_mUser.end());
					if ((!m_channel.isAnonymous && iu->second.accessibility.fCanAlert) || pNode->isGod()) {
						ASSERT(pNode->m_clientsock);
						CreateDialogParam(JClientApp::jpApp->hinstApp, MAKEINTRESOURCE(IDD_MSGSEND), pNode->hwndPage, JClient::JSplashRtfEditor::DlgProcStub, (LPARAM)(JDialog*)new JClient::JMessageEditor(pNode, iu->second.name, true));
					} else BaloonShow(iu->first, MAKEINTRESOURCE(IDS_MSG_ALERT), (HICON)1);
					break;
				}

			case IDS_SOUNDSIGNAL:
				{
					MapUser::const_iterator iu = getSelUser();
					ASSERT(iu != pNode->m_mUser.end());
					if (iu->second.accessibility.fCanSignal || pNode->isGod()) {
						ASSERT(pNode->m_clientsock);
						pNode->PushTrn(pNode->m_clientsock, pNode->Make_Cmd_BEEP(iu->first));
					} else BaloonShow(iu->first, MAKEINTRESOURCE(IDS_MSG_SOUNDSIGNAL), (HICON)1);
					break;
				}

			case IDC_CLIPBOARD:
				{
					MapUser::const_iterator iu = getSelUser();
					ASSERT(iu != pNode->m_mUser.end());
					if ((pNode->m_metrics.flags.bTransmitClipboard && iu->second.accessibility.fCanRecvClipboard) || pNode->isGod()) {
						ASSERT(pNode->m_clientsock);
						pNode->PushTrn(pNode->m_clientsock, pNode->Make_Cmd_CLIPBOARD(iu->first));
					} else BaloonShow(iu->first, MAKEINTRESOURCE(IDS_MSG_CLIPBOARD), (HICON)1);
					break;
				}

			case IDC_SPLASHRTF:
				{
					MapUser::const_iterator iu = getSelUser();
					ASSERT(iu != pNode->m_mUser.end());
					if (iu->second.accessibility.fCanSplash || pNode->isGod()) {
						ASSERT(pNode->m_clientsock);
						CreateDialogParam(JClientApp::jpApp->hinstApp, MAKEINTRESOURCE(IDD_SPLASHRTFEDITOR), pNode->hwndPage, JClient::JSplashRtfEditor::DlgProcStub, (LPARAM)(JDialog*)new JClient::JSplashRtfEditor(pNode, iu->first));
					} else BaloonShow(iu->first, MAKEINTRESOURCE(IDS_MSG_SPLASHRTF), (HICON)1);
					break;
				}

			case IDC_RENAME:
				{
					ASSERT(pNode->m_clientsock);
					int index = ListView_GetNextItem(m_hwndList, -1, LVNI_SELECTED);
					if (index >= 0) {
						ListView_EditLabel(m_hwndList, index);
					}
					break;
				}

			case IDC_KICK:
				{
					MapUser::const_iterator iu = getSelUser();
					ASSERT(iu != pNode->m_mUser.end());

					bool isModer = m_channel.getStatus(pNode->m_idOwn) >= eModerator;
					bool canKick = m_channel.getStatus(pNode->m_idOwn) >= m_channel.getStatus(iu->first);
					if (pNode->m_idOwn == iu->first || (isModer && canKick && !pNode->isDevil(iu->first)) || pNode->isDevil()) {
						ASSERT(pNode->m_clientsock);
						pNode->PushTrn(pNode->clientsock, pNode->Make_Cmd_PART(iu->first, m_ID));
					} else BaloonShow(iu->first, MAKEINTRESOURCE(IDS_MSG_KICK), (HICON)1);
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
					ASSERT(iu != pNode->m_mUser.end());

					EChanStatus statOwn = m_channel.getStatus(pNode->m_idOwn), statUser = m_channel.getStatus(iu->first);
					static const UINT idc[] = {
						IDC_OUTSIDER, IDC_READER, IDC_WRITER, IDC_MEMBER, IDC_MODERATOR, IDC_ADMIN, IDC_FOUNDER,
					};
					int i;
					for (i = eOutsider; idc[i] != LOWORD(wParam); i++) {}
					ASSERT(i <= eFounder);
					bool canModer = (statOwn == eFounder) || ((pNode->m_idOwn == iu->first || statOwn > statUser) && statOwn > i);
					if ((canModer || pNode->isGod()) && statUser != i) {
						ASSERT(pNode->m_clientsock);
						pNode->PushTrn(pNode->m_clientsock, pNode->Make_Cmd_ACCESS(iu->first, m_ID, (EChanStatus)i));
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
					MapUser::const_iterator iter = pNode->m_mUser.find((DWORD)lpDrawItem->itemData);
					bool valid = iter != pNode->m_mUser.end() && iter->second.name.length();
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
					COLORREF cr = SetTextColor(lpDrawItem->hDC, (DWORD)lpDrawItem->itemData != pNode->m_idOwn
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
			if (!pNode) break;

			NMHDR* pnmh = (NMHDR*)lParam;
			switch (pnmh->code)
			{
			case LVN_BEGINLABELEDIT:
				{
					NMLVDISPINFO* pdi = (NMLVDISPINFO*)lParam;
					if (!pNode->m_clientsock) break;
					if (pnmh->idFrom == IDC_USERLIST) {
						if (pNode->isGod()) {
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
						if (pdi->item.pszText && pNode->m_clientsock) {
							std::tstring nick = pdi->item.pszText;
							const TCHAR* msg;
							if (JClient::CheckNick(nick, msg)) {
								pNode->PushTrn(pNode->m_clientsock, pNode->Make_Cmd_NICK((DWORD)pdi->item.lParam, nick));
								retval = TRUE;
								break;
							} else {
								pNode->BaloonShow(ListView_GetEditControl(m_hwndList), msg, MAKEINTRESOURCE(IDS_MSG_NICKERROR), 2);
							}
						}
					}
					retval = FALSE;
					break;
				}

			case LVN_ITEMCHANGED:
				{
					LPNMLISTVIEW pnmv = (LPNMLISTVIEW)lParam;
					if (pnmh->idFrom == IDC_USERLIST && pnmv->uChanged == LVIF_STATE) {
						MapUser::const_iterator iu = pNode->m_mUser.find((DWORD)pnmv->lParam);
						if (iu != pNode->m_mUser.end() && pnmv->iItem >= 0
							&& (pnmv->uNewState & LVIS_SELECTED) != 0 && (pnmv->uOldState & LVIS_SELECTED) == 0
							&& !m_channel.isAnonymous)
						{
							ASSERT(pNode->m_clientsock);

							SYSTEMTIME st;
							FileTimeToLocalTime(iu->second.ftCreation, st);
							HICON hicon = ImageList_GetIcon(JClientApp::jpApp->himlStatusImg, iu->second.nStatusImg, ILD_TRANSPARENT);
							BaloonShow(
								iu->first,
								tformat(
								TEXT("%s\t%s%s%s\n")
								TEXT("Status text:\t\"%s\"\n")
								TEXT("IP-address:\t%i.%i.%i.%i\n")
								TEXT("Connected:\t%02i:%02i:%02i, %02i.%02i.%04i"),
								iu->second.isOnline ? TEXT("User online") : TEXT("User offline"),
								iu->second.cheat.isGod ? TEXT("god mode, ") : TEXT(""),
								iu->second.cheat.isDevil ? TEXT("devil mode, ") : TEXT(""),
								s_mapUserStatName[iu->second.nStatus].c_str(),
								iu->second.strStatus.c_str(),
								iu->second.IP.S_un.S_un_b.s_b1,
								iu->second.IP.S_un.S_un_b.s_b2,
								iu->second.IP.S_un.S_un_b.s_b3,
								iu->second.IP.S_un.S_un_b.s_b4,
								st.wHour, st.wMinute, st.wSecond,
								st.wDay, st.wMonth, st.wYear).c_str(),
								hicon,
								(DWORD)pnmv->lParam != pNode->m_idOwn
								? RGB(0xC0, 0x00, 0x00)
								: RGB(0x00, 0x00, 0xC0));
								DestroyIcon(hicon);
						}
					}
					break;
				}

			case NM_DBLCLK:
				{
					if (pnmh->idFrom == IDC_USERLIST) {
						if (pNode->m_clientsock) {
							SendMessage(hWnd, WM_COMMAND, IDC_PRIVATETALK, 0);
						}
					}
					break;
				}

			case NM_KILLFOCUS:
				{
					if (pnmh->idFrom == IDC_USERLIST) {
						pNode->BaloonHide(m_hwndList);
					}
					break;
				}

			case EN_MSGFILTER:
				{
					MSGFILTER* pmf = (MSGFILTER*)lParam;
					if (pnmh->idFrom == IDC_LOG) {
						if (pmf->msg == WM_LBUTTONDBLCLK) {
							if (m_channel.getStatus(pNode->m_idOwn) >= (m_channel.isOpened(pNode->m_idOwn) ? eMember : eAdmin))
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
			} else if ((HWND)wParam == GetDlgItem(hWnd, IDC_REBAR2)) {
				RBHITTESTINFO rbht;
				rbht.pt.x = GET_X_LPARAM(lParam);
				rbht.pt.y = GET_Y_LPARAM(lParam);
				SendMessage(m_hwndReBar2, RB_HITTEST, 0, (LPARAM)&rbht);
				if (rbht.iBand != -1) {
					if (rbht.iBand == 0) {
						wParam = (WPARAM)m_hwndLog;
						retval = __super::DlgProc(hWnd, message, wParam, lParam);
					} else if (rbht.iBand == 1) {
						wParam = (WPARAM)m_hwndList;
						if (ListView_GetSelectedCount(m_hwndList)) {
							RECT r;
							GetWindowRect((HWND)wParam, &r);
							TrackPopupMenu(GetSubMenu(pNode->isGod() ? JClientApp::jpApp->hmenuUserGod : JClientApp::jpApp->hmenuUser, 0), TPM_LEFTALIGN | TPM_RIGHTBUTTON,
								min(max(GET_X_LPARAM(lParam), r.left), r.right),
								min(max(GET_Y_LPARAM(lParam), r.top), r.bottom), 0, hWnd, 0);
						}
					}
				}
			} else {
				retval = __super::DlgProc(hWnd, message, wParam, lParam);
			}
			break;
		}

	case WM_INITMENUPOPUP:
		{
			if ((HMENU)wParam == GetSubMenu(JClientApp::jpApp->hmenuChannel, 0)) {
				bool canAdmin = m_channel.getStatus(pNode->m_idOwn) >= eAdmin || pNode->isGod();
				bool canMember = m_channel.getStatus(pNode->m_idOwn) >= (m_channel.isOpened(pNode->m_idOwn) ? eMember : eAdmin) || pNode->isGod();
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
				bool canAdmin = m_channel.getStatus(pNode->m_idOwn) >= eAdmin || pNode->isGod();
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
				bool valid = iu != pNode->m_mUser.end();

				pNode->BaloonHide(m_hwndList);

				VERIFY(SetMenuDefaultItem((HMENU)wParam, IDC_PRIVATETALK, FALSE));
				EnableMenuItem((HMENU)wParam, IDC_PRIVATETALK,
					MF_BYCOMMAND | (valid && ((!m_channel.isAnonymous && iu->second.accessibility.fCanOpenPrivate) || pNode->isGod()) ? MF_ENABLED : MF_GRAYED));
				EnableMenuItem((HMENU)wParam, IDC_PRIVATEMESSAGE,
					MF_BYCOMMAND | (valid && ((!m_channel.isAnonymous && iu->second.accessibility.fCanMessage) || pNode->isGod()) ? MF_ENABLED : MF_GRAYED));
				EnableMenuItem((HMENU)wParam, IDC_ALERT,
					MF_BYCOMMAND | (valid && ((!m_channel.isAnonymous && iu->second.accessibility.fCanAlert) || pNode->isGod()) ? MF_ENABLED : MF_GRAYED));
				EnableMenuItem((HMENU)wParam, IDS_SOUNDSIGNAL,
					MF_BYCOMMAND | (valid && (iu->second.accessibility.fCanSignal || pNode->isGod()) ? MF_ENABLED : MF_GRAYED));
				EnableMenuItem((HMENU)wParam, IDC_CLIPBOARD,
					MF_BYCOMMAND | (valid && ((iu->second.accessibility.fCanRecvClipboard && pNode->m_metrics.flags.bTransmitClipboard) || pNode->isGod()) ? MF_ENABLED : MF_GRAYED));
				EnableMenuItem((HMENU)wParam, IDC_SPLASHRTF,
					MF_BYCOMMAND | (valid && (iu->second.accessibility.fCanSplash || pNode->isGod()) ? MF_ENABLED : MF_GRAYED));

				EnableMenuItem((HMENU)wParam, IDC_RENAME,
					MF_BYCOMMAND | (valid && pNode->isGod() ? MF_ENABLED : MF_GRAYED));

				bool isModer = m_channel.getStatus(pNode->m_idOwn) >= eModerator;
				bool canKick = m_channel.getStatus(pNode->m_idOwn) >= m_channel.getStatus(iu->first);
				EnableMenuItem((HMENU)wParam, IDC_KICK,
					MF_BYCOMMAND | (valid && (iu->first == pNode->m_idOwn || (isModer && canKick && !pNode->isDevil(iu->first)) || pNode->isDevil()) ? MF_ENABLED : MF_GRAYED));
			} else if ((HMENU)wParam == GetSubMenu(GetSubMenu(JClientApp::jpApp->hmenuUser, 0), 8)) {
				MapUser::const_iterator iu = getSelUser();
				if (iu != pNode->m_mUser.end()) {
					EChanStatus statOwn = m_channel.getStatus(pNode->m_idOwn), statUser = m_channel.getStatus(iu->first);
					static const UINT idc[] = {
						IDC_OUTSIDER, IDC_READER, IDC_WRITER, IDC_MEMBER, IDC_MODERATOR, IDC_ADMIN, IDC_FOUNDER,
					};
					int i;
					for (i = eOutsider; i <= eFounder; i++) {
						bool canModer = (statOwn == eFounder) || ((pNode->m_idOwn == iu->first || statOwn > statUser) && statOwn > i);
						EnableMenuItem((HMENU)wParam, idc[i],
							MF_BYCOMMAND | (canModer || pNode->isGod() ? MF_ENABLED : MF_GRAYED));
					}
					CheckMenuRadioItem((HMENU)wParam, idc[eFounder], idc[eOutsider], idc[statUser], MF_BYCOMMAND);
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

void JClient::JPageChannel::BuildView()
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

void JClient::JPageChannel::ClearView()
{
	ListView_DeleteAllItems(m_hwndList);
	SetWindowText(m_hwndLog, TEXT(""));
	SetWindowText(m_hwndEdit, TEXT(""));
}

int  JClient::JPageChannel::AddLine(DWORD id)
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

void JClient::JPageChannel::DelLine(DWORD id)
{
	LVFINDINFO lvfi;
	lvfi.flags = LVFI_PARAM;
	lvfi.lParam = id;
	ListView_DeleteItem(m_hwndList, ListView_FindItem(m_hwndList, -1, &lvfi));
}

void JClient::JPageChannel::OnHook(JNode* src)
{
	using namespace fastdelegate;

	__super::OnHook(src);

	JNODE(JClient, node, src);
	if (node) {
		node->EvLinkStart += MakeDelegate(this, &JClient::JPageChannel::OnLinkStart);
		node->EvLinkClose += MakeDelegate(this, &JClient::JPageChannel::OnLinkClose);
		node->EvTopic += MakeDelegate(this, &JClient::JPageChannel::OnTopic);
		node->EvNick += MakeDelegate(this, &JClient::JPageChannel::OnNick);
	}
}

void JClient::JPageChannel::OnUnhook(JNode* src)
{
	using namespace fastdelegate;

	JNODE(JClient, node, src);
	if (node) {
		node->EvLinkStart -= MakeDelegate(this, &JClient::JPageChannel::OnLinkStart);
		node->EvLinkClose -= MakeDelegate(this, &JClient::JPageChannel::OnLinkClose);
		node->EvTopic -= MakeDelegate(this, &JClient::JPageChannel::OnTopic);
		node->EvNick -= MakeDelegate(this, &JClient::JPageChannel::OnNick);
	}

	__super::OnUnhook(src);
}

void JClient::JPageChannel::OnLinkStart(SOCKET sock)
{
	if (m_hwndPage) {
		pNode->PushTrn(pNode->m_clientsock, pNode->Make_Quest_JOIN(m_channel.name, m_channel.password, gettype()));
	}
}

void JClient::JPageChannel::OnLinkClose(SOCKET sock, UINT err)
{
	if (m_hwndPage) {
		AppendScript(TEXT("[style=Info]Disconnected[/style]"));
	}
	m_channel.opened.clear(); // no users on disconnected channel
}

void JClient::JPageChannel::OnNick(DWORD idOld, const std::tstring& oldname, DWORD idNew, const std::tstring& newname)
{
}

void JClient::JPageChannel::OnTopic(DWORD idWho, DWORD idWhere, const std::tstring& topic)
{
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