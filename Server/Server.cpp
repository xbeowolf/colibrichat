
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

//-----------------------------------------------------------------------------

using namespace colibrichat;

// Global Variables:
static TCHAR szHelpFile[MAX_PATH];

//-----------------------------------------------------------------------------

//
// class JServer
//

CALLBACK JServer::JServer()
: netengine::JEngine(), JWindow()
{
	// Dialogs
	jpConnections = new JConnections();

	m_IP = INADDR_ANY;
	m_port = CCP_PORT;
	m_password = TEXT("beowolf");

	m_bShowIcon = true;

	m_metrics.uNickMaxLength = 20;
	m_metrics.uChanMaxLength = 20;
	m_metrics.uPassMaxLength = 32;
}

void CALLBACK JServer::Init()
{
	__super::Init();

	m_mSocketId.clear();
	m_mIdSocket.clear();
	m_mUser.clear();
	m_mChannel.clear();

	CreateMsgWindow();

	// Create listening socket
	netengine::Link link;
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
	JWindow* wnd = this;
	if (!IsWindow(m_hwndPage)) m_hwndPage = CreateWindow(WC_MSG, WT_MSG,
		WS_OVERLAPPEDWINDOW,
		0, 0, 100, 100,
		HWND_MESSAGE,
		0, 0, (JWindow*)wnd);
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
					GetCursorPos(&p);
					SetMenuDefaultItem(GetSubMenu(hmenu, 0), IDC_SHELL_CONNECTIONS, FALSE);
					TrackPopupMenu(GetSubMenu(hmenu, 0), TPM_RIGHTALIGN | TPM_BOTTOMALIGN | TPM_RIGHTBUTTON,
						p.x, p.y, 0, hWnd, 0);
					DestroyMenu(hmenu);
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

bool CALLBACK JServer::CheckAccess(const TCHAR* password, netengine::SetAccess& access) const
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
		m_mUser.find(crc) != m_mUser.end() ||
		m_mChannel.find(crc) != m_mChannel.end();
}

bool CALLBACK JServer::linkCRC(DWORD crc1, DWORD crc2)
{
	Contact *cont1, *cont2;
	MapUser::iterator iter1;
	MapChannel::iterator iter2;
	iter1 = m_mUser.find(crc1);
	if (iter1 == m_mUser.end()) {
		iter2 = m_mChannel.find(crc1);
		if (iter2 == m_mChannel.end()) return false;
		else cont1 = &iter2->second;
	} else cont1 = &iter1->second;
	iter1 = m_mUser.find(crc2);
	if (iter1 == m_mUser.end()) {
		iter2 = m_mChannel.find(crc2);
		if (iter2 == m_mChannel.end()) return false;
		else cont2 = &iter2->second;
	} else cont2 = &iter1->second;
	cont1->opened.insert(crc2);
	cont2->opened.insert(crc1);
	return true;
}

void CALLBACK JServer::unlinkCRC(DWORD crc1, DWORD crc2)
{
	MapUser::iterator iter1;
	MapChannel::iterator iter2;
	iter1 = m_mUser.find(crc1);
	if (iter1 != m_mUser.end()) iter1->second.opened.erase(crc2);
	else {
		iter2 = m_mChannel.find(crc1);
		if (iter2 != m_mChannel.end()) {
			iter2->second.opened.erase(crc2);
			if (iter2->second.opened.empty())
				m_mChannel.erase(crc1);
		}
	}
	iter1 = m_mUser.find(crc2);
	if (iter1 != m_mUser.end()) iter1->second.opened.erase(crc1);
	else {
		iter2 = m_mChannel.find(crc2);
		if (iter2 != m_mChannel.end()) {
			iter2->second.opened.erase(crc1);
			if (iter2->second.opened.empty())
				m_mChannel.erase(crc2);
		}
	}
}

std::tstring CALLBACK JServer::getNearestName(const std::tstring& nick) const
{
	std::tstring buffer = nick;
	TCHAR digits[16];
	DWORD i = 0;
	while (hasCRC(dCRC(buffer.c_str()))) {
		StringCchPrintf(digits, _countof(digits), TEXT("%u"), i);
		buffer = nick.substr(0, m_metrics.uNickMaxLength - lstrlen(digits)) + digits;
		i++;
	}
	return buffer;
}

void CALLBACK JServer::RenameContact(DWORD idOld, DWORD idNew, const std::tstring& newname)
{
	ASSERT(idNew != idOld);
	SetId opened;
	MapUser::iterator iter1 = m_mUser.find(idOld);
	if (iter1 != m_mUser.end()) { // private talk
		opened = iter1->second.opened;
		iter1->second.name = newname;
		m_mUser[idNew] = iter1->second;
		m_mSocketId[m_mIdSocket[idOld]] = idNew;
		m_mIdSocket[idNew] = m_mIdSocket[idOld];
		for each (SetId::value_type const& v in opened) {
			linkCRC(idNew, v);
			unlinkCRC(idOld, v);
		}
		m_mUser.erase(idOld);
		m_mIdSocket.erase(idOld);
		// replace content of channels access rights
		for (MapChannel::iterator ic = m_mChannel.begin(); ic != m_mChannel.end(); ic++) {
			if (idOld == ic->second.idFounder)
				ic->second.idFounder = idNew;
			else if (ic->second.admin.find(idOld) != ic->second.admin.end()) {
				ic->second.admin.insert(idNew);
				ic->second.admin.erase(idOld);
			} else if (ic->second.moderator.find(idOld) != ic->second.moderator.end()) {
				ic->second.moderator.insert(idNew);
				ic->second.moderator.erase(idOld);
			} else if (ic->second.member.find(idOld) != ic->second.member.end()) {
				ic->second.member.insert(idNew);
				ic->second.member.erase(idOld);
			} else if (ic->second.writer.find(idOld) != ic->second.writer.end()) {
				ic->second.writer.insert(idNew);
				ic->second.writer.erase(idOld);
			}
		}
	} else {
		MapChannel::iterator iter2 = m_mChannel.find(idOld);
		if (iter2 != m_mChannel.end()) { // channel
			opened = iter2->second.opened;
			iter2->second.name = newname;
			m_mChannel[idNew] = iter2->second;
			for each (SetId::value_type const& v in opened) {
				linkCRC(idNew, v);
				unlinkCRC(idOld, v);
			}
			m_mChannel.erase(idOld);
		} else return;
	}
	Broadcast_Notify_RENAME(opened, idOld, idNew, newname);
}

void JServer::OnHook(JEventable* src)
{
	using namespace fastdelegate;

	__super::OnHook(src);

	// Setup hooks and pointer to this for dialogs objects at last
	jpConnections->SetSource(this);
	jpConnections->SetupHooks();
}

void JServer::OnUnhook(JEventable* src)
{
	using namespace fastdelegate;

	// Reset hooks and pointer to this for dialogs objects at last
	jpConnections->ResetHooks();
	jpConnections->SetSource(0);

	__super::OnUnhook(src);
}

void JServer::OnLinkDestroy(SOCKET sock)
{
	__super::OnLinkDestroy(sock);

	DWORD idSrc = m_mSocketId[sock];
	MapUser::iterator iter = m_mUser.find(idSrc);
	if (iter != m_mUser.end()) {
		SetId opened = iter->second.opened;
		for each (SetId::value_type const& v in opened) {
			unlinkCRC(idSrc, v);
			MapUser::const_iterator iu = m_mUser.find(v);
			if (iu != m_mUser.end()) { // private talk
				Send_Notify_PART(m_mIdSocket[v], idSrc, v, PART_DISCONNECT);
			} else {
				MapChannel::const_iterator ic = m_mChannel.find(v);
				if (ic != m_mChannel.end()) { // channel
					Broadcast_Notify_PART(ic->second.opened, idSrc, v, PART_DISCONNECT);
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

void JServer::OnTransactionProcess(SOCKET sock, WORD message, WORD trnid, io::mem is)
{
	typedef void (CALLBACK JServer::*TrnParser)(SOCKET, WORD, io::mem&);
	struct {
		WORD message;
		TrnParser parser;
	} responseTable[] =
	{
		{QUEST(CCPM_NICK), &JServer::Recv_Quest_NICK},
		{QUEST(CCPM_LIST), &JServer::Recv_Quest_LIST},
		{QUEST(CCPM_JOIN), &JServer::Recv_Quest_JOIN},
		{COMMAND(CCPM_PART), &JServer::Recv_Cmd_PART},
		{QUEST(CCPM_USERINFO), &JServer::Recv_Quest_USERINFO},
		{COMMAND(CCPM_ONLINE), &JServer::Recv_Cmd_ONLINE},
		{COMMAND(CCPM_STATUS), &JServer::Recv_Cmd_STATUS},
		{COMMAND(CCPM_SAY), &JServer::Recv_Cmd_SAY},
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

int  CALLBACK JServer::BroadcastTrn(const SetId& set, bool nested, WORD message, const std::string& str, size_t ssi) throw()
{
	ASSERT(NATIVEACTION(message) != BNPM_REPLY); // can not broadcast reply

	// Count users to prevent duplicate sents
	netengine::SetSocket broadcast;
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
	for each (netengine::SetSocket::value_type const& v in broadcast) {
		PushTrn(v, message, 0, str, ssi);
	}
	return (int)broadcast.size();
}

//
// Beowolf Network Protocol Messages reciving
//

void CALLBACK JServer::Recv_Quest_NICK(SOCKET sock, WORD trnid, io::mem& is)
{
	std::tstring name;

	try
	{
		io::unpack(is, name);
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

	DWORD idOld, idNew;
	idOld = m_mSocketId[sock];
	idNew = dCRC(name.c_str());
	DWORD result = NICK_TAKEN;
	if (hasCRC(idNew)) {
		if (idNew == CRC_SERVER || idNew == CRC_LIST) {
			result = NICK_TAKEN;
		} else if (m_mUser.find(idNew) != m_mUser.end()) {
			result = NICK_TAKENUSER;
		} else if (m_mChannel.find(idNew) != m_mChannel.end()) {
			result = NICK_TAKENCHANNEL;
		}
		name = getNearestName(name);
		idNew = dCRC(name.c_str());
	} else {
		result = NICK_OK;
	}

	if (m_mUser.find(idOld) == m_mUser.end()) {
		sockaddr_in si;
		int len = sizeof(si);
		getpeername(sock, (struct sockaddr*)&si, &len);

		// Create new user
		User user;
		user.Init();
		user.name = name;
		user.IP = si.sin_addr;

		m_mUser[idNew] = user;
		m_mSocketId[sock] = idNew;
		m_mIdSocket[idNew] = sock;
	} else {
		RenameContact(idOld, idNew, name);
	}

	Send_Reply_NICK(sock, trnid, result, idNew, name);

	// Report about message
	EvReport(tformat(TEXT("nickname added: %s"), name.c_str()), netengine::eInformation, netengine::eNormal);
}

void CALLBACK JServer::Recv_Quest_LIST(SOCKET sock, WORD trnid, io::mem& is)
{
	Send_Reply_LIST(sock, trnid);

	// Report about message
	EvReport(tformat(TEXT("channels list")), netengine::eInformation, netengine::eNormal);
}

void CALLBACK JServer::Recv_Quest_JOIN(SOCKET sock, WORD trnid, io::mem& is)
{
	std::tstring name, pass;

	try
	{
		io::unpack(is, name);
		io::unpack(is, pass);
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
			pass = TEXT("");
		}
	}

	DWORD idDst = dCRC(name.c_str());
	DWORD idSrc = m_mSocketId[sock];
	MapUser::const_iterator iu = m_mUser.find(idDst);
	if (iu != m_mUser.end()) { // private talk
		if (iu->second.opened.find(idDst) == iu->second.opened.end()) {
			Send_Reply_JOIN_User(sock, trnid, idDst, iu->second);
			Send_Notify_JOIN(m_mIdSocket[idDst], idSrc, idDst, m_mUser[idSrc]);
			linkCRC(idDst, idSrc);
		} else {
			Send_Reply_JOIN_Result(sock, trnid, CHAN_ALREADY, eUser, idDst);
		}
	} else {
		MapChannel::iterator ic = m_mChannel.find(idDst);
		if (ic != m_mChannel.end()) { // channel
			if (ic->second.opened.size() < ic->second.nLimit) {
				if (ic->second.password.empty() || ic->second.password == pass) {
					if (ic->second.opened.find(idSrc) == ic->second.opened.end()) {
						if (ic->second.getStatus(idSrc) == eOutsider)
							ic->second.setStatus(idSrc, ic->second.nAutoStatus);
						Broadcast_Notify_JOIN(ic->second.opened, idSrc, idDst, m_mUser[idSrc]);
						linkCRC(idDst, idSrc);
						Send_Reply_JOIN_Channel(sock, trnid, idDst, ic->second);
					} else {
						Send_Reply_JOIN_Result(sock, trnid, CHAN_ALREADY, eChannel, idDst);
					}
				} else {
					Send_Reply_JOIN_Result(sock, trnid, CHAN_DENY, eChannel, idDst);
				}
			} else {
				Send_Reply_JOIN_Result(sock, trnid, CHAN_LIMIT, eChannel, idDst);
			}
		} else { // create new channel if nothing was found
			FILETIME ft;
			GetSystemFileTime(ft);

			Channel chan;
			chan.name = name;
			chan.ftCreation = ft;
			chan.password = pass;
			chan.topic = TEXT("");
			chan.idFounder = idSrc;
			chan.nAutoStatus = eWriter;
			chan.nLimit = -1;
			chan.isHidden = false;
			chan.isAnonymous = false;
			m_mChannel[idDst] = chan;

			linkCRC(idDst, idSrc);
			Send_Reply_JOIN_Channel(sock, trnid, idDst, m_mChannel[idDst]);
		}
	}

	// Report about message
	EvReport(tformat(TEXT("joins %s to %s"), m_mUser[idSrc].name.c_str(), name.c_str()), netengine::eInformation, netengine::eNormal);
}

void CALLBACK JServer::Recv_Cmd_PART(SOCKET sock, WORD trnid, io::mem& is)
{
	DWORD idDst;
	DWORD reason;

	try
	{
		io::unpack(is, idDst);
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

	DWORD idSrc = m_mSocketId[sock];
	MapUser::iterator iu = m_mUser.find(idDst);
	if (iu != m_mUser.end()) { // private talk
		// Report about message
		EvReport(tformat(TEXT("parts %s from %s"), m_mUser[idSrc].name.c_str(), iu->second.name.c_str()), netengine::eInformation, netengine::eNormal);

		unlinkCRC(idSrc, idDst);
		Send_Notify_PART(m_mIdSocket[idDst], idSrc, idDst, reason);
	} else {
		MapChannel::const_iterator ic = m_mChannel.find(idDst);
		if (ic != m_mChannel.end()) { // channel
			// Report about message
			EvReport(tformat(TEXT("parts %s from %s"), m_mUser[idSrc].name.c_str(), ic->second.name.c_str()), netengine::eInformation, netengine::eNormal);

			unlinkCRC(idSrc, idDst);
			ic = m_mChannel.find(idDst);
			if (ic != m_mChannel.end()) { // check that channel still exist
				Broadcast_Notify_PART(ic->second.opened, idSrc, idDst, reason);
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
			EvReport(SZ_BADTRN, netengine::eWarning, netengine::eLow);
			return;
		}
	}

	Send_Reply_USERINFO(sock, trnid, wanted);

	// Report about message
	EvReport(tformat(TEXT("info for %u users"), wanted.size()), netengine::eInformation, netengine::eNormal);
}

void CALLBACK JServer::Recv_Cmd_ONLINE(SOCKET sock, WORD trnid, io::mem& is)
{
	bool isOnline;
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
			isOnline = true;
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
	EvReport(tformat(TEXT("user is %s"), isOnline ? TEXT("online") : TEXT("offline")), netengine::eInformation, netengine::eLowest);
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
			EvReport(SZ_BADTRN, netengine::eWarning, netengine::eLow);
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
	EvReport(tformat(TEXT("user changes status")), netengine::eInformation, netengine::eLowest);
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
			EvReport(SZ_BADTRN, netengine::eWarning, netengine::eLow);
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
			Broadcast_Notify_SAY(ic->second.opened, idSrc, idWhere, type, content);
		}
	}
}

//
// Beowolf Network Protocol Messages sending
//

void CALLBACK JServer::Send_Reply_NICK(SOCKET sock, WORD trnid, DWORD result, DWORD id, const std::tstring& nick)
{
	std::ostringstream os;
	io::pack(os, result);
	io::pack(os, id);
	io::pack(os, nick);
	PushTrn(sock, REPLY(CCPM_NICK), trnid, os.str());
}

void CALLBACK JServer::Broadcast_Notify_RENAME(const SetId& set, DWORD idOld, DWORD idNew, const std::tstring& newname)
{
	std::ostringstream os;
	io::pack(os, idOld);
	io::pack(os, idNew);
	io::pack(os, newname);
	BroadcastTrn(set, true, NOTIFY(CCPM_RENAME), os.str());
}

void CALLBACK JServer::Send_Reply_LIST(SOCKET sock, WORD trnid)
{
	std::ostringstream os;
	io::pack(os, m_mChannel.size());
	for each (MapChannel::value_type const& v in m_mChannel)
	{
		io::pack(os, v.first);
		io::pack(os, v.second);
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

void CALLBACK JServer::Send_Notify_PART(SOCKET sock, DWORD idWho, DWORD idWhere, DWORD reason)
{
	std::ostringstream os;
	io::pack(os, idWho);
	io::pack(os, idWhere);
	io::pack(os, reason);
	PushTrn(sock, NOTIFY(CCPM_PART), 0, os.str());
}

void CALLBACK JServer::Broadcast_Notify_PART(const SetId& set, DWORD idWho, DWORD idWhere, DWORD reason)
{
	std::ostringstream os;
	io::pack(os, idWho);
	io::pack(os, idWhere);
	io::pack(os, reason);
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

void CALLBACK JServer::Broadcast_Notify_ONLINE(const SetId& set, DWORD idWho, bool on, DWORD id)
{
	std::ostringstream os;
	io::pack(os, idWho);
	io::pack(os, on);
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