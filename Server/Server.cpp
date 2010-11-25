
//-----------------------------------------------------------------------------

//
// Includes
//

#pragma region Includes

#include "stdafx.h"

// Common
#include "CRC.h"
#include "Profile.h"

// Project
#include "..\ColibriProtocol.h"
#include "resource.h"
#include "server.h"

#pragma endregion

//-----------------------------------------------------------------------------

using namespace colibrichat;

//-----------------------------------------------------------------------------

// Global Variables:
static TCHAR szHelpFile[MAX_PATH];

//-----------------------------------------------------------------------------

//
// class JServer
//

JServer::JServer()
: JBNB(), JWindow()
{
	// Dialogs
	jpConnections = new JConnections();
	jpPasswords = new JPasswords();

	m_port = CCP_PORT;
	m_passwordNet = TEXT("beowolf");

	m_bShowIcon = true;

	m_metrics.uNameMaxLength = 20;
	m_metrics.uPassMaxLength = 32;
	m_metrics.uStatusMsgMaxLength = 32;
	m_metrics.uTopicMaxLength = 100;
	m_metrics.nMsgSpinMaxCount = 20;
	m_metrics.uChatLineMaxVolume = 80*1024;
	m_metrics.flags.bTransmitClipboard = true;

	m_encryptorname = ECRYPT_BINDEFAULT;
}

void JServer::Init()
{
	jpConnections->SetNode(this, false, true);
	jpPasswords->SetNode(this, false, true);

	__super::Init();

	m_mSocketId.clear();
	m_mIdSocket.clear();
	m_mUser.clear();
	m_mChannel.clear();

	CreateMsgWindow();

	// Create listening sockets
	int count = profile::getInt(RF_SERVER, RK_PORTCOUNT, 1);
	std::set<u_short> port;
	for (int i = 0; i < count; i++) {
		u_short v = (u_short)profile::getInt(RF_SERVER, tformat(TEXT("Port%02i"), i), CCP_PORT);
		if (v < 1000) v = CCP_PORT; // do not use system ports
		port.insert(v);
	}

	// create listening sockets
	int l = 0;
	for each (u_short v in port) {
		JPtr<JLink> link = createLink();
		link->m_saAddr.sin_family = AF_INET;
		link->m_saAddr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
		link->m_saAddr.sin_port = htons(v);
		link->Select(FD_ACCEPT | FD_CLOSE);
		link->Listen();
		if (link->State == eListening) {
			InsertLink(link);
			l++;
		} else
			JLink::destroy(link->ID);
	}
	ASSERT(l > 0);
}

void JServer::Done()
{
	__super::Done();

	DestroyMsgWindow();
}

HWND JServer::CreateMsgWindow()
{
	if (!IsWindow(m_hwndPage)) m_hwndPage = CreateWindow(WC_MSG, WT_MSG,
		WS_OVERLAPPEDWINDOW,
		0, 0, 100, 100,
		HWND_MESSAGE,
		0, 0, (JWindow*)this);
	return m_hwndPage;
}

BOOL JServer::DestroyMsgWindow()
{
	return DestroyWindow(m_hwndPage);
}

void JServer::LoadState()
{
	m_passwordNet = profile::getString(RF_SERVER, RK_PASSWORDNET, TEXT("beowolf"));
	m_passwordGod = profile::getString(RF_SERVER, RK_PASSWORDGOD, TEXT("godpassword"));
	m_passwordDevil = profile::getString(RF_SERVER, RK_PASSWORDDEVIL, TEXT("devilpassword"));

	m_bShowIcon = profile::getInt(RF_SERVER, RK_SHOWICON, TRUE) != 0;

	s_nCompression = profile::getInt(RF_SERVER, RK_COMPRESSION, -1);
	m_encryptorname = tstrToANSI(profile::getString(RF_SERVER, RK_ENCRYPTALG, ANSIToTstr(ECRYPT_BINDEFAULT)));

	m_metrics.uNameMaxLength = (size_t)profile::getInt(RF_METRICS, RK_NameMaxLength, 20);
	m_metrics.uPassMaxLength = (size_t)profile::getInt(RF_METRICS, RK_PassMaxLength, 32);
	m_metrics.uStatusMsgMaxLength = (size_t)profile::getInt(RF_METRICS, RK_StatusMsgMaxLength, 32);
	m_metrics.uTopicMaxLength = (size_t)profile::getInt(RF_METRICS, RK_TopicMaxLength, 100);
	m_metrics.nMsgSpinMaxCount = (size_t)profile::getInt(RF_METRICS, RK_MsgSpinMaxCount, 20);
	m_metrics.uChatLineMaxVolume = (size_t)profile::getInt(RF_METRICS, RK_ChatLineMaxVolume, 80*1024);
	m_metrics.flags.bTransmitClipboard = profile::getInt(RF_METRICS, RK_TransmitClipboard, true) != 0;
}

void JServer::SaveState()
{
	profile::setString(RF_SERVER, RK_PASSWORDNET, m_passwordNet);
	profile::setString(RF_SERVER, RK_PASSWORDGOD, m_passwordGod);
	profile::setString(RF_SERVER, RK_PASSWORDDEVIL, m_passwordDevil);

	profile::setInt(RF_METRICS, RK_NameMaxLength, (UINT)m_metrics.uNameMaxLength);
	profile::setInt(RF_METRICS, RK_PassMaxLength, (UINT)m_metrics.uPassMaxLength);
	profile::setInt(RF_METRICS, RK_StatusMsgMaxLength, (UINT)m_metrics.uStatusMsgMaxLength);
	profile::setInt(RF_METRICS, RK_TopicMaxLength, (UINT)m_metrics.uTopicMaxLength);
	profile::setInt(RF_METRICS, RK_MsgSpinMaxCount, (UINT)m_metrics.nMsgSpinMaxCount);
	profile::setInt(RF_METRICS, RK_ChatLineMaxVolume, (UINT)m_metrics.uChatLineMaxVolume);
	profile::setInt(RF_METRICS, RK_TransmitClipboard, (UINT)m_metrics.flags.bTransmitClipboard);

	profile::setString(RF_SERVER, RK_ENCRYPTALG, ANSIToTstr(m_encryptorname));
}

LRESULT WINAPI JServer::DlgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	LRESULT retval = 0;
	switch (message)
	{
	case WM_CREATE:
		{
			if (JServerApp::jpApp) JServerApp::jpApp->AddWindow(hWnd);
			if (State != JService::eInit) Init();
			Run();

			// Setup taskbar icon
			if (m_bShowIcon)
			{
				NOTIFYICONDATA tnid;
				tnid.cbSize = sizeof(NOTIFYICONDATA);
				tnid.hWnd = hWnd;
				tnid.uID = 1;
				tnid.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;
				tnid.uCallbackMessage = BEM_NOTIFYICON;
				tnid.hIcon = LoadIcon(JServerApp::jpApp->hinstApp, MAKEINTRESOURCE(IDI_SMALL));
				tnid.uVersion = NOTIFYICON_VERSION;
				LoadString(0, IDS_SHELL_TIP, tnid.szTip, _countof(tnid.szTip));
				Shell_NotifyIcon(NIM_ADD, &tnid);
				Shell_NotifyIcon(NIM_SETVERSION, &tnid);
			}
			break;
		}

	case WM_DESTROY:
		{
			// Remove taskbar icon
			if (m_bShowIcon)
			{
				NOTIFYICONDATA tnid;
				tnid.cbSize = sizeof(NOTIFYICONDATA);
				tnid.hWnd = hWnd;
				tnid.uID = 1;
				tnid.uVersion = NOTIFYICON_VERSION;
				Shell_NotifyIcon(NIM_DELETE, &tnid);
			}

			Stop();
			if (JServerApp::jpApp) JServerApp::jpApp->DelWindow(hWnd);
			break;
		}

	case WM_COMMAND:
		{
			switch (LOWORD(wParam))
			{
			case IDC_SHELL_CONNECTIONS:
				{
					Connections();
					break;
				}

			case IDC_SHELL_PASSWORDS:
				{
					Passwords();
					break;
				}

			case IDC_SHELL_ABOUT:
				{
					About();
					break;
				}

			case IDC_SHELL_EXIT:
				{
					PostQuitMessage(0);
					break;
				}

			default:
				retval = __super::DlgProc(hWnd, message, wParam, lParam);
			}
			break;
		}

	case BEM_NOTIFYICON:
		{
			switch (lParam)
			{
			case WM_LBUTTONDOWN:
				{
					SNDMSG(hWnd, WM_COMMAND, IDC_SHELL_CONNECTIONS, 0);
					break;
				}

			case WM_RBUTTONDOWN:
			case WM_CONTEXTMENU:
				{
					HMENU hmenu = LoadMenu(JServerApp::jpApp->hinstApp, MAKEINTRESOURCE(IDM_SHELL));
					POINT p;
					SetForegroundWindow(hWnd);
					VERIFY(GetCursorPos(&p));
					VERIFY(SetMenuDefaultItem(GetSubMenu(hmenu, 0), IDC_SHELL_CONNECTIONS, FALSE));
					TrackPopupMenu(GetSubMenu(hmenu, 0), TPM_RIGHTALIGN | TPM_BOTTOMALIGN | TPM_RIGHTBUTTON,
						p.x, p.y, 0, hWnd, 0);
					VERIFY(DestroyMenu(hmenu));
					break;
				}

			case WM_LBUTTONDBLCLK:
				{
					SNDMSG(hWnd, WM_COMMAND, IDC_SHELL_CONNECTIONS, 0);
					break;
				}

			case NIN_KEYSELECT:
				{
					SNDMSG(hWnd, WM_COMMAND, IDC_SHELL_CONNECTIONS, 0);
					break;
				}

			default:
				retval = __super::DlgProc(hWnd, message, wParam, lParam);
			}
			break;
		}

	default:
		retval = __super::DlgProc(hWnd, message, wParam, lParam);
	}
	return retval;
}

bool JServer::hasCRC(DWORD crc) const
{
	return
		crc == CRC_SERVER ||
		crc == CRC_LIST ||
		crc == CRC_NONAME ||
		crc == CRC_ANONYMOUS ||
		crc == CRC_GOD ||
		crc == CRC_DEVIL ||
		m_mUser.find(crc) != m_mUser.end() ||
		m_mChannel.find(crc) != m_mChannel.end();
}

bool JServer::linkCRC(DWORD crc1, DWORD crc2)
{
	Contact *cont1, *cont2;
	MapUser::iterator iu;
	MapChannel::iterator ic;
	iu = m_mUser.find(crc1);
	if (iu == m_mUser.end()) {
		ic = m_mChannel.find(crc1);
		if (ic == m_mChannel.end()) return false;
		else cont1 = &ic->second;
	} else cont1 = &iu->second;
	iu = m_mUser.find(crc2);
	if (iu == m_mUser.end()) {
		ic = m_mChannel.find(crc2);
		if (ic == m_mChannel.end()) return false;
		else cont2 = &ic->second;
	} else cont2 = &iu->second;
	cont1->opened.insert(crc2);
	cont2->opened.insert(crc1);
	return true;
}

void JServer::unlinkCRC(DWORD crc1, DWORD crc2)
{
	MapUser::iterator iu;
	MapChannel::iterator ic;
	iu = m_mUser.find(crc1);
	if (iu != m_mUser.end()) iu->second.opened.erase(crc2);
	else {
		ic = m_mChannel.find(crc1);
		if (ic != m_mChannel.end()) {
			ic->second.opened.erase(crc2);
			if (ic->second.opened.empty())
				m_mChannel.erase(crc1);
		}
	}
	iu = m_mUser.find(crc2);
	if (iu != m_mUser.end()) iu->second.opened.erase(crc1);
	else {
		ic = m_mChannel.find(crc2);
		if (ic != m_mChannel.end()) {
			ic->second.opened.erase(crc1);
			if (ic->second.opened.empty())
				m_mChannel.erase(crc2);
		}
	}
}

bool JServer::CheckNick(std::tstring& nick, const TCHAR*& msg)
{
	for each (std::tstring::value_type const& v in nick) {
		if (v < TEXT(' ')) {
			msg = TEXT("Content must not contain non-printing symbols");
			return false;
		}
	}
	while (!nick.empty() && *nick.begin() == TEXT(' ')) nick.erase(nick.begin());
	while (!nick.empty() && *(nick.end()-1) == TEXT(' ')) nick.erase(nick.end()-1);
	if (nick.empty()) {
		msg = TEXT("Content can not be empty");
		return false;
	}
	msg = TEXT("Content is valid");
	return true;
}

std::tstring JServer::getNearestName(const std::tstring& nick) const
{
	std::tstring buffer = nick;
	TCHAR digits[16];
	DWORD i = 0;
	while (hasCRC(tCRCJJ(buffer.c_str()))) {
		_stprintf_s(digits, _countof(digits), TEXT("%u"), i);
		buffer = nick.substr(0, m_metrics.uNameMaxLength - lstrlen(digits)) + digits;
		i++;
	}
	return buffer;
}

void JServer::RenameContact(DWORD idByOrSock, DWORD idOld, std::tstring newname)
{
	DWORD idNew = tCRCJJ(newname.c_str());
	if (idNew == idOld && idOld != CRC_NONAME) return;

	DWORD result;
	if (hasCRC(idNew)) {
		if (idNew == CRC_SERVER || idNew == CRC_LIST) {
			result = NICK_TAKEN;
		} else if (m_mUser.find(idNew) != m_mUser.end()) {
			result = NICK_TAKENUSER;
		} else if (m_mChannel.find(idNew) != m_mChannel.end()) {
			result = NICK_TAKENCHANNEL;
		} else {
			result = NICK_TAKEN;
		}
		newname = getNearestName(newname);
		idNew = tCRCJJ(newname.c_str());
	} else {
		result = NICK_OK;
	}

	SetId opened;
	MapUser::iterator iu = m_mUser.find(idOld);
	if (iu != m_mUser.end()) { // private talk
		opened = iu->second.opened;
		iu->second.name = newname;
		SOCKET sock = m_mIdSocket[idOld];
		m_mUser[idNew] = iu->second;
		m_mSocketId[sock] = idNew;
		m_mIdSocket[idNew] = sock;
		for each (SetId::value_type const& v in opened) {
			linkCRC(idNew, v);
			unlinkCRC(idOld, v);
		}
		m_mUser.erase(idOld);
		m_mIdSocket.erase(idOld);
		// replace content of channels access rights
		for (MapChannel::iterator ic = m_mChannel.begin(); ic != m_mChannel.end(); ic++) {
			ic->second.setStatus(idNew, ic->second.getStatus(idOld));
			ic->second.setStatus(idOld, eOutsider);
		}
		opened.insert(idNew); // send notify to receiver
		if (idByOrSock == idOld) idByOrSock = idNew;
		else opened.insert(idByOrSock); // send reply to sender
	} else if (idOld == CRC_NONAME) { // new user
		sockaddr_in si;
		int len = sizeof(si);
		getpeername((SOCKET)idByOrSock, (struct sockaddr*)&si, &len);

		// Create new user
		User user;
		user.name = newname;
		user.IP = si.sin_addr;

		m_mUser[idNew] = user;
		m_mSocketId[(SOCKET)idByOrSock] = idNew;
		m_mIdSocket[idNew] = (SOCKET)idByOrSock;

		opened.insert(idNew); // send notify to receiver
	} else {
		MapChannel::iterator ic = m_mChannel.find(idOld);
		if (ic != m_mChannel.end()) { // channel
			opened = ic->second.opened;
			ic->second.name = newname;
			m_mChannel[idNew] = ic->second;
			for each (SetId::value_type const& v in opened) {
				linkCRC(idNew, v);
				unlinkCRC(idOld, v);
			}
			m_mChannel.erase(idOld);
			opened.insert(idByOrSock); // send reply to sender
		} else {
			ASSERT(false); // no valid way to here
		}
	}
	BroadcastTrn(opened, true, Make_Notify_NICK(result, idOld, idNew, newname));
}

bool JServer::isGod(DWORD idUser) const
{
	MapUser::const_iterator iu = m_mUser.find(idUser);
	return iu != m_mUser.end() && iu->second.cheat.isGod;
}

bool JServer::isDevil(DWORD idUser) const
{
	MapUser::const_iterator iu = m_mUser.find(idUser);
	return iu != m_mUser.end() && iu->second.cheat.isDevil;
}

bool JServer::isCheats(DWORD idUser) const
{
	MapUser::const_iterator iu = m_mUser.find(idUser);
	return iu != m_mUser.end() && (iu->second.cheat.isGod || iu->second.cheat.isDevil);
}

void JServer::OnHook(JNode* src)
{
	using namespace fastdelegate;

	__super::OnHook(src);
}

void JServer::OnUnhook(JNode* src)
{
	using namespace fastdelegate;

	__super::OnUnhook(src);
}

//-----------------------------------------------------------------------------

// --- Register/unregister transactions parsers ---

void JServer::RegHandlers(JNode* src)
{
	__super::RegHandlers(src);

	JNODE(JServer, node, src);
	if (node) {
		// Transactions parsers
		node->m_mTrnCommand[CCPM_NICK] += fastdelegate::MakeDelegate(this, &JServer::Recv_Cmd_NICK);
		node->m_mTrnQuest[CCPM_LIST] += fastdelegate::MakeDelegate(this, &JServer::Recv_Quest_LIST);
		node->m_mTrnQuest[CCPM_JOIN] += fastdelegate::MakeDelegate(this, &JServer::Recv_Quest_JOIN);
		node->m_mTrnCommand[CCPM_PART] += fastdelegate::MakeDelegate(this, &JServer::Recv_Cmd_PART);
		node->m_mTrnQuest[CCPM_USERINFO] += fastdelegate::MakeDelegate(this, &JServer::Recv_Quest_USERINFO);
		node->m_mTrnCommand[CCPM_ONLINE] += fastdelegate::MakeDelegate(this, &JServer::Recv_Cmd_ONLINE);
		node->m_mTrnCommand[CCPM_STATUS] += fastdelegate::MakeDelegate(this, &JServer::Recv_Cmd_STATUS);
		node->m_mTrnCommand[CCPM_SAY] += fastdelegate::MakeDelegate(this, &JServer::Recv_Cmd_SAY);
		node->m_mTrnCommand[CCPM_TOPIC] += fastdelegate::MakeDelegate(this, &JServer::Recv_Cmd_TOPIC);
		node->m_mTrnCommand[CCPM_CHANOPTIONS] += fastdelegate::MakeDelegate(this, &JServer::Recv_Cmd_CHANOPTIONS);
		node->m_mTrnCommand[CCPM_ACCESS] += fastdelegate::MakeDelegate(this, &JServer::Recv_Cmd_ACCESS);
		node->m_mTrnCommand[CCPM_BEEP] += fastdelegate::MakeDelegate(this, &JServer::Recv_Cmd_BEEP);
		node->m_mTrnCommand[CCPM_CLIPBOARD] += fastdelegate::MakeDelegate(this, &JServer::Recv_Cmd_CLIPBOARD);
		node->m_mTrnQuest[CCPM_MESSAGE] += fastdelegate::MakeDelegate(this, &JServer::Recv_Quest_MESSAGE);
		node->m_mTrnCommand[CCPM_SPLASHRTF] += fastdelegate::MakeDelegate(this, &JServer::Recv_Cmd_SPLASHRTF);
	}
}

void JServer::UnregHandlers(JNode* src)
{
	JNODE(JServer, node, src);
	if (node) {
		// Transactions parsers
		node->m_mTrnCommand[CCPM_NICK] -= fastdelegate::MakeDelegate(this, &JServer::Recv_Cmd_NICK);
		node->m_mTrnQuest[CCPM_LIST] -= fastdelegate::MakeDelegate(this, &JServer::Recv_Quest_LIST);
		node->m_mTrnQuest[CCPM_JOIN] -= fastdelegate::MakeDelegate(this, &JServer::Recv_Quest_JOIN);
		node->m_mTrnCommand[CCPM_PART] -= fastdelegate::MakeDelegate(this, &JServer::Recv_Cmd_PART);
		node->m_mTrnQuest[CCPM_USERINFO] -= fastdelegate::MakeDelegate(this, &JServer::Recv_Quest_USERINFO);
		node->m_mTrnCommand[CCPM_ONLINE] -= fastdelegate::MakeDelegate(this, &JServer::Recv_Cmd_ONLINE);
		node->m_mTrnCommand[CCPM_STATUS] -= fastdelegate::MakeDelegate(this, &JServer::Recv_Cmd_STATUS);
		node->m_mTrnCommand[CCPM_SAY] -= fastdelegate::MakeDelegate(this, &JServer::Recv_Cmd_SAY);
		node->m_mTrnCommand[CCPM_TOPIC] -= fastdelegate::MakeDelegate(this, &JServer::Recv_Cmd_TOPIC);
		node->m_mTrnCommand[CCPM_CHANOPTIONS] -= fastdelegate::MakeDelegate(this, &JServer::Recv_Cmd_CHANOPTIONS);
		node->m_mTrnCommand[CCPM_ACCESS] -= fastdelegate::MakeDelegate(this, &JServer::Recv_Cmd_ACCESS);
		node->m_mTrnCommand[CCPM_BEEP] -= fastdelegate::MakeDelegate(this, &JServer::Recv_Cmd_BEEP);
		node->m_mTrnCommand[CCPM_CLIPBOARD] -= fastdelegate::MakeDelegate(this, &JServer::Recv_Cmd_CLIPBOARD);
		node->m_mTrnQuest[CCPM_MESSAGE] -= fastdelegate::MakeDelegate(this, &JServer::Recv_Quest_MESSAGE);
		node->m_mTrnCommand[CCPM_SPLASHRTF] -= fastdelegate::MakeDelegate(this, &JServer::Recv_Cmd_SPLASHRTF);
	}

	__super::UnregHandlers(src);
}

void JServer::OnLinkEstablished(SOCKET sock)
{
	__super::OnLinkEstablished(sock);

	m_mSocketId.insert(MapSocketId::value_type(sock, CRC_NONAME));

	if (m_hwndPage && m_bShowIcon)
	{
		NOTIFYICONDATA tnid;
		tnid.cbSize = sizeof(NOTIFYICONDATA);
		tnid.hWnd = m_hwndPage;
		tnid.uID = 1;
		tnid.uFlags = NIF_TIP;
		tnid.uVersion = NOTIFYICON_VERSION;
		_stprintf_s(tnid.szTip, _countof(tnid.szTip), APPNAME TEXT("\n%u connections"), countEstablished());
		Shell_NotifyIcon(NIM_MODIFY, &tnid);
	}
}

void JServer::OnLinkStart(SOCKET sock)
{
	PushTrn(sock, Make_Notify_METRICS(m_metrics));
}

void JServer::OnLinkClose(SOCKET sock, UINT err)
{
	size_t count = countEstablished() - 1;
	__super::OnLinkClose(sock, err);

	DWORD idSrc = m_mSocketId[sock];
	MapUser::iterator iter = m_mUser.find(idSrc);
	if (iter != m_mUser.end()) {
		SetId opened = iter->second.opened;
		for each (SetId::value_type const& v in opened) {
			unlinkCRC(idSrc, v);
			MapUser::const_iterator iu = m_mUser.find(v);
			if (iu != m_mUser.end()) { // private talk
				PushTrn(m_mIdSocket[v], Make_Notify_PART(idSrc, idSrc, CRC_SERVER));
			} else {
				MapChannel::const_iterator ic = m_mChannel.find(v);
				if (ic != m_mChannel.end()) { // channel
					BroadcastTrn(ic->second.opened, false, Make_Notify_PART(idSrc, v, CRC_SERVER));
				}
			}
		}
		m_mUser.erase(idSrc);
	}
	m_mSocketId.erase(sock);
	m_mIdSocket.erase(idSrc);

	if (m_hwndPage && m_bShowIcon)
	{
		NOTIFYICONDATA tnid;
		tnid.cbSize = sizeof(NOTIFYICONDATA);
		tnid.hWnd = m_hwndPage;
		tnid.uID = 1;
		tnid.uFlags = NIF_TIP;
		tnid.uVersion = NOTIFYICON_VERSION;
		_stprintf_s(tnid.szTip, _countof(tnid.szTip), APPNAME TEXT("\n%u connections"), count);
		Shell_NotifyIcon(NIM_MODIFY, &tnid);
	}
}

int  JServer::BroadcastTrn(const SetId& set, bool nested, JTransaction* jpTrn, size_t ssi) throw()
{
	// Count users to prevent duplicate sents
	SetJID broadcast;
	MapIdSocket::const_iterator iis;
	for each (SetId::value_type const& v in set)
	{
		iis = m_mIdSocket.find(v);
		if (iis != m_mIdSocket.end()) // private talk
			broadcast.insert((JID)iis->second);
		else {
			if (nested) {
				MapChannel::const_iterator ic = m_mChannel.find(v);
				if (ic != m_mChannel.end()) { // channel
					for each (SetId::value_type const& v in ic->second.opened) {
						iis = m_mIdSocket.find(v);
						if (iis != m_mIdSocket.end())
							broadcast.insert((JID)iis->second);
					}
				}
			}
		}
	}

	// Broadcast to unical users
	return __super::BroadcastTrn(broadcast, jpTrn, ssi);
}

int  JServer::BroadcastTrn(const SetId& set, bool nested, WORD message, const std::string& str, size_t ssi) throw()
{
	return BroadcastTrn(set, nested, MakeTrn(message, 0, str), ssi);
}

//
// Beowolf Network Protocol Messages reciving
//

void JServer::Recv_Cmd_NICK(SOCKET sock, io::mem& is)
{
	DWORD idOld;
	std::tstring name;

	try
	{
		io::unpack(is, idOld);
		io::unpack(is, name);
	}
	catch (io::exception e)
	{
		switch (e.count)
		{
		case 0:
			// Report about message
			EvLog(SZ_BADTRN, elogTrn);
			return;
		}
	}

	DWORD idBy = m_mSocketId[sock];
	MapUser::iterator iu = m_mUser.find(idOld);
	if (iu != m_mUser.end()) { // private talk
#ifdef CHEATS
		if (idOld == idBy || isGod(idBy)) {
#else
		if (idOld == idBy) {
#endif
			RenameContact(idBy, idOld, name);
			// Report about message
			EvLog(format("nickname renamed: %s", tstrToANSI(name).c_str()), elogInfo);
		}
	} else if (idOld == CRC_NONAME) { // new user
		ASSERT(idBy == idOld);
		RenameContact((DWORD)sock, idOld, name);
		// Report about message
		EvLog(format("nickname added: %s", tstrToANSI(name).c_str()), elogInfo);
	} else {
		MapChannel::const_iterator ic = m_mChannel.find(idOld);
		if (ic != m_mChannel.end()) { // channel
#ifdef CHEATS
			if (ic->second.getStatus(idBy) == eFounder || isGod(idBy)) {
#else
			if (ic->second.getStatus(idBy) == eFounder) {
#endif
				RenameContact(idBy, idOld, name);
				// Report about message
				EvLog(format("channel name modified to: %s", tstrToANSI(name).c_str()), elogInfo);
			}
		}
	}
}

void JServer::Recv_Quest_LIST(SOCKET sock, WORD trnid, io::mem& is, std::ostream& os)
{
#ifdef CHEATS
	DWORD idBy = m_mSocketId[sock];
	bool god = isGod(idBy);
#else
	bool god = false;
#endif
	Form_Reply_LIST(os, god);

	// Report about message
	EvLog("channels list", elogTrn);
}

void JServer::Recv_Quest_JOIN(SOCKET sock, WORD trnid, io::mem& is, std::ostream& os)
{
	std::tstring name, pass;
	int type;

	try
	{
		io::unpack(is, name);
		io::unpack(is, pass);
		io::unpack(is, type);
	}
	catch (io::exception e)
	{
		switch (e.count)
		{
		case 0:
			// Report about message
			EvLog(SZ_BADTRN, elogTrn);
			return;

		case 1:
			pass = TEXT("");
		case 2:
			type = eUser | eChannel | eBoard;
		}
	}

	DWORD idDst = tCRCJJ(name.c_str());
	DWORD idSrc = m_mSocketId[sock];
	if (name == NAME_GOD && type & eCheat) {
		if (pass == m_passwordGod) {
			MapUser::iterator iu = m_mUser.find(idSrc);
			if (iu != m_mUser.end()) {
				iu->second.cheat.isGod = !iu->second.cheat.isGod;
				SetId set = iu->second.opened;
				set.insert(iu->first);
				BroadcastTrn(set, true, Make_Notify_STATUS_God(iu->first, iu->second.cheat.isGod));
				Form_Reply_JOIN_Result(os, CHAN_OK, eCheat, idDst);
			}
		} else {
			Form_Reply_JOIN_Result(os, CHAN_BADPASS, eCheat, idDst);
		}
	} else if (name == NAME_DEVIL && type & eCheat) {
		if (pass == passwordDevil) {
			MapUser::iterator iu = m_mUser.find(idSrc);
			if (iu != m_mUser.end()) {
				iu->second.cheat.isDevil = !iu->second.cheat.isDevil;
				SetId set = iu->second.opened;
				set.insert(iu->first);
				BroadcastTrn(set, true, Make_Notify_STATUS_Devil(iu->first, iu->second.cheat.isDevil));
				Form_Reply_JOIN_Result(os, CHAN_OK, eCheat, idDst);
			}
		} else {
			Form_Reply_JOIN_Result(os, CHAN_BADPASS, eCheat, idDst);
		}
	} else {
		MapUser::const_iterator iu = m_mUser.find(idDst);
		if (iu != m_mUser.end() && type & eUser) { // private talk
			if (iu->second.opened.find(idDst) == iu->second.opened.end()) {
				Form_Reply_JOIN_User(os, idDst, iu->second);
				PushTrn(m_mIdSocket[idDst], Make_Notify_JOIN(idSrc, idDst, m_mUser[idSrc]));
				linkCRC(idDst, idSrc);
			} else {
				Form_Reply_JOIN_Result(os, CHAN_ALREADY, eUser, idDst);
			}
		} else {
			MapChannel::iterator ic = m_mChannel.find(idDst);
			if (ic != m_mChannel.end() && type & eChannel) { // channel
				if (ic->second.opened.find(idSrc) == ic->second.opened.end()) {
#ifdef CHEATS
					if (ic->second.opened.size() < ic->second.nLimit || isGod(idSrc)) {
#else
					if (ic->second.opened.size() < ic->second.nLimit) {
#endif
#ifdef CHEATS
						if (ic->second.nAutoStatus > eOutsider || isGod(idSrc)) {
#else
						if (ic->second.nAutoStatus > eOutsider) {
#endif
#ifdef CHEATS
							if (ic->second.password.empty() || ic->second.password == pass || isGod(idSrc)) {
#else
							if (ic->second.password.empty() || ic->second.password == pass) {
#endif
								if (ic->second.getStatus(idSrc) == eOutsider) { // give default status if user never been on channel
									ic->second.setStatus(idSrc, ic->second.nAutoStatus > eOutsider
										? ic->second.nAutoStatus
										: eWriter);
								}
								BroadcastTrn(ic->second.opened, false, Make_Notify_JOIN(idSrc, idDst, m_mUser[idSrc]));
								linkCRC(idDst, idSrc);
								Form_Reply_JOIN_Channel(os, idDst, ic->second);
							} else {
								Form_Reply_JOIN_Result(os, CHAN_BADPASS, eChannel, idDst);
							}
						} else {
							Form_Reply_JOIN_Result(os, CHAN_DENY, eChannel, idDst);
						}
					} else {
						Form_Reply_JOIN_Result(os, CHAN_LIMIT, eChannel, idDst);
					}
				} else {
					Form_Reply_JOIN_Result(os, CHAN_ALREADY, eChannel, idDst);
				}
			} else { // create new channel if nothing was found
				if (type & eChannel) {
					FILETIME ft;
					GetSystemFileTime(ft);

					Channel chan;
					chan.name = name;
					chan.ftCreation = ft;
					chan.password = pass;
					chan.topic = TEXT("");
					chan.idTopicWriter = 0;
					chan.nAutoStatus = eWriter;
					chan.nLimit = -1;
					chan.isHidden = false;
					chan.isAnonymous = false;
					chan.crBackground = RGB(0xFF, 0xFF, 0xFF);

					chan.setStatus(idSrc, eFounder);
					m_mChannel[idDst] = chan;
					linkCRC(idDst, idSrc);
					Form_Reply_JOIN_Channel(os, idDst, m_mChannel[idDst]);
				} else if (type & eBoard) {
					Form_Reply_JOIN_Result(os, CHAN_ABSENT, eBoard, idDst);
				} else Form_Reply_JOIN_Result(os, CHAN_ABSENT, (EContact)type, idDst);
			}
		}
	}

	// Report about message
	EvLog(format("joins %s to %s", tstrToANSI(m_mUser[idSrc].name).c_str(), tstrToANSI(name).c_str()), elogInfo);
}

void JServer::Recv_Cmd_PART(SOCKET sock, io::mem& is)
{
	DWORD idWho, idWhere;

	try
	{
		io::unpack(is, idWho);
		io::unpack(is, idWhere);
	}
	catch (io::exception e)
	{
		switch (e.count)
		{
		case 0:
		case 1:
			// Report about message
			EvLog(SZ_BADTRN, elogTrn);
			return;
		}
	}

	if (idWho == CRC_NONAME) idWho = m_mSocketId[sock];

	DWORD idBy = m_mSocketId[sock];
	MapUser::iterator iuWho = m_mUser.find(idWho), iuBy = m_mUser.find(idBy);
	ASSERT(iuWho != m_mUser.end());
	ASSERT(iuBy != m_mUser.end());

	MapUser::iterator iu = m_mUser.find(idWhere);
	if (iu != m_mUser.end()) { // private talk
		// Report about message
		EvLog(format("parts %s from %s", tstrToANSI(iuWho->second.name).c_str(), tstrToANSI(iu->second.name).c_str()), elogInfo);

		PushTrn(m_mIdSocket[idWhere], Make_Notify_PART(idWho, idWho, idBy)); // recieves to close private with idWho
		PushTrn(sock, Make_Notify_PART(idBy, idWhere, idBy));
		unlinkCRC(idWho, idWhere);
	} else {
		MapChannel::const_iterator ic = m_mChannel.find(idWhere);
		if (ic != m_mChannel.end()) { // channel
			bool isModer = ic->second.getStatus(idBy) >= eModerator;
			bool canKick = ic->second.getStatus(idBy) >= ic->second.getStatus(idWho);
#ifdef CHEATS
			if (idWho == idBy || (isModer && canKick && !isDevil(idWho)) || isDevil(idBy)) {
#else
			if (idWho == idBy || (isModer && canKick)) {
#endif
				// Report about message
				EvLog(format("parts %s from %s", tstrToANSI(iuWho->second.name).c_str(), tstrToANSI(ic->second.name).c_str()), elogInfo);

				ic = m_mChannel.find(idWhere);
				if (ic != m_mChannel.end()) { // check that channel still exist
					BroadcastTrn(ic->second.opened, false, Make_Notify_PART(idWho, idWhere, idBy));
				}
				unlinkCRC(idWho, idWhere);
			}
		}
	}
}

void JServer::Recv_Quest_USERINFO(SOCKET sock, WORD trnid, io::mem& is, std::ostream& os)
{
	SetId wanted;

	try
	{
		io::unpack(is, wanted);
	}
	catch (io::exception e)
	{
		switch (e.count)
		{
		case 0:
			// Report about message
			EvLog(SZ_BADTRN, elogTrn);
			return;
		}
	}

	Form_Reply_USERINFO(os, wanted);

	// Report about message
	EvLog(format("info for %u users", wanted.size()), elogTrn);
}

void JServer::Recv_Cmd_ONLINE(SOCKET sock, io::mem& is)
{
	EOnline isOnline;
	DWORD idOnline;

	try
	{
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

	DWORD idSrc = m_mSocketId[sock];
	MapUser::iterator iu = m_mUser.find(idSrc);
	if (iu != m_mUser.end()) {
		iu->second.isOnline = isOnline;
		iu->second.idOnline = idOnline;

		SetId set = iu->second.opened;
		set.insert(idSrc);
		BroadcastTrn(set, true, Make_Notify_ONLINE(idSrc, isOnline, idOnline));
	}

	// Report about message
	EvLog(format("user is %s", isOnline ? "online" : "offline"), elogTrn);
}

void JServer::Recv_Cmd_STATUS(SOCKET sock, io::mem& is)
{
	WORD type;
	EUserStatus stat;
	Alert a;
	int img;
	std::tstring msg;

	stat = eReady;
	img = 0;
	msg = TEXT("");

	try
	{
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
	}
	catch (io::exception e)
	{
		switch (e.count)
		{
		case 0:
			// Report about message
			EvLog(SZ_BADTRN, elogTrn);
			return;
		}
	}

	DWORD idSrc = m_mSocketId[sock];
	MapUser::iterator iu = m_mUser.find(idSrc);
	if (iu != m_mUser.end()) {
		if (type & STATUS_MODE) {
			iu->second.nStatus = stat;
			iu->second.accessibility = a;
		}
		if (type & STATUS_IMG) {
			iu->second.nStatusImg = img;
		}
		if (type & STATUS_MSG) {
			iu->second.strStatus = msg;
		}

		SetId set = iu->second.opened;
		set.insert(idSrc);
		BroadcastTrn(set, true, Make_Notify_STATUS(idSrc, type, stat, a, img, msg));
	}

	// Report about message
	EvLog("user changes status", elogTrn);
}

void JServer::Recv_Cmd_SAY(SOCKET sock, io::mem& is)
{
	DWORD idWhere;
	UINT type;
	std::string content;

	try
	{
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
			EvLog(SZ_BADTRN, elogTrn);
			return;
		}
	}

	DWORD idSrc = m_mSocketId[sock];
	MapUser::const_iterator iu = m_mUser.find(idWhere);
	if (iu != m_mUser.end()) { // private talk
		PushTrn(m_mIdSocket[idWhere], Make_Notify_SAY(idSrc, idSrc, type, content));
		PushTrn(sock, Make_Notify_SAY(idSrc, idWhere, type, content));
	} else {
		MapChannel::const_iterator ic = m_mChannel.find(idWhere);
		if (ic != m_mChannel.end()) { // channel
			MapUser::const_iterator iu2 = m_mUser.find(idSrc);
			bool isAnonymous = ic->second.isAnonymous || (iu2 != m_mUser.end() && iu2->second.nStatus == eInvisible);
#ifdef CHEATS
			if (ic->second.getStatus(idSrc) > eReader || isCheats(idSrc))
				BroadcastTrn(ic->second.opened, false, Make_Notify_SAY(!isAnonymous || isGod(idSrc) ? idSrc : CRC_ANONYMOUS, idWhere, type, content));
#else
			if (ic->second.getStatus(idSrc) > eReader)
				BroadcastTrn(ic->second.opened, false, Make_Notify_SAY(!isAnonymous ? idSrc : CRC_ANONYMOUS, idWhere, type, content));
#endif
		}
	}
}

void JServer::Recv_Cmd_TOPIC(SOCKET sock, io::mem& is)
{
	DWORD idWhere;
	std::tstring topic;

	try
	{
		io::unpack(is, idWhere);
		io::unpack(is, topic);
	}
	catch (io::exception e)
	{
		switch (e.count)
		{
		case 0:
			// Report about message
			EvLog(SZ_BADTRN, elogTrn);
			return;
		}
	}

	DWORD idSrc = m_mSocketId[sock];
	MapUser::const_iterator iu = m_mUser.find(idWhere);
	if (iu != m_mUser.end()) { // private talk
	} else {
		MapChannel::iterator ic = m_mChannel.find(idWhere);
		if (ic != m_mChannel.end()) { // channel
#ifdef CHEATS
			if (ic->second.getStatus(idSrc) >= eMember || isGod(idSrc))
#else
			if (ic->second.getStatus(idSrc) >= eMember)
#endif
			{
				ic->second.topic = topic;
				ic->second.idTopicWriter = idSrc;

				SetId set = ic->second.opened;
				set.insert(idSrc);
				BroadcastTrn(set, false, Make_Notify_TOPIC(idSrc, idWhere, topic));
			}
		}
	}
}

void JServer::Recv_Cmd_CHANOPTIONS(SOCKET sock, io::mem& is)
{
	DWORD idWhere;
	int op;
	DWORD val;

	try
	{
		io::unpack(is, idWhere);
		io::unpack(is, op);
		io::unpack(is, val);
	}
	catch (io::exception e)
	{
		switch (e.count)
		{
		case 0:
			// Report about message
			EvLog(SZ_BADTRN, elogTrn);
			return;

		case 1:
			op = CHANOP_ANONYMOUS;
			val = true;
			break;

		case 2:
			// Report about message
			EvLog(SZ_BADTRN, elogTrn);
			return;
		}
	}

	DWORD idSrc = m_mSocketId[sock];
	MapUser::const_iterator iu = m_mUser.find(idWhere);
	if (iu != m_mUser.end()) { // private talk
		ASSERT(op == CHANOP_BACKGROUND);
		PushTrn(m_mIdSocket[idWhere], Make_Notify_CHANOPTIONS(idSrc, idSrc, op, val));
	} else {
		MapChannel::iterator ic = m_mChannel.find(idWhere);
		if (ic != m_mChannel.end()) { // channel
#ifdef CHEATS
			if (ic->second.getStatus(idSrc) >= eAdmin || isGod(idSrc)) {
#else
			if (ic->second.getStatus(idSrc) >= eAdmin) {
#endif
				switch (op)
				{
				case CHANOP_AUTOSTATUS:
					ic->second.nAutoStatus = (EChanStatus)val;
					break;
				case CHANOP_LIMIT:
					ic->second.nLimit = (UINT)val;
					break;
				case CHANOP_HIDDEN:
					ic->second.isHidden = val != 0;
					break;
				case CHANOP_ANONYMOUS:
					ic->second.isAnonymous = val != 0;
					break;
				case CHANOP_BACKGROUND:
					ic->second.crBackground = (COLORREF)val;
					break;
				}

				SetId set = ic->second.opened;
				set.insert(idSrc);
				BroadcastTrn(set, false, Make_Notify_CHANOPTIONS(idSrc, idWhere, op, val));
			}
		}
	}
}

void JServer::Recv_Cmd_ACCESS(SOCKET sock, io::mem& is)
{
	DWORD idWho, idWhere;
	EChanStatus stat;

	try
	{
		io::unpack(is, idWho);
		io::unpack(is, idWhere);
		io::unpack(is, stat);
	}
	catch (io::exception e)
	{
		switch (e.count)
		{
		case 0:
			// Report about message
			EvLog(SZ_BADTRN, elogTrn);
			return;
		}
	}

	if (idWho == CRC_NONAME) idWho = m_mSocketId[sock];

	DWORD idSrc = m_mSocketId[sock];
	MapUser::const_iterator iu = m_mUser.find(idWhere);
	if (iu != m_mUser.end()) { // private talk
	} else {
		MapChannel::iterator ic = m_mChannel.find(idWhere);
		if (ic != m_mChannel.end()) { // channel
			EChanStatus statSrc = ic->second.getStatus(idSrc), statWho = ic->second.getStatus(idWho);
			bool canModer =
				(idWho == idSrc || statSrc > statWho || statSrc >= eModerator) &&
				(statSrc > stat || statSrc == eFounder);
#ifdef CHEATS
			if (canModer || isGod(idSrc)) {
#else
			if (canModer) {
#endif
				ic->second.setStatus(idWho, stat);

				SetId set = ic->second.opened;
				set.insert(idSrc);
				BroadcastTrn(set, false, Make_Notify_ACCESS(idWho, idWhere, stat, idSrc));
			}
		}
	}
}

void JServer::Recv_Cmd_BEEP(SOCKET sock, io::mem& is)
{
	DWORD idWho;

	try
	{
		io::unpack(is, idWho);
	}
	catch (io::exception e)
	{
		switch (e.count)
		{
		case 0:
			// Report about message
			EvLog(SZ_BADTRN, elogTrn);
			return;
		}
	}

	if (idWho == CRC_NONAME) idWho = m_mSocketId[sock];

	DWORD idBy = m_mSocketId[sock];
	MapUser::const_iterator iu = m_mUser.find(idWho);
	if (iu != m_mUser.end()) { // private talk
#ifdef CHEATS
		if (iu->second.accessibility.fCanSignal || isGod(idBy))
#else
		if (iu->second.accessibility.fCanSignal)
#endif
			PushTrn(m_mIdSocket[idWho], Make_Notify_BEEP(idBy));
	} else {
		MapChannel::iterator ic = m_mChannel.find(idWho);
		if (ic != m_mChannel.end()) { // channel
		}
	}
}

void JServer::Recv_Cmd_CLIPBOARD(SOCKET sock, io::mem& is)
{
	DWORD idWho;
	const void* ptr;
	size_t size;

	try
	{
		io::unpack(is, idWho);
		size = is.getsize();
		io::unpackptr(is, ptr, size);
	}
	catch (io::exception e)
	{
		switch (e.count)
		{
		case 0:
		case 1:
			// Report about message
			EvLog(SZ_BADTRN, elogTrn);
			return;
		}
	}

	if (idWho == CRC_NONAME) idWho = m_mSocketId[sock];

	DWORD idBy = m_mSocketId[sock];
	MapUser::const_iterator iu = m_mUser.find(idWho);
	if (iu != m_mUser.end()) { // private talk
#ifdef CHEATS
		if ((m_metrics.flags.bTransmitClipboard && iu->second.accessibility.fCanRecvClipboard) || isGod(idBy))
#else
		if (m_metrics.flags.bTransmitClipboard && iu->second.accessibility.fCanRecvClipboard)
#endif
			PushTrn(m_mIdSocket[idWho], Make_Notify_CLIPBOARD(idBy, (const char*)ptr, size));
	} else {
		MapChannel::iterator ic = m_mChannel.find(idWho);
		if (ic != m_mChannel.end()) { // channel
		}
	}
}

void JServer::Recv_Quest_MESSAGE(SOCKET sock, WORD trnid, io::mem& is, std::ostream& os)
{
	DWORD idWho;
	const void* ptr;
	size_t size;

	try
	{
		io::unpack(is, idWho);
		size = is.getsize();
		io::unpackptr(is, ptr, size);
	}
	catch (io::exception e)
	{
		switch (e.count)
		{
		case 0:
		case 1:
			// Report about message
			EvLog(SZ_BADTRN, elogTrn);
			return;
		}
	}

	if (idWho == CRC_NONAME) idWho = m_mSocketId[sock];

	FILETIME ft;
	GetSystemFileTime(ft);

	DWORD idBy = m_mSocketId[sock];
	MapUser::const_iterator iu = m_mUser.find(idWho);
	if (iu != m_mUser.end()) { // private talk
#ifdef CHEATS
		if (iu->second.accessibility.fCanMessage || isGod(idBy))
#else
		if (iu->second.accessibility.fCanMessage)
#endif
		{
			PushTrn(m_mIdSocket[idWho], Make_Notify_MESSAGE(idBy, ft, (const char*)ptr, size));
			Form_Reply_MESSAGE(os, idWho, MESSAGE_SENT);
		}
	} else {
		MapChannel::iterator ic = m_mChannel.find(idWho);
		if (ic != m_mChannel.end()) { // channel
			Form_Reply_MESSAGE(os, idWho, MESSAGE_IGNORE);
		} else {
			Form_Reply_MESSAGE(os, idWho, MESSAGE_IGNORE);
		}
	}
}

void JServer::Recv_Cmd_SPLASHRTF(SOCKET sock, io::mem& is)
{
	DWORD idWho;
	const void* ptr;
	size_t size;

	try
	{
		io::unpack(is, idWho);
		size = is.getsize();
		io::unpackptr(is, ptr, size);
	}
	catch (io::exception e)
	{
		switch (e.count)
		{
		case 0:
		case 1:
			// Report about message
			EvLog(SZ_BADTRN, elogTrn);
			return;
		}
	}

	if (idWho == CRC_NONAME) idWho = m_mSocketId[sock];

	DWORD idBy = m_mSocketId[sock];
	MapUser::const_iterator iu = m_mUser.find(idWho);
	if (iu != m_mUser.end()) { // private talk
#ifdef CHEATS
		if (iu->second.accessibility.fCanSplash || isGod(idBy))
#else
		if (iu->second.accessibility.fCanSplash)
#endif
			PushTrn(m_mIdSocket[idWho], Make_Notify_SPLASHRTF(idBy, (const char*)ptr, size));
	} else {
		MapChannel::iterator ic = m_mChannel.find(idWho);
		if (ic != m_mChannel.end()) { // channel
		}
	}
}

//
// Beowolf Network Protocol Messages sending
//

JPtr<JBTransaction> JServer::Make_Notify_METRICS(const Metrics& metrics) const
{
	std::ostringstream os;
	io::pack(os, metrics);
	return MakeTrn(NOTIFY(CCPM_METRICS), 0, os.str());
}

JPtr<JBTransaction> JServer::Make_Notify_NICK(DWORD result, DWORD idOld, DWORD idNew, const std::tstring& newname) const
{
	std::ostringstream os;
	io::pack(os, result);
	io::pack(os, idOld);
	io::pack(os, idNew);
	io::pack(os, newname);
	return MakeTrn(NOTIFY(CCPM_NICK), 0, os.str());
}

void JServer::Form_Reply_LIST(std::ostream& os, bool god) const
{
	// Count all visible channels
	size_t size = 0;
	for each (MapChannel::value_type const& v in m_mChannel)
	{
		if (!v.second.isHidden || god) size++;
	}
	io::pack(os, size);
	// Pack all visible channels
	for each (MapChannel::value_type const& v in m_mChannel)
	{
		if (!v.second.isHidden || god) {
			io::pack(os, v.first);
			io::pack(os, v.second);
		}
	}
}

void JServer::Form_Reply_JOIN_Result(std::ostream& os, DWORD result, EContact type, DWORD id) const
{
	io::pack(os, result);
	io::pack(os, type);
	io::pack(os, id);
}

void JServer::Form_Reply_JOIN_User(std::ostream& os, DWORD id, const User& user) const
{
	io::pack(os, (DWORD)CHAN_OK);
	io::pack(os, eUser);
	io::pack(os, id);
	io::pack(os, user);
}

void JServer::Form_Reply_JOIN_Channel(std::ostream& os, DWORD id, const Channel& chan) const
{
	io::pack(os, (DWORD)CHAN_OK);
	io::pack(os, eChannel);
	io::pack(os, id);
	io::pack(os, chan);
}

JPtr<JBTransaction> JServer::Make_Notify_JOIN(DWORD idWho, DWORD idWhere, const User& user) const
{
	std::ostringstream os;
	io::pack(os, idWho);
	io::pack(os, idWhere);
	io::pack(os, user);
	return MakeTrn(NOTIFY(CCPM_JOIN), 0, os.str());
}

JPtr<JBTransaction> JServer::Make_Notify_PART(DWORD idWho, DWORD idWhere, DWORD idBy) const
{
	std::ostringstream os;
	io::pack(os, idWho);
	io::pack(os, idWhere);
	io::pack(os, idBy);
	return MakeTrn(NOTIFY(CCPM_PART), 0, os.str());
}

void JServer::Form_Reply_USERINFO(std::ostream& os, const SetId& set) const
{
	// Validate given identifiers
	size_t count = 0;
	for each (SetId::value_type const& v in set)
		count++;

	io::pack(os, count);
	for each (SetId::value_type const& v in set) {
		MapUser::const_iterator iu = m_mUser.find(v);
		if (iu != m_mUser.end()) {
			io::pack(os, iu->first);
			io::pack(os, iu->second);
		}
	}
}

JPtr<JBTransaction> JServer::Make_Notify_ONLINE(DWORD idWho, EOnline online, DWORD id) const
{
	std::ostringstream os;
	io::pack(os, idWho);
	io::pack(os, online);
	io::pack(os, id);
	return MakeTrn(NOTIFY(CCPM_ONLINE), 0, os.str());
}

JPtr<JBTransaction> JServer::Make_Notify_STATUS(DWORD idWho, WORD type, EUserStatus stat, const Alert& a, int img, std::tstring msg) const
{
	std::ostringstream os;
	io::pack(os, idWho);
	io::pack(os, type);
	if (type & STATUS_MODE) {
		io::pack(os, stat);
		io::pack(os, a);
	}
	if (type & STATUS_IMG) {
		io::pack(os, img);
	}
	if (type & STATUS_MSG) {
		io::pack(os, msg);
	}
	return MakeTrn(NOTIFY(CCPM_STATUS), 0, os.str());
}

JPtr<JBTransaction> JServer::Make_Notify_STATUS_God(DWORD idWho, bool god) const
{
	std::ostringstream os;
	io::pack(os, idWho);
	io::pack(os, (WORD)STATUS_GOD);
	io::pack(os, god);
	return MakeTrn(NOTIFY(CCPM_STATUS), 0, os.str());
}

JPtr<JBTransaction> JServer::Make_Notify_STATUS_Devil(DWORD idWho, bool devil) const
{
	std::ostringstream os;
	io::pack(os, idWho);
	io::pack(os, (WORD)STATUS_DEVIL);
	io::pack(os, devil);
	return MakeTrn(NOTIFY(CCPM_STATUS), 0, os.str());
}

JPtr<JBTransaction> JServer::Make_Notify_SAY(DWORD idWho, DWORD idWhere, UINT type, const std::string& content) const
{
	std::ostringstream os;
	io::pack(os, idWho);
	io::pack(os, idWhere);
	io::pack(os, type);
	io::pack(os, content);
	return MakeTrn(NOTIFY(CCPM_SAY), 0, os.str());
}

JPtr<JBTransaction> JServer::Make_Notify_TOPIC(DWORD idWho, DWORD idWhere, const std::tstring& topic) const
{
	std::ostringstream os;
	io::pack(os, idWho);
	io::pack(os, idWhere);
	io::pack(os, topic);
	return MakeTrn(NOTIFY(CCPM_TOPIC), 0, os.str());
}

JPtr<JBTransaction> JServer::Make_Notify_CHANOPTIONS(DWORD idWho, DWORD idWhere, int op, DWORD val) const
{
	std::ostringstream os;
	io::pack(os, idWho);
	io::pack(os, idWhere);
	io::pack(os, op);
	io::pack(os, val);
	return MakeTrn(NOTIFY(CCPM_CHANOPTIONS), 0, os.str());
}

JPtr<JBTransaction> JServer::Make_Notify_ACCESS(DWORD idWho, DWORD idWhere, EChanStatus stat, DWORD idBy) const
{
	std::ostringstream os;
	io::pack(os, idWho);
	io::pack(os, idWhere);
	io::pack(os, stat);
	io::pack(os, idBy);
	return MakeTrn(NOTIFY(CCPM_ACCESS), 0, os.str());
}

JPtr<JBTransaction> JServer::Make_Notify_BEEP(DWORD idBy) const
{
	std::ostringstream os;
	io::pack(os, idBy);
	return MakeTrn(NOTIFY(CCPM_BEEP), 0, os.str());
}

JPtr<JBTransaction> JServer::Make_Notify_CLIPBOARD(DWORD idBy, const char* ptr, size_t size) const
{
	std::ostringstream os;
	io::pack(os, idBy);
	os.write(ptr, (std::streamsize)size);
	return MakeTrn(NOTIFY(CCPM_CLIPBOARD), 0, os.str());
}

JPtr<JBTransaction> JServer::Make_Notify_MESSAGE(DWORD idBy, const FILETIME& ft, const char* ptr, size_t size) const
{
	std::ostringstream os;
	io::pack(os, idBy);
	io::pack(os, ft);
	os.write(ptr, (std::streamsize)size);
	return MakeTrn(NOTIFY(CCPM_MESSAGE), 0, os.str());
}

void JServer::Form_Reply_MESSAGE(std::ostream& os, DWORD idWho, UINT type) const
{
	io::pack(os, idWho);
	io::pack(os, type);
}

JPtr<JBTransaction> JServer::Make_Notify_SPLASHRTF(DWORD idBy, const char* ptr, size_t size) const
{
	std::ostringstream os;
	io::pack(os, idBy);
	os.write(ptr, (std::streamsize)size);
	return MakeTrn(NOTIFY(CCPM_SPLASHRTF), 0, os.str());
}

void JServer::Connections()
{
	if (m_hwndPage)
	{
		if (jpConnections->hwndPage)
			SetForegroundWindow(jpConnections->hwndPage);
		else
			CreateDialogParam(JServerApp::jpApp->hinstApp, MAKEINTRESOURCE(IDD_CONNECTIONS), m_hwndPage, JConnections::DlgProcStub, (LPARAM)(JDialog*)jpConnections);
	}
}

void JServer::Passwords()
{
	if (m_hwndPage)
	{
		if (jpPasswords->hwndPage)
			SetForegroundWindow(jpPasswords->hwndPage);
		else
			CreateDialogParam(JServerApp::jpApp->hinstApp, MAKEINTRESOURCE(IDD_PASSWORDS), m_hwndPage, JPasswords::DlgProcStub, (LPARAM)(JDialog*)jpPasswords);
	}
}

void JServer::About()
{
	if (m_hwndPage && m_bShowIcon)
	{
		NOTIFYICONDATA tnid;
		tnid.cbSize = sizeof(NOTIFYICONDATA);
		tnid.hWnd = m_hwndPage;
		tnid.uID = 1;
		tnid.uFlags = NIF_INFO;
		tnid.uTimeout = 10;
		tnid.dwInfoFlags = NIIF_INFO;
		tnid.uVersion = NOTIFYICON_VERSION;
		LoadString(0, IDS_ABOUT_INFO, tnid.szInfo, _countof(tnid.szInfo));
		LoadString(0, IDS_ABOUT_TITLE, tnid.szInfoTitle, _countof(tnid.szInfoTitle));
		Shell_NotifyIcon(NIM_MODIFY, &tnid);
	}
}

//-----------------------------------------------------------------------------

JPtr<JServerApp> JServerApp::jpApp = new JServerApp();

JServerApp::JServerApp(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR szcl, int ncs)
: ::JApplication(hInstance, hPrevInstance, szcl, ncs),
jpServer(0)
{
	sAppName = APPNAME;
}

void JServerApp::Init()
{
	static WNDCLASS MsgClass =
	{
		CS_DBLCLKS | CS_GLOBALCLASS,      // style - redraw if size changes.
		JServer::WndProcStub,       // lpfnWndProc - points to window proc.
		0,                           // cbClsExtra - no extra class memory.
		sizeof(HGLOBAL),             // cbWndExtra - 4 or 2 bytes extra window memory.
		0,                            // hInstance - handle of instance.
		0,                                // hIcon - default application icon.
		0,                              // hCursor - default application cursor.
		(HBRUSH)(COLOR_MENU + 1), // hbrBackground - window background brush.
		0,                         // lpszMenuName - menu resource from instance.
		WC_MSG                    // lpszClassName - name of window class.
	};
	// Register the window RH-server classes.
	VERIFY(RegisterClass(&MsgClass));

	profile::setKey(TEXT("BEOWOLF"), APPNAME);

	hiMain16 = LoadIcon(hinstApp, MAKEINTRESOURCE(IDI_SMALL));
	hiMain32 = LoadIcon(hinstApp, MAKEINTRESOURCE(IDI_SERVER));

	// Load popup menus
	m_hmenuConnections = LoadMenu(hinstApp, MAKEINTRESOURCE(IDM_CONNECTIONS));

	jpServer = new JServer;
	jpServer->Init();
}

bool JServerApp::InitInstance()
{
	return true;
}

void JServerApp::Done()
{
	if (jpServer->State != JService::eStopped) jpServer->Stop();
	jpServer->Done();
	// Free associated resources
	VERIFY(DestroyMenu(m_hmenuConnections));
	// Unregister the window RH-server classes.
	VERIFY(UnregisterClass(WC_MSG, hinstApp));
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

		JServerApp::jpApp->hinstApp = hInstance;
		JServerApp::jpApp->hinstPrev = hPrevInstance;
		JServerApp::jpApp->lpszCmdLine = lpszCmdLine;
		JServerApp::jpApp->nCmdShow = nCmdShow;

		return JServerApp::jpApp->Iteration();
	}
	catch (std::exception& e)
	{
		MessageBoxA(0, format("%s\r\n%s", typeid(e).name(), e.what()).c_str(), "Unhandled Exception!", MB_OK | MB_ICONSTOP);
	}
	return -1;
}

//-----------------------------------------------------------------------------

// The End.