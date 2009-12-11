
//-----------------------------------------------------------------------------

//
// Includes
//

#pragma region Includes

#include "stdafx.h"

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

#define WM_ACTIVATEAPP2                (WM_USER+10)

// Global Variables:
static TCHAR szHelpFile[MAX_PATH];

//-----------------------------------------------------------------------------

//
// class JClient
//

std::map<EChanStatus, std::tstring> JClient::s_mapChanStatName;
std::map<UINT, std::tstring> JClient::s_mapWsaErr;

void JClient::initclass()
{
	// Channels access status descriptions
	JClient::s_mapChanStatName[eOutsider] = TEXT("outsider");
	JClient::s_mapChanStatName[eReader] = TEXT("reader");
	JClient::s_mapChanStatName[eWriter] = TEXT("writer");
	JClient::s_mapChanStatName[eMember] = TEXT("member");
	JClient::s_mapChanStatName[eModerator] = TEXT("moderator");
	JClient::s_mapChanStatName[eAdmin] = TEXT("administrator");
	JClient::s_mapChanStatName[eFounder] = TEXT("founder");

	//   WSA error codes
	// on FD_CONNECT
	JClient::s_mapWsaErr[WSAECONNREFUSED] = TEXT("The attempt to connect was rejected.");
	JClient::s_mapWsaErr[WSAENETUNREACH] = TEXT("The network cannot be reached from this host at this time.");
	JClient::s_mapWsaErr[WSAEMFILE] = TEXT("No more file descriptors are available.");
	JClient::s_mapWsaErr[WSAENOBUFS] = TEXT("No buffer space is available. The socket cannot be connected.");
	JClient::s_mapWsaErr[WSAENOTCONN] = TEXT("The socket is not connected.");
	JClient::s_mapWsaErr[WSAETIMEDOUT] = TEXT("Attempt to connect timed out without establishing a connection.");
	// on FD_CLOSE
	JClient::s_mapWsaErr[0] = TEXT("The connection was reset by software itself.");
	JClient::s_mapWsaErr[WSAENETDOWN] = TEXT("The network subsystem failed."); // and all other
	JClient::s_mapWsaErr[WSAECONNRESET] = TEXT("The connection was reset by the remote side.");
	JClient::s_mapWsaErr[WSAECONNABORTED] = TEXT("The connection was terminated due to a time-out or other failure.");
}

void JClient::doneclass()
{
	JClient::s_mapChanStatName.clear();
	JClient::s_mapWsaErr.clear();
}

CALLBACK JClient::JClient()
: JEngine(), JDialog(),
jpOnline(0)
{
	m_clientsock = 0;
	m_bReconnect = true;
	m_nConnectCount = 0;
	m_bSendByEnter = true;
	m_bCheatAnonymous = false;

	static Alert alert[] = {
		{
			true, true, true, true,
			true, true, true, true, true, true,
			true, true, true, true, true, true,
		}, // eReady
		{
			false, false, false, false,
			false, false, false, false, false, false,
			false, false, false, false, false, false,
		}, // eDND
		{
			false, true, false, false,
			true, true, true, false, true, false,
			false, true, true, false, true, true,
		}, // eBusy
		{
			false, false, false, false,
			false, true, false, false, true, true,
			false, true, true, false, true, true,
		}, // eNA
		{
			true, true, false, false,
			true, true, true, true, true, true,
			false, true, true, true, true, true,
		}, // eAway
		{
			true, true, false, true,
			true, true, true, true, true, true,
			true, true, true, true, true, true,
		}, // eInvisible
	};
	m_mAlert[eReady] = alert[eReady];
	m_mAlert[eDND] = alert[eDND];
	m_mAlert[eBusy] = alert[eBusy];
	m_mAlert[eNA] = alert[eNA];
	m_mAlert[eAway] = alert[eAway];
	m_mAlert[eInvisible] = alert[eInvisible];

	m_metrics.uNameMaxLength = 20;
	m_metrics.uPassMaxLength = 32;
	m_metrics.uStatusMsgMaxLength = 32;
	m_metrics.uTopicMaxLength = 100;
	m_metrics.nMsgSpinMaxCount = 20;
	m_metrics.uChatLineMaxVolume = 80*1024;
	m_metrics.flags.bTransmitClipboard = true;
}

void CALLBACK JClient::Init()
{
	__super::Init();

	// Inits Lua virtual machine
	m_luaEvents = lua_open();
	ASSERT(m_luaEvents);
	luaL_openlibs(m_luaEvents);
	// register Lua data
	CLuaGluer<JClient>::Register(m_luaEvents);
	// insert itself to script
	setglobal(m_luaEvents, this, "chat");
	ASSERT(lua_gettop(m_luaEvents) == 0);

	jpOnline = 0;
	jpPageServer = 0;
	jpPageList = 0;
	// Clear pages
	mPageUser.clear();
	mPageChannel.clear();
}

void CALLBACK JClient::Done()
{
	if (m_luaEvents) {
		ASSERT(lua_gettop(m_luaEvents) == 0);
		// Close Lua virtual machine
		lua_close(m_luaEvents);
		m_luaEvents = 0;
	}

	__super::Done();
}

int  CALLBACK JClient::Run()
{
	__super::Run();

	// process the script
	if (m_luaEvents) {
		if (luaL_dofile(m_luaEvents, "events.client.lua")) {
			const char* msg = lua_tostring(m_luaEvents, -1);
			int t = lua_gettop(m_luaEvents);
			EvReport(tformat(TEXT("Error in script: %s"), ANSIToTstr(msg).c_str()), eError, eTop);
			// Close Lua virtual machine
			lua_close(m_luaEvents);
			m_luaEvents = 0;
		}
	}
	ASSERT(lua_gettop(m_luaEvents) == 0);

	return m_State;
}

void CALLBACK JClient::LoadState()
{
	m_idOwn = CRC_NONAME;

	User& user = m_mUser[m_idOwn];
	user.name = Profile::GetString(RF_CLIENT, RK_NICK, NAME_NONAME);
	const TCHAR* msg;
	if (!CheckNick(user.name, msg)) user.name = NAME_NONAME; // ensure that nick is valid
	user.opened.insert(CRC_SERVER); // make it permanent
	user.IP.S_un.S_addr = ntohl(INADDR_LOOPBACK);
	user.isOnline = eOnline;
	user.idOnline = 0;
	user.nStatus = (EUserStatus)Profile::GetInt(RF_CLIENT, RK_STATUS, eReady);
	user.accessibility = m_mAlert[user.nStatus];
	user.nStatusImg = Profile::GetInt(RF_CLIENT, RK_STATUSIMG, 0);
	user.strStatus = Profile::GetString(RF_CLIENT, RK_STATUSMSG, TEXT("ready to talk"));

	m_nCompression = Profile::GetInt(RF_CLIENT, RK_COMPRESSION, -1);
	m_bUseEncoding = Profile::GetInt(RF_CLIENT, RK_USEENCODING, true) != 0;

	m_hostname = tstrToANSI(Profile::GetString(RF_CLIENT, RK_HOST, TEXT("127.0.0.1")));
	m_port = (u_short)Profile::GetInt(RF_CLIENT, RK_PORT, CCP_PORT);
	m_passwordNet = Profile::GetString(RF_CLIENT, RK_PASSWORDNET, TEXT("beowolf"));
	m_bSendByEnter = Profile::GetInt(RF_CLIENT, RK_SENDBYENTER, true) != 0;
	m_bCheatAnonymous = Profile::GetInt(RF_CLIENT, RK_CHEATANONYMOUS, false) != 0;
}

void CALLBACK JClient::SaveState()
{
	Profile::WriteString(RF_CLIENT, RK_HOST, ANSIToTstr(m_hostname).c_str());
	Profile::WriteInt(RF_CLIENT, RK_PORT, m_port);
	Profile::WriteString(RF_CLIENT, RK_PASSWORDNET, m_passwordNet.c_str());
	Profile::WriteInt(RF_CLIENT, RK_SENDBYENTER, m_bSendByEnter);

	User& user = m_mUser[m_idOwn];
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

			m_hwndTab = GetDlgItem(hWnd, IDC_TAB);

			// Get initial windows sizes
			MapControl(m_hwndTab, rcTab);

			// Set main window icons
			SNDMSG(hWnd, WM_SETICON, ICON_SMALL, (LPARAM)JClientApp::jpApp->hiMain16);
			SNDMSG(hWnd, WM_SETICON, ICON_BIG, (LPARAM)JClientApp::jpApp->hiMain32);

			// Create baloon tool tip for users list
			m_hwndBaloon = CreateWindowEx(WS_EX_TOPMOST, TOOLTIPS_CLASS, 0,
				WS_POPUP | TTS_NOPREFIX | TTS_ALWAYSTIP | TTS_BALLOON,
				CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
				hWnd, 0, JClientApp::jpApp->hinstApp, 0);
			ASSERT(m_hwndBaloon);
			SendMessage(m_hwndBaloon, TTM_SETMAXTIPWIDTH, 0, 300);
			m_isBaloon = 0;

			TOOLINFO ti;
			ti.cbSize = sizeof(ti);
			ti.uFlags = TTF_ABSOLUTE | TTF_IDISHWND | TTF_TRACK;
			ti.hwnd = hwndPage;
			ti.uId = (UINT_PTR)hwndPage;
			ti.hinst = JClientApp::jpApp->hinstApp;
			ti.lpszText = 0;
			VERIFY(SendMessage(m_hwndBaloon, TTM_ADDTOOL, 0, (LPARAM)&ti));

			// Inits Tab control
			TabCtrl_SetImageList(m_hwndTab, JClientApp::jpApp->himlTab);

			ContactSel(ContactAdd(NAME_SERVER, CRC_SERVER, eServer));
			ContactAdd(NAME_LIST, CRC_LIST, eList);

			if (State != JService::eInit) Init();
			Run();

			// Lua response
			if (m_luaEvents) {
				lua_getglobal(m_luaEvents, "wmCreate");
				ASSERT(lua_gettop(m_luaEvents) == 1);
				lua_call(m_luaEvents, 0, 0);
			} else {
				if (Profile::GetInt(RF_CLIENT, RK_STATE, FALSE))
					Connect();
				else
					EvLog(TEXT("[style=msg]Ready to connect[/style]"), true);
			}

			retval = TRUE;
			break;
		}

	case WM_DESTROY:
		{
			// Lua response
			if (m_luaEvents) {
				lua_getglobal(m_luaEvents, "wmDestroy");
				ASSERT(lua_gettop(m_luaEvents) == 1);
				lua_call(m_luaEvents, 0, 0);
			} else {
				Profile::WriteInt(RF_CLIENT, RK_STATE, m_clientsock != 0);
				if (m_clientsock != 0) DeleteLink(m_clientsock);
			}

			HideBaloon(hwndPage);
			TOOLINFO ti;
			ti.cbSize = sizeof(ti);
			ti.hwnd = hwndPage;
			ti.uId = (UINT_PTR)hwndPage;
			SendMessage(m_hwndBaloon, TTM_DELTOOL, 0, (LPARAM)&ti);

			// Close all opened waves
			MCI_GENERIC_PARMS mci;
			mci.dwCallback = MAKELONG(m_hwndPage, 0);
			for each (MCIDEVICEID const& v in m_wDeviceID) {
				VERIFY(!mciSendCommand(v, MCI_CLOSE, MCI_WAIT, (DWORD_PTR)&mci));
			}

			Stop();

			JClientApp::jpApp->DelWindow(hWnd);
			break;
		}

	case WM_CLOSE:
		{
			// Lua response
			if (m_luaEvents) {
				lua_getglobal(m_luaEvents, "wmClose");
				ASSERT(lua_gettop(m_luaEvents) == 1);
				lua_call(m_luaEvents, 0, 0);
			} else {
				DestroyWindow(hWnd);
			}
			break;
		}

	case WM_GETMINMAXINFO:
		{
			MINMAXINFO* mmi = (MINMAXINFO*)lParam;
			mmi->ptMinTrackSize.x = rcPageNC.right - rcPageNC.left;
			mmi->ptMinTrackSize.y = rcPageNC.bottom - rcPageNC.top;
			break;
		}

	case WM_ENTERSIZEMOVE:
		{
			// Lua response
			if (m_luaEvents) {
				lua_getglobal(m_luaEvents, "wmEnterSizeMove");
				ASSERT(lua_gettop(m_luaEvents) == 1);
				lua_call(m_luaEvents, 0, 0);
			} else {
				HideBaloon();
			}
			break;
		}

	case WM_SIZE:
		{
			if (wParam == SIZE_MINIMIZED) break;
			SendMessage(hWnd, BEM_ADJUSTSIZE, wParam, lParam);
			break;
		}

	case BEM_NETWORK:
		{
			if ((SOCKET)wParam == m_clientsock)
				EventSelector(m_clientsock, WSAGETSELECTEVENT(lParam), WSAGETSELECTERROR(lParam));
			break;
		}

	case BEM_ADJUSTSIZE:
		{
			RECT rc;
			int cx = LOWORD(lParam), cy = HIWORD(lParam);
			HDWP hdwp = BeginDeferWindowPos(1 + (jpPageServer ? 1 : 0) + (jpPageList ? 1 : 0) + (int)mPageUser.size() + (int)mPageChannel.size());
			SetRect(&rc, rcTab.left, rcTab.top,
				cx - rcPage.right + rcTab.right,
				cy - rcPage.bottom + rcTab.bottom);
			DeferWindowPos(hdwp, m_hwndTab, 0, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, SWP_NOMOVE | SWP_NOREPOSITION | SWP_NOZORDER);
			if (jpPageServer) {
				SetRect(&rc, jpPageServer->rcPageNC.left, jpPageServer->rcPageNC.top,
					cx - rcPage.right + jpPageServer->rcPageNC.right,
					cy - rcPage.bottom + jpPageServer->rcPageNC.bottom);
				DeferWindowPos(hdwp, jpPageServer->hwndPage, 0, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, SWP_NOMOVE | SWP_NOREPOSITION | SWP_NOZORDER);
			}
			if (jpPageList) {
				SetRect(&rc, jpPageList->rcPageNC.left, jpPageList->rcPageNC.top,
					cx - rcPage.right + jpPageList->rcPageNC.right,
					cy - rcPage.bottom + jpPageList->rcPageNC.bottom);
				DeferWindowPos(hdwp, jpPageList->hwndPage, 0, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, SWP_NOMOVE | SWP_NOREPOSITION | SWP_NOZORDER);
			}
			for each (MapPageUser::value_type const& v in mPageUser) {
				SetRect(&rc, v.second->rcPageNC.left, v.second->rcPageNC.top,
					cx - rcPage.right + v.second->rcPageNC.right,
					cy - rcPage.bottom + v.second->rcPageNC.bottom);
				DeferWindowPos(hdwp, v.second->hwndPage, 0, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, SWP_NOMOVE | SWP_NOREPOSITION | SWP_NOZORDER);
			}
			for each (MapPageChannel::value_type const& v in mPageChannel) {
				SetRect(&rc, v.second->rcPageNC.left, v.second->rcPageNC.top,
					cx - rcPage.right + v.second->rcPageNC.right,
					cy - rcPage.bottom + v.second->rcPageNC.bottom);
				DeferWindowPos(hdwp, v.second->hwndPage, 0, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, SWP_NOMOVE | SWP_NOREPOSITION | SWP_NOZORDER);
			}
			EndDeferWindowPos(hdwp);
			break;
		}

	case WM_ACTIVATE:
		JClientApp::jpApp->hdlgCurrent = wParam ? hWnd : 0;
		JClientApp::jpApp->haccelCurrent = JClientApp::jpApp->haccelMain;
		break;

	case WM_ACTIVATEAPP:
		{
			// Give opportunity to close application without online status sending
			PostMessage(hWnd, WM_ACTIVATEAPP2, wParam, lParam);
			break;
		}

	case WM_ACTIVATEAPP2:
		{
			if (jpOnline) jpOnline->activate();
			if (m_clientsock) Send_Cmd_ONLINE(m_clientsock, wParam != 0 ? eOnline : eOffline, jpOnline ? jpOnline->getID() : 0);

			// Lua response
			if (m_luaEvents) {
				lua_getglobal(m_luaEvents, "wmActivateApp");
				lua_pushboolean(m_luaEvents, wParam != 0);
				ASSERT(lua_gettop(m_luaEvents) == 2);
				lua_call(m_luaEvents, 1, 0);
			} else {
				HideBaloon();
			}
		}
		break;

	case BEM_JDIALOG:
		{
			JPtr<JDialog> jp = (JDialog*)lParam; // destroy object if unused
			CreateDialogParam(
				JClientApp::jpApp->hinstApp,
				(LPTSTR)wParam,
				hWnd, JDialog::DlgProcStub,
				lParam);
			break;
		}

	case WM_COMMAND:
		{
			// Lua response
			if (m_luaEvents) {
				lua_getglobal(m_luaEvents, "wmCommand");
				lua_pushinteger(m_luaEvents, LOWORD(wParam));
				ASSERT(lua_gettop(m_luaEvents) == 2);
				lua_call(m_luaEvents, 1, 0);
			} else {
				HideBaloon();
			}

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
							std::tstring nickbuf(m_metrics.uNameMaxLength, 0), nick;
							const TCHAR* msg;
							GetDlgItemText(jpPageServer->hwndPage, IDC_NICK, &nickbuf[0], (int)nickbuf.size()+1);
							nick = nickbuf.c_str();
							if (JClient::CheckNick(nick, msg)) { // check content
								ASSERT(!m_clientsock); // only for disconnected
								SendMessage(jpPageServer->hwndPage, WM_COMMAND, IDC_CONNECT, 0);
							} else {
								DisplayMessage(jpPageServer->hwndNick, msg, MAKEINTRESOURCE(IDS_MSG_NICKERROR), 2);
							}
							break;
						}

					case IDC_NICK:
						{
							std::tstring nickbuf(m_metrics.uNameMaxLength, 0), nick;
							const TCHAR* msg;
							GetDlgItemText(jpPageServer->hwndPage, IDC_NICK, &nickbuf[0], (int)nickbuf.size()+1);
							nick = nickbuf.c_str();
							if (JClient::CheckNick(nick, msg)) { // check content
								if (m_clientsock) Send_Cmd_NICK(m_clientsock, m_idOwn, nick);
								else if (!m_nConnectCount) SendMessage(jpPageServer->hwndPage, WM_COMMAND, IDC_CONNECT, 0);
							} else {
								DisplayMessage(jpPageServer->hwndNick, msg, MAKEINTRESOURCE(IDS_MSG_NICKERROR), 2);
							}
							break;
						}

					case IDC_STATUSMSG:
						{
							std::tstring msg(m_metrics.uStatusMsgMaxLength, 0);
							GetDlgItemText(jpPageServer->hwndPage, IDC_STATUSMSG, &msg[0], (int)msg.size()+1);
							Send_Cmd_STATUS_Msg(m_clientsock, msg);
							break;
						}

					case IDC_JOINCHAN:
					case IDC_JOINPASS:
						SendMessage(jpPageList->hwndPage, WM_COMMAND, IDC_JOIN, 0);
						break;
					}
					break;
				}

			case IDC_TABCLOSE:
				{
					if (jpOnline->IsPermanent()) {
						DisplayMessage(m_hwndTab, MAKEINTRESOURCE(IDS_MSG_TABCLOSE), 0, 2);
					} else {
						int sel;
						TCITEM tci;
						sel = TabCtrl_GetCurSel(m_hwndTab);
						tci.mask = TCIF_PARAM;
						VERIFY(TabCtrl_GetItem(m_hwndTab, sel, &tci));
						ContactDel((DWORD)tci.lParam);
						ContactSel(min(sel, TabCtrl_GetItemCount(m_hwndTab) - 1));
					}
					break;
				}

			case IDCANCEL:
				// Lua response
				if (m_luaEvents) {
					lua_getglobal(m_luaEvents, "idcCancel");
					ASSERT(lua_gettop(m_luaEvents) == 1);
					lua_call(m_luaEvents, 0, 0);
				} else {
					SendMessage(hWnd, WM_CLOSE, 0, 0);
				}
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
					if (pnmh->idFrom == IDC_TAB)
					{
						ContactSel(TabCtrl_GetCurSel(m_hwndTab));
					}
					break;
				}

			default: retval = __super::DlgProc(hWnd, message, wParam, lParam);
			}
			break;
		}

	case WM_CONTEXTMENU:
		{
			if ((HWND)wParam == m_hwndTab) {
				RECT r;
				GetWindowRect((HWND)wParam, &r);
				TrackPopupMenu(GetSubMenu(JClientApp::jpApp->hmenuTab, 0), TPM_LEFTALIGN | TPM_RIGHTBUTTON,
					min(max(GET_X_LPARAM(lParam), r.left), r.right),
					min(max(GET_Y_LPARAM(lParam), r.top), r.bottom), 0, hWnd, 0);
			} else {
				retval = __super::DlgProc(hWnd, message, wParam, lParam);
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

	case WM_TIMER:
		{
			switch (wParam)
			{
			case IDT_CONNECT:
				{
					Connect();
					break;
				}

			case IDT_BALOONPOP:
				{
					HideBaloon();
					break;
				}
			}
			break;
		}

	case WM_HELP:
		{
			WinHelp(hWnd, szHelpFile, HELP_CONTEXTPOPUP, ((LPHELPINFO)lParam)->dwContextId);
			break;
		}

	case MM_MCINOTIFY:
		{
			if (m_wDeviceID.find((MCIDEVICEID)lParam) != m_wDeviceID.end()) {
				MCI_GENERIC_PARMS mci;
				mci.dwCallback = MAKELONG(m_hwndPage, 0);
				VERIFY(!mciSendCommand((MCIDEVICEID)lParam, MCI_CLOSE, MCI_WAIT, (DWORD_PTR)&mci));
				m_wDeviceID.erase((MCIDEVICEID)lParam);
			} else {
				ASSERT(false); // no way to here
			}
			break;
		}

	default: retval = __super::DlgProc(hWnd, message, wParam, lParam);
	}
	return retval;
}

void CALLBACK JClient::Connect(bool getsetting)
{
	if (getsetting && jpPageServer) {
		TCHAR host[128], pass[64];
		ASSERT(jpPageServer);
		GetDlgItemText(jpPageServer->hwndPage, IDC_HOST, host, _countof(host));
		m_hostname = tstrToANSI(host);
		m_port = (u_short)GetDlgItemInt(jpPageServer->hwndPage, IDC_PORT, FALSE, FALSE);
		GetDlgItemText(jpPageServer->hwndPage, IDC_PASS, pass, _countof(pass));
		m_passwordNet = pass;

		// Checkup nick for future identification
		std::tstring nickbuf(m_metrics.uNameMaxLength, 0), nick;
		const TCHAR* msg;
		GetDlgItemText(jpPageServer->hwndPage, IDC_NICK, &nickbuf[0], (int)nickbuf.size()+1);
		nick = nickbuf.c_str();
		if (!JClient::CheckNick(nick, msg)) { // check content
			if (jpOnline != jpPageServer) ContactSel(getTabIndex(CRC_SERVER));
			DisplayMessage(jpPageServer->hwndNick, msg, MAKEINTRESOURCE(IDS_MSG_NICKERROR), 2);
			return;
		}
	}
	// Add into socket manager
	Link link;
	hostent* h = gethostbyname(m_hostname.c_str());
	u_long addr = h ? *(u_long*)h->h_addr : htonl(INADDR_LOOPBACK);
	link.m_saAddr.sin_family = AF_INET;
	link.m_saAddr.sin_addr.S_un.S_addr = addr;
	link.m_saAddr.sin_port = htons(m_port);
	link.Select(FD_CONNECT | FD_READ | FD_WRITE | FD_CLOSE, m_hwndPage, BEM_NETWORK);
	link.Connect();
	InsertLink(link);
	m_clientsock = link.Sock;
	m_nConnectCount++;

	KillTimer(m_hwndPage, IDT_CONNECT);

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
		jpPageServer->Enable();
		EnableWindow(jpPageServer->hwndNick, FALSE);
	}
}

void CALLBACK JClient::saveAutoopen() const
{
	int i = 0;
	for each (MapPageChannel::value_type const& v in mPageChannel) {
		Profile::WriteInt(RF_AUTOOPEN, tformat(TEXT("type%02i"), i).c_str(), (UINT)v.second->gettype());
		Profile::WriteString(RF_AUTOOPEN, tformat(TEXT("name%02i"), i).c_str(), v.second->m_channel.name.c_str());
		Profile::WriteString(RF_AUTOOPEN, tformat(TEXT("pass%02i"), i).c_str(), v.second->m_channel.password.c_str());
		i++;
	}
	Profile::WriteInt(RF_AUTOOPEN, RK_CHANCOUNT, i);
}

void CALLBACK JClient::openAutoopen()
{
	if (Profile::GetInt(RF_AUTOOPEN, RK_USEAUTOOPEN, FALSE)) {
		int count = Profile::GetInt(RF_AUTOOPEN, RK_CHANCOUNT, 0);
		EContact type;
		std::tstring name, pass;
		for (int i = 0; i < count; i++) {
			type = (EContact)Profile::GetInt(RF_AUTOOPEN, tformat(TEXT("type%02i"), i).c_str(), (UINT)eChannel);
			name = Profile::GetString(RF_AUTOOPEN, tformat(TEXT("name%02i"), i).c_str(), TEXT("main"));
			pass = Profile::GetString(RF_AUTOOPEN, tformat(TEXT("pass%02i"), i).c_str(), TEXT(""));
			Send_Quest_JOIN(m_clientsock, name, pass, type);
		}
	}
}

int  CALLBACK JClient::ContactAdd(const std::tstring& name, DWORD id, EContact type)
{
	if (m_mUser[m_idOwn].accessibility.fFlashPageNew
		&& Profile::GetInt(RF_CLIENT, RK_FLASHPAGENEW, TRUE)
		&& !m_mUser[m_idOwn].isOnline)
		FlashWindow(m_hwndPage, TRUE);

	JPtr<JPage> jp = getPage(id);
	int pos;
	if (jp) {
		pos = getTabIndex(id);
	} else {
		ASSERT(getTabIndex(id) < 0);

		switch (type)
		{
		case eServer:
			{
				pos = 0;
				jp = jpPageServer = new JPageServer;
				break;
			}
		case eList:
			{
				pos = (jpPageServer ? 1 : 0);
				jp = jpPageList = new JPageList;
				break;
			}
		case eUser:
			{
				pos = (jpPageServer ? 1 : 0) + (jpPageList ? 1 : 0) +
					(int)mPageChannel.size() + (int)mPageUser.size();
				jp = mPageUser[id] = new JPageUser(id, name);
				break;
			}
		case eChannel:
			{
				pos = (jpPageServer ? 1 : 0) + (jpPageList ? 1 : 0) +
					(int)mPageChannel.size();
				jp = mPageChannel[id] = new JPageChannel(id, name);
				break;
			}
		default:
			return -1;
		}

		ASSERT(jp);
		jp->SetSource(this);
		jp->SetupHooks();
		CreateDialogParam(JClientApp::jpApp->hinstApp, jp->Template(), m_hwndPage, (DLGPROC)JDialog::DlgProcStub, (LPARAM)(JDialog*)jp);

		RECT rcMain;
		GetClientRect(m_hwndPage, &rcMain);
		if (!EqualRect(&m_rcPage, &rcMain)) {
			RECT rc;
			int cx = rcMain.right - rcMain.left, cy = rcMain.bottom - rcMain.top;
			HDWP hdwp = BeginDeferWindowPos(1);
			SetRect(&rc, jp->rcPageNC.left, jp->rcPageNC.top,
				cx - rcPage.right + jp->rcPageNC.right,
				cy - rcPage.bottom + jp->rcPageNC.bottom);
			DeferWindowPos(hdwp, jp->hwndPage, 0, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, SWP_NOMOVE | SWP_NOREPOSITION | SWP_NOZORDER);
			EndDeferWindowPos(hdwp);
		}

		TCITEM tci;
		tci.mask = TCIF_TEXT | TCIF_PARAM | TCIF_IMAGE;
		tci.dwState = 0;
		tci.dwStateMask = 0;
		tci.pszText = (TCHAR*)name.c_str();
		tci.cchTextMax = (int)name.length();
		tci.iImage = jp->ImageIndex();
		tci.lParam = jp->getID();
		TabCtrl_InsertItem(m_hwndTab, pos, &tci);
		if (jpOnline) VERIFY(InvalidateRect(jpOnline->hwndPage, 0, TRUE));
	}

	jp->setAlert(eRed);
	m_clientsock ? jp->Enable() : jp->Disable();
	return pos;
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
	if (jpOnline == jp) jpOnline = 0;
	jp = 0;

	int i = getTabIndex(id);
	if (i >= 0) TabCtrl_DeleteItem(m_hwndTab, i);
}

void CALLBACK JClient::ContactSel(int index)
{
	ASSERT(index >= 0 && index < TabCtrl_GetItemCount(m_hwndTab));

	TabCtrl_SetCurSel(m_hwndTab, index);

	TCITEM tci;
	tci.mask = TCIF_PARAM;
	VERIFY(TabCtrl_GetItem(m_hwndTab, index, &tci));
	JPtr<JPage> jp = getPage((DWORD)tci.lParam);
	ShowWindow(jp->hwndPage, SW_SHOW);
	if (jpOnline && jpOnline != jp) ShowWindow(jpOnline->hwndPage, SW_HIDE);
	jp->activate();
	jpOnline = jp;

	// Hide any baloon content
	HideBaloon();
	// Set main window topic
	ShowTopic(jp->gettopic());
	// Set default focus control
	HWND focus = jp->getDefFocusWnd();
	if (focus && m_mUser[m_idOwn].isOnline && GetFocus() != m_hwndTab) SetFocus(focus);
}

void CALLBACK JClient::ContactRename(DWORD idOld, const std::tstring& oldname, DWORD idNew, const std::tstring& newname)
{
	if (m_mUser[m_idOwn].opened.find(idOld) != m_mUser[m_idOwn].opened.end()) {
		m_mUser[m_idOwn].opened.insert(idNew);
		m_mUser[m_idOwn].opened.erase(idOld);
	}
	if (m_mUser.find(idOld) != m_mUser.end()) {
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
		ipu->second->AppendScript(tformat(TEXT("[style=Info][b]%s[/b] is now known as [b]%s[/b][/style]"), oldname.c_str(), newname.c_str()));
		ipu->second->setAlert(eYellow);
		ipu->second->rename(idNew, newname);
		mPageUser[idNew] = ipu->second;
		mPageUser.erase(idOld);
	} else {
		MapPageChannel::iterator ipc = mPageChannel.find(idOld);
		if (ipc != mPageChannel.end()) {
			if (!ipc->second->m_channel.isAnonymous) {
				ipc->second->AppendScript(tformat(TEXT("[style=Info]channel name is now [b]%s[/b][/style]"), newname.c_str()));
				ipc->second->setAlert(eYellow);
			}
			ipc->second->rename(idNew, newname);
			mPageChannel[idNew] = ipc->second;
			mPageChannel.erase(idOld);
		}
	}

	int i = getTabIndex(idOld);
	if (i >= 0) {
		TCITEM tci;
		tci.mask = TCIF_TEXT | TCIF_PARAM;
		tci.pszText = (TCHAR*)newname.c_str();
		tci.cchTextMax = (int)newname.length();
		tci.lParam = idNew;
		VERIFY(TabCtrl_SetItem(m_hwndTab, i, &tci));
		if (jpOnline) VERIFY(InvalidateRect(jpOnline->hwndPage, 0, TRUE));
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
	for (i = TabCtrl_GetItemCount(m_hwndTab) - 1; i >= 0; i--) {
		tci.mask = TCIF_PARAM;
		VERIFY(TabCtrl_GetItem(m_hwndTab, i, &tci));
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

JPtr<JClient::JPageLog> CALLBACK JClient::getPageLog(DWORD id)
{
	if (id == CRC_SERVER)
		return jpPageServer;
	else if (mPageUser.find(id) != mPageUser.end())
		return mPageUser[id];
	else if (mPageChannel.find(id) != mPageChannel.end())
		return mPageChannel[id];
	else return 0;
}

bool CALLBACK JClient::CheckNick(std::tstring& nick, const TCHAR*& msg)
{
	for each (std::tstring::value_type const& v in nick) {
		if (v < TEXT(' ')) {
			msg = MAKEINTRESOURCE(IDS_MSG_NICKNONPRINT);
			return false;
		}
	}
	while (!nick.empty() && *nick.begin() == TEXT(' ')) nick.erase(nick.begin());
	while (!nick.empty() && *(nick.end()-1) == TEXT(' ')) nick.erase(nick.end()-1);
	if (nick.empty()) {
		msg = MAKEINTRESOURCE(IDS_MSG_NICKEMPTY);
		return false;
	}
	msg = MAKEINTRESOURCE(IDS_MSG_NICKVALID);
	return true;
}

void CALLBACK JClient::ShowTopic(const std::tstring& topic)
{
	SetWindowText(m_hwndPage, tformat(TEXT("[%s] - Colibri Chat"), topic.c_str()).c_str());
}

void CALLBACK JClient::DisplayMessage(HWND hwnd, const TCHAR* msg, const TCHAR* title, int icon, COLORREF cr)
{
	RECT r;
	VERIFY(GetWindowRect(hwnd, &r));
	POINT p;
	p.x = (r.left + r.right)/2, p.y = (r.top + r.bottom)/2;
	ShowBaloon(p, m_hwndPage, msg, title, (HICON)(INT_PTR)icon, cr);
}

void CALLBACK JClient::ShowBaloon(const POINT& p, HWND hwndId, const TCHAR* msg, const TCHAR* title, HICON hicon, COLORREF cr)
{
	TOOLINFO ti;
	static TCHAR bufmsg[1024], buftitle[100];
	if (HIWORD(msg)) _tcscpy_s(bufmsg, _countof(bufmsg), msg);
	else LoadString(JClientApp::jpApp->hinstApp, LOWORD(msg), bufmsg, _countof(bufmsg));
	ti.cbSize = sizeof(ti);
	ti.hwnd = m_hwndPage;
	ti.uId = (UINT_PTR)hwndId;
	ti.hinst = JClientApp::jpApp->hinstApp;
	ti.lpszText = bufmsg;
	SendMessage(m_hwndBaloon, TTM_UPDATETIPTEXT, 0, (LPARAM)&ti);
	if (HIWORD(title)) _tcscpy_s(buftitle, _countof(buftitle), title);
	else LoadString(JClientApp::jpApp->hinstApp, LOWORD(title), buftitle, _countof(buftitle));
	VERIFY(SendMessage(m_hwndBaloon, TTM_SETTITLE, (WPARAM)hicon, (LPARAM)buftitle));
	SendMessage(m_hwndBaloon, TTM_SETTIPTEXTCOLOR, cr, 0);
	SendMessage(m_hwndBaloon, TTM_TRACKPOSITION, 0, MAKELPARAM(p.x, p.y));
	SendMessage(m_hwndBaloon, TTM_TRACKACTIVATE, TRUE, (LPARAM)&ti);
	m_isBaloon = hwndId;
	SetTimer(hwndPage, IDT_BALOONPOP, TIMER_BALOONPOP, 0);
}

void CALLBACK JClient::HideBaloon(HWND hwnd)
{
	if (hwnd || m_isBaloon) {
		TOOLINFO ti;
		ti.cbSize = sizeof(ti);
		ti.hwnd = m_hwndPage;
		ti.uId = (UINT_PTR)(hwnd ? hwnd : m_isBaloon);
		SendMessage(m_hwndBaloon, TTM_TRACKACTIVATE, FALSE, (LPARAM)&ti);
		m_isBaloon = 0;
		KillTimer(m_hwndPage, IDT_BALOONPOP);
	}
}

void CALLBACK JClient::PlaySound(const TCHAR* snd)
{
	MCI_WAVE_OPEN_PARMS mci1;
	mci1.dwCallback = MAKELONG(m_hwndPage, 0);
	mci1.wDeviceID = 0;
	mci1.lpstrDeviceType = (LPCTSTR)MCI_DEVTYPE_WAVEFORM_AUDIO;
	mci1.lpstrElementName = snd;
	mci1.lpstrAlias = 0;
	mci1.dwBufferSeconds = 4;
	if (!mciSendCommand(0, MCI_OPEN,
		MCI_WAIT | MCI_OPEN_TYPE | MCI_OPEN_TYPE_ID | MCI_OPEN_ELEMENT | MCI_WAVE_OPEN_BUFFER, (DWORD_PTR)&mci1))
	{
		MCI_PLAY_PARMS mci2;
		mci2.dwCallback = MAKELONG(m_hwndPage, 0);
		mci2.dwFrom = 0;
		mci2.dwTo = 0;
		if (!mciSendCommand(mci1.wDeviceID, MCI_PLAY, MCI_NOTIFY, (DWORD_PTR)&mci2))
		{
			ASSERT(m_wDeviceID.find(mci1.wDeviceID) == m_wDeviceID.end()); // ID must be unical
			m_wDeviceID.insert(mci1.wDeviceID);
		} else {
			MCI_GENERIC_PARMS mci3;
			mci3.dwCallback = MAKELONG(m_hwndPage, 0);
			mciSendCommand(mci1.wDeviceID, MCI_CLOSE, MCI_WAIT, (DWORD_PTR)&mci3);
		}
	}
}

std::tstring JClient::getSafeName(DWORD idUser) const
{
	switch (idUser)
	{
	case CRC_SERVER:
		return NAME_SERVER;
	case CRC_LIST:
		return NAME_LIST;
	case CRC_NONAME:
		return NAME_NONAME;
	case CRC_ANONYMOUS:
		return NAME_ANONYMOUS;
	case CRC_GOD:
		return NAME_GOD;
	case CRC_DEVIL:
		return NAME_DEVIL;
	default:
		MapUser::const_iterator iu = m_mUser.find(idUser);
		if (iu == m_mUser.end() || iu->second.name.empty())
			return tformat(TEXT("0x%08X"), idUser);
		else
			return iu->second.name;
	}
}

bool CALLBACK JClient::isGod(DWORD idUser) const
{
	MapUser::const_iterator iu = m_mUser.find(idUser != CRC_NONAME ? idUser : m_idOwn);
	return iu != m_mUser.end() && iu->second.cheat.isGod;
}

bool CALLBACK JClient::isDevil(DWORD idUser) const
{
	MapUser::const_iterator iu = m_mUser.find(idUser != CRC_NONAME ? idUser : m_idOwn);
	return iu != m_mUser.end() && iu->second.cheat.isDevil;
}

bool CALLBACK JClient::isCheats(DWORD idUser) const
{
	MapUser::const_iterator iu = m_mUser.find(idUser != CRC_NONAME ? idUser : m_idOwn);
	return iu != m_mUser.end() && (iu->second.cheat.isGod || iu->second.cheat.isDevil);
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

	EvNick += MakeDelegate(this, &JClient::OnNick);
}

void JClient::OnUnhook(JEventable* src)
{
	using namespace fastdelegate;

	EvNick -= MakeDelegate(this, &JClient::OnNick);

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
		(m_clientsock, m_passwordNet.c_str());

	// Lua response
	if (m_luaEvents) {
		lua_getglobal(m_luaEvents, "onLinkConnect");
		ASSERT(lua_gettop(m_luaEvents) == 1);
		lua_call(m_luaEvents, 0, 0);
	} else {
		m_nConnectCount = 0;
	}
}

void JClient::OnLinkClose(SOCKET sock, UINT err)
{
	if (sock == m_clientsock) {
		m_clientsock = 0; // not connected

		User user = m_mUser[m_idOwn];
		user.opened.clear();
		user.opened.insert(CRC_SERVER); // make it permanent
		m_mUser.clear();
		m_idOwn = CRC_NONAME;
		m_mUser[m_idOwn] = user;
	}

	__super::OnLinkClose(sock, err);

	// Lua response
	if (m_luaEvents) {
		lua_getglobal(m_luaEvents, "onLinkClose");
		lua_pushinteger(m_luaEvents, err);
		ASSERT(lua_gettop(m_luaEvents) == 2);
		lua_call(m_luaEvents, 1, 0);
	} else {
		if (err && m_bReconnect) Connect();
		else if (jpPageServer->hwndPage) {
			CheckDlgButton(jpPageServer->hwndPage, IDC_CONNECT, FALSE);
		}

		// Update interface
		EvLog(tformat(TEXT("[style=msg]Disconnected. Reason: [i]%s[/i][/style]"), JClient::s_mapWsaErr[err].c_str()), true);
	}
}

void JClient::OnLinkFail(SOCKET sock, UINT err)
{
	if (sock == m_clientsock) {
		m_clientsock = 0; // not connected
	}

	__super::OnLinkFail(sock, err);

	// Lua response
	if (m_luaEvents) {
		lua_getglobal(m_luaEvents, "onLinkFail");
		lua_pushinteger(m_luaEvents, err);
		ASSERT(lua_gettop(m_luaEvents) == 2);
		lua_call(m_luaEvents, 1, 0);
	} else {
		// Update interface
		EvLog(tformat(TEXT("[style=msg]Connecting failed. Reason: [i]%s[/i][/style]"), JClient::s_mapWsaErr[err].c_str()), true);
		// If not disconnected by user, try to reconnect again
		if (err && m_bReconnect) {
			SetTimer(m_hwndPage, IDT_CONNECT, Profile::GetInt(RF_CLIENT, RK_TIMERCONNECT, TIMER_CONNECT), 0);
			// Update interface
			EvLog(tformat(TEXT("[style=msg]Waiting %u seconds and try again (attempt #%i).[/style]"), TIMER_CONNECT/1000, m_nConnectCount), true);
		}
	}
}

void JClient::OnLinkStart(SOCKET sock)
{
	std::tstring nickbuf(m_metrics.uNameMaxLength, 0), nick;
	GetDlgItemText(jpPageServer->hwndPage, IDC_NICK, &nickbuf[0], (int)nickbuf.size()+1);
	nick = nickbuf.c_str();
	Send_Cmd_NICK(sock, m_idOwn, nick);

	EUserStatus stat = (EUserStatus)SendDlgItemMessage(jpPageServer->hwndPage, IDC_STATUS, CB_GETCURSEL, 0, 0);
	int img = (int)SendDlgItemMessage(jpPageServer->hwndPage, IDC_STATUSIMG, CB_GETCURSEL, 0, 0);
	std::tstring msg(m_metrics.uStatusMsgMaxLength, 0);
	GetDlgItemText(jpPageServer->hwndPage, IDC_STATUSMSG, &msg[0], (int)msg.size()+1);
	Send_Cmd_STATUS(sock, stat, m_mAlert[stat], img, msg);

	// Lua response
	if (m_luaEvents) {
		lua_getglobal(m_luaEvents, "onLinkStart");
		ASSERT(lua_gettop(m_luaEvents) == 1);
		lua_call(m_luaEvents, 0, 0);
	}
}

void JClient::OnTransactionProcess(SOCKET sock, WORD message, WORD trnid, io::mem is)
{
	typedef void (CALLBACK JClient::*TrnParser)(SOCKET, WORD, io::mem&);
	struct {
		WORD message;
		TrnParser parser;
	} responseTable[] =
	{
		{NOTIFY(CCPM_METRICS), &JClient::Recv_Notify_METRICS},
		{REPLY(CCPM_NICK), &JClient::Recv_Notify_NICK},
		{NOTIFY(CCPM_NICK), &JClient::Recv_Notify_NICK},
		{REPLY(CCPM_JOIN), &JClient::Recv_Reply_JOIN},
		{NOTIFY(CCPM_JOIN), &JClient::Recv_Notify_JOIN},
		{NOTIFY(CCPM_PART), &JClient::Recv_Notify_PART},
		{REPLY(CCPM_USERINFO), &JClient::Recv_Reply_USERINFO},
		{NOTIFY(CCPM_ONLINE), &JClient::Recv_Notify_ONLINE},
		{NOTIFY(CCPM_STATUS), &JClient::Recv_Notify_STATUS},
		{NOTIFY(CCPM_SAY), &JClient::Recv_Notify_SAY},
		{NOTIFY(CCPM_TOPIC), &JClient::Recv_Notify_TOPIC},
		{NOTIFY(CCPM_CHANOPTIONS), &JClient::Recv_Notify_CHANOPTIONS},
		{NOTIFY(CCPM_ACCESS), &JClient::Recv_Notify_ACCESS},
		{REPLY(CCPM_MESSAGE), &JClient::Recv_Reply_MESSAGE},
		{NOTIFY(CCPM_MESSAGE), &JClient::Recv_Notify_MESSAGE},
		{NOTIFY(CCPM_BEEP), &JClient::Recv_Notify_BEEP},
		{NOTIFY(CCPM_CLIPBOARD), &JClient::Recv_Notify_CLIPBOARD},
		{NOTIFY(CCPM_SPLASHRTF), &JClient::Recv_Notify_SPLASHRTF},
	};
	for (int i = 0; i < _countof(responseTable); i++) {
		if (responseTable[i].message == message) {
			(this->*responseTable[i].parser)(sock, trnid, is);
			return;
		}
	}

	__super::OnTransactionProcess(sock, message, trnid, is);
}

void JClient::OnNick(DWORD idOld, const std::tstring& oldname, DWORD idNew, const std::tstring& newname)
{
	ContactRename(idOld, oldname, idNew, newname);
}

//
// Beowolf Network Protocol Messages reciving
//

void CALLBACK JClient::Recv_Notify_METRICS(SOCKET sock, WORD trnid, io::mem& is)
{
	try
	{
		io::unpack(is, m_metrics);
	}
	catch (io::exception e)
	{
		switch (e.count)
		{
		case 0:
			// Report about message
			EvReport(SZ_BADTRN, eWarning, eLow);
			return;
		}
	}

	EvMetrics(m_metrics);
	EvReport(TEXT("metrics from server"), eInformation, eLow);
}

void CALLBACK JClient::Recv_Notify_NICK(SOCKET sock, WORD trnid, io::mem& is)
{
	DWORD result;
	DWORD idOld, idNew;
	std::tstring newname, oldname;

	try
	{
		io::unpack(is, result);
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
		case 3:
			// Report about message
			EvReport(SZ_BADTRN, eWarning, eLow);
			return;
		}
	}

	ASSERT(idOld != idNew);
	bool isOwn = idOld == m_idOwn || idOld == CRC_NONAME;
	oldname = getSafeName(idOld);

	EvNick(idOld, oldname, idNew, newname);

	if (isOwn) {
		// Update interface
		SetWindowText(jpPageServer->hwndNick, newname.c_str());

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
		EvReport(tformat(TEXT("%s, now nickname is [b]%s[/b]"), msg, newname.c_str()), eInformation, eHigh);
	}

	// Lua response
	if (m_luaEvents) {
		if (isOwn) {
			lua_getglobal(m_luaEvents, "onNickOwn");
			lua_pushstring(m_luaEvents, tstrToANSI(newname).c_str());
			ASSERT(lua_gettop(m_luaEvents) == 2);
			lua_call(m_luaEvents, 1, 0);
		}
		{ // in any case
			lua_getglobal(m_luaEvents, "onNick");
			lua_pushstring(m_luaEvents, tstrToANSI(oldname).c_str());
			lua_pushstring(m_luaEvents, tstrToANSI(newname).c_str());
			ASSERT(lua_gettop(m_luaEvents) == 3);
			lua_call(m_luaEvents, 2, 0);
		}
	}
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
					int pos = ContactAdd(user.name, ID, type);

					JPtr<JPageUser> jp = mPageUser[ID];
					ASSERT(jp);
					jp->setuser(user);

					ContactSel(pos);

					// Lua response
					if (m_luaEvents) {
						lua_getglobal(m_luaEvents, "onOpenPrivate");
						lua_pushstring(m_luaEvents, tstrToANSI(getSafeName(ID)).c_str());
						ASSERT(lua_gettop(m_luaEvents) == 2);
						lua_call(m_luaEvents, 1, 0);
					} else {
						jp->AppendScript(tformat(TEXT("[style=Info]now talk with [b]%s[/b][/style]"), getSafeName(ID).c_str()));
					}

					EvReport(tformat(TEXT("opened private talk with [b]%s[/b]"), user.name.c_str()), eInformation, eHigher);
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
					int pos = ContactAdd(chan.name, ID, type);

					JPtr<JPageChannel> jp = mPageChannel[ID];
					ASSERT(jp);
					jp->setchannel(chan);

					ContactSel(pos);

					Send_Quest_USERINFO(sock, wanted);

					// Lua response
					if (m_luaEvents) {
						lua_getglobal(m_luaEvents, "onOpenChannel");
						lua_pushstring(m_luaEvents, tstrToANSI(chan.name).c_str());
						ASSERT(lua_gettop(m_luaEvents) == 2);
						lua_call(m_luaEvents, 1, 0);
					} else {
						jp->AppendScript(tformat(TEXT("[style=Info]now talking in [b]#%s[/b][/style]"), chan.name.c_str()));
					}

					EvReport(tformat(TEXT("joins to [b]#%s[/b] channel"), chan.name.c_str()), eInformation, eHigher);
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
			case CHAN_BADPASS:
				msg = TEXT("password does not satisfies");
				break;
			case CHAN_DENY:
				msg = TEXT("access denied, channel is invite-only");
				break;
			case CHAN_LIMIT:
				msg = TEXT("number of users exceeds the channel limits");
				break;
			case CHAN_ABSENT:
				msg = TEXT("contact is absent");
				break;
			default:
				msg = TEXT("can not join to contact");
				break;
			}
			EvReport(msg, eError, eHigher);
		}
	}
	catch (io::exception e)
	{
		// Report about message
		EvReport(SZ_BADTRN, eWarning, eLow);
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
			EvReport(SZ_BADTRN, eWarning, eLow);
			return;
		}
	}

	InsertUser(idWho, user);
	LinkUser(idWho, idWhere);
	if (idWhere == m_idOwn) { // private talk
		// Create interface
		ContactAdd(user.name, idWho, eUser);

		if (m_mUser[m_idOwn].accessibility.fPlayMessage)
			PlaySound(JClientApp::jpApp->strWavPrivate.c_str());

		// Lua response
		if (m_luaEvents) {
			lua_getglobal(m_luaEvents, "onJoinPrivate");
			lua_pushstring(m_luaEvents, tstrToANSI(getSafeName(idWho)).c_str());
			ASSERT(lua_gettop(m_luaEvents) == 2);
			lua_call(m_luaEvents, 1, 0);
		} else {
			JPtr<JPageUser> jp = mPageUser[idWho];
			ASSERT(jp);
			jp->AppendScript(tformat(TEXT("[style=Info][b]%s[/b] call you to private talk[/style]"), user.name.c_str()));
		}

		EvReport(tformat(TEXT("[b]%s[/b] opens private talk"),
			user.name.c_str()),
			eInformation, eAbove);
	} else { // channel
		MapPageChannel::iterator ipc = mPageChannel.find(idWhere);
		if (ipc != mPageChannel.end()) {
			ipc->second->Join(idWho);
			ipc->second->setAlert(eYellow);
			if (m_mUser[m_idOwn].accessibility.fPlayChatSounds)
				PlaySound(JClientApp::jpApp->strWavJoin.c_str());

			EvReport(tformat(TEXT("[b]%s[/b] joins to [b]%s[/b]"),
				getSafeName(idWho).c_str(),
				ipc->second->m_channel.name.c_str()),
				eInformation, eUnder);
		} else {
			Send_Cmd_PART(sock, m_idOwn, idWhere);
		}
	}
}

void CALLBACK JClient::Recv_Notify_PART(SOCKET sock, WORD trnid, io::mem& is)
{
	DWORD idWho, idWhere, idBy;

	try
	{
		io::unpack(is, idWho);
		io::unpack(is, idWhere);
		io::unpack(is, idBy);
	}
	catch (io::exception e)
	{
		switch (e.count)
		{
		case 0:
		case 1:
			// Report about message
			EvReport(SZ_BADTRN, eWarning, eLow);
			return;

		case 2:
			idBy = CRC_SERVER;
		}
	}

	MapPageUser::iterator ipu = mPageUser.find(idWhere);
	if (ipu != mPageUser.end()) { // private talk, can be already closed
		ipu->second->setAlert(eYellow);
		ipu->second->Disable();

		// Lua response
		if (m_luaEvents) {
			lua_getglobal(m_luaEvents, "onPartPrivate");
			lua_pushstring(m_luaEvents, tstrToANSI(getSafeName(idWho)).c_str());
			lua_pushstring(m_luaEvents, tstrToANSI(getSafeName(idBy)).c_str());
			ASSERT(lua_gettop(m_luaEvents) == 3);
			lua_call(m_luaEvents, 2, 0);
		} else {
			// Parting message
			std::tstring nick = getSafeName(idWho);
			std::tstring msg;
			if (idWho == idBy) {
				msg = tformat(TEXT("[style=Info][b]%s[/b] leave private talk[/style]"), nick.c_str());
			} else if (idBy == CRC_SERVER) {
				msg = tformat(TEXT("[style=Info][b]%s[/b] was disconnected[/style]"), nick.c_str());
			} else {
				std::tstring by = getSafeName(idBy);
				msg = tformat(TEXT("[style=Info][b]%s[/b] was kicked by [b]%s[/b][/style]"), nick.c_str(), by.c_str());
			}
			ipu->second->AppendScript(msg);
		}

		EvReport(tformat(TEXT("[style=Info]private with [b]%s[/b] closed[/style]"), getSafeName(idWho).c_str()), eInformation, eHigher);
	} else { // channel
		MapPageChannel::iterator ipc = mPageChannel.find(idWhere);
		if (ipc != mPageChannel.end()) {
			ipc->second->Part(idWho, idBy);
			ipc->second->setAlert(eYellow);
			if (m_mUser[m_idOwn].accessibility.fPlayChatSounds)
				PlaySound(JClientApp::jpApp->strWavPart.c_str());
			if (idWho == m_idOwn) {
				ipc->second->Disable();
			}
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
				ipc->second->redrawUser(id);
			}
		}
	}
	catch (io::exception e)
	{
		switch (e.count)
		{
		case 0:
			// Report about message
			EvReport(SZ_BADTRN, eWarning, eLow);
			return;
		}
	}

	// Report about message
	EvReport(tformat(TEXT("recieve users info")), eInformation, eLow);
}

void CALLBACK JClient::Recv_Notify_ONLINE(SOCKET sock, WORD trnid, io::mem& is)
{
	DWORD idWho;
	EOnline isOnline;
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
			isOnline = eOnline;
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
			ipc->second->redrawUser(idWho);
		}

		// Lua response
		if (m_luaEvents) {
			lua_getglobal(m_luaEvents, "onOnline");
			lua_pushstring(m_luaEvents, tstrToANSI(getSafeName(idWho)).c_str());
			lua_pushinteger(m_luaEvents, isOnline);
			ASSERT(lua_gettop(m_luaEvents) == 3);
			lua_call(m_luaEvents, 2, 0);
		}
	}

	// Report about message
	EvReport(tformat(TEXT("user %s is %s"), iu != m_mUser.end() ? iu->second.name.c_str() : TEXT("unknown"), isOnline ? TEXT("online") : TEXT("offline")), eInformation, eLowest);
}

void CALLBACK JClient::Recv_Notify_STATUS(SOCKET sock, WORD trnid, io::mem& is)
{
	DWORD idWho;
	WORD type;
	EUserStatus stat = eReady;
	Alert a = m_mAlert[stat];
	int img = 0;
	std::tstring msg;
	bool god = false;
	bool devil = false;

	try
	{
		io::unpack(is, idWho);
		io::unpack(is, type);
		if (type & STATUS_MODE) {
			io::unpack(is, stat);
			io::unpack(is, a);
		}
		if (type & STATUS_IMG) {
			io::unpack(is, img);
		}
		if (type & STATUS_MSG) {
			io::unpack(is, msg);
		}
		if (type & STATUS_GOD) {
			io::unpack(is, god);
		}
		if (type & STATUS_DEVIL) {
			io::unpack(is, devil);
		}
	}
	catch (io::exception e)
	{
		switch (e.count)
		{
		case 0:
		case 1:
			// Report about message
			EvReport(SZ_BADTRN, eWarning, eLow);
			return;
		}
	}

	MapUser::iterator iu = m_mUser.find(idWho);
	if (iu != m_mUser.end()) {
		if (type & STATUS_MODE) {
			iu->second.nStatus = stat;
			iu->second.accessibility = a;

			// Lua response
			if (m_luaEvents) {
				lua_getglobal(m_luaEvents, "onStatusMode");
				lua_pushstring(m_luaEvents, tstrToANSI(getSafeName(idWho)).c_str());
				lua_pushinteger(m_luaEvents, stat);
				ASSERT(lua_gettop(m_luaEvents) == 3);
				lua_call(m_luaEvents, 2, 0);
			}
		}
		if (type & STATUS_IMG) {
			iu->second.nStatusImg = img;

			// Lua response
			if (m_luaEvents) {
				lua_getglobal(m_luaEvents, "onStatusImage");
				lua_pushstring(m_luaEvents, tstrToANSI(getSafeName(idWho)).c_str());
				lua_pushinteger(m_luaEvents, img);
				ASSERT(lua_gettop(m_luaEvents) == 3);
				lua_call(m_luaEvents, 2, 0);
			}
		}
		if (type & STATUS_MSG) {
			iu->second.strStatus = msg;

			// Lua response
			if (m_luaEvents) {
				lua_getglobal(m_luaEvents, "onStatusMessage");
				lua_pushstring(m_luaEvents, tstrToANSI(getSafeName(idWho)).c_str());
				lua_pushstring(m_luaEvents, tstrToANSI(msg).c_str());
				ASSERT(lua_gettop(m_luaEvents) == 3);
				lua_call(m_luaEvents, 2, 0);
			}
		}
		if (type & STATUS_GOD) {
			iu->second.cheat.isGod = god;

			// Lua response
			if (m_luaEvents) {
				lua_getglobal(m_luaEvents, "onStatusGod");
				lua_pushstring(m_luaEvents, tstrToANSI(getSafeName(idWho)).c_str());
				lua_pushboolean(m_luaEvents, god);
				ASSERT(lua_gettop(m_luaEvents) == 3);
				lua_call(m_luaEvents, 2, 0);
			}
		}
		if (type & STATUS_DEVIL) {
			iu->second.cheat.isDevil = devil;

			// Lua response
			if (m_luaEvents) {
				lua_getglobal(m_luaEvents, "onStatusDevil");
				lua_pushstring(m_luaEvents, tstrToANSI(getSafeName(idWho)).c_str());
				lua_pushboolean(m_luaEvents, devil);
				ASSERT(lua_gettop(m_luaEvents) == 3);
				lua_call(m_luaEvents, 2, 0);
			}
		}

		MapPageChannel::iterator ipc = mPageChannel.find(jpOnline ? jpOnline->getID() : 0);
		if (ipc != mPageChannel.end()) {
			ipc->second->redrawUser(idWho);
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
			EvReport(SZ_BADTRN, eWarning, eLow);
			return;
		}
	}

	JPtr<JPageLog> jp;
	MapPageUser::iterator ipu = mPageUser.find(idWhere);
	if (ipu != mPageUser.end()) { // private talk
		jp = ipu->second;
		jp->setAlert(eRed);
		if (m_mUser[m_idOwn].accessibility.fFlashPageSayPrivate
			&& Profile::GetInt(RF_CLIENT, RK_FLASHPAGESAYPRIVATE, TRUE)
			&& !m_mUser[m_idOwn].isOnline)
			FlashWindow(m_hwndPage, TRUE);
		if (m_mUser[m_idOwn].accessibility.fPlayPrivateSounds)
			PlaySound(idWho != m_idOwn
			? JClientApp::jpApp->strWavPrivateline.c_str()
			: JClientApp::jpApp->strWavMeline.c_str());
	} else {
		MapPageChannel::iterator ipc = mPageChannel.find(idWhere);
		if (ipc != mPageChannel.end()) { // channel
			jp = ipc->second;
			jp->setAlert(eRed);
			if (m_mUser[m_idOwn].accessibility.fFlahPageSayChannel
				&& Profile::GetInt(RF_CLIENT, RK_FLASHPAGESAYCHANNEL, FALSE)
				&& !m_mUser[m_idOwn].isOnline)
				FlashWindow(m_hwndPage, TRUE);
			if (m_mUser[m_idOwn].accessibility.fPlayChatSounds)
				PlaySound(idWho != m_idOwn
				? JClientApp::jpApp->strWavChatline.c_str()
				: JClientApp::jpApp->strWavMeline.c_str());
		}
	}
	if (jp) {
		jp->Say(idWho, content);
	}
}

void CALLBACK JClient::Recv_Notify_TOPIC(SOCKET sock, WORD trnid, io::mem& is)
{
	DWORD idWho, idWhere;
	std::tstring topic;

	try
	{
		io::unpack(is, idWho);
		io::unpack(is, idWhere);
		io::unpack(is, topic);
	}
	catch (io::exception e)
	{
		switch (e.count)
		{
		case 0:
		case 1:
			// Report about message
			EvReport(SZ_BADTRN, eWarning, eLow);
			return;
		}
	}

	EvTopic(idWho, idWhere, topic);

	JPtr<JPageLog> jp;
	MapPageUser::iterator ipu = mPageUser.find(idWhere);
	if (ipu != mPageUser.end()) { // private talk
	} else {
		MapPageChannel::iterator ipc = mPageChannel.find(idWhere);
		if (ipc != mPageChannel.end()) { // channel
			jp = ipc->second;
			if (m_mUser[m_idOwn].accessibility.fFlashPageChangeTopic
				&& Profile::GetInt(RF_CLIENT, RK_FLASHPAGETOPIC, TRUE)
				&& !m_mUser[m_idOwn].isOnline)
				FlashWindow(m_hwndPage, TRUE);

			// Lua response
			if (m_luaEvents) {
				lua_getglobal(m_luaEvents, "onTopic");
				lua_pushstring(m_luaEvents, tstrToANSI(getSafeName(idWho)).c_str());
				lua_pushstring(m_luaEvents, tstrToANSI(ipc->second->m_channel.name).c_str());
				lua_pushstring(m_luaEvents, tstrToANSI(topic).c_str());
				ASSERT(lua_gettop(m_luaEvents) == 4);
				lua_call(m_luaEvents, 3, 0);
			} else {
				jp->AppendScript(tformat(TEXT("[style=Descr][color=%s]%s[/color] changes topic to:\n[u]%s[/u][/style]"),
					idWho != m_idOwn ? TEXT("red") : TEXT("blue"),
					getSafeName(idWho).c_str(),
					topic.c_str()));
			}
		}
	}
	if (jp) {
		if (jpOnline == jp) ShowTopic(jp->gettopic());
		if (m_mUser[m_idOwn].accessibility.fPlayChatSounds)
			PlaySound(JClientApp::jpApp->strWavTopic.c_str());
	}
}

void CALLBACK JClient::Recv_Notify_CHANOPTIONS(SOCKET sock, WORD trnid, io::mem& is)
{
	DWORD idWho, idWhere;
	int op;
	DWORD val;

	try
	{
		io::unpack(is, idWho);
		io::unpack(is, idWhere);
		io::unpack(is, op);
		io::unpack(is, val);
	}
	catch (io::exception e)
	{
		switch (e.count)
		{
		case 0:
		case 1:
			// Report about message
			EvReport(SZ_BADTRN, eWarning, eLow);
			return;

		case 2:
			op = CHANOP_ANONYMOUS;
			val = true;
			break;

		case 3:
			// Report about message
			EvReport(SZ_BADTRN, eWarning, eLow);
			return;
		}
	}

	JPtr<JPageLog> jp;
	std::tstring msg;
	MapPageUser::iterator ipu = mPageUser.find(idWhere);
	if (ipu != mPageUser.end()) { // private talk
		jp = ipu->second;

		ASSERT(op == CHANOP_BACKGROUND);
		SendMessage(ipu->second->hwndEdit, EM_SETBKGNDCOLOR, FALSE, (LPARAM)val);
		SendMessage(ipu->second->hwndLog, EM_SETBKGNDCOLOR, FALSE, (LPARAM)val);

		// Lua response
		if (m_luaEvents) {
			lua_getglobal(m_luaEvents, "onBackground");
			lua_pushstring(m_luaEvents, tstrToANSI(getSafeName(idWho)).c_str());
			lua_pushstring(m_luaEvents, tstrToANSI(getSafeName(idWhere)).c_str());
			lua_pushinteger(m_luaEvents, GetRValue(val));
			lua_pushinteger(m_luaEvents, GetGValue(val));
			lua_pushinteger(m_luaEvents, GetBValue(val));
			ASSERT(lua_gettop(m_luaEvents) == 6);
			lua_call(m_luaEvents, 5, 0);
		} else {
			msg = TEXT("changes sheet color");
		}
	} else {
		MapPageChannel::iterator ipc = mPageChannel.find(idWhere);
		if (ipc != mPageChannel.end()) { // channel
			jp = ipc->second;

			switch (op)
			{
				case CHANOP_AUTOSTATUS:
					ipc->second->m_channel.nAutoStatus = (EChanStatus)val;

					// Lua response
					if (m_luaEvents) {
						lua_getglobal(m_luaEvents, "onChanAutostatus");
						lua_pushstring(m_luaEvents, tstrToANSI(getSafeName(idWho)).c_str());
						lua_pushstring(m_luaEvents, tstrToANSI(ipc->second->m_channel.name).c_str());
						lua_pushinteger(m_luaEvents, val);
						ASSERT(lua_gettop(m_luaEvents) == 4);
						lua_call(m_luaEvents, 3, 0);
					} else {
						msg = TEXT("changes channel users entry status");
					}
					break;
				case CHANOP_LIMIT:
					ipc->second->m_channel.nLimit = (UINT)val;

					// Lua response
					if (m_luaEvents) {
						lua_getglobal(m_luaEvents, "onChanLimit");
						lua_pushstring(m_luaEvents, tstrToANSI(getSafeName(idWho)).c_str());
						lua_pushstring(m_luaEvents, tstrToANSI(ipc->second->m_channel.name).c_str());
						lua_pushinteger(m_luaEvents, val);
						ASSERT(lua_gettop(m_luaEvents) == 4);
						lua_call(m_luaEvents, 3, 0);
					} else {
						msg = tformat(TEXT("sets channel limit to %u users"), val);
					}
					break;
				case CHANOP_HIDDEN:
					ipc->second->m_channel.isHidden = val != 0;

					// Lua response
					if (m_luaEvents) {
						lua_getglobal(m_luaEvents, "onChanHidden");
						lua_pushstring(m_luaEvents, tstrToANSI(getSafeName(idWho)).c_str());
						lua_pushstring(m_luaEvents, tstrToANSI(ipc->second->m_channel.name).c_str());
						lua_pushboolean(m_luaEvents, val != 0);
						ASSERT(lua_gettop(m_luaEvents) == 4);
						lua_call(m_luaEvents, 3, 0);
					} else {
						msg = val
							? TEXT("sets channel as hidden")
							: TEXT("sets channel as visible");
					}
					break;
				case CHANOP_ANONYMOUS:
					ipc->second->m_channel.isAnonymous = val != 0;
					InvalidateRect(ipc->second->hwndList, 0, TRUE);

					// Lua response
					if (m_luaEvents) {
						lua_getglobal(m_luaEvents, "onChanAnonymous");
						lua_pushstring(m_luaEvents, tstrToANSI(getSafeName(idWho)).c_str());
						lua_pushstring(m_luaEvents, tstrToANSI(ipc->second->m_channel.name).c_str());
						lua_pushboolean(m_luaEvents, val != 0);
						ASSERT(lua_gettop(m_luaEvents) == 4);
						lua_call(m_luaEvents, 3, 0);
					} else {
						msg = val
							? TEXT("sets channel as anonymous")
							: TEXT("sets channel as not anonymous");
					}
					break;
				case CHANOP_BACKGROUND:
					ipc->second->m_channel.crBackground = (COLORREF)val;

					SendMessage(ipc->second->hwndEdit, EM_SETBKGNDCOLOR, FALSE, (LPARAM)val);
					SendMessage(ipc->second->hwndLog, EM_SETBKGNDCOLOR, FALSE, (LPARAM)val);
					ListView_SetBkColor(ipc->second->hwndList, val);
					InvalidateRect(ipc->second->hwndList, 0, TRUE);

					// Lua response
					if (m_luaEvents) {
						lua_getglobal(m_luaEvents, "onBackground");
						lua_pushstring(m_luaEvents, tstrToANSI(getSafeName(idWho)).c_str());
						lua_pushstring(m_luaEvents, tstrToANSI(ipc->second->m_channel.name).c_str());
						lua_pushinteger(m_luaEvents, GetRValue(val));
						lua_pushinteger(m_luaEvents, GetGValue(val));
						lua_pushinteger(m_luaEvents, GetBValue(val));
						ASSERT(lua_gettop(m_luaEvents) == 6);
						lua_call(m_luaEvents, 5, 0);
					} else {
						msg = TEXT("changes sheet color");
					}
					break;
			}
		}
	}
	if (jp && !msg.empty()) {
		jp->AppendScript(tformat(TEXT("[style=Descr][color=%s]%s[/color] %s[/style]"),
			idWho != m_idOwn ? TEXT("red") : TEXT("blue"),
			jp->getSafeName(idWho).c_str(),
			msg.c_str()));
	}
	if (m_mUser[m_idOwn].accessibility.fPlayChatSounds)
		PlaySound(JClientApp::jpApp->strWavTopic.c_str());
}

void CALLBACK JClient::Recv_Notify_ACCESS(SOCKET sock, WORD trnid, io::mem& is)
{
	DWORD idWho, idWhere, idBy;
	EChanStatus stat;

	try
	{
		io::unpack(is, idWho);
		io::unpack(is, idWhere);
		io::unpack(is, stat);
		io::unpack(is, idBy);
	}
	catch (io::exception e)
	{
		switch (e.count)
		{
		case 0:
		case 1:
			// Report about message
			EvReport(SZ_BADTRN, eWarning, eLow);
			return;
		}
	}

	JPtr<JPageLog> jp;
	MapPageUser::iterator ipu = mPageUser.find(idWhere);
	if (ipu != mPageUser.end()) { // private talk
	} else {
		MapPageChannel::iterator ipc = mPageChannel.find(idWhere);
		if (ipc != mPageChannel.end()) { // channel
			jp = ipc->second;
			ipc->second->m_channel.setStatus(idWho, stat);
			ipc->second->redrawUser(idWho);

			// Lua response
			if (m_luaEvents) {
				lua_getglobal(m_luaEvents, "onAccess");
				lua_pushstring(m_luaEvents, tstrToANSI(getSafeName(idWho)).c_str());
				lua_pushstring(m_luaEvents, tstrToANSI(ipc->second->m_channel.name).c_str());
				lua_pushinteger(m_luaEvents, stat);
				lua_pushstring(m_luaEvents, tstrToANSI(getSafeName(idBy)).c_str());
				ASSERT(lua_gettop(m_luaEvents) == 5);
				lua_call(m_luaEvents, 4, 0);
			} else {
				jp->AppendScript(tformat(TEXT("[style=Descr][color=%s]%s[/color] changed channel status to [b]%s[/b] by [color=%s]%s[/color][/style]"),
					idWho != m_idOwn ? TEXT("red") : TEXT("blue"), jp->getSafeName(idWho).c_str(),
					s_mapChanStatName[stat].c_str(),
					idBy != m_idOwn ? TEXT("red") : TEXT("blue"), jp->getSafeName(idBy).c_str()));
			}
		}
	}
	if (jp) {
		if (m_mUser[m_idOwn].accessibility.fPlayChatSounds)
			PlaySound(JClientApp::jpApp->strWavConfirm.c_str());
	}
}

void CALLBACK JClient::Recv_Reply_MESSAGE(SOCKET sock, WORD trnid, io::mem& is)
{
	DWORD idWho;
	UINT type;

	try
	{
		io::unpack(is, idWho);
		io::unpack(is, type);
	}
	catch (io::exception e)
	{
		switch (e.count)
		{
		case 0:
		case 1:
			// Report about message
			EvReport(SZ_BADTRN, eWarning, eLow);
			return;
		}
	}

	// Report about message
	std::tstring msg;
	switch (type)
	{
	case MESSAGE_IGNORE:
		msg = TEXT("message to [b]%s[/b] ignored");
		break;
	case MESSAGE_SENT:
		msg = TEXT("message sent to [b]%s[/b]");
		break;
	case MESSAGE_SAVED:
		msg = TEXT("message to [b]%s[/b] saved");
		break;
	}
	EvReport(tformat(msg.c_str(), getSafeName(idWho).c_str()), eInformation, eNormal);
}

void CALLBACK JClient::Recv_Notify_MESSAGE(SOCKET sock, WORD trnid, io::mem& is)
{
	DWORD idBy;
	FILETIME ft;
	DWORD dwRtfSize;
	const void* ptr;
	bool bCloseOnDisconnect;
	bool fAlert;
	COLORREF crSheet;

	try
	{
		io::unpack(is, idBy);
		io::unpack(is, ft);
		io::unpack(is, dwRtfSize);
		io::unpackptr(is, ptr, dwRtfSize);
		io::unpack(is, bCloseOnDisconnect);
		io::unpack(is, fAlert);
		io::unpack(is, crSheet);
	}
	catch (io::exception e)
	{
		switch (e.count)
		{
		case 0:
		case 1:
		case 2:
		case 3:
			// Report about message
			EvReport(SZ_BADTRN, eWarning, eLow);
			return;

		case 4:
			bCloseOnDisconnect = false;
		case 5:
			fAlert = false;
		case 6:
			crSheet = GetSysColor(COLOR_WINDOW);
		}
	}

	if (m_hwndPage)
	{
		JMessage* jp = new JMessage(this);
		jp->idWho = idBy;
		jp->content = std::string((const char*)ptr, dwRtfSize);
		jp->bCloseOnDisconnect = bCloseOnDisconnect;
		jp->fAlert = fAlert;
		jp->crSheet = crSheet;
		jp->ftRecv = ft;
		PostMessage(m_hwndPage, BEM_JDIALOG, IDD_MSGRECV, (LPARAM)(JDialog*)jp);
	}
	// Report about message
	EvReport(tformat(TEXT("%s from [b]%s[/b]"), fAlert ? TEXT("alert") : TEXT("message"), getSafeName(idBy).c_str()), eInformation, eNormal);
}

void CALLBACK JClient::Recv_Notify_BEEP(SOCKET sock, WORD trnid, io::mem& is)
{
	DWORD idBy;

	try
	{
		io::unpack(is, idBy);
	}
	catch (io::exception e)
	{
		switch (e.count)
		{
		case 0:
			// Report about message
			EvReport(SZ_BADTRN, eWarning, eLow);
			return;
		}
	}

	PlaySound(JClientApp::jpApp->strWavBeep.c_str());

	// Lua response
	if (m_luaEvents) {
		lua_getglobal(m_luaEvents, "onBeep");
		lua_pushstring(m_luaEvents, tstrToANSI(getSafeName(idBy)).c_str());
		ASSERT(lua_gettop(m_luaEvents) == 2);
		lua_call(m_luaEvents, 1, 0);
	}

	// Report about message
	EvReport(tformat(TEXT("sound signal from [b]%s[/b]"), getSafeName(idBy).c_str()), eInformation, eHigh);
}

void CALLBACK JClient::Recv_Notify_CLIPBOARD(SOCKET sock, WORD trnid, io::mem& is)
{
	DWORD idBy;

	try
	{
		io::unpack(is, idBy);
		if (OpenClipboard(m_hwndPage)) {
			UINT format;
			std::tstring name;
			HANDLE hMem;
			std::streamsize size;
			void* ptr;
			EmptyClipboard();
			while (io::unpack(is, format), format) {
				io::unpack(is, name);
				io::unpack(is, size);
				if (format >= 0xC000 && format <= 0xFFFF) {
					format = RegisterClipboardFormat(name.c_str());
				}
				if (format) {
					hMem = GlobalAlloc(GMEM_MOVEABLE, (SIZE_T)size);
					ptr = GlobalLock(hMem);
					if (ptr) {
						io::unpackmem(is, ptr, size);
						GlobalUnlock(hMem);
						SetClipboardData(format, hMem);
					} else {
						GlobalFree(hMem);
						io::skip(is, size);
					}
				} else {
					io::skip(is, size);
				}
			}
			VERIFY(CloseClipboard());

			if (m_mUser[m_idOwn].accessibility.fPlayClipboard)
				PlaySound(JClientApp::jpApp->strWavClipboard.c_str());

			// Lua response
			if (m_luaEvents) {
				lua_getglobal(m_luaEvents, "onClipboard");
				lua_pushstring(m_luaEvents, tstrToANSI(getSafeName(idBy)).c_str());
				ASSERT(lua_gettop(m_luaEvents) == 2);
				lua_call(m_luaEvents, 1, 0);
			}

			// Report about message
			EvReport(tformat(TEXT("recieve clipboard content from [b]%s[/b]"), getSafeName(idBy).c_str()), eInformation, eHigh);
		} else {
			// Report about message
			EvReport(tformat(TEXT("can not paste recieved clipboard content from [b]%s[/b]"), getSafeName(idBy).c_str()), eError, eHigh);
		}
	}
	catch (io::exception e)
	{
		switch (e.count)
		{
		case 0:
			// Report about message
			EvReport(SZ_BADTRN, eWarning, eLow);
			return;
		}
	}
}

void CALLBACK JClient::Recv_Notify_SPLASHRTF(SOCKET sock, WORD trnid, io::mem& is)
{
	DWORD idBy;
	DWORD dwRtfSize;
	const void* ptr;
	RECT rcPos;
	bool bCloseOnDisconnect;
	DWORD dwCanclose, dwAutoclose;
	bool fTransparent;
	COLORREF crSheet;

	try
	{
		io::unpack(is, idBy);
		io::unpack(is, dwRtfSize);
		io::unpackptr(is, ptr, dwRtfSize);
		io::unpack(is, rcPos);
		io::unpack(is, bCloseOnDisconnect);
		io::unpack(is, dwCanclose);
		io::unpack(is, dwAutoclose);
		io::unpack(is, fTransparent);
		io::unpack(is, crSheet);
	}
	catch (io::exception e)
	{
		switch (e.count)
		{
		case 0:
		case 1:
		case 2:
			// Report about message
			EvReport(SZ_BADTRN, eWarning, eLow);
			return;

		case 3:
			rcPos.left = rcPos.top = rcPos.right = rcPos.bottom = 0;
		case 4:
			bCloseOnDisconnect = true;
		case 5:
			dwCanclose = 2500;
		case 6:
			dwAutoclose = INFINITE;
		case 7:
			fTransparent = true;
		case 8:
			crSheet = GetSysColor(COLOR_WINDOW);
		}
	}

	if (m_hwndPage)
	{
		JSplash* jp = new JSplashRtf(this, (const char*)ptr, dwRtfSize);
		jp->trnid = trnid;
		*(&jp->rcPos) = rcPos;
		jp->bCloseOnDisconnect = bCloseOnDisconnect;
		jp->dwCanclose = dwCanclose;
		jp->dwAutoclose = dwAutoclose;
		jp->fTransparent = fTransparent;
		jp->crSheet = crSheet;
		PostMessage(m_hwndPage, BEM_JDIALOG, IDD_SPLASHRTF, (LPARAM)(JDialog*)jp);
	}
	// Report about message
	EvReport(tformat(TEXT("splash text from [b]%s[/b]"), getSafeName(idBy).c_str()), eInformation, eNormal);
}

//
// Beowolf Network Protocol Messages sending
//

void CALLBACK JClient::Send_Cmd_NICK(SOCKET sock, DWORD idWho, const std::tstring& nick)
{
	std::ostringstream os;
	io::pack(os, idWho);
	io::pack(os, nick);
	PushTrn(sock, COMMAND(CCPM_NICK), 0, os.str());
}

void CALLBACK JClient::Send_Quest_JOIN(SOCKET sock, const std::tstring& name, const std::tstring& pass, int type)
{
	std::ostringstream os;
	io::pack(os, name);
	io::pack(os, pass);
	io::pack(os, type);
	PushTrn(sock, QUEST(CCPM_JOIN), 0, os.str());
}

void CALLBACK JClient::Send_Cmd_PART(SOCKET sock, DWORD idWho, DWORD idWhere)
{
	std::ostringstream os;
	io::pack(os, idWho);
	io::pack(os, idWhere);
	PushTrn(sock, COMMAND(CCPM_PART), 0, os.str());
}

void CALLBACK JClient::Send_Quest_USERINFO(SOCKET sock, const SetId& set)
{
	std::ostringstream os;
	io::pack(os, set);
	PushTrn(sock, QUEST(CCPM_USERINFO), 0, os.str());
}

void CALLBACK JClient::Send_Cmd_ONLINE(SOCKET sock, EOnline online, DWORD id)
{
	std::ostringstream os;
	io::pack(os, online);
	io::pack(os, id);
	PushTrn(sock, COMMAND(CCPM_ONLINE), 0, os.str());
}

void CALLBACK JClient::Send_Cmd_STATUS_Mode(SOCKET sock, EUserStatus stat, const Alert& a)
{
	std::ostringstream os;
	io::pack(os, (WORD)STATUS_MODE);
	io::pack(os, stat);
	io::pack(os, a);
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

void CALLBACK JClient::Send_Cmd_STATUS(SOCKET sock, EUserStatus stat, const Alert& a, int img, const std::tstring& msg)
{
	std::ostringstream os;
	io::pack(os, (WORD)(STATUS_MODE | STATUS_IMG | STATUS_MSG));
	io::pack(os, stat);
	io::pack(os, a);
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

void CALLBACK JClient::Send_Cmd_TOPIC(SOCKET sock, DWORD idWhere, const std::tstring& topic)
{
	std::ostringstream os;
	io::pack(os, idWhere);
	io::pack(os, topic);
	PushTrn(sock, COMMAND(CCPM_TOPIC), 0, os.str());
}

void CALLBACK JClient::Send_Cmd_CHANOPTIONS(SOCKET sock, DWORD idWhere, int op, DWORD val)
{
	std::ostringstream os;
	io::pack(os, idWhere);
	io::pack(os, op);
	io::pack(os, val);
	PushTrn(sock, COMMAND(CCPM_CHANOPTIONS), 0, os.str());
}

void CALLBACK JClient::Send_Cmd_ACCESS(SOCKET sock, DWORD idWho, DWORD idWhere, EChanStatus stat)
{
	std::ostringstream os;
	io::pack(os, idWho);
	io::pack(os, idWhere);
	io::pack(os, stat);
	PushTrn(sock, COMMAND(CCPM_ACCESS), 0, os.str());
}

void CALLBACK JClient::Send_Quest_MESSAGE(SOCKET sock, DWORD idWho, const std::string& text, bool fAlert, COLORREF crSheet)
{
	std::ostringstream os;
	io::pack(os, idWho);
	io::pack(os, (DWORD)text.size());
	os.write(text.c_str(), (std::streamsize)text.size());
	io::pack(os, false);
	io::pack(os, fAlert);
	io::pack(os, crSheet);
	PushTrn(sock, QUEST(CCPM_MESSAGE), 0, os.str());
}

void CALLBACK JClient::Send_Cmd_BEEP(SOCKET sock, DWORD idWho)
{
	std::ostringstream os;
	io::pack(os, idWho);
	PushTrn(sock, COMMAND(CCPM_BEEP), 0, os.str());
}

void CALLBACK JClient::Send_Cmd_CLIPBOARD(SOCKET sock, DWORD idWho)
{
	std::ostringstream os;
	io::pack(os, idWho);
	if (!OpenClipboard(m_hwndPage)) return;
	UINT format = 0;
	TCHAR buffer[64];
	int len;
	HANDLE hMem;
	std::streamsize size;
	const char* ptr;
	while ((format = EnumClipboardFormats(format)) != 0) {
		len = GetClipboardFormatName(format, buffer, _countof(buffer));
		hMem = GetClipboardData(format);
		size = (std::streamsize)GlobalSize(hMem);
		ptr = (const char*)GlobalLock(hMem);
		if (ptr) {
			io::pack(os, format);
			io::pack(os, std::tstring(buffer, len));
			io::pack(os, size);
			os.write(ptr, size);

			GlobalUnlock(hMem);
		}
	}
	io::pack(os, (UINT)0); // write end marker
	VERIFY(CloseClipboard());
	PushTrn(sock, COMMAND(CCPM_CLIPBOARD), 0, os.str());
}

void CALLBACK JClient::Send_Cmd_SPLASHRTF(SOCKET sock, DWORD idWho, const std::string& text, const RECT& rcPos, bool bCloseOnDisconnect, DWORD dwCanclose, DWORD dwAutoclose, bool fTransparent, COLORREF crSheet)
{
	std::ostringstream os;
	io::pack(os, idWho);
	io::pack(os, (DWORD)text.size());
	os.write(text.c_str(), (std::streamsize)text.size());
	io::pack(os, rcPos);
	io::pack(os, bCloseOnDisconnect);
	io::pack(os, dwCanclose);
	io::pack(os, dwAutoclose);
	io::pack(os, fTransparent);
	io::pack(os, crSheet);
	PushTrn(sock, COMMAND(CCPM_SPLASHRTF), 0, os.str());
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
	m_haccelRichEdit = 0;

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

	// Waves
	m_strWavMeline = Profile::GetString(RF_SOUNDS, RK_WAVMELINE, TEXT("Sounds\\me_line.wav"));
	m_strWavChatline = Profile::GetString(RF_SOUNDS, RK_WAVCHATLINE, TEXT("Sounds\\chat_line.wav"));
	m_strWavConfirm = Profile::GetString(RF_SOUNDS, RK_WAVCONFIRM, TEXT("Sounds\\confirm.wav"));
	m_strWavPrivateline = Profile::GetString(RF_SOUNDS, RK_WAVPRIVATELINE, TEXT("Sounds\\chat_line.wav"));
	m_strWavTopic = Profile::GetString(RF_SOUNDS, RK_WAVTOPIC, TEXT("Sounds\\topic_change.wav"));
	m_strWavJoin = Profile::GetString(RF_SOUNDS, RK_WAVJOIN, TEXT("Sounds\\channel_join.wav"));
	m_strWavPart = Profile::GetString(RF_SOUNDS, RK_WAVPART, TEXT("Sounds\\channel_leave.wav"));
	m_strWavPrivate = Profile::GetString(RF_SOUNDS, RK_WAVPRIVATE, TEXT("Sounds\\private_start.wav"));
	m_strWavAlert = Profile::GetString(RF_SOUNDS, RK_WAVALERT, TEXT("Sounds\\alert.wav"));
	m_strWavMessage = Profile::GetString(RF_SOUNDS, RK_WAVMESSAGE, TEXT("Sounds\\message.wav"));
	m_strWavBeep = Profile::GetString(RF_SOUNDS, RK_WAVBEEP, TEXT("Sounds\\beep.wav"));
	m_strWavClipboard = Profile::GetString(RF_SOUNDS, RK_WAVCLIPBOARD, TEXT("Sounds\\clipboard.wav"));

	hiMain16 = LoadIcon(hinstApp, MAKEINTRESOURCE(IDI_SMALL));
	hiMain32 = LoadIcon(hinstApp, MAKEINTRESOURCE(IDI_CLIENT));

	// Load accel table for main window
	m_haccelMain = LoadAccelerators(hinstApp, MAKEINTRESOURCE(IDA_MAIN));
	m_haccelRichEdit = LoadAccelerators(hinstApp, MAKEINTRESOURCE(IDA_RICHEDIT));

	// Load popup menus
	m_hmenuTab = LoadMenu(hinstApp, MAKEINTRESOURCE(IDM_TAB));
	m_hmenuLog = LoadMenu(hinstApp, MAKEINTRESOURCE(IDM_LOG));
	m_hmenuChannel = LoadMenu(hinstApp, MAKEINTRESOURCE(IDM_CHANNEL));
	m_hmenuList = LoadMenu(hinstApp, MAKEINTRESOURCE(IDM_CHANLIST));
	m_hmenuRichEdit = LoadMenu(hinstApp, MAKEINTRESOURCE(IDM_RICHEDIT));
	m_hmenuUser = LoadMenu(hinstApp, MAKEINTRESOURCE(IDM_USER));
	m_hmenuUserGod = LoadMenu(hinstApp, MAKEINTRESOURCE(IDM_USERGOD));
	// Create the image list
	m_himlEdit = ImageList_LoadImage(hinstApp, MAKEINTRESOURCE(IDB_EDIT), 16, 12, CLR_DEFAULT, IMAGE_BITMAP, LR_CREATEDIBSECTION);
	m_himlTab = ImageList_LoadImage(hinstApp, MAKEINTRESOURCE(IDB_TAB), 16, 12, CLR_DEFAULT, IMAGE_BITMAP, LR_CREATEDIBSECTION);
	m_himlMan = ImageList_LoadImage(hinstApp, MAKEINTRESOURCE(IDB_MAN), 16, 8, CLR_DEFAULT, IMAGE_BITMAP, LR_CREATEDIBSECTION);
	m_himlStatus = ImageList_LoadImage(hinstApp, MAKEINTRESOURCE(IDB_STATUS), 16, 8, CLR_DEFAULT, IMAGE_BITMAP, LR_CREATEDIBSECTION);
	m_himlStatusImg = ImageList_LoadImage(hinstApp, MAKEINTRESOURCE(IDB_STATUSIMG), 16, 8, CLR_DEFAULT, IMAGE_BITMAP, LR_CREATEDIBSECTION);
	m_himgSend = LoadImage(hinstApp, MAKEINTRESOURCE(IDB_SEND), IMAGE_BITMAP, 60, 48, LR_LOADMAP3DCOLORS);
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
	VERIFY(DestroyMenu(m_hmenuTab));
	VERIFY(DestroyMenu(m_hmenuLog));
	VERIFY(DestroyMenu(m_hmenuChannel));
	VERIFY(DestroyMenu(m_hmenuList));
	VERIFY(DestroyMenu(m_hmenuRichEdit));
	VERIFY(DestroyMenu(m_hmenuUser));
	VERIFY(DestroyMenu(m_hmenuUserGod));
	// Destroy the image list
	VERIFY(ImageList_Destroy(m_himlEdit));
	VERIFY(ImageList_Destroy(m_himlTab));
	VERIFY(ImageList_Destroy(m_himlMan));
	VERIFY(ImageList_Destroy(m_himlStatus));
	VERIFY(ImageList_Destroy(m_himlStatusImg));
	VERIFY(DeleteObject(m_himgSend));
	VERIFY(DeleteObject(m_himgULBG));
	VERIFY(DeleteObject(m_himgULFoc));
	VERIFY(DeleteObject(m_himgULSel));
	VERIFY(DeleteObject(m_himgULHot));
	// Close all MCI devices
	VERIFY(!mciSendCommand(MCI_ALL_DEVICE_ID, MCI_CLOSE, MCI_WAIT, NULL));
	// Free RichEdit library
	VERIFY(FreeLibrary(hinstRichEdit));
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
		int retval = JClientApp::jpApp->Iteration();

		return retval;
	}
	catch (std::exception& e)
	{
		MessageBoxA(0, format("%s\r\n%s", typeid(e).name(), e.what()).c_str(), "Unhandled Exception!", MB_OK | MB_ICONSTOP);
	}
	return -1;
}

//-----------------------------------------------------------------------------

// The End.