
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
#include "dCRC.h"
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
}

void JClient::JPage::OnHook(JEventable* src)
{
	ASSERT(pSource);
	using namespace fastdelegate;

	__super::OnHook(src);
}

void JClient::JPage::OnUnhook(JEventable* src)
{
	ASSERT(pSource);
	using namespace fastdelegate;

	__super::OnUnhook(src);

	SetSource(0);
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

	case BEM_ADJUSTSIZE:
		{
			//RECT rc;
			//int cx = LOWORD(lParam), cy = HIWORD(lParam);
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

	pSource->EvLinkConnecting += MakeDelegate(this, &JClient::JPageServer::OnLinkConnecting);
	pSource->EvLinkConnect += MakeDelegate(this, &JClient::JPageServer::OnLinkConnect);
	pSource->EvLinkDestroy += MakeDelegate(this, &JClient::JPageServer::OnLinkDestroy);
	pSource->EvLog += MakeDelegate(this, &JClient::JPageServer::OnLog);
	pSource->EvReport += MakeDelegate(this, &JClient::JPageServer::OnReport);
}

void JClient::JPageServer::OnUnhook(JEventable* src)
{
	ASSERT(pSource);
	using namespace fastdelegate;

	pSource->EvLinkConnecting -= MakeDelegate(this, &JClient::JPageServer::OnLinkConnecting);
	pSource->EvLinkConnect -= MakeDelegate(this, &JClient::JPageServer::OnLinkConnect);
	pSource->EvLinkDestroy -= MakeDelegate(this, &JClient::JPageServer::OnLinkDestroy);
	pSource->EvLog -= MakeDelegate(this, &JClient::JPageServer::OnLog);
	pSource->EvReport -= MakeDelegate(this, &JClient::JPageServer::OnReport);

	__super::OnUnhook(src);
}

void JClient::JPageServer::OnLinkConnecting(SOCKET sock)
{
	ASSERT(pSource);
	if (m_hwndPage)
	{
		EnableWindow(m_hwndHost, FALSE);
		EnableWindow(m_hwndPort, FALSE);
		EnableWindow(m_hwndPass, FALSE);
	}
}

void JClient::JPageServer::OnLinkConnect(SOCKET sock)
{
	ASSERT(pSource);
}

void JClient::JPageServer::OnLinkDestroy(SOCKET sock)
{
	ASSERT(pSource);
	if (m_hwndPage)
	{
		EnableWindow(m_hwndHost, TRUE);
		EnableWindow(m_hwndPort, TRUE);
		EnableWindow(m_hwndPass, TRUE);
	}
}

void JClient::JPageServer::OnLog(const std::tstring& str, bool withtime)
{
	ASSERT(pSource);
	AppendScript(str, withtime);
}

void JClient::JPageServer::OnReport(const std::tstring& str, netengine::EGroup gr, netengine::EPriority prior)
{
	ASSERT(pSource);
	if (m_Groups.find(gr) != m_Groups.end() && prior < m_Priority) return;

	std::tstring msg;
	switch (gr)
	{
	case netengine::eMessage:
		msg = TEXT("[style=Msg]") + str + TEXT("[/style]");
		break;
	case netengine::eDescription:
		msg = TEXT("[style=Descr]") + str + TEXT("[/style]");
		break;
	case netengine::eInformation:
		msg = TEXT("[style=Info]") + str + TEXT("[/style]");
		break;
	case netengine::eWarning:
		msg = TEXT("[style=Warning]") + str + TEXT("[/style]");
		break;
	case netengine::eError:
		msg = TEXT("[style=Error]") + str + TEXT("[/style]");
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

	case BEM_ADJUSTSIZE:
		{
			//RECT rc;
			//int cx = LOWORD(lParam), cy = HIWORD(lParam);
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
						std::tstring chan, msg;
						chan = chanbuf;
						if (pSource->CheckNick(chan, msg)) { // check content
							pSource->Send_Quest_JOIN(pSource->m_clientsock, chan, passbuf);
						} else {
							pSource->ShowErrorMessage(m_hwndChan, msg);
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
								switch (iter->second.nAutoStatus)
								{
								case eWriter:
									pnmv->item.pszText = TEXT("writer");
									break;
								case eMember:
									pnmv->item.pszText = TEXT("member");
									break;
								case eModerator:
									pnmv->item.pszText = TEXT("moderator");
									break;
								case eAdmin:
									pnmv->item.pszText = TEXT("administrator");
									break;
								case eFounder:
									pnmv->item.pszText = TEXT("founder");
									break;
								default:
									pnmv->item.pszText = TEXT("reader");
									break;
								}
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

	pSource->EvLinkConnect += MakeDelegate(this, &JClient::JPageList::OnLinkConnect);
	pSource->EvLinkDestroy += MakeDelegate(this, &JClient::JPageList::OnLinkDestroy);
	pSource->EvLinkIdentify += MakeDelegate(this, &JClient::JPageList::OnLinkIdentify);
	pSource->EvTransactionProcess += MakeDelegate(this, &JClient::JPageList::OnTransactionProcess);
}

void JClient::JPageList::OnUnhook(JEventable* src)
{
	ASSERT(pSource);
	using namespace fastdelegate;

	pSource->EvLinkConnect -= MakeDelegate(this, &JClient::JPageList::OnLinkConnect);
	pSource->EvLinkDestroy -= MakeDelegate(this, &JClient::JPageList::OnLinkDestroy);
	pSource->EvLinkIdentify -= MakeDelegate(this, &JClient::JPageList::OnLinkIdentify);
	pSource->EvTransactionProcess -= MakeDelegate(this, &JClient::JPageList::OnTransactionProcess);

	__super::OnUnhook(src);
}

void JClient::JPageList::OnLinkConnect(SOCKET sock)
{
	ASSERT(pSource);
	if (m_hwndPage)
	{
		EnableWindow(m_hwndChan, TRUE);
		EnableWindow(m_hwndPass, TRUE);
		EnableWindow(GetDlgItem(m_hwndPage, IDC_JOIN), TRUE);
		EnableWindow(GetDlgItem(m_hwndPage, IDC_REFRESHLIST), TRUE);
	}
}

void JClient::JPageList::OnLinkDestroy(SOCKET sock)
{
	ASSERT(pSource);
	if (m_hwndPage)
	{
		EnableWindow(m_hwndChan, FALSE);
		EnableWindow(m_hwndPass, FALSE);
		EnableWindow(GetDlgItem(m_hwndPage, IDC_JOIN), FALSE);
		EnableWindow(GetDlgItem(m_hwndPage, IDC_REFRESHLIST), FALSE);
	}
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

			hwndTB = GetDlgItem(hWnd, IDC_TOOLBAR);
			hwndEdit = GetDlgItem(hWnd, IDC_RICHEDIT);
			m_hwndSend = GetDlgItem(hWnd, IDC_SEND);
			m_hwndMsgSpin = GetDlgItem(hWnd, IDC_MSGSPIN);

			// Get initial windows sizes
			MapControl(hwndEdit, rcEdit);
			MapControl(m_hwndSend, rcSend);

			fTransparent = true;
			wCharFormatting = SCF_SELECTION;

			// Inits tool bar
			SendMessage(hwndTB, TB_BUTTONSTRUCTSIZE, (WPARAM)sizeof(TBBUTTON), 0);
			SendMessage(hwndTB, TB_SETBITMAPSIZE, 0, MAKELONG(16, 16));
			SendMessage(hwndTB, TB_SETIMAGELIST, 0, (LPARAM)JClientApp::jpApp->himlEdit);
			SendMessage(hwndTB, TB_ADDBUTTONS, _countof(tbButtons), (LPARAM)&tbButtons);
			// Setup font and checks buttons
			SendMessage(hwndEdit, EM_SETCHARFORMAT, SCF_ALL, (LPARAM)&cfDefault);
			UpdateCharacterButtons();
			UpdateParagraphButtons();
			SendMessage(hwndTB, TB_CHECKBUTTON, rtf::idcBkMode, MAKELONG(fTransparent, 0));

			// Inits Edit control
			crSheet = GetSysColor(COLOR_WINDOW);
			SendMessage(hwndEdit, EM_SETEVENTMASK, 0, EN_DRAGDROPDONE | ENM_SELCHANGE);
			SendMessage(hwndEdit, EM_SETBKGNDCOLOR, FALSE, (LPARAM)crSheet);

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
			pSource->EvPageClose.Invoke(getID());

			pSource->Send_Cmd_PART(pSource->clientsock, m_ID, PART_LEAVE);
			pSource->UnlinkUser(m_ID, pSource->m_idOwn);
			pSource->EvReport(tformat(TEXT("parts from [b]%s[/b] private talk"), m_user.name.c_str()), netengine::eInformation, netengine::eHigher);

			JPageLog::DlgProc(hWnd, message, wParam, lParam);
			rtf::Editor::DlgProc(hWnd, message, wParam, lParam);
			break;
		}

	case BEM_ADJUSTSIZE:
		{
			//RECT rc;
			//int cx = LOWORD(lParam), cy = HIWORD(lParam);
			break;
		}

	case WM_COMMAND:
		{
			switch (LOWORD(wParam))
			{
			case IDC_SEND:
				{
					if (pSource->m_clientsock) { // send content
						if (GetWindowTextLength(hwndEdit)) {
							std::string content;
							getContent(content, SF_RTF);
							pSource->Send_Cmd_SAY(pSource->m_clientsock, getID(), SF_RTF, content);
							SetWindowText(hwndEdit, TEXT(""));

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
				SendMessage(hwndEdit, WM_COPY, 0, 0);
				break;

			case IDC_CUT:
				SendMessage(hwndEdit, WM_CUT, 0, 0);
				break;

			case IDC_PASTE:
				SendMessage(hwndEdit, WM_PASTE, 0, 0);
				break;

			case IDC_DELETE:
				SendMessage(hwndEdit, WM_CLEAR, 0, 0);
				break;

			case IDC_SELECTALL:
				SendMessage(hwndEdit, EM_SETSEL, 0, -1);
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
						SetWindowText(hwndEdit, ANSIToTstr(vecMsgSpin[vecMsgSpin.size() - 1 - lpnmud->iPos]).c_str());
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
			if ((HWND)wParam == hwndEdit) {
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
				SetMenuDefaultItem((HMENU)wParam, IDC_SEND, FALSE);
				CheckMenuRadioItem((HMENU)wParam,
					IDC_SENDBYENTER, IDC_SENDBYCTRLENTER,
					pSource->bSendByEnter ? IDC_SENDBYENTER : IDC_SENDBYCTRLENTER,
					MF_BYCOMMAND);

				CHARRANGE cr;
				SendMessage(hwndEdit, EM_EXGETSEL, 0, (LPARAM)&cr);
				bool cancopy = cr.cpMin != cr.cpMax;
				bool canpaste = SendMessage(hwndEdit, EM_CANPASTE, CF_TEXT, 0) != 0;
				bool candelete = cancopy && (GetWindowLong(hwndEdit, GWL_STYLE) & ES_READONLY) == 0;
				bool canselect = GetWindowTextLength(hwndEdit) > 0;
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

	pSource->EvLinkConnect += MakeDelegate(this, &JClient::JPageUser::OnLinkConnect);
	pSource->EvLinkDestroy += MakeDelegate(this, &JClient::JPageUser::OnLinkDestroy);
}

void JClient::JPageUser::OnUnhook(JEventable* src)
{
	ASSERT(pSource);
	using namespace fastdelegate;

	pSource->EvLinkConnect -= MakeDelegate(this, &JClient::JPageUser::OnLinkConnect);
	pSource->EvLinkDestroy -= MakeDelegate(this, &JClient::JPageUser::OnLinkDestroy);

	__super::OnUnhook(src);
}

void JClient::JPageUser::OnLinkConnect(SOCKET sock)
{
	ASSERT(pSource);
	if (m_hwndPage)
	{
		EnableWindow(hwndEdit, TRUE);
	}
}

void JClient::JPageUser::OnLinkDestroy(SOCKET sock)
{
	ASSERT(pSource);
	if (m_hwndPage)
	{
		if (m_user.idOnline) // only one disconnect message during connecting
			AppendScript(TEXT("[style=Info]Disconnected[/style]"));

		EnableWindow(hwndEdit, FALSE);
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
}

std::tstring JClient::JPageChannel::gettopic() const
{
	ASSERT(pSource);

	if (m_channel.topic.empty()) {
		return tformat(TEXT("#%s"), m_channel.name.c_str());
	} else if (pSource->mUser.find(m_channel.idTopicWriter) == pSource->mUser.end()) {
		return tformat(TEXT("#%s: %s"), m_channel.name.c_str(), m_channel.topic.c_str());
	} else {
		return tformat(TEXT("#%s: %s (%s)"), m_channel.name.c_str(), m_channel.topic.c_str(), pSource->mUser.find(m_channel.idTopicWriter)->second.name.c_str());
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

void CALLBACK JClient::JPageChannel::settopic(const std::tstring& topic, DWORD id)
{
	m_channel.topic = topic;
	m_channel.idTopicWriter = id;
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

void CALLBACK JClient::JPageChannel::Part(DWORD idUser, DWORD reason)
{
	m_channel.opened.erase(idUser);
	if (m_hwndPage) DelLine(idUser);

	// Parting message
	MapUser::const_iterator iu = pSource->m_mUser.find(idUser);
	if (iu != pSource->m_mUser.end() && iu->second.name.length()) {
		std::tstring msg;
		switch (reason)
		{
		case PART_LEAVE:
			msg = tformat(TEXT("[style=Info]parts: [b]%s[/b][/style]"), iu->second.name.c_str());
			break;
		case PART_DISCONNECT:
			msg = tformat(TEXT("[style=Info]quits: [b]%s[/b][/style]"), iu->second.name.c_str());
			break;
		default:
			msg = tformat(TEXT("[style=Info][b]%s[/b] is out[/style]"), iu->second.name.c_str());
			break;
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

			hwndTB = GetDlgItem(hWnd, IDC_TOOLBAR);
			hwndEdit = GetDlgItem(hWnd, IDC_RICHEDIT);
			m_hwndList = GetDlgItem(hWnd, IDC_USERLIST);
			m_hwndSend = GetDlgItem(hWnd, IDC_SEND);
			m_hwndMsgSpin = GetDlgItem(hWnd, IDC_MSGSPIN);

			// Get initial windows sizes
			MapControl(hwndEdit, rcEdit);
			MapControl(m_hwndList, rcList);
			MapControl(m_hwndSend, rcSend);

			fTransparent = true;
			wCharFormatting = SCF_SELECTION;

			// Inits tool bar
			SendMessage(hwndTB, TB_BUTTONSTRUCTSIZE, (WPARAM)sizeof(TBBUTTON), 0);
			SendMessage(hwndTB, TB_SETBITMAPSIZE, 0, MAKELONG(16, 16));
			SendMessage(hwndTB, TB_SETIMAGELIST, 0, (LPARAM)JClientApp::jpApp->himlEdit);
			SendMessage(hwndTB, TB_ADDBUTTONS, _countof(tbButtons), (LPARAM)&tbButtons);
			// Setup font and checks buttons
			SendMessage(hwndEdit, EM_SETCHARFORMAT, SCF_ALL, (LPARAM)&cfDefault);
			UpdateCharacterButtons();
			UpdateParagraphButtons();
			SendMessage(hwndTB, TB_CHECKBUTTON, rtf::idcBkMode, MAKELONG(fTransparent, 0));

			// Inits Edit control
			crSheet = GetSysColor(COLOR_WINDOW);
			SendMessage(hwndEdit, EM_SETEVENTMASK, 0, EN_DRAGDROPDONE | ENM_SELCHANGE);
			SendMessage(hwndEdit, EM_SETBKGNDCOLOR, FALSE, (LPARAM)crSheet);
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
			pSource->EvPageClose.Invoke(getID());

			pSource->Send_Cmd_PART(pSource->clientsock, m_ID, PART_LEAVE);
			for each (SetId::value_type const& v in m_channel.opened) {
				pSource->UnlinkUser(v, m_ID);
			}
			pSource->EvReport(tformat(TEXT("parts from [b]#%s[/b] channel"), m_channel.name.c_str()), netengine::eInformation, netengine::eHigher);

			JPageLog::DlgProc(hWnd, message, wParam, lParam);
			rtf::Editor::DlgProc(hWnd, message, wParam, lParam);
			break;
		}

	case BEM_ADJUSTSIZE:
		{
			//RECT rc;
			//int cx = LOWORD(lParam), cy = HIWORD(lParam);
			break;
		}

	case WM_COMMAND:
		{
			switch (LOWORD(wParam))
			{
			case IDC_SEND:
				{
					if (pSource->m_clientsock) { // send content
						if (GetWindowTextLength(hwndEdit)) {
							std::string content;
							getContent(content, SF_RTF);
							pSource->Send_Cmd_SAY(pSource->m_clientsock, getID(), SF_RTF, content);
							SetWindowText(hwndEdit, TEXT(""));

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
				SendMessage(hwndEdit, WM_COPY, 0, 0);
				break;

			case IDC_CUT:
				SendMessage(hwndEdit, WM_CUT, 0, 0);
				break;

			case IDC_PASTE:
				SendMessage(hwndEdit, WM_PASTE, 0, 0);
				break;

			case IDC_DELETE:
				SendMessage(hwndEdit, WM_CLEAR, 0, 0);
				break;

			case IDC_SELECTALL:
				SendMessage(hwndEdit, EM_SETSEL, 0, -1);
				break;

			case IDC_CHANTOPIC:
				CreateDialogParam(JClientApp::jpApp->hinstApp, MAKEINTRESOURCE(IDD_TOPIC), pSource->hwndPage, (DLGPROC)JDialog::DlgProcStub, (LPARAM)(JDialog*)new JTopic(pSource, this));
				break;

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
						w = 16, h = 256;
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
					DrawText(lpDrawItem->hDC, valid ? iter->second.name.c_str() : tformat(TEXT("id 0x%08X"), lpDrawItem->itemData).c_str(), -1, &rc, DT_LEFT | DT_NOCLIP | DT_VCENTER);
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
					SYSTEMTIME st;
					FileTimeToLocalTime(iter->second.ftCreation, st);
					DrawText(lpDrawItem->hDC, valid
						? tformat(TEXT("%02i:%02i:%02i, %02i.%02i.%04i"),
						st.wHour, st.wMinute, st.wSecond,
						st.wDay, st.wMonth, st.wYear).c_str()
						: TEXT("N/A"), -1, &rc, DT_LEFT | DT_NOCLIP | DT_VCENTER);
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
			/*case LVN_GETDISPINFO:
				{
					static TCHAR buffer[32];
					LV_DISPINFO* pnmv = (LV_DISPINFO*)lParam;
					if (pnmh->idFrom == IDC_USERLIST)
					{
						if (pnmv->item.mask & LVIF_TEXT) {
							MapUser::const_iterator iter = pSource->m_mUser.find((DWORD)pnmv->item.lParam);
							bool valid = iter != pSource->m_mUser.end() && iter->second.name.length();
							switch (pnmv->item.iSubItem)
							{
							case 0:
								if (valid) {
									pnmv->item.pszText = (TCHAR*)iter->second.name.c_str();
								} else {
									StringCchPrintf(buffer, _countof(buffer),
										TEXT("id 0x%08X"), (DWORD)pnmv->item.lParam);
									pnmv->item.pszText = buffer;
								}
								break;

							case 1:
								if (valid) {
									StringCchPrintf(buffer, _countof(buffer),
										TEXT("%i.%i.%i.%i"),
										iter->second.IP.S_un.S_un_b.s_b1,
										iter->second.IP.S_un.S_un_b.s_b2,
										iter->second.IP.S_un.S_un_b.s_b3,
										iter->second.IP.S_un.S_un_b.s_b4);
									pnmv->item.pszText = buffer;
								} else {
									pnmv->item.pszText = TEXT("N/A");
								}
								break;

							case 2:
								if (valid) {
									SYSTEMTIME st;
									FileTimeToLocalTime(iter->second.ftCreation, st);

									StringCchPrintf(buffer, _countof(buffer),
										TEXT("%02i:%02i:%02i, %02i.%02i.%04i"),
										st.wHour, st.wMinute, st.wSecond,
										st.wDay, st.wMonth, st.wYear);
									pnmv->item.pszText = buffer;
								} else {
									pnmv->item.pszText = TEXT("N/A");
								}
								break;
							}
							pnmv->item.cchTextMax = lstrlen(pnmv->item.pszText);
						}
						if (pnmv->item.mask & LVIF_IMAGE) {
							switch (pnmv->item.iSubItem)
							{
							case 0:
								{
									pnmv->item.iImage = indexIcon((DWORD)pnmv->item.lParam);
									break;
								}
							case 1:
								{
									MapUser::const_iterator iter = pSource->m_mUser.find((DWORD)pnmv->item.lParam);
									pnmv->item.iImage = IML_MAN_COUNT + iter->second.nStatusImg;
									break;
								}
							}
						}
					}
					break;
				}*/

			case NM_DBLCLK:
				{
					if (pnmh->idFrom == IDC_USERLIST) {
						LPNMITEMACTIVATE lpnmitem = (LPNMITEMACTIVATE)lParam;
						LVITEM lvi;
						lvi.mask = LVIF_PARAM;
						lvi.iItem = lpnmitem->iItem;
						lvi.iSubItem = 0;
						if (lpnmitem->iItem >= 0 && ListView_GetItem(pnmh->hwndFrom, &lvi)) {
							MapUser::const_iterator iu = pSource->m_mUser.find((DWORD)lvi.lParam);
							if (iu != pSource->m_mUser.end()) {
								ASSERT(pSource->m_clientsock);
								if ((DWORD)lvi.lParam != pSource->m_idOwn) {
									pSource->Send_Quest_JOIN(pSource->m_clientsock, iu->second.name);
								} else {
									pSource->ShowErrorMessage(pnmh->hwndFrom, TEXT("It's your own nickname"));
								}
							}
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
						SetWindowText(hwndEdit, ANSIToTstr(vecMsgSpin[vecMsgSpin.size() - 1 - lpnmud->iPos]).c_str());
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
			} else if ((HWND)wParam == hwndEdit) {
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
			if ((HMENU)wParam == GetSubMenu(JClientApp::jpApp->hmenuChannel, 0)) {
				SetMenuDefaultItem((HMENU)wParam, IDC_CHANTOPIC, FALSE);
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
				SetMenuDefaultItem((HMENU)wParam, IDC_SEND, FALSE);
				CheckMenuRadioItem((HMENU)wParam,
					IDC_SENDBYENTER, IDC_SENDBYCTRLENTER,
					pSource->bSendByEnter ? IDC_SENDBYENTER : IDC_SENDBYCTRLENTER,
					MF_BYCOMMAND);

				CHARRANGE cr;
				SendMessage(hwndEdit, EM_EXGETSEL, 0, (LPARAM)&cr);
				bool cancopy = cr.cpMin != cr.cpMax;
				bool canpaste = SendMessage(hwndEdit, EM_CANPASTE, CF_TEXT, 0) != 0;
				bool candelete = cancopy && (GetWindowLong(hwndEdit, GWL_STYLE) & ES_READONLY) == 0;
				bool canselect = GetWindowTextLength(hwndEdit) > 0;
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

void CALLBACK JClient::JPageChannel::BuildView()
{
	for each (SetId::value_type const& v in m_channel.opened)
		AddLine(v);
}

void CALLBACK JClient::JPageChannel::ClearView()
{
	ListView_DeleteAllItems(m_hwndList);
	SetWindowText(m_hwndLog, TEXT(""));
	SetWindowText(hwndEdit, TEXT(""));
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

	pSource->EvLinkConnect += MakeDelegate(this, &JClient::JPageChannel::OnLinkConnect);
	pSource->EvLinkDestroy += MakeDelegate(this, &JClient::JPageChannel::OnLinkDestroy);
}

void JClient::JPageChannel::OnUnhook(JEventable* src)
{
	ASSERT(pSource);
	using namespace fastdelegate;

	pSource->EvLinkConnect -= MakeDelegate(this, &JClient::JPageChannel::OnLinkConnect);
	pSource->EvLinkDestroy -= MakeDelegate(this, &JClient::JPageChannel::OnLinkDestroy);

	__super::OnUnhook(src);
}

void JClient::JPageChannel::OnLinkConnect(SOCKET sock)
{
	ASSERT(pSource);
	if (m_hwndPage)
	{
		EnableWindow(hwndEdit, TRUE);
	}
}

void JClient::JPageChannel::OnLinkDestroy(SOCKET sock)
{
	ASSERT(pSource);
	if (m_hwndPage)
	{
		if (m_channel.opened.size()) // only one disconnect message during connecting
			AppendScript(TEXT("[style=Info]Disconnected[/style]"));
		ListView_DeleteAllItems(m_hwndList);

		EnableWindow(hwndEdit, FALSE);
	}
	m_channel.opened.clear(); // no users on disconnected channel
}

//-----------------------------------------------------------------------------

// The End.