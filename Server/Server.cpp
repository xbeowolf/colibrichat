
//-----------------------------------------------------------------------------

//
// Includes
//

#pragma region Includes

#include "stdafx.h"

// Windows API
#include <strsafe.h>

// Common
#include "dCRC.h"
#include "Profile.h"

// Project
#include "..\ColibriProtocol.h"
#include "resource.h"
#include "server.h"

#pragma endregion

using namespace colibrichat;

//-----------------------------------------------------------------------------

// Global Variables:
static TCHAR szHelpFile[MAX_PATH];

//-----------------------------------------------------------------------------

//
// class JServer
//

CALLBACK JServer::JServer()
: JEngine(), JWindow()
{
	// Dialogs
	jpConnections = new JConnections();

	m_IP = INADDR_ANY;
	m_port = CCP_PORT;
	m_password = TEXT("beowolf");

	m_bShowIcon = true;

	m_metrics.uNameMaxLength = 20;
	m_metrics.uPassMaxLength = 32;
	m_metrics.uTopicMaxLength = 100;
	m_metrics.uStatusMsgMaxLength = 32;
	m_metrics.nMsgSpinMaxCount = 20;
	m_metrics.uChatLineMaxSize = 80*1024;
}

void CALLBACK JServer::Init()
{
	jpConnections->SetSource(this);
	jpConnections->SetupHooks();

	__super::Init();

	m_mSocketId.clear();
	m_mIdSocket.clear();
	m_mUser.clear();
	m_mChannel.clear();

	CreateMsgWindow();

	// Create listening socket
	Link link;
	link.m_saAddr.sin_family = AF_INET;
	link.m_saAddr.sin_addr.S_un.S_addr = htonl(m_IP);
	link.m_saAddr.sin_port = htons(m_port);
	link.Select(FD_ACCEPT | FD_CLOSE);
	link.Listen();
	InsertLink(link);
}

void CALLBACK JServer::Done()
{
	__super::Done();

	DestroyMsgWindow();
}

HWND CALLBACK JServer::CreateMsgWindow()
{
	if (!IsWindow(m_hwndPage)) m_hwndPage = CreateWindow(WC_MSG, WT_MSG,
		WS_OVERLAPPEDWINDOW,
		0, 0, 100, 100,
		HWND_MESSAGE,
		0, 0, (JWindow*)this);
	return m_hwndPage;
}

BOOL CALLBACK JServer::DestroyMsgWindow()
{
	return DestroyWindow(m_hwndPage);
}

void CALLBACK JServer::LoadState()
{
}

void CALLBACK JServer::SaveState()
{
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

bool CALLBACK JServer::CheckAccess(const TCHAR* password, SetAccess& access) const
{
	if (!password) {
		__super::CheckAccess(password, access);
	} else if (m_password == password || m_password.empty()) {
		access.insert(BNPM_MESSAGE);
	} else return false;
	return true;
}

bool CALLBACK JServer::hasCRC(DWORD crc) const
{
	return
		crc == CRC_SERVER ||
		crc == CRC_LIST ||
		crc == CRC_NONAME ||
		crc == CRC_ANONYMOUS ||
		m_mUser.find(crc) != m_mUser.end() ||
		m_mChannel.find(crc) != m_mChannel.end();
}

bool CALLBACK JServer::linkCRC(DWORD crc1, DWORD crc2)
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

void CALLBACK JServer::unlinkCRC(DWORD crc1, DWORD crc2)
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

bool CALLBACK JServer::CheckNick(std::tstring& nick, const TCHAR*& msg)
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

std::tstring CALLBACK JServer::getNearestName(const std::tstring& nick) const
{
	std::tstring buffer = nick;
	TCHAR digits[16];
	DWORD i = 0;
	while (hasCRC(dCRC(buffer.c_str()))) {
		StringCchPrintf(digits, _countof(digits), TEXT("%u"), i);
		buffer = nick.substr(0, m_metrics.uNameMaxLength - lstrlen(digits)) + digits;
		i++;
	}
	return buffer;
}

void CALLBACK JServer::RenameContact(SOCKET sock, DWORD idOld, std::tstring newname)
{
	DWORD idNew = dCRC(newname.c_str());
	if (idNew == idOld) return;

	DWORD result = NICK_TAKEN;
	if (hasCRC(idNew)) {
		if (idNew == CRC_SERVER || idNew == CRC_LIST) {
			result = NICK_TAKEN;
		} else if (m_mUser.find(idNew) != m_mUser.end()) {
			result = NICK_TAKENUSER;
		} else if (m_mChannel.find(idNew) != m_mChannel.end()) {
			result = NICK_TAKENCHANNEL;
		}
		newname = getNearestName(newname);
		idNew = dCRC(newname.c_str());
	} else {
		result = NICK_OK;
	}

	SetId opened;
	MapUser::iterator iu = m_mUser.find(idOld);
	if (iu != m_mUser.end()) { // private talk
		opened = iu->second.opened;
		iu->second.name = newname;
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
		opened.insert(idNew); // send reply to sender
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
		} else { // create new user by default
			sockaddr_in si;
			int len = sizeof(si);
			getpeername(sock, (struct sockaddr*)&si, &len);

			// Create new user
			User user;
			user.Init();
			user.name = newname;
			user.IP = si.sin_addr;

			m_mUser[idNew] = user;
			m_mSocketId[sock] = idNew;
			m_mIdSocket[idNew] = sock;

			opened.insert(idNew); // send reply to sender
		}
	}
	Broadcast_Notify_NICK(opened, result, idOld, idNew, newname);
}

void JServer::OnHook(JEventable* src)
{
	using namespace fastdelegate;

	__super::OnHook(src);
}

void JServer::OnUnhook(JEventable* src)
{
	using namespace fastdelegate;

	__super::OnUnhook(src);
}

void JServer::OnLinkClose(SOCKET sock, UINT err)
{
	__super::OnLinkClose(sock, err);

	DWORD idSrc = m_mSocketId[sock];
	MapUser::iterator iter = m_mUser.find(idSrc);
	if (iter != m_mUser.end()) {
		SetId opened = iter->second.opened;
		for each (SetId::value_type const& v in opened) {
			unlinkCRC(idSrc, v);
			MapUser::const_iterator iu = m_mUser.find(v);
			if (iu != m_mUser.end()) { // private talk
				Send_Notify_PART(m_mIdSocket[v], idSrc, idSrc, CRC_SERVER);
			} else {
				MapChannel::const_iterator ic = m_mChannel.find(v);
				if (ic != m_mChannel.end()) { // channel
					Broadcast_Notify_PART(ic->second.opened, idSrc, v, CRC_SERVER);
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
		StringCchPrintf(tnid.szTip, _countof(tnid.szTip), APPNAME TEXT("\n%u connections"), countEstablished());
		Shell_NotifyIcon(NIM_MODIFY, &tnid);
	}
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
		StringCchPrintf(tnid.szTip, _countof(tnid.szTip), APPNAME TEXT("\n%u connections"), countEstablished());
		Shell_NotifyIcon(NIM_MODIFY, &tnid);
	}
}

void JServer::OnLinkPassword(SOCKET sock, const TCHAR* password, const SetAccess& access)
{
	__super::OnLinkPassword(sock, password, access);

	Send_Notify_METRICS(sock, m_metrics);
}

void JServer::OnTransactionProcess(SOCKET sock, WORD message, WORD trnid, io::mem is)
{
	typedef void (CALLBACK JServer::*TrnParser)(SOCKET, WORD, io::mem&);
	struct {
		WORD message;
		TrnParser parser;
	} responseTable[] =
	{
		{COMMAND(CCPM_NICK), &JServer::Recv_Cmd_NICK},
		{QUEST(CCPM_LIST), &JServer::Recv_Quest_LIST},
		{QUEST(CCPM_JOIN), &JServer::Recv_Quest_JOIN},
		{COMMAND(CCPM_PART), &JServer::Recv_Cmd_PART},
		{QUEST(CCPM_USERINFO), &JServer::Recv_Quest_USERINFO},
		{COMMAND(CCPM_ONLINE), &JServer::Recv_Cmd_ONLINE},
		{COMMAND(CCPM_STATUS), &JServer::Recv_Cmd_STATUS},
		{COMMAND(CCPM_SAY), &JServer::Recv_Cmd_SAY},
		{COMMAND(CCPM_TOPIC), &JServer::Recv_Cmd_TOPIC},
		{COMMAND(CCPM_CHANOPTIONS), &JServer::Recv_Cmd_CHANOPTIONS},
		{COMMAND(CCPM_ACCESS), &JServer::Recv_Cmd_ACCESS},
		{COMMAND(CCPM_BEEP), &JServer::Recv_Cmd_BEEP},
		{COMMAND(CCPM_CLIPBOARD), &JServer::Recv_Cmd_CLIPBOARD},
		{QUEST(CCPM_MESSAGE), &JServer::Recv_Quest_MESSAGE},
		{COMMAND(CCPM_SPLASHRTF), &JServer::Recv_Cmd_SPLASHRTF},
	};
	for (int i = 0; i < _countof(responseTable); i++) {
		if (responseTable[i].message == message) {
			(this->*responseTable[i].parser)(sock, trnid, is);
			return;
		}
	}

	__super::OnTransactionProcess(sock, message, trnid, is);
}

int  CALLBACK JServer::BroadcastTrn(const SetId& set, bool nested, WORD message, const std::string& str, size_t ssi) throw()
{
	ASSERT(NATIVEACTION(message) != BNPM_REPLY); // can not broadcast reply

	// Count users to prevent duplicate sents
	SetSocket broadcast;
	MapIdSocket::const_iterator iis;
	for each (SetId::value_type const& v in set)
	{
		iis = m_mIdSocket.find(v);
		if (iis != m_mIdSocket.end()) // private talk
			broadcast.insert(iis->second);
		else {
			if (nested) {
				MapChannel::const_iterator ic = m_mChannel.find(v);
				if (ic != m_mChannel.end()) { // channel
					for each (SetId::value_type const& v in ic->second.opened) {
						iis = m_mIdSocket.find(v);
						if (iis != m_mIdSocket.end())
							broadcast.insert(iis->second);
					}
				}
			}
		}
	}

	// Broadcast to unical users
	for each (SetSocket::value_type const& v in broadcast) {
		PushTrn(v, message, 0, str, ssi);
	}
	return (int)broadcast.size();
}

//
// Beowolf Network Protocol Messages reciving
//

void CALLBACK JServer::Recv_Cmd_NICK(SOCKET sock, WORD trnid, io::mem& is)
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
			EvReport(SZ_BADTRN, eWarning, eLow);
			return;
		}
	}

	DWORD idBy = m_mSocketId[sock];
	MapUser::iterator iu = m_mUser.find(idOld);
	if (iu != m_mUser.end()) { // private talk
		if (idOld == idBy) {
			RenameContact(sock, idOld, name);
			// Report about message
			EvReport(tformat(TEXT("nickname renamed: %s"), name.c_str()), eInformation, eNormal);
		}
	} else if (idOld == CRC_NONAME) { // new user
		RenameContact(sock, idOld, name);
		// Report about message
		EvReport(tformat(TEXT("nickname added: %s"), name.c_str()), eInformation, eNormal);
	} else {
		MapChannel::const_iterator ic = m_mChannel.find(idOld);
		if (ic != m_mChannel.end()) { // channel
			if (ic->second.getStatus(idBy) == eFounder) {
				RenameContact(sock, idOld, name);
				// Report about message
				EvReport(tformat(TEXT("channel name modified to: %s"), name.c_str()), eInformation, eNormal);
			}
		}
	}
}

void CALLBACK JServer::Recv_Quest_LIST(SOCKET sock, WORD trnid, io::mem& is)
{
	Send_Reply_LIST(sock, trnid);

	// Report about message
	EvReport(tformat(TEXT("channels list")), eInformation, eNormal);
}

void CALLBACK JServer::Recv_Quest_JOIN(SOCKET sock, WORD trnid, io::mem& is)
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
			EvReport(SZ_BADTRN, eWarning, eLow);
			return;

		case 1:
			pass = TEXT("");
		case 2:
			type = eUser | eChannel | eBoard;
		}
	}

	DWORD idDst = dCRC(name.c_str());
	DWORD idSrc = m_mSocketId[sock];
	MapUser::const_iterator iu = m_mUser.find(idDst);
	if (iu != m_mUser.end() && type & eUser) { // private talk
		if (iu->second.opened.find(idDst) == iu->second.opened.end()) {
			Send_Reply_JOIN_User(sock, trnid, idDst, iu->second);
			Send_Notify_JOIN(m_mIdSocket[idDst], idSrc, idDst, m_mUser[idSrc]);
			linkCRC(idDst, idSrc);
		} else {
			Send_Reply_JOIN_Result(sock, trnid, CHAN_ALREADY, eUser, idDst);
		}
	} else {
		MapChannel::iterator ic = m_mChannel.find(idDst);
		if (ic != m_mChannel.end() && type & eChannel) { // channel
			if (ic->second.opened.size() < ic->second.nLimit) {
				if (ic->second.nAutoStatus > eOutsider) {
					if (ic->second.password.empty() || ic->second.password == pass) {
						if (ic->second.opened.find(idSrc) == ic->second.opened.end()) {
							if (ic->second.getStatus(idSrc) == eOutsider) {
								ic->second.setStatus(idSrc, ic->second.nAutoStatus);
							}
							Broadcast_Notify_JOIN(ic->second.opened, idSrc, idDst, m_mUser[idSrc]);
							linkCRC(idDst, idSrc);
							Send_Reply_JOIN_Channel(sock, trnid, idDst, ic->second);
						} else {
							Send_Reply_JOIN_Result(sock, trnid, CHAN_ALREADY, eChannel, idDst);
						}
					} else {
						Send_Reply_JOIN_Result(sock, trnid, CHAN_BADPASS, eChannel, idDst);
					}
				} else {
					Send_Reply_JOIN_Result(sock, trnid, CHAN_DENY, eChannel, idDst);
				}
			} else {
				Send_Reply_JOIN_Result(sock, trnid, CHAN_LIMIT, eChannel, idDst);
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
				chan.founder.insert(idSrc);
				chan.nAutoStatus = eWriter;
				chan.nLimit = -1;
				chan.isHidden = false;
				chan.isAnonymous = false;
				chan.crBackground = RGB(0xFF, 0xFF, 0xFF);
				m_mChannel[idDst] = chan;

				linkCRC(idDst, idSrc);
				Send_Reply_JOIN_Channel(sock, trnid, idDst, m_mChannel[idDst]);
			} else if (type & eBoard) {
				Send_Reply_JOIN_Result(sock, trnid, CHAN_ABSENT, eBoard, idDst);
			} else Send_Reply_JOIN_Result(sock, trnid, CHAN_ABSENT, (EContact)type, idDst);
		}
	}

	// Report about message
	EvReport(tformat(TEXT("joins %s to %s"), m_mUser[idSrc].name.c_str(), name.c_str()), eInformation, eNormal);
}

void CALLBACK JServer::Recv_Cmd_PART(SOCKET sock, WORD trnid, io::mem& is)
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
			EvReport(SZ_BADTRN, eWarning, eLow);
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
		EvReport(tformat(TEXT("parts %s from %s"), iuWho->second.name.c_str(), iu->second.name.c_str()), eInformation, eNormal);

		Send_Notify_PART(m_mIdSocket[idWhere], idWho, idWho, idBy); // recieves to close private with idWho
		Send_Notify_PART(sock, idBy, idWhere, idBy);
		unlinkCRC(idWho, idWhere);
	} else {
		MapChannel::const_iterator ic = m_mChannel.find(idWhere);
		if (ic != m_mChannel.end()) { // channel
			bool isModer = ic->second.getStatus(idBy) >= eModerator;
			bool canKick = ic->second.getStatus(idBy) >= ic->second.getStatus(idWho);
			if (idWho == idBy || (isModer && canKick)) {
				// Report about message
				EvReport(tformat(TEXT("parts %s from %s"), iuWho->second.name.c_str(), ic->second.name.c_str()), eInformation, eNormal);

				ic = m_mChannel.find(idWhere);
				if (ic != m_mChannel.end()) { // check that channel still exist
					Broadcast_Notify_PART(ic->second.opened, idWho, idWhere, idBy);
				}
				unlinkCRC(idWho, idWhere);
			}
		}
	}
}

void CALLBACK JServer::Recv_Quest_USERINFO(SOCKET sock, WORD trnid, io::mem& is)
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
			EvReport(SZ_BADTRN, eWarning, eLow);
			return;
		}
	}

	Send_Reply_USERINFO(sock, trnid, wanted);

	// Report about message
	EvReport(tformat(TEXT("info for %u users"), wanted.size()), eInformation, eNormal);
}

void CALLBACK JServer::Recv_Cmd_ONLINE(SOCKET sock, WORD trnid, io::mem& is)
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
		Broadcast_Notify_ONLINE(set, idSrc, isOnline, idOnline);
	}

	// Report about message
	EvReport(tformat(TEXT("user is %s"), isOnline ? TEXT("online") : TEXT("offline")), eInformation, eLowest);
}

void CALLBACK JServer::Recv_Cmd_STATUS(SOCKET sock, WORD trnid, io::mem& is)
{
	WORD type;
	EUserStatus stat;
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
			EvReport(SZ_BADTRN, eWarning, eLow);
			return;
		}
	}

	DWORD idSrc = m_mSocketId[sock];
	MapUser::iterator iu = m_mUser.find(idSrc);
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

		SetId set = iu->second.opened;
		set.insert(idSrc);
		Broadcast_Notify_STATUS(set, idSrc, type, stat, img, msg);
	}

	// Report about message
	EvReport(tformat(TEXT("user changes status")), eInformation, eLowest);
}

void CALLBACK JServer::Recv_Cmd_SAY(SOCKET sock, WORD trnid, io::mem& is)
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
			EvReport(SZ_BADTRN, eWarning, eLow);
			return;
		}
	}

	DWORD idSrc = m_mSocketId[sock];
	MapUser::const_iterator iu = m_mUser.find(idWhere);
	if (iu != m_mUser.end()) { // private talk
		Send_Notify_SAY(m_mIdSocket[idWhere], idSrc, idSrc, type, content);
		Send_Notify_SAY(sock, idSrc, idWhere, type, content);
	} else {
		MapChannel::const_iterator ic = m_mChannel.find(idWhere);
		if (ic != m_mChannel.end()) { // channel
			if (ic->second.getStatus(idSrc) > eReader)
				Broadcast_Notify_SAY(ic->second.opened, !ic->second.isAnonymous ? idSrc : CRC_ANONYMOUS, idWhere, type, content);
		}
	}
}

void CALLBACK JServer::Recv_Cmd_TOPIC(SOCKET sock, WORD trnid, io::mem& is)
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
			EvReport(SZ_BADTRN, eWarning, eLow);
			return;
		}
	}

	DWORD idSrc = m_mSocketId[sock];
	MapUser::const_iterator iu = m_mUser.find(idWhere);
	if (iu != m_mUser.end()) { // private talk
	} else {
		MapChannel::iterator ic = m_mChannel.find(idWhere);
		if (ic != m_mChannel.end()) { // channel
			if (ic->second.getStatus(idSrc) >= eMember) {
				ic->second.topic = topic;
				ic->second.idTopicWriter = idSrc;

				Broadcast_Notify_TOPIC(ic->second.opened, idSrc, idWhere, topic);
			}
		}
	}
}

void CALLBACK JServer::Recv_Cmd_CHANOPTIONS(SOCKET sock, WORD trnid, io::mem& is)
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
			EvReport(SZ_BADTRN, eWarning, eLow);
			return;

		case 1:
			op = CHANOP_ANONYMOUS;
			val = true;
			break;

		case 2:
			// Report about message
			EvReport(SZ_BADTRN, eWarning, eLow);
			return;
		}
	}

	DWORD idSrc = m_mSocketId[sock];
	MapUser::const_iterator iu = m_mUser.find(idWhere);
	if (iu != m_mUser.end()) { // private talk
		ASSERT(op == CHANOP_BACKGROUND);
		Send_Notify_CHANOPTIONS(m_mIdSocket[idWhere], idSrc, idSrc, op, val);
	} else {
		MapChannel::iterator ic = m_mChannel.find(idWhere);
		if (ic != m_mChannel.end()) { // channel
			if (ic->second.getStatus(idSrc) >= eAdmin) {
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

				Broadcast_Notify_CHANOPTIONS(ic->second.opened, idSrc, idWhere, op, val);
			}
		}
	}
}

void CALLBACK JServer::Recv_Cmd_ACCESS(SOCKET sock, WORD trnid, io::mem& is)
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
			EvReport(SZ_BADTRN, eWarning, eLow);
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
			ic->second.setStatus(idWho, stat);
			Broadcast_Notify_ACCESS(ic->second.opened, idWho, idWhere, stat, idSrc);
		}
	}
}

void CALLBACK JServer::Recv_Cmd_BEEP(SOCKET sock, WORD trnid, io::mem& is)
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
			EvReport(SZ_BADTRN, eWarning, eLow);
			return;
		}
	}

	if (idWho == CRC_NONAME) idWho = m_mSocketId[sock];

	DWORD idBy = m_mSocketId[sock];
	MapUser::const_iterator iu = m_mUser.find(idWho);
	if (iu != m_mUser.end()) { // private talk
		Send_Notify_BEEP(m_mIdSocket[idWho], idBy);
	} else {
		MapChannel::iterator ic = m_mChannel.find(idWho);
		if (ic != m_mChannel.end()) { // channel
		}
	}
}

void CALLBACK JServer::Recv_Cmd_CLIPBOARD(SOCKET sock, WORD trnid, io::mem& is)
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
			EvReport(SZ_BADTRN, eWarning, eLow);
			return;
		}
	}

	if (idWho == CRC_NONAME) idWho = m_mSocketId[sock];

	DWORD idBy = m_mSocketId[sock];
	MapUser::const_iterator iu = m_mUser.find(idWho);
	if (iu != m_mUser.end()) { // private talk
		Send_Notify_CLIPBOARD(m_mIdSocket[idWho], idBy, (const char*)ptr, size);
	} else {
		MapChannel::iterator ic = m_mChannel.find(idWho);
		if (ic != m_mChannel.end()) { // channel
		}
	}
}

void CALLBACK JServer::Recv_Quest_MESSAGE(SOCKET sock, WORD trnid, io::mem& is)
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
			EvReport(SZ_BADTRN, eWarning, eLow);
			return;
		}
	}

	if (idWho == CRC_NONAME) idWho = m_mSocketId[sock];

	FILETIME ft;
	GetSystemFileTime(ft);

	DWORD idBy = m_mSocketId[sock];
	MapUser::const_iterator iu = m_mUser.find(idWho);
	if (iu != m_mUser.end()) { // private talk
		Send_Notify_MESSAGE(m_mIdSocket[idWho], idBy, ft, (const char*)ptr, size);
		Send_Reply_MESSAGE(sock, trnid, idWho, MESSAGE_SENT);
	} else {
		MapChannel::iterator ic = m_mChannel.find(idWho);
		if (ic != m_mChannel.end()) { // channel
			Send_Reply_MESSAGE(sock, trnid, idWho, MESSAGE_IGNORE);
		} else {
			Send_Reply_MESSAGE(sock, trnid, idWho, MESSAGE_IGNORE);
		}
	}
}

void CALLBACK JServer::Recv_Cmd_SPLASHRTF(SOCKET sock, WORD trnid, io::mem& is)
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
			EvReport(SZ_BADTRN, eWarning, eLow);
			return;
		}
	}

	if (idWho == CRC_NONAME) idWho = m_mSocketId[sock];

	DWORD idBy = m_mSocketId[sock];
	MapUser::const_iterator iu = m_mUser.find(idWho);
	if (iu != m_mUser.end()) { // private talk
		Send_Notify_SPLASHRTF(m_mIdSocket[idWho], idBy, (const char*)ptr, size);
	} else {
		MapChannel::iterator ic = m_mChannel.find(idWho);
		if (ic != m_mChannel.end()) { // channel
		}
	}
}

//
// Beowolf Network Protocol Messages sending
//

void CALLBACK JServer::Send_Notify_METRICS(SOCKET sock, const Metrics& metrics)
{
	std::ostringstream os;
	io::pack(os, metrics);
	PushTrn(sock, NOTIFY(CCPM_METRICS), 0, os.str());
}

void CALLBACK JServer::Broadcast_Notify_NICK(const SetId& set, DWORD result, DWORD idOld, DWORD idNew, const std::tstring& newname)
{
	std::ostringstream os;
	io::pack(os, result);
	io::pack(os, idOld);
	io::pack(os, idNew);
	io::pack(os, newname);
	BroadcastTrn(set, true, NOTIFY(CCPM_NICK), os.str());
}

void CALLBACK JServer::Send_Reply_LIST(SOCKET sock, WORD trnid)
{
	std::ostringstream os;
	// Count all visible channels
	size_t size = 0;
	for each (MapChannel::value_type const& v in m_mChannel)
	{
		if (!v.second.isHidden) size++;
	}
	io::pack(os, size);
	// Pack all visible channels
	for each (MapChannel::value_type const& v in m_mChannel)
	{
		if (!v.second.isHidden) {
			io::pack(os, v.first);
			io::pack(os, v.second);
		}
	}
	PushTrn(sock, REPLY(CCPM_LIST), trnid, os.str());
}

void CALLBACK JServer::Send_Reply_JOIN_Result(SOCKET sock, WORD trnid, DWORD result, EContact type, DWORD id)
{
	std::ostringstream os;
	io::pack(os, result);
	io::pack(os, type);
	io::pack(os, id);
	PushTrn(sock, REPLY(CCPM_JOIN), trnid, os.str());
}

void CALLBACK JServer::Send_Reply_JOIN_User(SOCKET sock, WORD trnid, DWORD id, const User& user)
{
	std::ostringstream os;
	io::pack(os, (DWORD)CHAN_OK);
	io::pack(os, eUser);
	io::pack(os, id);
	io::pack(os, user);
	PushTrn(sock, REPLY(CCPM_JOIN), trnid, os.str());
}

void CALLBACK JServer::Send_Reply_JOIN_Channel(SOCKET sock, WORD trnid, DWORD id, const Channel& chan)
{
	std::ostringstream os;
	io::pack(os, (DWORD)CHAN_OK);
	io::pack(os, eChannel);
	io::pack(os, id);
	io::pack(os, chan);
	PushTrn(sock, REPLY(CCPM_JOIN), trnid, os.str());
}

void CALLBACK JServer::Send_Notify_JOIN(SOCKET sock, DWORD idWho, DWORD idWhere, const User& user)
{
	std::ostringstream os;
	io::pack(os, idWho);
	io::pack(os, idWhere);
	io::pack(os, user);
	PushTrn(sock, NOTIFY(CCPM_JOIN), 0, os.str());
}

void CALLBACK JServer::Broadcast_Notify_JOIN(const SetId& set, DWORD idWho, DWORD idWhere, const User& user)
{
	std::ostringstream os;
	io::pack(os, idWho);
	io::pack(os, idWhere);
	io::pack(os, user);
	BroadcastTrn(set, false, NOTIFY(CCPM_JOIN), os.str());
}

void CALLBACK JServer::Send_Notify_PART(SOCKET sock, DWORD idWho, DWORD idWhere, DWORD idBy)
{
	std::ostringstream os;
	io::pack(os, idWho);
	io::pack(os, idWhere);
	io::pack(os, idBy);
	PushTrn(sock, NOTIFY(CCPM_PART), 0, os.str());
}

void CALLBACK JServer::Broadcast_Notify_PART(const SetId& set, DWORD idWho, DWORD idWhere, DWORD idBy)
{
	std::ostringstream os;
	io::pack(os, idWho);
	io::pack(os, idWhere);
	io::pack(os, idBy);
	BroadcastTrn(set, false, NOTIFY(CCPM_PART), os.str());
}

void CALLBACK JServer::Send_Reply_USERINFO(SOCKET sock, WORD trnid, const SetId& set)
{
	// Validate given identifiers
	size_t count = 0;
	for each (SetId::value_type const& v in set)
		count++;

	std::ostringstream os;
	io::pack(os, count);
	for each (SetId::value_type const& v in set) {
		MapUser::const_iterator iu = m_mUser.find(v);
		if (iu != m_mUser.end()) {
			io::pack(os, iu->first);
			io::pack(os, iu->second);
		}
	}
	PushTrn(sock, REPLY(CCPM_USERINFO), trnid, os.str());
}

void CALLBACK JServer::Broadcast_Notify_ONLINE(const SetId& set, DWORD idWho, EOnline online, DWORD id)
{
	std::ostringstream os;
	io::pack(os, idWho);
	io::pack(os, online);
	io::pack(os, id);
	BroadcastTrn(set, true, NOTIFY(CCPM_ONLINE), os.str());
}

void CALLBACK JServer::Broadcast_Notify_STATUS(const SetId& set, DWORD idWho, WORD type, EUserStatus stat, int img, std::tstring msg)
{
	std::ostringstream os;
	io::pack(os, idWho);
	io::pack(os, type);
	if (type & STATUS_MODE) {
		io::pack(os, stat);
	}
	if (type & STATUS_IMG) {
		io::pack(os, img);
	}
	if (type & STATUS_MSG) {
		io::pack(os, msg);
	}
	BroadcastTrn(set, true, NOTIFY(CCPM_STATUS), os.str());
}

void CALLBACK JServer::Send_Notify_SAY(SOCKET sock, DWORD idWho, DWORD idWhere, UINT type, const std::string& content)
{
	std::ostringstream os;
	io::pack(os, idWho);
	io::pack(os, idWhere);
	io::pack(os, type);
	io::pack(os, content);
	PushTrn(sock, NOTIFY(CCPM_SAY), 0, os.str());
}

void CALLBACK JServer::Broadcast_Notify_SAY(const SetId& set, DWORD idWho, DWORD idWhere, UINT type, const std::string& content)
{
	std::ostringstream os;
	io::pack(os, idWho);
	io::pack(os, idWhere);
	io::pack(os, type);
	io::pack(os, content);
	BroadcastTrn(set, false, NOTIFY(CCPM_SAY), os.str());
}

void CALLBACK JServer::Broadcast_Notify_TOPIC(const SetId& set, DWORD idWho, DWORD idWhere, const std::tstring& topic)
{
	std::ostringstream os;
	io::pack(os, idWho);
	io::pack(os, idWhere);
	io::pack(os, topic);
	BroadcastTrn(set, false, NOTIFY(CCPM_TOPIC), os.str());
}

void CALLBACK JServer::Send_Notify_CHANOPTIONS(SOCKET sock, DWORD idWho, DWORD idWhere, int op, DWORD val)
{
	std::ostringstream os;
	io::pack(os, idWho);
	io::pack(os, idWhere);
	io::pack(os, op);
	io::pack(os, val);
	PushTrn(sock, NOTIFY(CCPM_CHANOPTIONS), 0, os.str());
}

void CALLBACK JServer::Broadcast_Notify_CHANOPTIONS(const SetId& set, DWORD idWho, DWORD idWhere, int op, DWORD val)
{
	std::ostringstream os;
	io::pack(os, idWho);
	io::pack(os, idWhere);
	io::pack(os, op);
	io::pack(os, val);
	BroadcastTrn(set, false, NOTIFY(CCPM_CHANOPTIONS), os.str());
}

void CALLBACK JServer::Broadcast_Notify_ACCESS(const SetId& set, DWORD idWho, DWORD idWhere, EChanStatus stat, DWORD idBy)
{
	std::ostringstream os;
	io::pack(os, idWho);
	io::pack(os, idWhere);
	io::pack(os, stat);
	io::pack(os, idBy);
	BroadcastTrn(set, false, NOTIFY(CCPM_ACCESS), os.str());
}

void CALLBACK JServer::Send_Notify_BEEP(SOCKET sock, DWORD idBy)
{
	std::ostringstream os;
	io::pack(os, idBy);
	PushTrn(sock, NOTIFY(CCPM_BEEP), 0, os.str());
}

void CALLBACK JServer::Send_Notify_CLIPBOARD(SOCKET sock, DWORD idBy, const char* ptr, size_t size)
{
	std::ostringstream os;
	io::pack(os, idBy);
	os.write(ptr, (std::streamsize)size);
	PushTrn(sock, NOTIFY(CCPM_CLIPBOARD), 0, os.str());
}

void CALLBACK JServer::Send_Notify_MESSAGE(SOCKET sock, DWORD idBy, const FILETIME& ft, const char* ptr, size_t size)
{
	std::ostringstream os;
	io::pack(os, idBy);
	io::pack(os, ft);
	os.write(ptr, (std::streamsize)size);
	PushTrn(sock, NOTIFY(CCPM_MESSAGE), 0, os.str());
}

void CALLBACK JServer::Send_Reply_MESSAGE(SOCKET sock, WORD trnid, DWORD idWho, UINT type)
{
	std::ostringstream os;
	io::pack(os, idWho);
	io::pack(os, type);
	PushTrn(sock, REPLY(CCPM_MESSAGE), trnid, os.str());
}

void CALLBACK JServer::Send_Notify_SPLASHRTF(SOCKET sock, DWORD idBy, const char* ptr, size_t size)
{
	std::ostringstream os;
	io::pack(os, idBy);
	os.write(ptr, (std::streamsize)size);
	PushTrn(sock, NOTIFY(CCPM_SPLASHRTF), 0, os.str());
}

void CALLBACK JServer::Connections()
{
	if (m_hwndPage)
	{
		if (jpConnections->hwndPage)
			SetForegroundWindow(jpConnections->hwndPage);
		else
			CreateDialogParam(JServerApp::jpApp->hinstApp, MAKEINTRESOURCE(IDD_CONNECTIONS), m_hwndPage, JConnections::DlgProcStub, (LPARAM)(JDialog*)jpConnections);
	}
}

void CALLBACK JServer::About()
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

CALLBACK JServerApp::JServerApp(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR szcl, int ncs)
: ::JApplication(hInstance, hPrevInstance, szcl, ncs),
jpServer(0)
{
	sAppName = APPNAME;
}

void CALLBACK JServerApp::Init()
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

	Profile::SetKey(TEXT("BEOWOLF"), APPNAME);

	jpServer = new JServer;
	jpServer->Init();
}

bool CALLBACK JServerApp::InitInstance()
{
	return true;
}

void CALLBACK JServerApp::Done()
{
	if (jpServer->State != JService::eStopped) jpServer->Stop();
	jpServer->Done();
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
		{
			TCHAR mpath[_MAX_PATH];
			TCHAR drive[_MAX_DRIVE];
			TCHAR dir[_MAX_DIR];
			GetModuleFileName(NULL, mpath, _countof(mpath));
			_tsplitpath_s(mpath, drive, _countof(drive), dir, _countof(dir), NULL, 0, NULL, 0);
			SetCurrentDirectory((std::tstring(drive)+dir).c_str());
		}

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