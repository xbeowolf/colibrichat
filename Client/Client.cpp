
//-----------------------------------------------------------------------------

//
// Includes
//

#pragma region Includes

#include "stdafx.h"
#define _WINSOCK_DEPRECATED_NO_WARNINGS

// Common
#include "protocol.h"
#include "stylepr.h"
//#include "CRC.h"
#include "Profile.h"

// Project
#include "resource.h"
#include "client.h"

#pragma endregion

//-----------------------------------------------------------------------------

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
std::map<EUserStatus, std::tstring> JClient::s_mapUserStatName;
std::map<UINT, std::string> JClient::s_mapWsaErr;
HWND JClient::m_isBaloon = 0;
HWND JClient::m_hwndBaloon = 0;

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

	JClient::s_mapUserStatName[eReady] = TEXT("Ready");
	JClient::s_mapUserStatName[eDND] = TEXT("DND");
	JClient::s_mapUserStatName[eBusy] = TEXT("Busy");
	JClient::s_mapUserStatName[eNA] = TEXT("N/A");
	JClient::s_mapUserStatName[eAway] = TEXT("Away");
	JClient::s_mapUserStatName[eInvisible] = TEXT("Glass");

	//   WSA error codes
	// on FD_CONNECT
	JClient::s_mapWsaErr[WSAECONNREFUSED] = "The attempt to connect was rejected.";
	JClient::s_mapWsaErr[WSAENETUNREACH] = "The network cannot be reached from this host at this time.";
	JClient::s_mapWsaErr[WSAEMFILE] = "No more file descriptors are available.";
	JClient::s_mapWsaErr[WSAENOBUFS] = "No buffer space is available. The socket cannot be connected.";
	JClient::s_mapWsaErr[WSAENOTCONN] = "The socket is not connected.";
	JClient::s_mapWsaErr[WSAETIMEDOUT] = "Attempt to connect timed out without establishing a connection.";
	// on FD_CLOSE
	JClient::s_mapWsaErr[0] = "The connection was reset by software itself.";
	JClient::s_mapWsaErr[1] = "The connection reset by validate timeout.";
	JClient::s_mapWsaErr[2] = "The connection reset because was recieved transaction with bad CRC code";
	JClient::s_mapWsaErr[3] = "The connection reset because IP-address is banned.";
	JClient::s_mapWsaErr[WSAENETDOWN] = "The network subsystem failed."; // and all other
	JClient::s_mapWsaErr[WSAECONNRESET] = "The connection was reset by the remote side.";
	JClient::s_mapWsaErr[WSAECONNABORTED] = "The connection was terminated due to a time-out or other failure.";

	// Create baloon tool tip for users list
	_ASSERT(JClientApp::jpApp);
	m_hwndBaloon = CreateWindowEx(WS_EX_TOPMOST, TOOLTIPS_CLASS, 0,
		WS_POPUP | TTS_NOPREFIX | TTS_ALWAYSTIP | TTS_BALLOON,
		CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
		0, 0, JClientApp::jpApp->hinstApp, 0);
	_ASSERT(m_hwndBaloon);
	SendMessage(m_hwndBaloon, TTM_SETMAXTIPWIDTH, 0, 300);
}

void JClient::doneclass()
{
	DestroyWindow(m_hwndBaloon);
	m_hwndBaloon = 0;

	JClient::s_mapChanStatName.clear();
	JClient::s_mapWsaErr.clear();
}

JClient::JClient(lua_State* L)
: JLuaEngine<JBNB>(), JDialog(),
jpOnline(0)
{
	_ASSERT(L == 0);

	m_clientsock = 0;
	m_bReconnect = true;
	m_nConnectCount = 0;
	m_bSendByEnter = true;
	m_bCheatAnonymous = false;

	m_hwndTab = 0;

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

	m_encryptorname = ECRYPT_BINDEFAULT;
}

void JClient::lua_openVM()
{
	__super::lua_openVM();

	lua_State* L = m_luaVM;
	_ASSERT(L);
	// register Lua data
	lunareg_colibri(L);
	CLuaGluer<JClient>::Register(L);
	if (luaL_dofile(L, "events.client.lua") && lua_isstring(L, -1)) {
		throw std::exception(lua_tostring(L, -1));
	}
	_ASSERT(lua_gettop(L) == 0); // Lua stack must be empty
}

void JClient::beforeDestruct()
{
	__super::beforeDestruct();
}

void JClient::Init()
{
	__super::Init();

	jpOnline = 0;
	jpPageServer = 0;
	jpPageList = 0;
	// Clear pages
	mPageUser.clear();
	mPageChannel.clear();
}

void JClient::Done()
{
	__super::Done();
}

int  JClient::Run()
{
	__super::Run();

	return m_State;
}

void JClient::LoadState()
{
	m_idOwn = CRC_NONAME;

	User& user = m_mUser[m_idOwn];
	user.name = profile::getString(RF_CLIENT, RK_NICK, NAME_NONAME);
	const TCHAR* msg;
	if (!CheckNick(user.name, msg)) user.name = NAME_NONAME; // ensure that nick is valid
	user.opened.insert(CRC_SERVER); // make it permanent
	user.IP.S_un.S_addr = ntohl(INADDR_LOOPBACK);
	user.isOnline = eOnline;
	user.idOnline = 0;
	user.nStatus = (EUserStatus)profile::getInt(RF_CLIENT, RK_STATUS, eReady);
	user.accessibility = m_mAlert[user.nStatus];
	user.nStatusImg = profile::getInt(RF_CLIENT, RK_STATUSIMG, 0);
	user.strStatus = profile::getString(RF_CLIENT, RK_STATUSMSG, TEXT("ready to talk"));

	std::string name = tstr_to_utf8(getSafeName(m_idOwn));
	// Lua response
	{
		DOLUACS;
		lua_getmethod(L, "onNickOwn");
		if (lua_isfunction(L, -1)) {
			lua_insert(L, -2);
			lua_pushstring(L, name.c_str());
			lua_call(L, 2, 0);
		} else lua_pop(L, 2);
		lua_getmethod(L, "onOnline");
		if (lua_isfunction(L, -1)) {
			lua_insert(L, -2);
			lua_pushstring(L, name.c_str());
			lua_pushinteger(L, user.isOnline);
			lua_call(L, 3, 0);
		} else lua_pop(L, 2);
		lua_getmethod(L, "onStatusMode");
		if (lua_isfunction(L, -1)) {
			lua_insert(L, -2);
			lua_pushstring(L, name.c_str());
			lua_pushinteger(L, user.nStatus);
			pushAlert(L, user.accessibility);
			lua_call(L, 4, 0);
		} else lua_pop(L, 2);
		lua_getmethod(L, "onStatusImage");
		if (lua_isfunction(L, -1)) {
			lua_insert(L, -2);
			lua_pushstring(L, name.c_str());
			lua_pushinteger(L, user.nStatusImg);
			lua_call(L, 3, 0);
		} else lua_pop(L, 2);
		lua_getmethod(L, "onStatusMessage");
		if (lua_isfunction(L, -1)) {
			lua_insert(L, -2);
			lua_pushstring(L, name.c_str());
			lua_pushstring(L, tstr_to_utf8(user.strStatus).c_str());
			lua_call(L, 3, 0);
		} else lua_pop(L, 2);
	}

	s_nCompression = profile::getInt(RF_CLIENT, RK_COMPRESSION, -1);
	m_encryptorname = tstr_to_utf8(profile::getString(RF_CLIENT, RK_ENCRYPTALG, utf8_to_tstr(ECRYPT_BINDEFAULT)));

	m_hostname = tstr_to_utf8(profile::getString(RF_CLIENT, RK_HOST, TEXT("127.0.0.1")));
	m_port = (u_short)profile::getInt(RF_CLIENT, RK_PORT, CCP_PORT);
	m_passwordNet = profile::getString(RF_CLIENT, RK_PASSWORDNET, TEXT("beowolf"));
	m_bSendByEnter = profile::getInt(RF_CLIENT, RK_SENDBYENTER, true) != 0;
	m_bCheatAnonymous = profile::getInt(RF_CLIENT, RK_CHEATANONYMOUS, false) != 0;
}

void JClient::SaveState()
{
	profile::setString(RF_CLIENT, RK_HOST, utf8_to_tstr(m_hostname));
	profile::setInt(RF_CLIENT, RK_PORT, m_port);
	profile::setString(RF_CLIENT, RK_PASSWORDNET, m_passwordNet);
	profile::setInt(RF_CLIENT, RK_SENDBYENTER, m_bSendByEnter);

	User& user = m_mUser[m_idOwn];
	profile::setString(RF_CLIENT, RK_NICK, user.name);
	profile::setInt(RF_CLIENT, RK_STATUS, user.nStatus);
	profile::setInt(RF_CLIENT, RK_STATUSIMG, user.nStatusImg);
	profile::setString(RF_CLIENT, RK_STATUSMSG, user.strStatus);

	profile::setString(RF_CLIENT, RK_ENCRYPTALG, utf8_to_tstr(m_encryptorname));
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

			TOOLINFO ti;
			ti.cbSize = sizeof(ti);
			ti.uFlags = TTF_ABSOLUTE | TTF_IDISHWND | TTF_TRACK;
			ti.hwnd = hWnd;
			ti.uId = (UINT_PTR)hWnd;
			ti.hinst = JClientApp::jpApp->hinstApp;
			ti.lpszText = 0;
			_VERIFY(SendMessage(m_hwndBaloon, TTM_ADDTOOL, 0, (LPARAM)&ti));

			// Inits Tab control
			TabCtrl_SetImageList(m_hwndTab, JClientApp::jpApp->himlTab);

			ContactSel(ContactAdd(NAME_SERVER, CRC_SERVER, eServer));
			ContactAdd(NAME_LIST, CRC_LIST, eList);

			if (State != JService::eInit) Init();
			Run();

			// Lua response
			{
				DOLUACS;
				lua_getmethod(L, "wmCreate");
				if (lua_isfunction(L, -1)) {
					lua_insert(L, -2);
					lua_call(L, 1, 0);
				} else lua_pop(L, 2);
			}

			retval = TRUE;
			break;
		}

	case WM_DESTROY:
		{
			// Lua response
			{
				DOLUACS;
				lua_getmethod(L, "wmDestroy");
				if (lua_isfunction(L, -1)) {
					lua_insert(L, -2);
					lua_call(L, 1, 0);
				} else lua_pop(L, 2);
			}

			m_hwndTab = 0;

			BaloonHide(hWnd);
			TOOLINFO ti;
			ti.cbSize = sizeof(ti);
			ti.hwnd = hWnd;
			ti.uId = (UINT_PTR)hWnd;
			SendMessage(m_hwndBaloon, TTM_DELTOOL, 0, (LPARAM)&ti);

			// Close all opened waves
			MCI_GENERIC_PARMS mci;
			mci.dwCallback = MAKELONG(m_hwndPage, 0);
			for each (MCIDEVICEID const& v in m_wDeviceID) {
				_VERIFY(!mciSendCommand(v, MCI_CLOSE, MCI_WAIT, (DWORD_PTR)&mci));
			}

			Stop();

			JClientApp::jpApp->DelWindow(hWnd);
			break;
		}

	case WM_CLOSE:
		{
			// Lua response
			DOLUACS;
			lua_getmethod(L, "wmClose");
			if (lua_isfunction(L, -1)) {
				lua_insert(L, -2);
				lua_call(L, 1, 0);
			} else lua_pop(L, 2);
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
			DOLUACS;
			lua_getmethod(L, "wmEnterSizeMove");
			if (lua_isfunction(L, -1)) {
				lua_insert(L, -2);
				lua_call(L, 1, 0);
			} else lua_pop(L, 2);
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
			if (wParam != 0 && jpOnline) {
				jpOnline->activate();
				HWND focus = jpOnline->getDefFocusWnd();
				if (focus) SetFocus(focus);
			}
			if (m_clientsock) PushTrn(m_clientsock, Make_Cmd_ONLINE(wParam != 0 ? eOnline : eOffline, jpOnline ? jpOnline->getID() : 0));

			// Lua response
			{
				DOLUACS;
				lua_getmethod(L, "wmActivateApp");
				if (lua_isfunction(L, -1)) {
					lua_insert(L, -2);
					lua_pushboolean(L, wParam != 0);
					lua_call(L, 2, 0);
				} else lua_pop(L, 2);
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
			{
				DOLUACS;
				lua_getmethod(L, "wmCommand");
				if (lua_isfunction(L, -1)) {
					lua_insert(L, -2);
					lua_pushinteger(L, LOWORD(wParam));
					lua_call(L, 2, 0);
				} else lua_pop(L, 2);
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
						{
							std::tstring nickbuf(m_metrics.uNameMaxLength, 0), nick;
							const TCHAR* msg;
							GetDlgItemText(jpPageServer->hwndPage, IDC_NICK, &nickbuf[0], (int)nickbuf.size()+1);
							nick = nickbuf.c_str();
							if (JClient::CheckNick(nick, msg)) { // check content
								_ASSERT(!m_clientsock); // only for disconnected
								SendMessage(jpPageServer->hwndPage, WM_COMMAND, IDC_CONNECT, 0);
							} else {
								BaloonShow(jpPageServer->hwndNick, msg, MAKEINTRESOURCE(IDS_MSG_NICKERROR), 2);
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
								if (m_clientsock) PushTrn(m_clientsock, Make_Cmd_NICK(m_idOwn, nick));
								else if (!m_nConnectCount) SendMessage(jpPageServer->hwndPage, WM_COMMAND, IDC_CONNECT, 0);
							} else {
								BaloonShow(jpPageServer->hwndNick, msg, MAKEINTRESOURCE(IDS_MSG_NICKERROR), 2);
							}
							break;
						}

					case IDC_STATUSMSG:
						{
							std::tstring msg(m_metrics.uStatusMsgMaxLength, 0);
							GetDlgItemText(jpPageServer->hwndPage, IDC_STATUSMSG, &msg[0], (int)msg.size()+1);
							PushTrn(m_clientsock, Make_Cmd_STATUS_Msg(msg));
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
						BaloonShow(m_hwndTab, MAKEINTRESOURCE(IDS_MSG_TABCLOSE), 0, 2);
					} else {
						int sel;
						TCITEM tci;
						sel = TabCtrl_GetCurSel(m_hwndTab);
						tci.mask = TCIF_PARAM;
						_VERIFY(TabCtrl_GetItem(m_hwndTab, sel, &tci));
						ContactDel((DWORD)tci.lParam);
						ContactSel(min(sel, TabCtrl_GetItemCount(m_hwndTab) - 1));
					}
					break;
				}

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
					BaloonHide();
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
				_VERIFY(!mciSendCommand((MCIDEVICEID)lParam, MCI_CLOSE, MCI_WAIT, (DWORD_PTR)&mci));
				m_wDeviceID.erase((MCIDEVICEID)lParam);
			} else {
				_ASSERT(false); // no way to here
			}
			break;
		}

	default: retval = __super::DlgProc(hWnd, message, wParam, lParam);
	}
	return retval;
}

void JClient::Connect(bool getsetting)
{
	if (getsetting && jpPageServer) {
		char host[128];
		_ASSERT(jpPageServer);
		GetDlgItemTextA(jpPageServer->hwndPage, IDC_HOST, host, _countof(host));
		m_hostname = host;
		m_port = (u_short)GetDlgItemInt(jpPageServer->hwndPage, IDC_PORT, FALSE, FALSE);

		// Checkup nick for future identification
		std::tstring nickbuf(m_metrics.uNameMaxLength, 0), nick;
		const TCHAR* msg;
		GetDlgItemText(jpPageServer->hwndPage, IDC_NICK, &nickbuf[0], (int)nickbuf.size()+1);
		nick = nickbuf.c_str();
		if (!JClient::CheckNick(nick, msg)) { // check content
			if (jpOnline != jpPageServer) ContactSel(getTabIndex(CRC_SERVER));
			BaloonShow(jpPageServer->hwndNick, msg, MAKEINTRESOURCE(IDS_MSG_NICKERROR), 2);
			return;
		}
	}
	hostent* h = gethostbyname(m_hostname.c_str());
	u_long addr = h ? *(u_long*)h->h_addr : htonl(INADDR_LOOPBACK);
	// Add into socket manager
	JPtr<JLink> link = createLink();
	link->m_saAddr.sin_family = AF_INET;
	link->m_saAddr.sin_addr.S_un.S_addr = addr;
	link->m_saAddr.sin_port = htons(m_port);
	link->SelectWindow(FD_CONNECT | FD_READ | FD_WRITE | FD_CLOSE, m_hwndPage, BEM_NETWORK);
	link->Connect();
	if (link->State == eConnecting || link->State == eConnected)
		InsertLink(link);
	else
		JLink::destroy(link->ID);
	m_clientsock = link->ID;
	m_nConnectCount++;

	KillTimer(m_hwndPage, IDT_CONNECT);

	// Write it's to log
	EvLog(
		format("Connecting to %s (%i.%i.%i.%i:%i)",
		m_hostname.c_str(),
		(addr >> 0) & 0xFF,
		(addr >> 8) & 0xFF,
		(addr >> 16) & 0xFF,
		(addr >> 24) & 0xFF,
		m_port), elogMsg);
	// Update interface
	if (jpPageServer->hwndPage) {
		CheckDlgButton(jpPageServer->hwndPage, IDC_CONNECT, TRUE);
		jpPageServer->Enable();
		EnableWindow(jpPageServer->hwndNick, FALSE);
	}
}

void JClient::saveAutoopen() const
{
	int i = 0;
	for each (MapPageChannel::value_type const& v in mPageChannel) {
		profile::setInt(RF_AUTOOPEN, tformat(TEXT("type%02i"), i), (UINT)v.second->gettype());
		profile::setString(RF_AUTOOPEN, tformat(TEXT("name%02i"), i), v.second->m_channel.name);
		profile::setString(RF_AUTOOPEN, tformat(TEXT("pass%02i"), i), v.second->m_channel.password);
		i++;
	}
	profile::setInt(RF_AUTOOPEN, RK_CHANCOUNT, i);
}

void JClient::openAutoopen()
{
	if (profile::getInt(RF_AUTOOPEN, RK_USEAUTOOPEN, FALSE)) {
		int count = profile::getInt(RF_AUTOOPEN, RK_CHANCOUNT, 0);
		EContact type;
		std::tstring name, pass;
		for (int i = 0; i < count; i++) {
			type = (EContact)profile::getInt(RF_AUTOOPEN, tformat(TEXT("type%02i"), i), (UINT)eChannel);
			name = profile::getString(RF_AUTOOPEN, tformat(TEXT("name%02i"), i), TEXT("main"));
			pass = profile::getString(RF_AUTOOPEN, tformat(TEXT("pass%02i"), i), TEXT(""));
			PushTrn(m_clientsock, Make_Quest_JOIN(name, pass, type));
		}
	}
}

int  CALLBACK JClient::ContactAdd(const std::tstring& name, DWORD id, EContact type)
{
	if (m_mUser[m_idOwn].accessibility.fFlashPageNew
		&& profile::getInt(RF_CLIENT, RK_FLASHPAGENEW, TRUE)
		&& !m_mUser[m_idOwn].isOnline)
		FlashWindow(m_hwndPage, TRUE);

	JPtr<JPage> jp = getPage(id);
	int pos;
	if (jp) {
		pos = getTabIndex(id);
	} else {
		_ASSERT(getTabIndex(id) < 0);

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

		_ASSERT(jp);
		jp->SetNode(this, false, true);
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
		if (jpOnline) _VERIFY(InvalidateRect(jpOnline->hwndPage, 0, TRUE));
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
	_ASSERT(index >= 0 && index < TabCtrl_GetItemCount(m_hwndTab));

	TabCtrl_SetCurSel(m_hwndTab, index);

	TCITEM tci;
	tci.mask = TCIF_PARAM;
	_VERIFY(TabCtrl_GetItem(m_hwndTab, index, &tci));
	JPtr<JPage> jp = getPage((DWORD)tci.lParam);
	ShowWindow(jp->hwndPage, SW_SHOW);
	if (jpOnline && jpOnline != jp) ShowWindow(jpOnline->hwndPage, SW_HIDE);
	jp->activate();
	jpOnline = jp;

	// Hide any baloon content
	BaloonHide();
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
		_VERIFY(TabCtrl_SetItem(m_hwndTab, i, &tci));
		if (jpOnline) _VERIFY(InvalidateRect(jpOnline->hwndPage, 0, TRUE));
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
		_VERIFY(TabCtrl_GetItem(m_hwndTab, i, &tci));
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

void CALLBACK JClient::BaloonShow(HWND hwndCtrl, const TCHAR* msg, const TCHAR* title, int icon, COLORREF cr)
{
	BaloonShow(m_hwndPage, hwndCtrl, msg, title, icon, cr);
}

void CALLBACK JClient::BaloonShow(HWND hwndId, HWND hwndCtrl, const TCHAR* msg, const TCHAR* title, int icon, COLORREF cr)
{
	RECT r;
	_VERIFY(GetWindowRect(hwndCtrl, &r));
	POINT p;
	p.x = (r.left + r.right)/2, p.y = (r.top + r.bottom)/2;
	BaloonShow(hwndId, p, msg, title, (HICON)(INT_PTR)icon, cr);
}

void CALLBACK JClient::BaloonShow(HWND hwndId, const POINT& p, const TCHAR* msg, const TCHAR* title, HICON hicon, COLORREF cr)
{
	static TCHAR buftitle[256];
	if (HIWORD(title)) _tcscpy_s(buftitle, _countof(buftitle), title);
	else LoadString(JClientApp::jpApp->hinstApp, LOWORD(title), buftitle, _countof(buftitle));
	_VERIFY(SendMessage(m_hwndBaloon, TTM_SETTITLE, (WPARAM)hicon, (LPARAM)buftitle));

	static TCHAR bufmsg[1024]; // resorce string may be greater than 80 chars
	if (HIWORD(msg)) _tcscpy_s(bufmsg, _countof(bufmsg), msg);
	else LoadString(JClientApp::jpApp->hinstApp, LOWORD(msg), bufmsg, _countof(bufmsg));
	TOOLINFO ti;
	ti.cbSize = sizeof(ti);
	ti.hwnd = hwndId;
	ti.uId = (UINT_PTR)hwndId;
	ti.hinst = JClientApp::jpApp->hinstApp;
	ti.lpszText = (TCHAR*)bufmsg;
	SendMessage(m_hwndBaloon, TTM_UPDATETIPTEXT, 0, (LPARAM)&ti);

	SendMessage(m_hwndBaloon, TTM_SETTIPTEXTCOLOR, cr, 0);
	SendMessage(m_hwndBaloon, TTM_TRACKPOSITION, 0, MAKELPARAM(p.x, p.y));
	SendMessage(m_hwndBaloon, TTM_TRACKACTIVATE, TRUE, (LPARAM)&ti);

	m_isBaloon = hwndId;
	SetTimer(hwndId, IDT_BALOONPOP, TIMER_BALOONPOP, 0);
}

void CALLBACK JClient::BaloonHide(HWND hwndId)
{
	if (!hwndId) hwndId = m_isBaloon;
	if (hwndId) {
		TOOLINFO ti;
		ti.cbSize = sizeof(ti);
		ti.hwnd = hwndId;
		ti.uId = (UINT_PTR)hwndId;
		SendMessage(m_hwndBaloon, TTM_TRACKACTIVATE, FALSE, (LPARAM)&ti);
		if (hwndId == m_isBaloon) m_isBaloon = 0;
		KillTimer(hwndId, IDT_BALOONPOP);
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
			_ASSERT(m_wDeviceID.find(mci1.wDeviceID) == m_wDeviceID.end()); // ID must be unical
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

bool JClient::isGod(DWORD idUser) const
{
	MapUser::const_iterator iu = m_mUser.find(idUser != CRC_NONAME ? idUser : m_idOwn);
	return iu != m_mUser.end() && iu->second.cheat.isGod;
}

bool JClient::isDevil(DWORD idUser) const
{
	MapUser::const_iterator iu = m_mUser.find(idUser != CRC_NONAME ? idUser : m_idOwn);
	return iu != m_mUser.end() && iu->second.cheat.isDevil;
}

bool JClient::isCheats(DWORD idUser) const
{
	MapUser::const_iterator iu = m_mUser.find(idUser != CRC_NONAME ? idUser : m_idOwn);
	return iu != m_mUser.end() && (iu->second.cheat.isGod || iu->second.cheat.isDevil);
}

void JClient::InsertUser(DWORD idUser, const User& user)
{
	SetId set;
	if (m_mUser.find(idUser) != m_mUser.end()) {
		set = m_mUser[idUser].opened;
	}
	m_mUser[idUser] = user;
	m_mUser[idUser].opened = set;
}

void JClient::LinkUser(DWORD idUser, DWORD idLink)
{
	m_mUser[idUser].opened.insert(idLink);
}

void JClient::UnlinkUser(DWORD idUser, DWORD idLink)
{
	MapUser::iterator iu = m_mUser.find(idUser);
	if (iu != m_mUser.end()) {
		iu->second.opened.erase(idLink);
		if (iu->second.opened.empty())
			m_mUser.erase(iu);
	}
}

void JClient::OnHook(JNode* src)
{
	using namespace fastdelegate;

	__super::OnHook(src);

	EvNick += MakeDelegate(this, &JClient::OnNick);
}

void JClient::OnUnhook(JNode* src)
{
	using namespace fastdelegate;

	EvNick -= MakeDelegate(this, &JClient::OnNick);

	__super::OnUnhook(src);
}

//-----------------------------------------------------------------------------

// --- Register/unregister transactions parsers ---

void JClient::RegHandlers(JNode* src)
{
	__super::RegHandlers(src);

	JNODE(JClient, node, src);
	if (node) {
		// Transactions parsers
		node->m_mTrnNotify[CCPM_METRICS] += fastdelegate::MakeDelegate(this, &JClient::Recv_Notify_METRICS);
		node->m_mTrnNotify[CCPM_NICK] += fastdelegate::MakeDelegate(this, &JClient::Recv_Notify_NICK);
		node->m_mTrnReply[CCPM_JOIN] += fastdelegate::MakeDelegate(this, &JClient::Recv_Reply_JOIN);
		node->m_mTrnNotify[CCPM_JOIN] += fastdelegate::MakeDelegate(this, &JClient::Recv_Notify_JOIN);
		node->m_mTrnNotify[CCPM_PART] += fastdelegate::MakeDelegate(this, &JClient::Recv_Notify_PART);
		node->m_mTrnReply[CCPM_USERINFO] += fastdelegate::MakeDelegate(this, &JClient::Recv_Reply_USERINFO);
		node->m_mTrnNotify[CCPM_ONLINE] += fastdelegate::MakeDelegate(this, &JClient::Recv_Notify_ONLINE);
		node->m_mTrnNotify[CCPM_STATUS] += fastdelegate::MakeDelegate(this, &JClient::Recv_Notify_STATUS);
		node->m_mTrnNotify[CCPM_SAY] += fastdelegate::MakeDelegate(this, &JClient::Recv_Notify_SAY);
		node->m_mTrnNotify[CCPM_TOPIC] += fastdelegate::MakeDelegate(this, &JClient::Recv_Notify_TOPIC);
		node->m_mTrnNotify[CCPM_CHANOPTIONS] += fastdelegate::MakeDelegate(this, &JClient::Recv_Notify_CHANOPTIONS);
		node->m_mTrnNotify[CCPM_ACCESS] += fastdelegate::MakeDelegate(this, &JClient::Recv_Notify_ACCESS);
		node->m_mTrnReply[CCPM_MESSAGE] += fastdelegate::MakeDelegate(this, &JClient::Recv_Reply_MESSAGE);
		node->m_mTrnNotify[CCPM_MESSAGE] += fastdelegate::MakeDelegate(this, &JClient::Recv_Notify_MESSAGE);
		node->m_mTrnNotify[CCPM_BEEP] += fastdelegate::MakeDelegate(this, &JClient::Recv_Notify_BEEP);
		node->m_mTrnNotify[CCPM_CLIPBOARD] += fastdelegate::MakeDelegate(this, &JClient::Recv_Notify_CLIPBOARD);
		node->m_mTrnNotify[CCPM_SPLASHRTF] += fastdelegate::MakeDelegate(this, &JClient::Recv_Notify_SPLASHRTF);
	}
}

void JClient::UnregHandlers(JNode* src)
{
	JNODE(JClient, node, src);
	if (node) {
		// Transactions parsers
		node->m_mTrnNotify[CCPM_METRICS] -= fastdelegate::MakeDelegate(this, &JClient::Recv_Notify_METRICS);
		node->m_mTrnNotify[CCPM_NICK] -= fastdelegate::MakeDelegate(this, &JClient::Recv_Notify_NICK);
		node->m_mTrnReply[CCPM_JOIN] -= fastdelegate::MakeDelegate(this, &JClient::Recv_Reply_JOIN);
		node->m_mTrnNotify[CCPM_JOIN] -= fastdelegate::MakeDelegate(this, &JClient::Recv_Notify_JOIN);
		node->m_mTrnNotify[CCPM_PART] -= fastdelegate::MakeDelegate(this, &JClient::Recv_Notify_PART);
		node->m_mTrnReply[CCPM_USERINFO] -= fastdelegate::MakeDelegate(this, &JClient::Recv_Reply_USERINFO);
		node->m_mTrnNotify[CCPM_ONLINE] -= fastdelegate::MakeDelegate(this, &JClient::Recv_Notify_ONLINE);
		node->m_mTrnNotify[CCPM_STATUS] -= fastdelegate::MakeDelegate(this, &JClient::Recv_Notify_STATUS);
		node->m_mTrnNotify[CCPM_SAY] -= fastdelegate::MakeDelegate(this, &JClient::Recv_Notify_SAY);
		node->m_mTrnNotify[CCPM_TOPIC] -= fastdelegate::MakeDelegate(this, &JClient::Recv_Notify_TOPIC);
		node->m_mTrnNotify[CCPM_CHANOPTIONS] -= fastdelegate::MakeDelegate(this, &JClient::Recv_Notify_CHANOPTIONS);
		node->m_mTrnNotify[CCPM_ACCESS] -= fastdelegate::MakeDelegate(this, &JClient::Recv_Notify_ACCESS);
		node->m_mTrnReply[CCPM_MESSAGE] -= fastdelegate::MakeDelegate(this, &JClient::Recv_Reply_MESSAGE);
		node->m_mTrnNotify[CCPM_MESSAGE] -= fastdelegate::MakeDelegate(this, &JClient::Recv_Notify_MESSAGE);
		node->m_mTrnNotify[CCPM_BEEP] -= fastdelegate::MakeDelegate(this, &JClient::Recv_Notify_BEEP);
		node->m_mTrnNotify[CCPM_CLIPBOARD] -= fastdelegate::MakeDelegate(this, &JClient::Recv_Notify_CLIPBOARD);
		node->m_mTrnNotify[CCPM_SPLASHRTF] -= fastdelegate::MakeDelegate(this, &JClient::Recv_Notify_SPLASHRTF);
	}

	__super::UnregHandlers(src);
}

void JClient::OnLinkEstablished(SOCKET sock)
{
	__super::OnLinkEstablished(sock);

	sockaddr_in si;
	int len = sizeof(si);

	getpeername(sock, (struct sockaddr*)&si, &len);
	EvLog(format("Connected to %i.%i.%i.%i:%i",
		si.sin_addr.S_un.S_un_b.s_b1,
		si.sin_addr.S_un.S_un_b.s_b2,
		si.sin_addr.S_un.S_un_b.s_b3,
		si.sin_addr.S_un.S_un_b.s_b4,
		ntohs(si.sin_port)), elogMsg);

	PushTrn(sock, Make_Quest_Identify(JBLink::get((JID)sock)->getEncryptorName()));
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
}

void JClient::OnLinkFail(SOCKET sock, UINT err)
{
	if (sock == m_clientsock) {
		m_clientsock = 0; // not connected
	}

	__super::OnLinkFail(sock, err);
}

void JClient::OnLinkStart(SOCKET sock)
{
	std::tstring nickbuf(m_metrics.uNameMaxLength, 0), nick;
	GetDlgItemText(jpPageServer->hwndPage, IDC_NICK, &nickbuf[0], (int)nickbuf.size()+1);
	nick = nickbuf.c_str();
	PushTrn(sock, Make_Cmd_NICK(m_idOwn, nick));

	EUserStatus stat = (EUserStatus)SendDlgItemMessage(jpPageServer->hwndPage, IDC_STATUS, CB_GETCURSEL, 0, 0);
	int img = (int)SendDlgItemMessage(jpPageServer->hwndPage, IDC_STATUSIMG, CB_GETCURSEL, 0, 0);
	std::tstring msg(m_metrics.uStatusMsgMaxLength, 0);
	GetDlgItemText(jpPageServer->hwndPage, IDC_STATUSMSG, &msg[0], (int)msg.size()+1);
	PushTrn(sock, Make_Cmd_STATUS(stat, m_mAlert[stat], img, msg));

	__super::OnLinkStart(sock);
}

void JClient::OnNick(DWORD idOld, const std::tstring& oldname, DWORD idNew, const std::tstring& newname)
{
	ContactRename(idOld, oldname, idNew, newname);
}

//
// Beowolf Network Protocol Messages reciving
//

void JClient::Recv_Notify_METRICS(SOCKET sock, io::mem& is)
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
			EvLog(SZ_BADTRN, elogItrn);
			return;
		}
	}

	EvMetrics(m_metrics);
	EvLog("metrics from server", elogItrn);
}

void JClient::Recv_Notify_NICK(SOCKET sock, io::mem& is)
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
			EvLog(SZ_BADTRN, elogItrn);
			return;
		}
	}

	_ASSERT(idOld != idNew);
	bool isOwn = idOld == m_idOwn || idOld == CRC_NONAME;
	oldname = getSafeName(idOld);

	EvNick(idOld, oldname, idNew, newname);

	if (isOwn) {
		// Update interface
		SetWindowText(jpPageServer->hwndNick, newname.c_str());

		// Report about message
		const char* msg = "unknown result";
		switch (result)
		{
		case NICK_OK:
			msg = "nickname successfully assigned";
			break;
		case NICK_TAKEN:
			msg = "nickname is taken";
			break;
		case NICK_TAKENUSER:
			msg = "nickname is taken by other user";
			break;
		case NICK_TAKENCHANNEL:
			msg = "nickname is taken by channel";
			break;
		}
		EvLog(format("%s, now nickname is [b]%s[/b]", msg, tstr_to_utf8(newname).c_str()), elogInfo);
	}

	// Lua response
	if (isOwn) {
		DOLUACS;
		lua_getmethod(L, "onNickOwn");
		if (lua_isfunction(L, -1)) {
			lua_insert(L, -2);
			lua_pushstring(L, tstr_to_utf8(newname).c_str());
			lua_call(L, 2, 0);
		} else lua_pop(L, 2);
	}
	{ // in any case
		DOLUACS;
		lua_getmethod(L, "onNick");
		if (lua_isfunction(L, -1)) {
			lua_insert(L, -2);
			lua_pushstring(L, tstr_to_utf8(oldname).c_str());
			lua_pushstring(L, tstr_to_utf8(newname).c_str());
			lua_call(L, 3, 0);
		} else lua_pop(L, 2);
	}
}

void JClient::Recv_Reply_JOIN(SOCKET sock, WORD trnid, io::mem& is)
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
					_ASSERT(jp);
					jp->setuser(user);

					ContactSel(pos);

					// Lua response
					{
						DOLUACS;
						lua_getmethod(L, "onOpenPrivate");
						if (lua_isfunction(L, -1)) {
							lua_insert(L, -2);
							lua_pushstring(L, tstr_to_utf8(getSafeName(ID)).c_str());
							lua_call(L, 2, 0);
						} else lua_pop(L, 2);
					}

					EvLog(format("opened private talk with [b]%s[/b]", tstr_to_utf8(user.name).c_str()), elogInfo);
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
					_ASSERT(jp);
					jp->setchannel(chan);

					ContactSel(pos);

					PushTrn(sock, Make_Quest_USERINFO(wanted));

					// Lua response
					{
						DOLUACS;
						lua_getmethod(L, "onOpenChannel");
						if (lua_isfunction(L, -1)) {
							lua_insert(L, -2);
							lua_pushstring(L, tstr_to_utf8(chan.name).c_str());
							lua_call(L, 2, 0);
						} else lua_pop(L, 2);
					}

					EvLog(format("joins to [b]#%s[/b] channel", tstr_to_utf8(chan.name).c_str()), elogInfo);
					break;
				}
			}
		} else {
			const char* msg;
			switch (result)
			{
			case CHAN_ALREADY:
				{
					int i = getTabIndex(ID);
					if (i >= 0) ContactSel(i);
				}
				msg = "contact is already opened";
				break;
			case CHAN_BADPASS:
				msg = "password does not satisfies";
				break;
			case CHAN_DENY:
				msg = "access denied, channel is invite-only";
				break;
			case CHAN_LIMIT:
				msg = "number of users exceeds the channel limits";
				break;
			case CHAN_ABSENT:
				msg = "contact is absent";
				break;
			default:
				msg = "can not join to contact";
				break;
			}
			EvLog(msg, elogError);
		}
	}
	catch (io::exception e)
	{
		// Report about message
		EvLog(SZ_BADTRN, elogItrn);
		return;
	}
}

void JClient::Recv_Notify_JOIN(SOCKET sock, io::mem& is)
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
			EvLog(SZ_BADTRN, elogItrn);
			return;
		}
	}

	InsertUser(idWho, user);
	LinkUser(idWho, idWhere);
	if (idWhere == m_idOwn) { // private talk
		// Create interface
		ContactAdd(user.name, idWho, eUser);

		// Lua response
		{
			DOLUACS;
			lua_getmethod(L, "onJoinPrivate");
			if (lua_isfunction(L, -1)) {
				lua_insert(L, -2);
				lua_pushstring(L, tstr_to_utf8(getSafeName(idWho)).c_str());
				lua_call(L, 2, 0);
			} else lua_pop(L, 2);
		}

		EvLog(format("[b]%s[/b] opens private talk",
			tstr_to_utf8(user.name).c_str()),
			elogInfo);
	} else { // channel
		MapPageChannel::iterator ipc = mPageChannel.find(idWhere);
		if (ipc != mPageChannel.end()) {
			ipc->second->Join(idWho);
			ipc->second->setAlert(eYellow);
			if (m_mUser[m_idOwn].accessibility.fPlayChatSounds)
				PlaySound(JClientApp::jpApp->strWavJoin.c_str());

			EvLog(format("[b]%s[/b] joins to [b]%s[/b]",
				tstr_to_utf8(getSafeName(idWho)).c_str(),
				tstr_to_utf8(ipc->second->m_channel.name).c_str()),
				elogInfo);
		} else {
			PushTrn(sock, Make_Cmd_PART(m_idOwn, idWhere));
		}
	}
}

void JClient::Recv_Notify_PART(SOCKET sock, io::mem& is)
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
			EvLog(SZ_BADTRN, elogItrn);
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
		{
			DOLUACS;
			lua_getmethod(L, "onPartPrivate");
			if (lua_isfunction(L, -1)) {
				lua_insert(L, -2);
				lua_pushstring(L, tstr_to_utf8(getSafeName(idWho)).c_str());
				lua_pushstring(L, tstr_to_utf8(getSafeName(idBy)).c_str());
				lua_call(L, 3, 0);
			} else lua_pop(L, 2);
		}

		EvLog(format("[style=Info]private with [b]%s[/b] closed[/style]", tstr_to_utf8(getSafeName(idWho)).c_str()), elogInfo);
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

void JClient::Recv_Reply_USERINFO(SOCKET sock, WORD trnid, io::mem& is)
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
			EvLog(SZ_BADTRN, elogItrn);
			return;
		}
	}

	// Report about message
	EvLog("recieve users info", elogItrn);
}

void JClient::Recv_Notify_ONLINE(SOCKET sock, io::mem& is)
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
		{
			DOLUACS;
			lua_getmethod(L, "onOnline");
			if (lua_isfunction(L, -1)) {
				lua_insert(L, -2);
				lua_pushstring(L, tstr_to_utf8(getSafeName(idWho)).c_str());
				lua_pushinteger(L, isOnline);
				lua_call(L, 3, 0);
			} else lua_pop(L, 2);
		}
	}

	// Report about message
	EvLog(format("user %s is %s", iu != m_mUser.end() ? tstr_to_utf8(iu->second.name).c_str() : "unknown", isOnline ? "online" : "offline"), elogItrn);
}

void JClient::Recv_Notify_STATUS(SOCKET sock, io::mem& is)
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
			EvLog(SZ_BADTRN, elogItrn);
			return;
		}
	}

	MapUser::iterator iu = m_mUser.find(idWho);
	if (iu != m_mUser.end()) {
		if (type & STATUS_MODE) {
			iu->second.nStatus = stat;
			iu->second.accessibility = a;

			// Lua response
			{
				DOLUACS;
				lua_getmethod(L, "onStatusMode");
				if (lua_isfunction(L, -1)) {
					lua_insert(L, -2);
					lua_pushstring(L, tstr_to_utf8(getSafeName(idWho)).c_str());
					lua_pushinteger(L, stat);
					pushAlert(L, a);
					lua_call(L, 4, 0);
				} else lua_pop(L, 2);
			}
		}
		if (type & STATUS_IMG) {
			iu->second.nStatusImg = img;

			// Lua response
			{
				DOLUACS;
				lua_getmethod(L, "onStatusImage");
				if (lua_isfunction(L, -1)) {
					lua_insert(L, -2);
					lua_pushstring(L, tstr_to_utf8(getSafeName(idWho)).c_str());
					lua_pushinteger(L, img);
					lua_call(L, 3, 0);
				} else lua_pop(L, 2);
			}
		}
		if (type & STATUS_MSG) {
			iu->second.strStatus = msg;

			// Lua response
			{
				DOLUACS;
				lua_getmethod(L, "onStatusMessage");
				if (lua_isfunction(L, -1)) {
					lua_insert(L, -2);
					lua_pushstring(L, tstr_to_utf8(getSafeName(idWho)).c_str());
					lua_pushstring(L, tstr_to_utf8(msg).c_str());
					lua_call(L, 3, 0);
				} else lua_pop(L, 2);
			}
		}
		if (type & STATUS_GOD) {
			iu->second.cheat.isGod = god;

			// Lua response
			{
				DOLUACS;
				lua_getmethod(L, "onStatusGod");
				if (lua_isfunction(L, -1)) {
					lua_insert(L, -2);
					lua_pushstring(L, tstr_to_utf8(getSafeName(idWho)).c_str());
					lua_pushboolean(L, god);
					lua_call(L, 3, 0);
				} else lua_pop(L, 2);
			}
		}
		if (type & STATUS_DEVIL) {
			iu->second.cheat.isDevil = devil;

			// Lua response
			{
				DOLUACS;
				lua_getmethod(L, "onStatusDevil");
				if (lua_isfunction(L, -1)) {
					lua_insert(L, -2);
					lua_pushstring(L, tstr_to_utf8(getSafeName(idWho)).c_str());
					lua_pushboolean(L, devil);
					lua_call(L, 3, 0);
				} else lua_pop(L, 2);
			}
		}

		MapPageChannel::iterator ipc = mPageChannel.find(jpOnline ? jpOnline->getID() : 0);
		if (ipc != mPageChannel.end()) {
			ipc->second->redrawUser(idWho);
		}
	}
}

void JClient::Recv_Notify_SAY(SOCKET sock, io::mem& is)
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
			EvLog(SZ_BADTRN, elogItrn);
			return;
		}
	}

	JPtr<JPageLog> jp;
	MapPageUser::iterator ipu = mPageUser.find(idWhere);
	if (ipu != mPageUser.end()) { // private talk
		jp = ipu->second;
		jp->setAlert(eRed);
		if (m_mUser[m_idOwn].accessibility.fFlashPageSayPrivate
			&& profile::getInt(RF_CLIENT, RK_FLASHPAGESAYPRIVATE, TRUE)
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
				&& profile::getInt(RF_CLIENT, RK_FLASHPAGESAYCHANNEL, FALSE)
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

void JClient::Recv_Notify_TOPIC(SOCKET sock, io::mem& is)
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
			EvLog(SZ_BADTRN, elogItrn);
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
				&& profile::getInt(RF_CLIENT, RK_FLASHPAGETOPIC, TRUE)
				&& !m_mUser[m_idOwn].isOnline)
				FlashWindow(m_hwndPage, TRUE);

			// Lua response
			{
				DOLUACS;
				lua_getmethod(L, "onTopic");
				if (lua_isfunction(L, -1)) {
					lua_insert(L, -2);
					lua_pushstring(L, tstr_to_utf8(getSafeName(idWho)).c_str());
					lua_pushstring(L, tstr_to_utf8(ipc->second->m_channel.name).c_str());
					lua_pushstring(L, tstr_to_utf8(topic).c_str());
					lua_call(L, 4, 0);
				} else lua_pop(L, 2);
			}
		}
	}
	if (jp) {
		if (jpOnline == jp) ShowTopic(jp->gettopic());
		if (m_mUser[m_idOwn].accessibility.fPlayChatSounds)
			PlaySound(JClientApp::jpApp->strWavTopic.c_str());
	}
}

void JClient::Recv_Notify_CHANOPTIONS(SOCKET sock, io::mem& is)
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
			EvLog(SZ_BADTRN, elogItrn);
			return;

		case 2:
			op = CHANOP_ANONYMOUS;
			val = true;
			break;

		case 3:
			// Report about message
			EvLog(SZ_BADTRN, elogItrn);
			return;
		}
	}

	JPtr<JPageLog> jp;
	std::tstring msg;
	MapPageUser::iterator ipu = mPageUser.find(idWhere);
	if (ipu != mPageUser.end()) { // private talk
		jp = ipu->second;

		_ASSERT(op == CHANOP_BACKGROUND);
		SendMessage(ipu->second->hwndEdit, EM_SETBKGNDCOLOR, FALSE, (LPARAM)val);
		SendMessage(ipu->second->hwndLog, EM_SETBKGNDCOLOR, FALSE, (LPARAM)val);

		// Lua response
		{
			DOLUACS;
			lua_getmethod(L, "onBackground");
			if (lua_isfunction(L, -1)) {
				lua_insert(L, -2);
				lua_pushstring(L, tstr_to_utf8(getSafeName(idWho)).c_str());
				lua_pushstring(L, tstr_to_utf8(getSafeName(idWhere)).c_str());
				lua_pushinteger(L, GetRValue(val));
				lua_pushinteger(L, GetGValue(val));
				lua_pushinteger(L, GetBValue(val));
				lua_call(L, 6, 0);
			} else lua_pop(L, 2);
		}
	} else {
		MapPageChannel::iterator ipc = mPageChannel.find(idWhere);
		if (ipc != mPageChannel.end()) { // channel
			jp = ipc->second;

			switch (op)
			{
			case CHANOP_AUTOSTATUS:
				{
					ipc->second->m_channel.nAutoStatus = (EChanStatus)val;

					// Lua response
					{
						DOLUACS;
						lua_getmethod(L, "onChanAutostatus");
						if (lua_isfunction(L, -1)) {
							lua_insert(L, -2);
							lua_pushstring(L, tstr_to_utf8(getSafeName(idWho)).c_str());
							lua_pushstring(L, tstr_to_utf8(ipc->second->m_channel.name).c_str());
							lua_pushinteger(L, val);
							lua_call(L, 4, 0);
						} else lua_pop(L, 2);
					}
					break;
				}
			case CHANOP_LIMIT:
				{
					ipc->second->m_channel.nLimit = (UINT)val;

					// Lua response
					{
						DOLUACS;
						lua_getmethod(L, "onChanLimit");
						if (lua_isfunction(L, -1)) {
							lua_insert(L, -2);
							lua_pushstring(L, tstr_to_utf8(getSafeName(idWho)).c_str());
							lua_pushstring(L, tstr_to_utf8(ipc->second->m_channel.name).c_str());
							lua_pushinteger(L, val);
							lua_call(L, 4, 0);
						} else lua_pop(L, 2);
					}
					break;
				}
			case CHANOP_HIDDEN:
				{
					ipc->second->m_channel.isHidden = val != 0;

					// Lua response
					{
						DOLUACS;
						lua_getmethod(L, "onChanHidden");
						if (lua_isfunction(L, -1)) {
							lua_insert(L, -2);
							lua_pushstring(L, tstr_to_utf8(getSafeName(idWho)).c_str());
							lua_pushstring(L, tstr_to_utf8(ipc->second->m_channel.name).c_str());
							lua_pushboolean(L, val != 0);
							lua_call(L, 4, 0);
						} else lua_pop(L, 2);
					}
					break;
				}
			case CHANOP_ANONYMOUS:
				{
					ipc->second->m_channel.isAnonymous = val != 0;
					InvalidateRect(ipc->second->hwndList, 0, TRUE);

					// Lua response
					{
						DOLUACS;
						lua_getmethod(L, "onChanAnonymous");
						if (lua_isfunction(L, -1)) {
							lua_insert(L, -2);
							lua_pushstring(L, tstr_to_utf8(getSafeName(idWho)).c_str());
							lua_pushstring(L, tstr_to_utf8(ipc->second->m_channel.name).c_str());
							lua_pushboolean(L, val != 0);
							lua_call(L, 4, 0);
						} else lua_pop(L, 2);
					}
				}
				break;
			case CHANOP_BACKGROUND:
				{
					ipc->second->m_channel.crBackground = (COLORREF)val;

					SendMessage(ipc->second->hwndEdit, EM_SETBKGNDCOLOR, FALSE, (LPARAM)val);
					SendMessage(ipc->second->hwndLog, EM_SETBKGNDCOLOR, FALSE, (LPARAM)val);
					ListView_SetBkColor(ipc->second->hwndList, val);
					InvalidateRect(ipc->second->hwndList, 0, TRUE);

					// Lua response
					{
						DOLUACS;
						lua_getmethod(L, "onBackground");
						if (lua_isfunction(L, -1)) {
							lua_insert(L, -2);
							lua_pushstring(L, tstr_to_utf8(getSafeName(idWho)).c_str());
							lua_pushstring(L, tstr_to_utf8(ipc->second->m_channel.name).c_str());
							lua_pushinteger(L, GetRValue(val));
							lua_pushinteger(L, GetGValue(val));
							lua_pushinteger(L, GetBValue(val));
							lua_call(L, 6, 0);
						} else lua_pop(L, 2);
					}
					break;
				}
			}
		}
	}
	if (jp && !msg.empty()) {
		jp->AppendScript(tformat(TEXT("[style=Descr][color=%s]%s[/color] %s[/style]"),
			idWho != m_idOwn ? TEXT("red") : TEXT("blue"),
			jp->getSafeName(idWho).c_str(),
			msg.c_str()));
	}
}

void JClient::Recv_Notify_ACCESS(SOCKET sock, io::mem& is)
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
			EvLog(SZ_BADTRN, elogItrn);
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
			{
				DOLUACS;
				lua_getmethod(L, "onAccess");
				if (lua_isfunction(L, -1)) {
					lua_insert(L, -2);
					lua_pushstring(L, tstr_to_utf8(getSafeName(idWho)).c_str());
					lua_pushstring(L, tstr_to_utf8(ipc->second->m_channel.name).c_str());
					lua_pushinteger(L, stat);
					lua_pushstring(L, tstr_to_utf8(getSafeName(idBy)).c_str());
					lua_call(L, 5, 0);
				} else lua_pop(L, 2);
			}
		}
	}
	if (jp) {
		if (m_mUser[m_idOwn].accessibility.fPlayChatSounds)
			PlaySound(JClientApp::jpApp->strWavConfirm.c_str());
	}
}

void JClient::Recv_Reply_MESSAGE(SOCKET sock, WORD trnid, io::mem& is)
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
			EvLog(SZ_BADTRN, elogItrn);
			return;
		}
	}

	// Report about message
	const char* msg;
	switch (type)
	{
	case MESSAGE_IGNORE:
		msg = "message to [b]%s[/b] ignored";
		break;
	case MESSAGE_SENT:
		msg = "message sent to [b]%s[/b]";
		break;
	case MESSAGE_SAVED:
		msg = "message to [b]%s[/b] saved";
		break;
	default: return;
	}
	EvLog(format(msg, tstr_to_utf8(getSafeName(idWho)).c_str()), elogInfo);
}

void JClient::Recv_Notify_MESSAGE(SOCKET sock, io::mem& is)
{
	DWORD idBy;
	FILETIME ft;
	DWORD dwRtfSize;
	const void* ptr = "";
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
			EvLog(SZ_BADTRN, elogItrn);
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
	EvLog(format("%s from [b]%s[/b]", fAlert ? "alert" : "message", tstr_to_utf8(getSafeName(idBy)).c_str()), elogInfo);
}

void JClient::Recv_Notify_BEEP(SOCKET sock, io::mem& is)
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
			EvLog(SZ_BADTRN, elogItrn);
			return;
		}
	}

	// Lua response
	{
		DOLUACS;
		lua_getmethod(L, "onBeep");
		if (lua_isfunction(L, -1)) {
			lua_insert(L, -2);
			lua_pushstring(L, tstr_to_utf8(getSafeName(idBy)).c_str());
			lua_call(L, 2, 0);
		} else lua_pop(L, 2);
	}
}

void JClient::Recv_Notify_CLIPBOARD(SOCKET sock, io::mem& is)
{
	DWORD idBy;

	try
	{
		io::unpack(is, idBy);
		if (OpenClipboard(m_hwndPage)) {
			UINT fmt;
			std::tstring name;
			HANDLE hMem;
			size_t size;
			void* ptr;
			EmptyClipboard();
			while (io::unpack(is, fmt), fmt) {
				io::unpack(is, name);
				io::unpack(is, size);
				if (fmt >= 0xC000 && fmt <= 0xFFFF) {
					fmt = RegisterClipboardFormat(name.c_str());
				}
				if (fmt) {
					hMem = GlobalAlloc(GMEM_MOVEABLE, (SIZE_T)size);
					ptr = GlobalLock(hMem);
					if (ptr) {
						io::unpackmem(is, ptr, size);
						GlobalUnlock(hMem);
						SetClipboardData(fmt, hMem);
					} else {
						GlobalFree(hMem);
						io::skip(is, size);
					}
				} else {
					io::skip(is, size);
				}
			}
			_VERIFY(CloseClipboard());

			// Lua response
			{
				DOLUACS;
				lua_getmethod(L, "onClipboard");
				if (lua_isfunction(L, -1)) {
					lua_insert(L, -2);
					lua_pushstring(L, tstr_to_utf8(getSafeName(idBy)).c_str());
					lua_call(L, 2, 0);
				} else lua_pop(L, 2);
			}

			// Report about message
			EvLog(format("recieve clipboard content from [b]%s[/b]", tstr_to_utf8(getSafeName(idBy)).c_str()), elogInfo);
		} else {
			// Report about message
			EvLog(format("can not paste recieved clipboard content from [b]%s[/b]", tstr_to_utf8(getSafeName(idBy)).c_str()), elogError);
		}
	}
	catch (io::exception e)
	{
		switch (e.count)
		{
		case 0:
			// Report about message
			EvLog(SZ_BADTRN, elogItrn);
			return;
		}
	}
}

void JClient::Recv_Notify_SPLASHRTF(SOCKET sock, io::mem& is)
{
	DWORD idBy;
	DWORD dwRtfSize;
	const void* ptr = "";
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
			EvLog(SZ_BADTRN, elogItrn);
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
		jp->trnid = 0;
		*(&jp->rcPos) = rcPos;
		jp->bCloseOnDisconnect = bCloseOnDisconnect;
		jp->dwCanclose = dwCanclose;
		jp->dwAutoclose = dwAutoclose;
		jp->fTransparent = fTransparent;
		jp->crSheet = crSheet;
		PostMessage(m_hwndPage, BEM_JDIALOG, IDD_SPLASHRTF, (LPARAM)(JDialog*)jp);
	}
	// Report about message
	EvLog(format("splash text from [b]%s[/b]", tstr_to_utf8(getSafeName(idBy)).c_str()), elogInfo);
}

//
// Beowolf Network Protocol Messages sending
//

JPtr<JBTransaction> JClient::Make_Cmd_NICK(DWORD idWho, const std::tstring& nick) const
{
	std::ostringstream os;
	io::pack(os, idWho);
	io::pack(os, nick);
	return MakeTrn(COMMAND(CCPM_NICK), 0, os.str());
}

JPtr<JBTransaction> JClient::Make_Quest_JOIN(const std::tstring& name, const std::tstring& pass, int type) const
{
	std::ostringstream os;
	io::pack(os, name);
	io::pack(os, pass);
	io::pack(os, type);
	return MakeTrn(QUEST(CCPM_JOIN), 0, os.str());
}

JPtr<JBTransaction> JClient::Make_Cmd_PART(DWORD idWho, DWORD idWhere) const
{
	std::ostringstream os;
	io::pack(os, idWho);
	io::pack(os, idWhere);
	return MakeTrn(COMMAND(CCPM_PART), 0, os.str());
}

JPtr<JBTransaction> JClient::Make_Quest_USERINFO(const SetId& set) const
{
	std::ostringstream os;
	io::pack(os, set);
	return MakeTrn(QUEST(CCPM_USERINFO), 0, os.str());
}

JPtr<JBTransaction> JClient::Make_Cmd_ONLINE(EOnline online, DWORD id) const
{
	std::ostringstream os;
	io::pack(os, online);
	io::pack(os, id);
	return MakeTrn(COMMAND(CCPM_ONLINE), 0, os.str());
}

JPtr<JBTransaction> JClient::Make_Cmd_STATUS_Mode(EUserStatus stat, const Alert& a) const
{
	std::ostringstream os;
	io::pack(os, (WORD)STATUS_MODE);
	io::pack(os, stat);
	io::pack(os, a);
	return MakeTrn(COMMAND(CCPM_STATUS), 0, os.str());
}

JPtr<JBTransaction> JClient::Make_Cmd_STATUS_Img(int img) const
{
	std::ostringstream os;
	io::pack(os, (WORD)STATUS_IMG);
	io::pack(os, img);
	return MakeTrn(COMMAND(CCPM_STATUS), 0, os.str());
}

JPtr<JBTransaction> JClient::Make_Cmd_STATUS_Msg(const std::tstring& msg) const
{
	std::ostringstream os;
	io::pack(os, (WORD)STATUS_MSG);
	io::pack(os, msg);
	return MakeTrn(COMMAND(CCPM_STATUS), 0, os.str());
}

JPtr<JBTransaction> JClient::Make_Cmd_STATUS(EUserStatus stat, const Alert& a, int img, const std::tstring& msg) const
{
	std::ostringstream os;
	io::pack(os, (WORD)(STATUS_MODE | STATUS_IMG | STATUS_MSG));
	io::pack(os, stat);
	io::pack(os, a);
	io::pack(os, img);
	io::pack(os, msg);
	return MakeTrn(COMMAND(CCPM_STATUS), 0, os.str());
}

JPtr<JBTransaction> JClient::Make_Cmd_SAY(DWORD idWhere, UINT type, const std::string& content) const
{
	std::ostringstream os;
	io::pack(os, idWhere);
	io::pack(os, type);
	io::pack(os, content);
	return MakeTrn(COMMAND(CCPM_SAY), 0, os.str());
}

JPtr<JBTransaction> JClient::Make_Cmd_TOPIC(DWORD idWhere, const std::tstring& topic) const
{
	std::ostringstream os;
	io::pack(os, idWhere);
	io::pack(os, topic);
	return MakeTrn(COMMAND(CCPM_TOPIC), 0, os.str());
}

JPtr<JBTransaction> JClient::Make_Cmd_CHANOPTIONS(DWORD idWhere, int op, DWORD val) const
{
	std::ostringstream os;
	io::pack(os, idWhere);
	io::pack(os, op);
	io::pack(os, val);
	return MakeTrn(COMMAND(CCPM_CHANOPTIONS), 0, os.str());
}

JPtr<JBTransaction> JClient::Make_Cmd_ACCESS(DWORD idWho, DWORD idWhere, EChanStatus stat) const
{
	std::ostringstream os;
	io::pack(os, idWho);
	io::pack(os, idWhere);
	io::pack(os, stat);
	return MakeTrn(COMMAND(CCPM_ACCESS), 0, os.str());
}

JPtr<JBTransaction> JClient::Make_Quest_MESSAGE(DWORD idWho, const std::string& text, bool fAlert, COLORREF crSheet) const
{
	std::ostringstream os;
	io::pack(os, idWho);
	io::pack(os, (DWORD)text.size());
	os.write(text.c_str(), (std::streamsize)text.size());
	io::pack(os, false);
	io::pack(os, fAlert);
	io::pack(os, crSheet);
	return MakeTrn(QUEST(CCPM_MESSAGE), 0, os.str());
}

JPtr<JBTransaction> JClient::Make_Cmd_BEEP(DWORD idWho) const
{
	std::ostringstream os;
	io::pack(os, idWho);
	return MakeTrn(COMMAND(CCPM_BEEP), 0, os.str());
}

JPtr<JBTransaction> JClient::Make_Cmd_CLIPBOARD(DWORD idWho) const
{
	std::ostringstream os;
	io::pack(os, idWho);
	if (!OpenClipboard(m_hwndPage)) return 0;
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
	_VERIFY(CloseClipboard());
	return MakeTrn(COMMAND(CCPM_CLIPBOARD), 0, os.str());
}

JPtr<JBTransaction> JClient::Make_Cmd_SPLASHRTF(DWORD idWho, const std::string& text, const RECT& rcPos, bool bCloseOnDisconnect, DWORD dwCanclose, DWORD dwAutoclose, bool fTransparent, COLORREF crSheet) const
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
	return MakeTrn(COMMAND(CCPM_SPLASHRTF), 0, os.str());
}

//-----------------------------------------------------------------------------

JPtr<JClientApp> JClientApp::jpApp = new JClientApp();
WSADATA JClientApp::wsaData;

JClientApp::JClientApp(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR szcl, int ncs)
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

void JClientApp::Init()
{
	CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
	INITCOMMONCONTROLSEX InitCtrls = {
		sizeof(INITCOMMONCONTROLSEX),
		ICC_WIN95_CLASSES |
		ICC_STANDARD_CLASSES |
		ICC_COOL_CLASSES |
		ICC_LINK_CLASS
	};
	_VERIFY(InitCommonControlsEx(&InitCtrls));
	// Ensure that the RichEdit library is loaded
	_VERIFY(false
		//|| (hinstRichEdit = LoadLibrary(TEXT("Msftedit.dll"))) != 0 // version 4.1
		|| (hinstRichEdit = LoadLibrary(TEXT("Riched20.dll"))) != 0 // version 2.0 or 3.0
		|| (hinstRichEdit = LoadLibrary(TEXT("Riched32.dll"))) != 0 // version 1.0
		);
	_VERIFY(!WSAStartup(MAKEWORD(2, 2), &wsaData));

	profile::setKey(TEXT("BEOWOLF"), APPNAME);

	// Waves
	m_strWavMeline = profile::getString(RF_SOUNDS, RK_WAVMELINE, TEXT("Sounds\\me_line.wav"));
	m_strWavChatline = profile::getString(RF_SOUNDS, RK_WAVCHATLINE, TEXT("Sounds\\chat_line.wav"));
	m_strWavConfirm = profile::getString(RF_SOUNDS, RK_WAVCONFIRM, TEXT("Sounds\\confirm.wav"));
	m_strWavPrivateline = profile::getString(RF_SOUNDS, RK_WAVPRIVATELINE, TEXT("Sounds\\chat_line.wav"));
	m_strWavTopic = profile::getString(RF_SOUNDS, RK_WAVTOPIC, TEXT("Sounds\\topic_change.wav"));
	m_strWavJoin = profile::getString(RF_SOUNDS, RK_WAVJOIN, TEXT("Sounds\\channel_join.wav"));
	m_strWavPart = profile::getString(RF_SOUNDS, RK_WAVPART, TEXT("Sounds\\channel_leave.wav"));
	m_strWavPrivate = profile::getString(RF_SOUNDS, RK_WAVPRIVATE, TEXT("Sounds\\private_start.wav"));
	m_strWavAlert = profile::getString(RF_SOUNDS, RK_WAVALERT, TEXT("Sounds\\alert.wav"));
	m_strWavMessage = profile::getString(RF_SOUNDS, RK_WAVMESSAGE, TEXT("Sounds\\message.wav"));
	m_strWavBeep = profile::getString(RF_SOUNDS, RK_WAVBEEP, TEXT("Sounds\\beep.wav"));
	m_strWavClipboard = profile::getString(RF_SOUNDS, RK_WAVCLIPBOARD, TEXT("Sounds\\clipboard.wav"));

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

	jpClient = new JClient(0);
}

bool JClientApp::InitInstance()
{
	_ASSERT(jpClient);
#ifdef _DEBUG
	jpClient->DoHelper();
#else
	if (!profile::getInt(RF_HOSTLIST, RK_HOSTCOUNT, 0)) {
		jpClient->DoHelper();
	}
#endif
	jpClient->Init();
	if (CreateDialogParam(hinstApp, MAKEINTRESOURCE(IDD_MAIN), 0, (DLGPROC)JClient::DlgProcStub, (LPARAM)(JDialog*)jpClient)) {
		return true;
	} else {
		MessageBox(0, TEXT("Cannot create main window"), sAppName.c_str(), MB_OK | MB_ICONSTOP);
		return false;
	}
}

void JClientApp::Done()
{
	if (jpClient->State != JService::eStopped) jpClient->Stop();
	jpClient->Done();
	// Free associated resources
	_VERIFY(DestroyMenu(m_hmenuTab));
	_VERIFY(DestroyMenu(m_hmenuLog));
	_VERIFY(DestroyMenu(m_hmenuChannel));
	_VERIFY(DestroyMenu(m_hmenuList));
	_VERIFY(DestroyMenu(m_hmenuRichEdit));
	_VERIFY(DestroyMenu(m_hmenuUser));
	_VERIFY(DestroyMenu(m_hmenuUserGod));
	// Destroy the image list
	_VERIFY(ImageList_Destroy(m_himlEdit));
	_VERIFY(ImageList_Destroy(m_himlTab));
	_VERIFY(ImageList_Destroy(m_himlMan));
	_VERIFY(ImageList_Destroy(m_himlStatus));
	_VERIFY(ImageList_Destroy(m_himlStatusImg));
	_VERIFY(DeleteObject(m_himgSend));
	_VERIFY(DeleteObject(m_himgULBG));
	_VERIFY(DeleteObject(m_himgULFoc));
	_VERIFY(DeleteObject(m_himgULSel));
	_VERIFY(DeleteObject(m_himgULHot));
	// Close all MCI devices
	_VERIFY(!mciSendCommand(MCI_ALL_DEVICE_ID, MCI_CLOSE, MCI_WAIT, NULL));
	// Free RichEdit library
	_VERIFY(FreeLibrary(hinstRichEdit));

	_VERIFY(!WSACleanup());
	CoUninitialize();
}

//-----------------------------------------------------------------------------

int WINAPI _tWinMain(
	HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPTSTR    lpszCmdLine,
	int       nCmdShow)
{
	try
	{
		// Set current directory to executable location
#ifndef _DEBUG
		{
			TCHAR mpath[_MAX_PATH];
			TCHAR drive[_MAX_DRIVE];
			TCHAR dir[_MAX_DIR];
			GetModuleFileName(NULL, mpath, _countof(mpath));
			_tsplitpath_s(mpath, drive, _countof(drive), dir, _countof(dir), NULL, 0, NULL, 0);
			SetCurrentDirectory((std::tstring(drive)+dir).c_str());
		}
#endif

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