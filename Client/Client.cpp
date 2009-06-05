
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

//-----------------------------------------------------------------------------

#define WM_ACTIVATEAPP2                (WM_USER+10)

using namespace colibrichat;

// Global Variables:
static TCHAR szHelpFile[MAX_PATH];

//-----------------------------------------------------------------------------

//
// class JClient
//

CALLBACK JClient::JClient()
: netengine::JEngine(), JDialog(),
jpOnline(0)
{
	m_clientsock = 0;
	m_bReconnect = true;
	m_bSendByEnter = true;

	m_metrics.uNickMaxLength = 20;
	m_metrics.uChanMaxLength = 20;
	m_metrics.uPassMaxLength = 32;
	m_metrics.uStatusMsgMaxLength = 32;
	m_metrics.nMsgSpinMaxCount = 20;
}

void CALLBACK JClient::Init()
{
	__super::Init();
}

void CALLBACK JClient::Done()
{
	__super::Done();
}

int  CALLBACK JClient::Run()
{
	// Clear pages if was previous run
	jpOnline = 0;
	jpPageServer = 0;
	jpPageList = 0;
	mPageUser.clear();
	mPageChannel.clear();

	__super::Run();

	return m_State;
}

void CALLBACK JClient::LoadState()
{
	FILETIME ft;
	GetSystemFileTime(ft);

	m_idOwn = CRC_NONAME;
	User& user = m_mUser[m_idOwn];
	user.name = Profile::GetString(RF_CLIENT, RK_NICK, NAME_NONAME);
	user.opened.insert(CRC_SERVER); // make it permanent
	user.ftCreation = ft;
	user.IP.S_un.S_addr = ntohl(INADDR_LOOPBACK);
	user.isOnline = true;
	user.idOnline = 0;
	user.nStatus = (EUserStatus)Profile::GetInt(RF_CLIENT, RK_STATUS, eReady);
	user.nStatusImg = Profile::GetInt(RF_CLIENT, RK_STATUSIMG, 0);
	user.strStatus = Profile::GetString(RF_CLIENT, RK_STATUSMSG, TEXT("ready to talk"));
	m_hostname = tstrToANSI(Profile::GetString(RF_CLIENT, RK_HOST, TEXT("127.0.0.1")));
	m_port = (u_short)Profile::GetInt(RF_CLIENT, RK_PORT, CCP_PORT);
	m_password = Profile::GetString(RF_CLIENT, RK_PASSWORD, TEXT("beowolf"));
	m_bSendByEnter = Profile::GetInt(RF_CLIENT, RK_SENDBYENTER, true) != 0;
}

void CALLBACK JClient::SaveState()
{
	User& user = m_mUser[m_idOwn];

	Profile::WriteString(RF_CLIENT, RK_HOST, ANSIToTstr(m_hostname).c_str());
	Profile::WriteInt(RF_CLIENT, RK_PORT, m_port);
	Profile::WriteString(RF_CLIENT, RK_PASSWORD, m_password.c_str());
	Profile::WriteInt(RF_CLIENT, RK_SENDBYENTER, m_bSendByEnter);

	Profile::WriteString(RF_CLIENT, RK_NICK, user.name.c_str());
	Profile::WriteInt(RF_CLIENT, RK_STATUS, user.nStatus);
	Profile::WriteInt(RF_CLIENT, RK_STATUSIMG, user.nStatusImg);
	Profile::WriteString(RF_CLIENT, RK_STATUSMSG, user.strStatus.c_str());
}

LRESULT WINAPI JClient::DlgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	LRESULT retval = TRUE;
	switch (message)
	{
	case WM_INITDIALOG:
		{
			JClientApp::jpApp->AddWindow(hWnd);
			if (State != JService::eInit) Init();

			Run();

			hwndTab = GetDlgItem(hWnd, IDC_CHANNELS);

			// Set main window icons
			SNDMSG(hWnd, WM_SETICON, ICON_SMALL, (LPARAM)JClientApp::jpApp->hiMain16);
			SNDMSG(hWnd, WM_SETICON, ICON_BIG, (LPARAM)JClientApp::jpApp->hiMain32);

			// Inits Tab control
			TabCtrl_SetImageList(hwndTab, JClientApp::jpApp->himlTab);

			ContactAdd(NAME_LIST, CRC_LIST, eList);
			ContactAdd(NAME_SERVER, CRC_SERVER, eServer);

			if (Profile::GetInt(RF_CLIENT, RK_STATE, FALSE))
				Connect();
			else
				EvLog(TEXT("[style=msg]Ready to connect[/style]"), true);

			retval = TRUE;
			break;
		}

	case WM_DESTROY:
		{
			Profile::WriteInt(RF_CLIENT, RK_STATE, m_clientsock != 0);
			//Sleep(Profile::GetInt(RF_CLIENT, RK_QUITPAUSE, 500)); // gives opportunity to send final messages
			Disconnect();

			Stop();

			JClientApp::jpApp->DelWindow(hWnd);
			break;
		}

	case WM_SIZE:
		{
			if (wParam == SIZE_MINIMIZED) break;
			SendDlgItemMessage(hWnd, IDC_TOOLBAR, TB_AUTOSIZE, 0, 0);
			SendMessage(hWnd, BEM_ADJUSTSIZE, wParam, lParam);
		}

	case BEM_NETWORK:
		{
			if ((SOCKET)wParam == m_clientsock)
				EventSelector(m_clientsock, WSAGETSELECTEVENT(lParam));
			break;
		}

	case BEM_ADJUSTSIZE:
		{
			//RECT rc;
			//int cx = LOWORD(lParam), cy = HIWORD(lParam);
			break;
		}

	case WM_ACTIVATE:
		JClientApp::jpApp->hdlgCurrent = wParam ? hWnd : 0;
		JClientApp::jpApp->haccelCurrent = JClientApp::jpApp->haccelMain;
		break;

	case WM_ACTIVATEAPP:
		// Give opportunity to close application without online status sending
		PostMessage(hWnd, WM_ACTIVATEAPP2, wParam, lParam);
		break;

	case WM_ACTIVATEAPP2:
		if (m_clientsock) Send_Cmd_ONLINE(m_clientsock, wParam != 0, jpOnline ? jpOnline->getID() : 0);
		break;

	case WM_COMMAND:
		{
			switch (LOWORD(wParam))
			{
			case IDOK:
			case IDOK2:
				{
					int id = (int)(INT_PTR)GetMenu(GetFocus());
					switch (id)
					{
					case IDC_RICHEDIT:
						if ((m_bSendByEnter && LOWORD(wParam) == IDOK)
							|| (!m_bSendByEnter && LOWORD(wParam) == IDOK2))
							SendMessage(hWnd, WM_COMMAND, IDC_SEND, 0);
						else
							SendDlgItemMessage(jpOnline->hwndPage, IDC_RICHEDIT, WM_KEYDOWN, '\n', 0); // send "enter"
						break;

					case IDC_HOST:
					case IDC_PORT:
					case IDC_PASS:
						{
							std::tstring nick(m_metrics.uNickMaxLength, 0), msg;
							GetDlgItemText(jpOnline->hwndPage, IDC_NICK, &nick[0], (int)nick.size()+1);
							if (CheckNick(nick, msg)) { // check content
								SendMessage(jpOnline->hwndPage, WM_COMMAND, IDC_CONNECT, 0);
							} else {
								ShowErrorMessage(jpPageServer->hwndNick, msg);
							}
							break;
						}

					case IDC_NICK:
						{
							std::tstring nick(m_metrics.uNickMaxLength, 0), msg;
							GetDlgItemText(jpOnline->hwndPage, IDC_NICK, &nick[0], (int)nick.size()+1);
							if (CheckNick(nick, msg)) { // check content
								Send_Quest_NICK(m_clientsock, nick);
							} else {
								ShowErrorMessage(jpPageServer->hwndNick, msg);
							}
							break;
						}

					case IDC_STATUSMSG:
						{
							std::tstring msg(m_metrics.uStatusMsgMaxLength, 0);
							GetDlgItemText(jpOnline->hwndPage, IDC_STATUSMSG, &msg[0], (int)msg.size()+1);
							Send_Cmd_STATUS_Msg(m_clientsock, msg);
							break;
						}

					case IDC_JOINCHAN:
					case IDC_JOINPASS:
						SendMessage(jpOnline->hwndPage, WM_COMMAND, IDC_JOIN, 0);
						break;
					}
					break;
				}

			case IDC_TABCLOSE:
				{
					if (jpOnline->IsPermanent()) {
						ShowErrorMessage(hwndTab, TEXT("This page can not be closed"));
					} else {
						int sel;
						TCITEM tci;
						sel = TabCtrl_GetCurSel(hwndTab);
						tci.mask = TCIF_PARAM;
						VERIFY(TabCtrl_GetItem(hwndTab, sel, &tci));
						ContactDel((DWORD)tci.lParam);
						ContactSel(min(sel, TabCtrl_GetItemCount(hwndTab) - 1));
					}
					break;
				}

			case IDCANCEL:
				DestroyWindow(hWnd);
				break;

			default:
				retval =
					(jpOnline && SendMessage(jpOnline->hwndPage, message, wParam, lParam)) ||
					__super::DlgProc(hWnd, message, wParam, lParam);
			}
			break;
		}

	case WM_NOTIFY:
		{
			NMHDR* pnmh = (NMHDR*)lParam;
			switch (pnmh->code)
			{
			case TCN_SELCHANGE:
				{
					if (pnmh->idFrom == IDC_CHANNELS)
					{
						ContactSel(TabCtrl_GetCurSel(hwndTab));
					}
					break;
				}

			default: retval = FALSE;
			}
			break;
		}

	case WM_CONTEXTMENU:
		{
			if ((HWND)wParam == hwndTab) {
				RECT r;
				GetWindowRect((HWND)wParam, &r);
				TrackPopupMenu(GetSubMenu(JClientApp::jpApp->hmenuTab, 0), TPM_LEFTALIGN | TPM_RIGHTBUTTON,
					min(max(GET_X_LPARAM(lParam), r.left), r.right),
					min(max(GET_Y_LPARAM(lParam), r.top), r.bottom), 0, hWnd, 0);
			} else {
				__super::DlgProc(hWnd, message, wParam, lParam);
			}
			break;
		}

	case WM_INITMENUPOPUP:
		{
			if ((HMENU)wParam == GetSubMenu(JClientApp::jpApp->hmenuTab, 0))
			{
				EnableMenuItem((HMENU)wParam, IDC_TABCLOSE,
					MF_BYCOMMAND | (jpOnline && !jpOnline->IsPermanent() ? MF_ENABLED : MF_GRAYED));
				break;
			} else {
				__super::DlgProc(hWnd, message, wParam, lParam);
			}
			break;
		}

	case WM_HELP:
		{
			WinHelp(hWnd, szHelpFile, HELP_CONTEXTPOPUP, ((LPHELPINFO)lParam)->dwContextId);
			break;
		}

	default: retval = FALSE;
	}
	return retval;
}

void CALLBACK JClient::Connect(bool getsetting)
{
	if (getsetting && jpPageServer) {
		TCHAR host[128], pass[64];
		GetDlgItemText(jpPageServer->hwndPage, IDC_HOST, host, _countof(host));
		m_hostname = tstrToANSI(host);
		m_port = (u_short)GetDlgItemInt(jpPageServer->hwndPage, IDC_PORT, FALSE, FALSE);
		GetDlgItemText(jpPageServer->hwndPage, IDC_PASS, pass, _countof(pass));
		m_password = pass;
	}
	// Add into socket manager
	netengine::Link link;
	hostent* h = gethostbyname(m_hostname.c_str());
	u_long addr = h ? *(u_long*)h->h_addr : htonl(INADDR_LOOPBACK);
	link.m_saAddr.sin_family = AF_INET;
	link.m_saAddr.sin_addr.S_un.S_addr = addr;
	link.m_saAddr.sin_port = htons(m_port);
	link.Select(FD_CONNECT | FD_READ | FD_WRITE | FD_CLOSE, m_hwndPage, BEM_NETWORK);
	link.Connect();
	InsertLink(link);
	m_clientsock = link.Sock;
	EvLinkConnecting(m_clientsock);
	// Write it's to log
	EvLog(
		tformat(TEXT("[style=msg]Connecting to %s (%i.%i.%i.%i:%i)[/style]"),
		ANSIToTstr(m_hostname).c_str(),
		(addr >> 0) & 0xFF,
		(addr >> 8) & 0xFF,
		(addr >> 16) & 0xFF,
		(addr >> 24) & 0xFF,
		m_port), true);
	// Update interface
	if (jpPageServer->hwndPage) {
		CheckDlgButton(jpPageServer->hwndPage, IDC_CONNECT, TRUE);
	}
}

void CALLBACK JClient::Disconnect()
{
	if (m_clientsock != 0)
		DeleteLink(m_clientsock);
	// Update interface
	if (jpPageServer->hwndPage) {
		CheckDlgButton(jpPageServer->hwndPage, IDC_CONNECT, FALSE);
	}
}

void CALLBACK JClient::ContactAdd(const std::tstring& name, DWORD id, EContact type)
{
	JPtr<JPage> jp;
	int pos;

	if (Profile::GetInt(RF_CLIENT, RK_FLASHPAGENEW, TRUE) && !m_mUser[m_idOwn].isOnline)
		FlashWindow(m_hwndPage, TRUE);
	switch (type)
	{
	case eServer:
		{
			if (jpPageServer) return; // already opened

			jp = jpPageServer = new JPageServer;

			pos = 0;
			break;
		}
	case eList:
		{
			if (jpPageList) return; // already opened

			jp = jpPageList = new JPageList;

			pos = (jpPageServer ? 1 : 0);
			break;
		}
	case eUser:
		{
			if (mPageUser.find(id) != mPageUser.end()) return; // already opened

			jp = mPageUser[id] = new JPageUser(id, name);

			pos = (jpPageServer ? 1 : 0) + (jpPageList ? 1 : 0) +
				(int)mPageChannel.size() + (int)mPageUser.size() - 1;
			break;
		}
	case eChannel:
		{
			if (mPageChannel.find(id) != mPageChannel.end()) return; // already opened

			jp = mPageChannel[id] = new JPageChannel(id, name);

			pos = (jpPageServer ? 1 : 0) + (jpPageList ? 1 : 0) +
				(int)mPageChannel.size() - 1;
			break;
		}
	default:
		return;
	}

	ASSERT(jp);
	jp->SetSource(this);
	jp->SetupHooks();
	CreateDialogParam(JClientApp::jpApp->hinstApp, jp->Template(), m_hwndPage, (DLGPROC)JDialog::DlgProcStub, (LPARAM)(JDialog*)jp);

	TCITEM tci;
	tci.mask = TCIF_TEXT | TCIF_PARAM | TCIF_IMAGE;
	tci.dwState = 0;
	tci.dwStateMask = 0;
	tci.pszText = (TCHAR*)name.c_str();
	tci.cchTextMax = (int)name.length();
	tci.iImage = jp->ImageIndex();
	tci.lParam = jp->getID();
	TabCtrl_InsertItem(hwndTab, pos, &tci);
	TabCtrl_SetCurSel(hwndTab, pos);

	ShowWindow(jp->hwndPage, SW_SHOW);
	if (jpOnline) ShowWindow(jpOnline->hwndPage, SW_HIDE);
	jpOnline = jp;
}

void CALLBACK JClient::ContactDel(DWORD id)
{
	JPtr<JPage> jp;
	if (id == CRC_SERVER) {
		jp = jpPageServer;
		jpPageServer = 0;
	} else if (id == CRC_LIST) {
		jp = jpPageList;
		jpPageList = 0;
	} else if (mPageUser.find(id) != mPageUser.end()) {
		jp = mPageUser[id];
		mPageUser.erase(id);
	} else if (mPageChannel.find(id) != mPageChannel.end()) {
		jp = mPageChannel[id];
		mPageChannel.erase(id);
	} else return;
	DestroyWindow(jp->hwndPage);
	jp->ResetHooks();
	jp->SetSource(0);
	if (jpOnline == jp) jpOnline = 0;
	jp = 0;

	int i = getTabIndex(id);
	if (i >= 0) TabCtrl_DeleteItem(hwndTab, i);
}

void CALLBACK JClient::ContactSel(int index)
{
	ASSERT(index >= 0 && index < TabCtrl_GetItemCount(hwndTab));

	if (TabCtrl_GetCurSel(hwndTab) != index)
		TabCtrl_SetCurSel(hwndTab, index);

	TCITEM tci;
	tci.mask = TCIF_PARAM;
	VERIFY(TabCtrl_GetItem(hwndTab, index, &tci));
	JPtr<JPage> jp = getPage((DWORD)tci.lParam);
	ShowWindow(jp->hwndPage, SW_SHOW);
	if (jpOnline) ShowWindow(jpOnline->hwndPage, SW_HIDE);
	jpOnline = jp;
}

void CALLBACK JClient::ContactRename(DWORD idOld, DWORD idNew, const std::tstring& newname)
{
	std::tstring oldname;
	if (m_mUser[m_idOwn].opened.find(idOld) != m_mUser[m_idOwn].opened.end()) {
		m_mUser[m_idOwn].opened.insert(idNew);
		m_mUser[m_idOwn].opened.erase(idOld);
	}
	if (m_mUser.find(idOld) != m_mUser.end()) {
		oldname = m_mUser[idOld].name;
		m_mUser[idOld].name = newname;
		m_mUser[idNew] = m_mUser[idOld];
		m_mUser.erase(idOld);
	}
	if (m_idOwn == idOld) {
		m_idOwn = idNew;
	}

	int pos = -1;
	MapPageUser::iterator ipu = mPageUser.find(idOld);
	if (ipu != mPageUser.end()) {
		ipu->second->rename(idNew, newname);
		mPageUser[idNew] = ipu->second;
		mPageUser.erase(idOld);
		ipu->second->AppendScript(tformat(TEXT("[style=Info][b]%s[/b] is now known as [b]%s[/b][/style]"), oldname.c_str(), newname.c_str()));
	} else {
		MapPageChannel::iterator ipc = mPageChannel.find(idOld);
		if (ipc != mPageChannel.end()) {
			ipc->second->rename(idNew, newname);
			mPageChannel[idNew] = ipc->second;
			mPageChannel.erase(idOld);
			ipu->second->AppendScript(tformat(TEXT("[style=Info]channel name is now [b]%s[/b][/style]"), newname.c_str()));
		}
	}

	int i = getTabIndex(idOld);
	if (i >= 0) {
		TCITEM tci;
		tci.mask = TCIF_TEXT | TCIF_PARAM;
		tci.pszText = (TCHAR*)newname.c_str();
		tci.cchTextMax = (int)newname.length();
		tci.lParam = idNew;
		VERIFY(TabCtrl_SetItem(hwndTab, i, &tci));
		if (idNew == jpOnline->getID()) {
			VERIFY(InvalidateRect(jpOnline->hwndPage, 0, TRUE));
		}
	}
	for each (MapPageChannel::value_type const& v in mPageChannel) {
		if (v.second->replace(idOld, idNew)) {
			v.second->AppendScript(tformat(TEXT("[style=Info][b]%s[/b] is now known as [b]%s[/b][/style]"), oldname.c_str(), newname.c_str()));
		}
	}
}

int  CALLBACK JClient::getTabIndex(DWORD id)
{
	TCITEM tci;
	int i;
	for (i = TabCtrl_GetItemCount(hwndTab) - 1; i <= 0; i--) {
		tci.mask = TCIF_PARAM;
		VERIFY(TabCtrl_GetItem(hwndTab, i, &tci));
		if (tci.lParam == id) break;
	}
	return i;
}

JPtr<JClient::JPage> CALLBACK JClient::getPage(DWORD id)
{
	if (id == CRC_SERVER)
		return jpPageServer;
	else if (id == CRC_LIST)
		return jpPageList;
	else if (mPageUser.find(id) != mPageUser.end())
		return mPageUser[id];
	else if (mPageChannel.find(id) != mPageChannel.end())
		return mPageChannel[id];
	else return 0;
}

bool JClient::CheckNick(std::tstring& nick, std::tstring& msg)
{
	while (!nick.empty() && *nick.begin() <= TEXT(' ')) nick.erase(nick.begin());
	while (!nick.empty() && *(nick.end()-1) <= TEXT(' ')) nick.erase(nick.end()-1);
	if (nick.empty()) {
		msg = TEXT("Content can not be empty");
		return false;
	}
	for each (std::tstring::value_type const& v in nick) {
		if (v < TEXT(' ')) {
			msg = TEXT("Content must not contain non-printing symbols");
			return false;
		}
	}
	msg = TEXT("Content is valid");
	return true;
}

void JClient::ShowErrorMessage(HWND hwnd, const std::tstring& msg)
{
	MessageBeep(MB_ICONHAND);
	SetFocus(hwnd);
}

void CALLBACK JClient::InsertUser(DWORD idUser, const User& user)
{
	SetId set;
	if (m_mUser.find(idUser) != m_mUser.end()) {
		set = m_mUser[idUser].opened;
	}
	m_mUser[idUser] = user;
	m_mUser[idUser].opened = set;
}

void CALLBACK JClient::LinkUser(DWORD idUser, DWORD idLink)
{
	m_mUser[idUser].opened.insert(idLink);
}

void CALLBACK JClient::UnlinkUser(DWORD idUser, DWORD idLink)
{
	MapUser::iterator iu = m_mUser.find(idUser);
	if (iu != m_mUser.end()) {
		iu->second.opened.erase(idLink);
		if (iu->second.opened.empty())
			m_mUser.erase(iu);
	}
}

void JClient::OnHook(JEventable* src)
{
	using namespace fastdelegate;

	__super::OnHook(src);
}

void JClient::OnUnhook(JEventable* src)
{
	using namespace fastdelegate;

	// Reset hooks and pointer to this for dialogs objects at last
	if (jpPageServer) {
		jpPageServer->ResetHooks();
		jpPageServer->SetSource(0);
	}
	if (jpPageList) {
		jpPageList->ResetHooks();
		jpPageList->SetSource(0);
	}
	for each (MapPage::value_type const& v in mPageUser)
	{
		v.second->ResetHooks();
		v.second->SetSource(0);
	}
	for each (MapPage::value_type const& v in mPageChannel)
	{
		v.second->ResetHooks();
		v.second->SetSource(0);
	}

	__super::OnUnhook(src);
}

void JClient::OnLinkConnect(SOCKET sock)
{
	__super::OnLinkConnect(sock);

	sockaddr_in si;
	int len = sizeof(si);

	getpeername(sock, (struct sockaddr*)&si, &len);
	EvLog(tformat(TEXT("[style=msg]Connected to %i.%i.%i.%i:%i[/style]"),
		si.sin_addr.S_un.S_un_b.s_b1,
		si.sin_addr.S_un.S_un_b.s_b2,
		si.sin_addr.S_un.S_un_b.s_b3,
		si.sin_addr.S_un.S_un_b.s_b4,
		ntohs(si.sin_port)), true);

#ifdef UNICODE
	Send_Quest_IdentifyW
#else
	Send_Quest_IdentifyA
#endif
		(m_clientsock, m_password.c_str());
}

void JClient::OnLinkClose(SOCKET sock)
{
	bool isClient = sock == m_clientsock;

	__super::OnLinkClose(sock);

	// If not disconnected by user, try to reconnect again
	if (isClient && m_bReconnect) Connect();
}

void JClient::OnLinkDestroy(SOCKET sock)
{
	if (sock == m_clientsock)
	{
		m_clientsock = 0; // not connected
		EvLog(TEXT("[style=msg]Disconnected[/style]"), true);
	}

	__super::OnLinkDestroy(sock);
}

void JClient::OnLinkIdentify(SOCKET sock, const netengine::SetAccess& access)
{
	__super::OnLinkIdentify(sock, access);

	std::tstring nick(m_metrics.uNickMaxLength, 0);
	GetDlgItemText(jpPageServer->hwndPage, IDC_NICK, &nick[0], (int)nick.size()+1);
	EUserStatus stat = (EUserStatus)SendDlgItemMessage(jpPageServer->hwndPage, IDC_STATUS, CB_GETCURSEL, 0, 0);
	int img = (int)SendDlgItemMessage(jpPageServer->hwndPage, IDC_STATUSIMG, CB_GETCURSEL, 0, 0);
	std::tstring msg(m_metrics.uStatusMsgMaxLength, 0);
	GetDlgItemText(jpPageServer->hwndPage, IDC_STATUSMSG, &msg[0], (int)msg.size()+1);

	Send_Quest_NICK(sock, nick);
	Send_Cmd_STATUS(sock, stat, img, msg);
}

void JClient::OnTransactionProcess(SOCKET sock, WORD message, WORD trnid, io::mem is)
{
	typedef void (CALLBACK JClient::*TrnParser)(SOCKET, WORD, io::mem&);
	struct {
		WORD message;
		TrnParser parser;
	} responseTable[] =
	{
		{REPLY(CCPM_NICK), &JClient::Recv_Reply_NICK},
		{NOTIFY(CCPM_RENAME), &JClient::Recv_Notify_RENAME},
		{REPLY(CCPM_JOIN), &JClient::Recv_Reply_JOIN},
		{NOTIFY(CCPM_JOIN), &JClient::Recv_Notify_JOIN},
		{NOTIFY(CCPM_PART), &JClient::Recv_Notify_PART},
		{REPLY(CCPM_USERINFO), &JClient::Recv_Reply_USERINFO},
		{NOTIFY(CCPM_ONLINE), &JClient::Recv_Notify_ONLINE},
		{NOTIFY(CCPM_STATUS), &JClient::Recv_Notify_STATUS},
		{NOTIFY(CCPM_SAY), &JClient::Recv_Notify_SAY},
	};
	for (int i = 0; i < _countof(responseTable); i++)
	{
		if (responseTable[i].message == message)
		{
			(this->*responseTable[i].parser)(sock, trnid, is);
			return;
		}
	}

	__super::OnTransactionProcess(sock, message, trnid, is);
}

//
// Beowolf Network Protocol Messages reciving
//

void CALLBACK JClient::Recv_Reply_NICK(SOCKET sock, WORD trnid, io::mem& is)
{
	DWORD result;
	DWORD idNew;
	std::tstring newname;

	try
	{
		io::unpack(is, result);
		io::unpack(is, idNew);
		io::unpack(is, newname);
	}
	catch (io::exception e)
	{
		switch (e.count)
		{
		case 0:
		case 1:
			// Report about message
			EvReport(SZ_BADTRN, netengine::eWarning, netengine::eLow);
			return;
		}
	}

	if (m_idOwn != idNew) {
		m_mUser[m_idOwn].name = newname;
		m_mUser[idNew] = m_mUser[m_idOwn];
		m_mUser.erase(m_idOwn);
		m_idOwn = idNew;
	}
	// Update interface
	SetWindowText(jpPageServer->hwndNick, m_mUser[m_idOwn].name.c_str());

	// Report about message
	const TCHAR* msg = TEXT("unknown result");
	switch (result)
	{
	case NICK_OK:
		msg = TEXT("nickname successfully assigned");
		break;
	case NICK_TAKEN:
		msg = TEXT("nickname is taken");
		break;
	case NICK_TAKENUSER:
		msg = TEXT("nickname is taken by other user");
		break;
	case NICK_TAKENCHANNEL:
		msg = TEXT("nickname is taken by channel");
		break;
	}
	EvReport(tformat(TEXT("%s, now nickname is [b]%s[/b]"), msg, m_mUser[m_idOwn].name.c_str()), netengine::eInformation, netengine::eHigh);
}

void CALLBACK JClient::Recv_Notify_RENAME(SOCKET sock, WORD trnid, io::mem& is)
{
	DWORD idOld, idNew;
	std::tstring newname;

	try
	{
		io::unpack(is, idOld);
		io::unpack(is, idNew);
		io::unpack(is, newname);
	}
	catch (io::exception e)
	{
		switch (e.count)
		{
		case 0:
		case 1:
		case 2:
			// Report about message
			EvReport(SZ_BADTRN, netengine::eWarning, netengine::eLow);
			return;
		}
	}

	ContactRename(idOld, idNew, newname);
}

void CALLBACK JClient::Recv_Reply_JOIN(SOCKET sock, WORD trnid, io::mem& is)
{
	DWORD result;
	EContact type;
	DWORD ID;

	try
	{
		io::unpack(is, result);
		io::unpack(is, type);
		io::unpack(is, ID);
		if (result == CHAN_OK) {
			switch (type)
			{
			case eUser:
				{
					User user;

					io::unpack(is, user);

					InsertUser(ID, user);
					LinkUser(ID, m_idOwn);
					// Create interface
					ContactAdd(user.name, ID, type);

					JPtr<JPageUser> jp = mPageUser[ID];
					ASSERT(jp);
					jp->setuser(user);

					EvReport(tformat(TEXT("opens private talk with [b]%s[/b]"), user.name.c_str()), netengine::eInformation, netengine::eHigher);
					break;
				}

			case eChannel:
				{
					Channel chan;
					SetId wanted;

					io::unpack(is, chan);

					for each (SetId::value_type const& v in chan.opened) {
						MapUser::const_iterator iu = m_mUser.find(v);
						if (iu == m_mUser.end() || iu->second.name.empty())
							wanted.insert(v);
						LinkUser(v, ID);
					}

					// Create interface
					ContactAdd(chan.name, ID, type);

					JPtr<JPageChannel> jp = mPageChannel[ID];
					ASSERT(jp);
					jp->setchannel(chan);

					Send_Quest_USERINFO(sock, wanted);

					EvReport(tformat(TEXT("joins to [b]#%s[/b] channel"), chan.name.c_str()), netengine::eInformation, netengine::eHigher);
					break;
				}
			}
		} else {
			std::tstring msg;
			switch (result)
			{
			case CHAN_ALREADY:
				{
					int i = getTabIndex(ID);
					if (i >= 0) ContactSel(i);
				}
				msg = TEXT("contact is already opened");
				break;
			case CHAN_DENY:
				msg = TEXT("password does not satisfies");
				break;
			default:
				msg = TEXT("can not join to contact");
				break;
			}
			EvReport(msg, netengine::eError, netengine::eHigher);
		}
	}
	catch (io::exception e)
	{
		// Report about message
		EvReport(SZ_BADTRN, netengine::eWarning, netengine::eLow);
		return;
	}
}

void CALLBACK JClient::Recv_Notify_JOIN(SOCKET sock, WORD trnid, io::mem& is)
{
	DWORD idWho, idWhere;
	User user;

	try
	{
		io::unpack(is, idWho);
		io::unpack(is, idWhere);

		io::unpack(is, user);
	}
	catch (io::exception e)
	{
		switch (e.count)
		{
		case 0:
		case 1:
			// Report about message
			EvReport(SZ_BADTRN, netengine::eWarning, netengine::eLow);
			return;
		}
	}

	InsertUser(idWho, user);
	LinkUser(idWho, m_idOwn);
	if (idWhere == m_idOwn) { // private talk
		// Create interface
		ContactAdd(user.name, idWho, eUser);

		JPtr<JPageUser> jp = mPageUser[idWho];
		ASSERT(jp);
		jp->AppendScript(tformat(TEXT("[style=Info][b]%s[/b] call you to private talk[/style]"), user.name.c_str()));

		EvReport(tformat(TEXT("opens private talk with [b]%s[/b]"), user.name.c_str()), netengine::eInformation, netengine::eHigher);
	} else { // channel
		MapPageChannel::iterator ipc = mPageChannel.find(idWhere);
		if (ipc != mPageChannel.end()) {
			ipc->second->Join(idWho);
		} else {
			Send_Cmd_PART(sock, idWhere, PART_LEAVE);
		}
	}
}

void CALLBACK JClient::Recv_Notify_PART(SOCKET sock, WORD trnid, io::mem& is)
{
	DWORD idWho, idWhere;
	DWORD reason;

	try
	{
		io::unpack(is, idWho);
		io::unpack(is, idWhere);
		io::unpack(is, reason);
	}
	catch (io::exception e)
	{
		switch (e.count)
		{
		case 0:
			// Report about message
			EvReport(SZ_BADTRN, netengine::eWarning, netengine::eLow);
			return;

		case 1:
			reason = PART_DISCONNECT;
		}
	}

	if (idWhere == m_idOwn) { // private talk
		MapUser::const_iterator iu = m_mUser.find(idWho);
		std::tstring msg;
		std::tstring nick = iu != m_mUser.end() ? iu->second.name : TEXT("interlocutor");
		switch (reason)
		{
		case PART_LEAVE:
			msg = tformat(TEXT("[b]%s[/b] leave private talk"), nick.c_str());
			break;
		case PART_DISCONNECT:
			msg = tformat(TEXT("[b]%s[/b] was disconnected"), nick.c_str());
			break;
		default:
			msg = tformat(TEXT("[b]%s[/b] is out"), nick.c_str());
			break;
		}

		MapPageUser::iterator ipu = mPageUser.find(idWho);
		if (ipu != mPageUser.end()) { // private talk can be already closed
			ipu->second->AppendScript(TEXT("[style=Info]")+msg+TEXT("[/style]"));
		}

		EvReport(msg, netengine::eInformation, netengine::eHigher);
	} else { // channel
		MapPageChannel::iterator ipc = mPageChannel.find(idWhere);
		if (ipc != mPageChannel.end()) {
			ipc->second->Part(idWho, reason);
		} // no else matters
	}
	UnlinkUser(idWho, idWhere);
}

void CALLBACK JClient::Recv_Reply_USERINFO(SOCKET sock, WORD trnid, io::mem& is)
{
	User user;
	size_t count;
	DWORD id;

	try
	{
		io::unpack(is, count);
		while (count--) {
			io::unpack(is, id);
			io::unpack(is, user);

			InsertUser(id, user);
			MapPageChannel::iterator ipc = mPageChannel.find(jpOnline ? jpOnline->getID() : 0);
			if (ipc != mPageChannel.end()) {
				LVFINDINFO lvfi;
				lvfi.flags = LVFI_PARAM;
				lvfi.lParam = id;
				int index = ListView_FindItem(ipc->second->hwndList, -1, &lvfi);
				if (index != -1) VERIFY(ListView_RedrawItems(ipc->second->hwndList, index, index));
			}
		}
	}
	catch (io::exception e)
	{
		switch (e.count)
		{
		case 0:
			// Report about message
			EvReport(SZ_BADTRN, netengine::eWarning, netengine::eLow);
			return;
		}
	}

	// Report about message
	EvReport(tformat(TEXT("recieve users info")), netengine::eInformation, netengine::eLow);
}

void CALLBACK JClient::Recv_Notify_ONLINE(SOCKET sock, WORD trnid, io::mem& is)
{
	DWORD idWho;
	bool isOnline;
	DWORD idOnline;

	try
	{
		io::unpack(is, idWho);
		io::unpack(is, isOnline);
		io::unpack(is, idOnline);
	}
	catch (io::exception e)
	{
		switch (e.count)
		{
		case 0:
			isOnline = true;
		case 1:
			idOnline = 0;
		}
	}

	MapUser::iterator iu = m_mUser.find(idWho);
	if (iu != m_mUser.end()) {
		iu->second.isOnline = isOnline;
		iu->second.idOnline = idOnline;

		MapPageChannel::iterator ipc = mPageChannel.find(jpOnline ? jpOnline->getID() : 0);
		if (ipc != mPageChannel.end()) {
			LVFINDINFO lvfi;
			lvfi.flags = LVFI_PARAM;
			lvfi.lParam = idWho;
			int index = ListView_FindItem(ipc->second->hwndList, -1, &lvfi);
			if (index != -1) VERIFY(ListView_RedrawItems(ipc->second->hwndList, index, index));
		}
	}

	// Report about message
	EvReport(tformat(TEXT("user %s is %s"), iu != m_mUser.end() ? iu->second.name.c_str() : TEXT("unknown"), isOnline ? TEXT("online") : TEXT("offline")), netengine::eInformation, netengine::eLowest);
}

void CALLBACK JClient::Recv_Notify_STATUS(SOCKET sock, WORD trnid, io::mem& is)
{
	DWORD idWho;
	WORD type;
	EUserStatus stat;
	int img;
	std::tstring msg;

	try
	{
		io::unpack(is, idWho);
		io::unpack(is, type);
		if (type & STATUS_MODE) {
			io::unpack(is, stat);
		}
		if (type & STATUS_IMG) {
			io::unpack(is, img);
		}
		if (type & STATUS_MSG) {
			io::unpack(is, msg);
		}
	}
	catch (io::exception e)
	{
		switch (e.count)
		{
		case 0:
		case 1:
			// Report about message
			EvReport(SZ_BADTRN, netengine::eWarning, netengine::eLow);
			return;
		}
	}

	MapUser::iterator iu = m_mUser.find(idWho);
	if (iu != m_mUser.end()) {
		if (type & STATUS_MODE) {
			iu->second.nStatus = stat;
		}
		if (type & STATUS_IMG) {
			iu->second.nStatusImg = img;
		}
		if (type & STATUS_MSG) {
			iu->second.strStatus = msg;
		}

		MapPageChannel::iterator ipc = mPageChannel.find(jpOnline ? jpOnline->getID() : 0);
		if (ipc != mPageChannel.end()) {
			LVFINDINFO lvfi;
			lvfi.flags = LVFI_PARAM;
			lvfi.lParam = idWho;
			int index = ListView_FindItem(ipc->second->hwndList, -1, &lvfi);
			if (index != -1) VERIFY(ListView_RedrawItems(ipc->second->hwndList, index, index));
		}
	}
}

void CALLBACK JClient::Recv_Notify_SAY(SOCKET sock, WORD trnid, io::mem& is)
{
	DWORD idWho, idWhere;
	UINT type;
	std::string content;

	try
	{
		io::unpack(is, idWho);
		io::unpack(is, idWhere);
		io::unpack(is, type);
		io::unpack(is, content);
	}
	catch (io::exception e)
	{
		switch (e.count)
		{
		case 0:
		case 1:
			// Report about message
			EvReport(SZ_BADTRN, netengine::eWarning, netengine::eLow);
			return;
		}
	}

	JPtr<JPageLog> jp;
	MapPageUser::iterator iu = mPageUser.find(idWhere);
	if (iu != mPageUser.end()) { // private talk
		jp = iu->second;
		if (Profile::GetInt(RF_CLIENT, RK_FLASHPAGESAYPRIVATE, TRUE) && !m_mUser[m_idOwn].isOnline)
			FlashWindow(m_hwndPage, TRUE);
	} else {
		MapPageChannel::iterator ic = mPageChannel.find(idWhere);
		if (ic != mPageChannel.end()) { // channel
			jp = ic->second;
			if (Profile::GetInt(RF_CLIENT, RK_FLASHPAGESAYCHANNEL, TRUE) && !m_mUser[m_idOwn].isOnline)
				FlashWindow(m_hwndPage, TRUE);
		}
	}
	if (jp) {
		MapUser::const_iterator iu = m_mUser.find(idWho);
		jp->AppendScript(tformat(TEXT(" [color=%s]%s[/color]:"),
			iu->first != m_idOwn ? TEXT("red") : TEXT("blue"),
			iu->second.name.c_str()));
		jp->AppendRtf(content);
	}
}

//
// Beowolf Network Protocol Messages sending
//

void CALLBACK JClient::Send_Quest_NICK(SOCKET sock, const std::tstring& nick)
{
	std::ostringstream os;
	io::pack(os, nick);
	PushTrn(sock, QUEST(CCPM_NICK), 0, os.str());
}

void CALLBACK JClient::Send_Quest_JOIN(SOCKET sock, const std::tstring& name, const std::tstring& pass)
{
	std::ostringstream os;
	io::pack(os, name);
	io::pack(os, pass);
	PushTrn(sock, QUEST(CCPM_JOIN), 0, os.str());
}

void CALLBACK JClient::Send_Cmd_PART(SOCKET sock, DWORD id, DWORD reason)
{
	std::ostringstream os;
	io::pack(os, id);
	io::pack(os, reason);
	PushTrn(sock, COMMAND(CCPM_PART), 0, os.str());
}

void CALLBACK JClient::Send_Quest_USERINFO(SOCKET sock, const SetId& set)
{
	std::ostringstream os;
	io::pack(os, set);
	PushTrn(sock, QUEST(CCPM_USERINFO), 0, os.str());
}

void CALLBACK JClient::Send_Cmd_ONLINE(SOCKET sock, bool on, DWORD id)
{
	std::ostringstream os;
	io::pack(os, on);
	io::pack(os, id);
	PushTrn(sock, COMMAND(CCPM_ONLINE), 0, os.str());
}

void CALLBACK JClient::Send_Cmd_STATUS_Mode(SOCKET sock, EUserStatus stat)
{
	std::ostringstream os;
	io::pack(os, (WORD)STATUS_MODE);
	io::pack(os, stat);
	PushTrn(sock, COMMAND(CCPM_STATUS), 0, os.str());
}

void CALLBACK JClient::Send_Cmd_STATUS_Img(SOCKET sock, int img)
{
	std::ostringstream os;
	io::pack(os, (WORD)STATUS_IMG);
	io::pack(os, img);
	PushTrn(sock, COMMAND(CCPM_STATUS), 0, os.str());
}

void CALLBACK JClient::Send_Cmd_STATUS_Msg(SOCKET sock, const std::tstring& msg)
{
	std::ostringstream os;
	io::pack(os, (WORD)STATUS_MSG);
	io::pack(os, msg);
	PushTrn(sock, COMMAND(CCPM_STATUS), 0, os.str());
}

void CALLBACK JClient::Send_Cmd_STATUS(SOCKET sock, EUserStatus stat, int img, const std::tstring& msg)
{
	std::ostringstream os;
	io::pack(os, (WORD)(STATUS_MODE | STATUS_IMG | STATUS_MSG));
	io::pack(os, stat);
	io::pack(os, img);
	io::pack(os, msg);
	PushTrn(sock, COMMAND(CCPM_STATUS), 0, os.str());
}

void CALLBACK JClient::Send_Cmd_SAY(SOCKET sock, DWORD idWhere, UINT type, const std::string& content)
{
	std::ostringstream os;
	io::pack(os, idWhere);
	io::pack(os, type);
	io::pack(os, content);
	PushTrn(sock, COMMAND(CCPM_SAY), 0, os.str());
}

//-----------------------------------------------------------------------------

JPtr<JClientApp> JClientApp::jpApp = new JClientApp();

CALLBACK JClientApp::JClientApp(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR szcl, int ncs)
: JApplication(hInstance, hPrevInstance, szcl, ncs),
jpClient(0)
{
	sAppName = APPNAME;

	hinstRichEdit = 0;
	m_haccelMain = 0;
	m_himlEdit = 0;
	m_himlTab = 0;
	m_himlMan = 0;
	m_himgSend = 0;
}

void CALLBACK JClientApp::Init()
{
	INITCOMMONCONTROLSEX InitCtrls = {
		sizeof(INITCOMMONCONTROLSEX),
		ICC_WIN95_CLASSES |
		ICC_STANDARD_CLASSES |
		ICC_LINK_CLASS
	};
	VERIFY(InitCommonControlsEx(&InitCtrls));
	// Ensure that the RichEdit library is loaded
	VERIFY(false
		//|| (hinstRichEdit = LoadLibrary(TEXT("Msftedit.dll"))) != 0 // version 4.1
		|| (hinstRichEdit = LoadLibrary(TEXT("Riched20.dll"))) != 0 // version 2.0 or 3.0
		|| (hinstRichEdit = LoadLibrary(TEXT("Riched32.dll"))) != 0 // version 1.0
		);

	Profile::SetKey(TEXT("BEOWOLF"), APPNAME);

	hiMain16 = LoadIcon(hinstApp, MAKEINTRESOURCE(IDI_SMALL));
	hiMain32 = LoadIcon(hinstApp, MAKEINTRESOURCE(IDI_MAIN));

	// Load accel table for main window
	m_haccelMain = LoadAccelerators(hinstApp, MAKEINTRESOURCE(IDA_MAIN));

	// Load popup menus
	m_hmenuTab = LoadMenu(hinstApp, MAKEINTRESOURCE(IDM_TAB));
	m_hmenuLog = LoadMenu(hinstApp, MAKEINTRESOURCE(IDM_LOG));
	m_hmenuRichEdit = LoadMenu(hinstApp, MAKEINTRESOURCE(IDM_RICHEDIT));
	// Create the image list
	m_himlEdit = ImageList_LoadImage(hinstApp, MAKEINTRESOURCE(IDB_EDIT), 16, 12, CLR_DEFAULT, IMAGE_BITMAP, LR_CREATEDIBSECTION);
	m_himlTab = ImageList_LoadImage(hinstApp, MAKEINTRESOURCE(IDB_TAB), 16, 12, CLR_DEFAULT, IMAGE_BITMAP, LR_CREATEDIBSECTION);
	m_himlMan = ImageList_LoadImage(hinstApp, MAKEINTRESOURCE(IDB_MAN), 16, 8, CLR_DEFAULT, IMAGE_BITMAP, LR_CREATEDIBSECTION);
	m_himlStatus = ImageList_LoadImage(hinstApp, MAKEINTRESOURCE(IDB_STATUS), 16, 8, CLR_DEFAULT, IMAGE_BITMAP, LR_CREATEDIBSECTION);
	m_himlStatusImg = ImageList_LoadImage(hinstApp, MAKEINTRESOURCE(IDB_STATUSIMG), 16, 8, CLR_DEFAULT, IMAGE_BITMAP, LR_CREATEDIBSECTION);
	m_himgSend = LoadImage(hinstApp, MAKEINTRESOURCE(IDB_SEND), IMAGE_BITMAP, 72, 48, LR_LOADMAP3DCOLORS);
	m_himgULBG = LoadBitmap(hinstApp, MAKEINTRESOURCE(IDB_UL_BG));
	m_himgULFoc = LoadBitmap(hinstApp, MAKEINTRESOURCE(IDB_UL_FOC));
	m_himgULSel = LoadBitmap(hinstApp, MAKEINTRESOURCE(IDB_UL_SEL));
	m_himgULHot = LoadBitmap(hinstApp, MAKEINTRESOURCE(IDB_UL_HOT));

	jpClient = new JClient;
	jpClient->Init();
}

bool CALLBACK JClientApp::InitInstance()
{
	if (CreateDialogParam(hinstApp, MAKEINTRESOURCE(IDD_MAIN), 0, (DLGPROC)JClient::DlgProcStub, (LPARAM)(JDialog*)jpClient)) {
		return true;
	} else {
		MessageBox(0, TEXT("Cannot create main window"), sAppName.c_str(), MB_OK | MB_ICONSTOP);
		return false;
	}
}

void CALLBACK JClientApp::Done()
{
	if (jpClient->State != JService::eStopped) jpClient->Stop();
	jpClient->Done();
	// Free associated resources
	DestroyMenu(m_hmenuTab);
	DestroyMenu(m_hmenuLog);
	DestroyMenu(m_hmenuRichEdit);
	// Destroy the image list
	ImageList_Destroy(m_himlEdit);
	ImageList_Destroy(m_himlTab);
	ImageList_Destroy(m_himlMan);
	ImageList_Destroy(m_himlStatus);
	ImageList_Destroy(m_himlStatusImg);
	DeleteObject(m_himgSend);
	DeleteObject(m_himgULBG);
	DeleteObject(m_himgULFoc);
	DeleteObject(m_himgULSel);
	DeleteObject(m_himgULHot);
	// Free RichEdit library
	FreeLibrary(hinstRichEdit);
}

//-----------------------------------------------------------------------------

int WINAPI _tWinMain(HINSTANCE hInstance,
										 HINSTANCE hPrevInstance,
										 LPTSTR    lpszCmdLine,
										 int       nCmdShow)
{
	try
	{
		// Set current directory to executable location
		{
			TCHAR mpath[_MAX_PATH];
			TCHAR drive[_MAX_DRIVE];
			TCHAR dir[_MAX_DIR];
			GetModuleFileName(NULL, mpath, _countof(mpath));
			_tsplitpath_s(mpath, drive, _countof(drive), dir, _countof(dir), NULL, 0, NULL, 0);
			SetCurrentDirectory((std::tstring(drive)+dir).c_str());
		}

		JClientApp::jpApp->hinstApp = hInstance;
		JClientApp::jpApp->hinstPrev = hPrevInstance;
		JClientApp::jpApp->lpszCmdLine = lpszCmdLine;
		JClientApp::jpApp->nCmdShow = nCmdShow;

		return JClientApp::jpApp->Iteration();
	}
	catch (std::exception& e)
	{
		MessageBoxA(0, format("%s\r\n%s", typeid(e).name(), e.what()).c_str(), "Unhandled Exception!", MB_OK | MB_ICONSTOP);
	}
	return -1;
}

//-----------------------------------------------------------------------------

// The End.