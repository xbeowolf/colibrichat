// netengine.cpp
//

//-----------------------------------------------------------------------------

//
// Includes
//

#pragma region Includes

#include "stdafx.h"
#define _WINSOCK_DEPRECATED_NO_WARNINGS

// Common
#include "netengine.h"

#pragma endregion

//-----------------------------------------------------------------------------

using namespace netengine;

//-----------------------------------------------------------------------------

// Maximum events handles by each thread, set it not more then MAXIMUM_WAIT_OBJECTS
#define HANDLEBYTHREAD MAXIMUM_WAIT_OBJECTS

//-----------------------------------------------------------------------------

static void CALLBACK GetSystemFileTime(FILETIME& ft)
{
	SYSTEMTIME st;
	GetSystemTime(&st);
	SystemTimeToFileTime(&st, &ft);
}

//
// JLink
//

WORD JLink::s_wQuestCount = 0;

bool CALLBACK JLink::MakeProxy(JPtr<JLink> link1, JPtr<JLink> link2)
{
	if (link1->m_ID && link2->m_ID && link1->m_ID != link2->m_ID)
	{
		link1->m_proxyID = link2->m_ID;
		link2->m_proxyID = link1->m_ID;
		return true;
	}
	return false;
}

void CALLBACK JLink::BreakProxy(JPtr<JLink> link1, JPtr<JLink> link2)
{
	link1->m_proxyID = 0;
	link2->m_proxyID = 0;
}

WORD JLink::getNextQuestId()
{
	s_wQuestCount++;
	if (!s_wQuestCount) s_wQuestCount++;
	_ASSERT(s_wQuestCount != 0);
	return s_wQuestCount;
}

JLink::JLink()
: JIDClass(WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, 0, 0, WSA_FLAG_OVERLAPPED))
{
	_ASSERT(m_ID && m_ID != INVALID_SOCKET);
	Clear();
}

JLink::JLink(SOCKET sock, const sockaddr_in& addr)
: JIDClass(sock)
{
	_ASSERT(m_ID && m_ID != INVALID_SOCKET);
	m_saAddr = addr;
	Clear();
}

void JLink::beforeDestruct()
{
	Close();
	__super::beforeDestruct();
}

void JLink::Listen()
{
	if (m_State != eEmpty) return; // only one connection can be established

	if (!bind(m_ID, (const struct sockaddr*)&m_saAddr, sizeof(m_saAddr)) && !listen(m_ID, SOMAXCONN)) {
		m_State = eListening;
		GetSystemFileTime(m_ftTime);
	} else {
		Close();
	}
}

void JLink::Connect()
{
	if (m_State != eEmpty) return; // only one connection can be established

	if (!connect(m_ID, (const struct sockaddr*)&m_saAddr, sizeof(m_saAddr))) {
		m_State = eConnected;
		GetSystemFileTime(m_ftTime);
	} else if (WSAGetLastError() == WSAEWOULDBLOCK) {
		m_State = eConnecting;
		GetSystemFileTime(m_ftTime);
	} else {
		Close();
	}
}

void JLink::Connected()
{
	if (m_State == eConnecting) {
		m_State = eConnected;
		GetSystemFileTime(m_ftTime);
	}
}

void JLink::Accepted()
{
	if (m_State != eEmpty) return; // only one connection can be established

	m_State = eAccepted;
	GetSystemFileTime(m_ftTime);
}

int JLink::SelectEvent(long ne, HANDLE hevent)
{
	_ASSERT(m_ID); // only established connections can be selected

	m_Mode = eEvent;
	m_lNetworkEvents = ne;
	m_wMsg = 0;
	m_bAutocloseHandle = hevent == 0;
	if (hevent) m_Event = hevent;
	else m_Event = WSACreateEvent(); // create only if not already exist

	return WSAEventSelect(m_ID, m_Event, m_lNetworkEvents);
}

int JLink::SelectWindow(long ne, HWND hwnd, WORD msg)
{
	_ASSERT(m_ID); // only established connections can be selected
	_ASSERT(hwnd != 0 && msg != 0);

	m_Mode = eAsync;
	m_lNetworkEvents = ne;
	m_wMsg = msg;
	m_Event = hwnd;

	return WSAAsyncSelect(m_ID, (HWND)m_Event, m_wMsg, m_lNetworkEvents);
}

bool JLink::SelectIocp(HANDLE iocp)
{
	m_Mode = eIocp;
	m_Event = iocp;
	m_bReadyWrite = true;
	return CreateIoCompletionPort((HANDLE)m_ID, iocp, (ULONG_PTR)m_ID, 0) == iocp;
}

void JLink::Close()
{
	if (m_State != eClosed) {
		shutdown(m_ID, SD_BOTH);
		closesocket(m_ID);
		// do not zero socket to give opportunity to check closed handle
	}
	if (m_bAutocloseHandle && m_Event) {
		if (m_Mode == eEvent) { // reusing can be without reselection on acync sockets
			WSACloseEvent(m_Event);
		}
		m_Event = 0;
	}

	m_bReadyWrite = false;
	m_bAccessAllowed = false;

	m_Mode = eNosock;
	m_State = eClosed;
	GetSystemFileTime(m_ftTime);
}

void JLink::Clear()
{
	m_Mode = eBlock;
	m_Event = 0;
	m_bAutocloseHandle = true;
	m_wMsg = 0;
	s_wQuestCount = 0;
	m_lNetworkEvents = 0;

	m_bReadyWrite = false;
	m_bAccessAllowed = false;
	m_dwLastTransmission = GetTickCount();
	m_State = eEmpty;
	GetSystemFileTime(m_ftTime);

	m_nFreezeCount = 0;
	m_aBufferRecv.clear();
	m_aBufferSend.clear();
	m_aStorage.resize(1);
	m_aStorage[0].clear();
	m_nStorageIndex = 0;
	m_bHalfTrnRecv = 0;

	ZeroMemory(&ioRecv, sizeof(ioRecv));
	ioRecv.op = eRecv;
	ZeroMemory(&ioSend, sizeof(ioSend));
	ioSend.op = eSend;

	int sz;
	m_nSreamable = 4096;
	sz = sizeof(m_nSreamable);
	_VERIFY(getsockopt(m_ID, SOL_SOCKET, SO_SNDBUF, (char*)&m_nSreamable, &sz) != SOCKET_ERROR);

	int rcvlen = 4096;
	sz = sizeof(rcvlen);
	_VERIFY(getsockopt(m_ID, SOL_SOCKET, SO_RCVBUF, (char*)&rcvlen, &sz) != SOCKET_ERROR);
	recvbuf.resize(max(4096, rcvlen));
}

int JLink::Freeze()
{
	m_nFreezeCount++;
	return m_nFreezeCount;
}

int JLink::Unfreeze()
{
	m_nFreezeCount--;
	SendData();
	return m_nFreezeCount;
}

void JLink::AddRecv(const char* data, size_t len) throw()
{
	m_aBufferRecv.append(data, len);
}

void JLink::AddSend(JPtr<JTransaction> jpTrn, size_t ssi) throw()
{
	_ASSERT(m_aStorage.size() > 0);
	if (ssi >= m_aStorage.size()) ssi = m_aStorage.size() - 1;
	m_aStorage[ssi].push_back(jpTrn);
}

void JLink::SubRecv(size_t len) throw()
{
	m_aBufferRecv.erase(0, len);
}

void JLink::SubSend(size_t len) throw()
{
	m_aBufferSend.erase(0, len);
}

size_t JLink::RecvData() throw()
{
	ioRecv.buf.buf = (char*)recvbuf.data();
	ioRecv.buf.len = recvbuf.size();

	DWORD dwNumberOfBytes = 0;
	DWORD dwFlags = 0;
	int err = WSARecv(m_ID, &ioRecv.buf, 1, &dwNumberOfBytes, &dwFlags, (LPWSAOVERLAPPED)&ioRecv, 0);
	err = WSAGetLastError();
	m_dwLastTransmission = GetTickCount();

	if (err == ERROR_SUCCESS || err == WSA_IO_PENDING) {
		if (m_Mode != eIocp) {
			AddRecv(ioRecv.buf.buf, dwNumberOfBytes);
			JEngine::Stat.dlRecvBytes += dwNumberOfBytes; // update statistics
		}
	} else {
	}
	return dwNumberOfBytes;
}

size_t JLink::SendData() throw()
{
	if (!m_bReadyWrite || m_nFreezeCount > 0) return 0;

	size_t nWantedPos;
	for (nWantedPos = 0; nWantedPos < m_aStorage.size() && m_aStorage[nWantedPos].empty(); nWantedPos++) {}

	if (m_aBufferSend.empty()) {
		if (nWantedPos >= m_aStorage.size()) return 0; // no data to send
		m_nStorageIndex = nWantedPos; // switch to other stream
	}

	if (m_nStorageIndex == nWantedPos) {
		std::list<JPtr<JTransaction>>& queue = m_aStorage[m_nStorageIndex];
		std::list<JPtr<JTransaction>>::iterator iter = queue.begin();
		if (m_nSreamable) {
			while (m_aBufferSend.size() < m_nSreamable && iter != queue.end()) {
				(*iter)->serialize(this);
				iter = queue.erase(iter);
			}
		} else {
			if (m_aBufferSend.empty()) {
				(*iter)->serialize(this);
				iter = queue.erase(iter);
			}
		}
	}

	ioSend.buf.buf = (char*)m_aBufferSend.data();
	ioSend.buf.len = m_aBufferSend.size();

	DWORD dwNumberOfBytes = 0;
	int err = WSASend(m_ID, &ioSend.buf, 1, &dwNumberOfBytes, 0, (LPWSAOVERLAPPED)&ioSend, 0);
	m_bReadyWrite = err == 0;
	err = WSAGetLastError();
	m_dwLastTransmission = GetTickCount();

	if (err == ERROR_SUCCESS || err == WSA_IO_PENDING) {
		_ASSERT(err != ERROR_SUCCESS || dwNumberOfBytes == ioSend.buf.len);
		SubSend(ioSend.buf.len);
		JEngine::Stat.dlSentBytes += ioSend.buf.len; // update statistics
	} else {
	}
	return dwNumberOfBytes;
}

bool JLink::isEstablished() const
{
	return m_State == eAccepted || m_State == eConnected;
}

int  JLink::getError() const
{
	int err, len = sizeof(int);
	if (!getsockopt(m_ID, SOL_SOCKET, SO_ERROR, (char*)&err, &len)) return err;
	else return SOCKET_ERROR;
}

size_t JLink::getSendStreamCount() const
{
	return m_aStorage.size();
}

void JLink::setSendStreamCount(size_t val)
{
	m_aStorage.resize(max(1, val));
}

//
// JEngine::JEventSock
//

JEngine::JEventSock::JEventSock(JEngine* p)
: JNode(p, false), JThread()
{
	m_bSafeStop = true;
	m_dwSleepStep = 250;
}

DWORD JEngine::JEventSock::ThreadProc()
{
	int retval;
	try
	{
		SOCKET aSock[HANDLEBYTHREAD];
		WSAEVENT aEvent[HANDLEBYTHREAD];
		DWORD count;
		WSANETWORKEVENTS wneGet;

		_ASSERT(!m_hSleep);
		m_hSleep = CreateEvent(0, TRUE, FALSE, 0);
		while (m_aLinks.size() && m_State == eRunning || m_State == eSuspended) {
			// --- Prepare socks & events arrays ---
			count = 0;
			aEvent[count++] = m_hSleep;
			{
				DoCS cs(&pNode->m_csLinks);
				for each (SetJID::value_type const& v in m_aLinks) {
					if (count >= _countof(aSock)) break;
					JPtr<JLink> link = JLink::get(v);
					_ASSERT(link);
					aSock[count] = v;
					aEvent[count] = link->Event;
					count++;
				}
			}

			// --- Wait and process socket's event ---
			_ASSERT(count > 1);

			// Get some socket's event
			DWORD dwEvent = WSAWaitForMultipleEvents(count, aEvent, FALSE, m_dwSleepStep, FALSE);

			// Process given event
			if (dwEvent == WSA_WAIT_EVENT_0) {
				ResetEvent(m_hSleep); // reset sleep handle for next iteration
			} else if (dwEvent >= WSA_WAIT_EVENT_0 && dwEvent < WSA_WAIT_EVENT_0 + count) {
				DWORD i = dwEvent - WSA_WAIT_EVENT_0;
				WSAEnumNetworkEvents(aSock[i], aEvent[i], &wneGet);
				for (long j = 0, ev = 1; j < FD_MAX_EVENTS; (ev <<= 1), ++j) {
					if (wneGet.lNetworkEvents & ev)
						pNode->EventSelector(aSock[i], ev, wneGet.iErrorCode[j]);
				}
			} else if (dwEvent == WAIT_IO_COMPLETION) {
			} else if (dwEvent == WSA_WAIT_TIMEOUT) {
			} else if (dwEvent == WSA_WAIT_FAILED) {
				int err = WSAGetLastError();
				Sleep(0); // give opportunity for other threads
			}
		} // Return to new iteration
		_VERIFY(CloseHandle(m_hSleep));
		m_hSleep = 0;

		retval = 0;
	}
	catch (std::exception& e)
	{
		std::string err = format("Unhandled Exception!\r\n%s\r\n%s\r\n", typeid(e).name(), e.what());
		WriteConsoleA(s_hErrlog, err.c_str(), err.size(), 0, 0);

		retval = -1;
	}
	return retval;
}

//
// JEngine::JManager
//

JEngine::JManager::JManager(JEngine* p)
: JNode(p, false), JThread()
{
	m_bSafeStop = true;
	m_dwSleepStep = 250;
}

DWORD JEngine::JManager::ThreadProc()
{
	int retval;
	try
	{
		_ASSERT(!m_hSleep);
		m_hSleep = CreateEvent(0, TRUE, FALSE, 0);
		while (m_State == eRunning || m_State == eSuspended) {
			DWORD dwEvent = WaitForSingleObject(m_hSleep, m_dwSleepStep);

			// Process given event
			if (dwEvent == WAIT_OBJECT_0) {
				ResetEvent(m_hSleep); // reset sleep handle for next iteration
			} else if (dwEvent == WSA_WAIT_TIMEOUT) {
			} else if (dwEvent == WSA_WAIT_FAILED) {
				DWORD err = GetLastError();
				Sleep(0); // give opportunity for other threads
			}

			// Copy links that must be closed
			SetJID close;
			{
				DoCS cs(&pNode->m_csClose);
				close = m_aClose;
				m_aClose.clear();
			}
			// Check validate timeout
			{
				DWORD now = GetTickCount();
				DoCS cs(&pNode->m_csValidate);
				MapValidate::iterator iter = pNode->mValidate.begin();
				while (iter != pNode->mValidate.end()) {
					if (now >= iter->second) {
						close.insert(iter->first);
						iter = pNode->mValidate.erase(iter);
					} else iter++;
				}
			}
			// Close links
			if (close.size()) {
				for each (SetJID::value_type const& v in close) {
					pNode->DeleteLink(v);
				}
			}
		} // Return to new iteration
		_VERIFY(CloseHandle(m_hSleep));
		m_hSleep = 0;

		retval = 0;
	}
	catch (std::exception& e)
	{
		std::string err = format("Unhandled Exception!\r\n%s\r\n%s\r\n", typeid(e).name(), e.what());
		WriteConsoleA(s_hErrlog, err.c_str(), err.size(), 0, 0);

		retval = -1;
	}
	return retval;
}

//
// JEngine::JIocpListener
//

JEngine::JIocpListener::JIocpListener(JEngine* p)
: JNode(p, false), JThread()
{
	m_bSafeStop = true;
	m_dwSleepStep = 250;
}

DWORD JEngine::JIocpListener::ThreadProc()
{
	int retval;
	try
	{
		SOCKET listensocket[MAXIMUM_WAIT_OBJECTS];
		HANDLE listenevent[MAXIMUM_WAIT_OBJECTS];
		int count;

		_ASSERT(!m_hSleep);
		m_hSleep = CreateEvent(0, TRUE, FALSE, 0);

		while (m_State == eRunning || m_State == eSuspended) {
			// --- Prepare socks & events arrays ---
			count = 0;
			listenevent[count++] = m_hSleep;

			for each (MapListen::value_type const& v in pNode->m_mListen) {
				listensocket[count] = v.first;
				listenevent[count] = v.second;
				count++;
			}

			// --- Wait and process socket's event ---
			DWORD result = WSAWaitForMultipleEvents(count, listenevent, FALSE, m_dwSleepStep, FALSE);
			if (result == WSA_WAIT_EVENT_0) {
				ResetEvent(m_hSleep); // reset sleep handle for next iteration
			} else if (result >= WSA_WAIT_EVENT_0 && result < WSA_WAIT_EVENT_0 + count) {
				WSANETWORKEVENTS wneGet;
				int index = result - WSA_WAIT_EVENT_0;
				WSAEnumNetworkEvents(listensocket[index], listenevent[index], &wneGet);
				if (wneGet.lNetworkEvents & FD_ACCEPT) {
					SOCKET clientsocket;
					sockaddr_in clientaddr;
					int clientsize = sizeof(clientaddr);

					clientsocket = WSAAccept(listensocket[index], (SOCKADDR*)&clientaddr, &clientsize, 0, 0);
					if (clientsocket != INVALID_SOCKET) {
						if (!pNode->isBanned(clientaddr.sin_addr)) {
							DoCS cs(&pNode->m_csLinks);
							JPtr<JLink> link = pNode->createLink(clientsocket, clientaddr);
							bool sel = link->SelectIocp(pNode->m_hCompPort);
							link->Accepted();
							pNode->ValidateOn(clientsocket);
							pNode->InsertLink(link);
							pNode->jpManager->Wakeup();
							pNode->EvLinkAccept(clientsocket);
							pNode->EvLinkAccess(clientsocket, false);
							pNode->EvLinkEstablished(clientsocket);
							link->RecvData();
						} else {
							closesocket(clientsocket);
						}
					}
				} else if (wneGet.lNetworkEvents & FD_CLOSE) {
					pNode->m_mListen.erase(listensocket[index]);
				}
			} else if (result == WAIT_IO_COMPLETION) {
			} else if (result == WSA_WAIT_TIMEOUT) {
			} else if (result == WSA_WAIT_FAILED) {
				int err = WSAGetLastError();
				Sleep(0); // give opportunity for other threads
			}
		} // Return to new iteration
		_VERIFY(CloseHandle(m_hSleep));
		m_hSleep = 0;

		retval = 0;
	}
	catch (std::exception& e)
	{
		std::string err = format("Unhandled Exception!\r\n%s\r\n%s\r\n", typeid(e).name(), e.what());
		WriteConsoleA(s_hErrlog, err.c_str(), err.size(), 0, 0);

		retval = -1;
	}
	return retval;
}

//
// JEngine::JIocpSock
//

JEngine::JIocpSock::JIocpSock(JEngine* p)
: JNode(p, false), JThread()
{
	m_bSafeStop = true;
	m_dwSleepStep = 250;
}

DWORD JEngine::JIocpSock::ThreadProc()
{
	int retval;
	try
	{
		DWORD dwNumberOfBytes = 0;
		SOCKET sock = 0;
		BOVERLAPPED* io = 0;
		while (m_State == eRunning || m_State == eSuspended) {
			if (!GetQueuedCompletionStatus(pNode->m_hCompPort, &dwNumberOfBytes, (PULONG_PTR)&sock, (LPOVERLAPPED *)&io, m_dwSleepStep)) {
				int err = GetLastError();
				if (err == WAIT_TIMEOUT) { // wait timeout
					continue;
				} else if (err == ERROR_NETNAME_DELETED || !dwNumberOfBytes) { // socket is closed
					pNode->EvLinkClose(sock, 0);
					continue;
				} else if (err == ERROR_INVALID_HANDLE) { // completion port is closed
					_ASSERT(!dwNumberOfBytes);
					break;
				}
			}
			_ASSERT(io && sock);
			switch (io->op) {
		case eRecv:
			{
				DoCS cs(&pNode->m_csLinks);

				JPtr<JLink> link = JLink::get(sock);
				_ASSERT(link);

				link->AddRecv(io->buf.buf, dwNumberOfBytes);
				JEngine::Stat.dlRecvBytes += dwNumberOfBytes; // update statistics

				pNode->DispatchLoop(sock);
				link->RecvData();
				break;
			}

		case eSend:
			{
				DoCS cs(&pNode->m_csLinks);

				JPtr<JLink> link = JLink::get(sock);
				_ASSERT(link);

				link->bReadyWrite = true;
				link->SendData();
				break;
			}
			}
		}

		retval = 0;
	}
	catch (std::exception& e)
	{
		std::string err = format("Unhandled Exception!\r\n%s\r\n%s\r\n", typeid(e).name(), e.what());
		WriteConsoleA(s_hErrlog, err.c_str(), err.size(), 0, 0);

		retval = -1;
	}
	return retval;
}

//
// JEngine
//

WSADATA JEngine::wsaData;
JEngine::Statistics JEngine::Stat = {0};
HANDLE JEngine::s_hErrlog = GetStdHandle(STD_OUTPUT_HANDLE);

bool JEngine::IPFilter::operator==(const IPFilter& r) const
{
	return m_addr.S_un.S_addr == r.m_addr.S_un.S_addr && m_mask.S_un.S_addr == r.m_mask.S_un.S_addr;
}

bool JEngine::IPFilter::operator<(const IPFilter& r) const
{
	return (m_addr.S_un.S_addr & m_mask.S_un.S_addr) < (r.m_addr.S_un.S_addr & r.m_mask.S_un.S_addr);
}

bool JEngine::IPFilter::operator>(const IPFilter& r) const
{
	return (m_addr.S_un.S_addr & m_mask.S_un.S_addr) > (r.m_addr.S_un.S_addr & r.m_mask.S_un.S_addr);
}

void JEngine::IPFilter::setCIDR(in_addr sa, u_short m)
{
	_ASSERT(m <= 32);
	m_addr.S_un.S_addr = ntohl(sa.S_un.S_addr);
	m_mask.S_un.S_addr = 0xFFFFFFFF << (32 - m);
}

void JEngine::IPFilter::setCIDR(const std::string& cidr)
{
	int pos = cidr.find('/');
	m_addr.S_un.S_addr = ntohl(inet_addr(cidr.substr(0, pos).c_str()));
	m_mask.S_un.S_addr = pos != std::string::npos
		? 0xFFFFFFFF << (32 - atoi(cidr.substr(pos+1).c_str()))
		: 0xFFFFFFFF;
}

bool JEngine::IPFilter::contain(in_addr sa) const
{
	sa.S_un.S_addr = ntohl(sa.S_un.S_addr);
	return ((sa.S_un.S_addr ^ m_addr.S_un.S_addr) & m_mask.S_un.S_addr) == 0;
}

JEngine::JEngine()
: JNode(NULL, false)
{
	jpManager = new JManager(this);
	InsertChild(jpManager);
	jpIocpManager = new JIocpListener(this);
	InsertChild(jpIocpManager);

	InitializeCriticalSection(&m_csLinks);
	InitializeCriticalSection(&m_csValidate);
	InitializeCriticalSection(&m_csClose);

	m_hCompPort = 0;
}

JEngine::~JEngine()
{
	DeleteCriticalSection(&m_csLinks);
	DeleteCriticalSection(&m_csValidate);
	DeleteCriticalSection(&m_csClose);
}

void JEngine::Init()
{
	_VERIFY(!WSAStartup(MAKEWORD(2, 2), &wsaData));

	SetNode(this, false, false);
	__super::Init();

	m_hCompPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, (u_long)0, 0);
}

void JEngine::Done()
{
	CloseHandle(m_hCompPort);

	__super::Done();

	_VERIFY(!WSACleanup());
}

bool JEngine::HasLink(SOCKET sock) const
{
	DoCS cs(&m_csLinks);
	return HASMAP(m_aLinksEvent, sock) || HASMAP(m_aLinksAsync, sock) || HASMAP(m_aLinksIocp, sock);
}

size_t JEngine::countEstablished() const
{
	DoCS cs(&m_csLinks);
	size_t count = 0;
	for each (SetJID::value_type const& v in m_aLinksEvent)
		if (JLink::get(v)->isEstablished()) count++;
	for each (SetJID::value_type const& v in m_aLinksAsync)
		if (JLink::get(v)->isEstablished()) count++;
	for each (SetJID::value_type const& v in m_aLinksIocp)
		if (JLink::get(v)->isEstablished()) count++;
	return count;
}

void JEngine::ValidateOn(SOCKET sock)
{
	DWORD timeout = ValidateTimeout(sock);
	if (timeout) {
		DoCS cs(&m_csValidate);
		m_mValidate[sock] = GetTickCount() + timeout;
	}
}

void JEngine::ValidateOff(SOCKET sock)
{
	DoCS cs(&m_csValidate);
	m_mValidate.erase(sock);
}

bool JEngine::isBanned(in_addr sa) const
{
	for each (IPFilter const& v in m_aBanList) {
		if (v.contain(sa))
			return true;
	}
	return false;
}

bool JEngine::setBan(const IPFilter& ipf)
{
	return m_aBanList.insert(ipf).second;
}

bool JEngine::setBan(const std::string& cidr)
{
	IPFilter ipf;
	ipf.setCIDR(cidr);
	return m_aBanList.insert(ipf).second;
}

int  JEngine::resBan(in_addr sa)
{
	int count = 0;
	SetIPFilter::iterator iter = m_aBanList.begin();
	while (iter != m_aBanList.end()) {
		if (iter->contain(sa)) {
			iter = m_aBanList.erase(iter);
			count++;
		} else iter++;
	}
	return count;
}

void JEngine::InsertLink(JPtr<JLink> link)
{
	DoCS cs(&m_csLinks);
	if (link->Mode == eEvent) {
		m_aLinksEvent.insert(link->ID);

		JPtr<JEventSock> jp;
		if (m_aEventSock.empty() || m_aEventSock.back()->m_aLinks.size() >= (HANDLEBYTHREAD-1)) {
			jp = new JEventSock(this);
			jp->m_aLinks.insert(link->ID);
			InsertChild(jp);
			m_aEventSock.push_back(jp);
		} else {
			jp = m_aEventSock.back();
			jp->m_aLinks.insert(link->ID);
		}
		jp->Wakeup();
	} else if (link->Mode == eAsync) {
		m_aLinksAsync.insert(link->ID);
	} else if (link->Mode == eIocp) {
		m_aLinksIocp.insert(link->ID);
	}
}

void JEngine::DeleteLink(SOCKET sock)
{
	ValidateOff(sock);

	DoCS cs(&m_csLinks);
	JPtr<JLink> link = JLink::get(sock);
	if (link) {
		if (link->Mode == eEvent) {
			m_aLinksEvent.erase(sock);

			JPtr<JEventSock> jp = 0;
			for each (VecEventSock::value_type const& v in m_aEventSock) {
				if (HASMAP(v->m_aLinks, sock)) {
					jp = m_aEventSock.back();
					if (v != jp) {
						_ASSERT(jp->m_aLinks.size());
						JID sockm = *jp->m_aLinks.begin();
						jp->m_aLinks.erase(sockm);
						_VERIFY(v->m_aLinks.insert(sockm).second);
						jp->Wakeup();
					}
					v->m_aLinks.erase(sock);
					v->Wakeup();
					if (jp->m_aLinks.empty()) {
						m_aEventSock.pop_back();
						DeleteChild(jp);
					}
					break;
				}
			}
			_ASSERT(jp);
		} else if (link->Mode == eAsync) {
			m_aLinksAsync.erase(sock);
		} else if (link->Mode == eIocp) {
			m_aLinksIocp.erase(sock);
		}
		link->Close();
		JLink::destroy(sock);
	}
}

//-----------------------------------------------------------------------------

// --- Sending ---

bool JEngine::PushTrn(SOCKET sock, JTransaction* jpTrn, size_t ssi) throw()
{
	if (!HasLink(sock) || !jpTrn) return false;

	DoCS cs(&m_csLinks);
	JPtr<JLink> link = JLink::get(sock);
	if (link->bAccessAllowed || jpTrn->isPrimary()) {
		link->AddSend(jpTrn, ssi);
		link->SendData();

		JEngine::Stat.dlSentTrn++; // update statistics
		return true;
	} else {
		return false;
	}
}

int  JEngine::BroadcastTrn(const SetJID& set, JTransaction* jpTrn, size_t ssi) throw()
{
	int sent = 0;
	for each (SetJID::value_type const& v in set) {
		if (PushTrn(v, jpTrn, ssi))
			sent++;
	}
	return sent;
}

SOCKET JEngine::MakeListener(const sockaddr_in& service)
{
	SOCKET listensocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (listensocket == INVALID_SOCKET) return INVALID_SOCKET;

	if (bind(listensocket,(SOCKADDR*) &service, sizeof(service)) == SOCKET_ERROR) {
		closesocket(listensocket);
		return INVALID_SOCKET;
	} else if (listen(listensocket, SOMAXCONN) == SOCKET_ERROR) {
		closesocket(listensocket);
		return INVALID_SOCKET;
	} else {
		HANDLE h = WSACreateEvent();
		WSAEventSelect(listensocket, h, FD_ACCEPT | FD_CLOSE);
		m_mListen[listensocket] = h;
		jpIocpManager->Wakeup();
		return listensocket;
	}
}

JPtr<JLink> JEngine::createLink() const
{
	DoCS cs(&m_csLinks);
	return new JLink();
}

JPtr<JLink> JEngine::createLink(SOCKET sock, const sockaddr_in& addr) const
{
	DoCS cs(&m_csLinks);
	return new JLink(sock, addr);
}

int JEngine::DispatchLoop(SOCKET sock)
{
	JPtr<JLink> link = JLink::get(sock);
	JPtr<JTransaction> jpTrn = createTrn();
	int count = 0;
	while (HasLink(sock) && jpTrn->unserialize(link)) {
		JEngine::Stat.dlRecvTrn++; // update statistics
		DispatchTrn(sock, jpTrn);
		count++;
	}
	return count;
}

//-----------------------------------------------------------------------------

// --- Events response ---

void JEngine::EventSelector(SOCKET sock, UINT ev, UINT err) throw()
{
	_ASSERT(sock);
	switch (ev)
	{
	case FD_ACCEPT:
		{
			sockaddr_in addr;
			int len = sizeof(addr);
			sock = accept(sock, (sockaddr*)&addr, &len); // workig with accepted socket now
			if (sock != INVALID_SOCKET) {
				if (!isBanned(addr.sin_addr)) {
					bool sel;
					DoCS cs(&m_csLinks);
					JPtr<JLink> link = createLink(sock, addr);
					sel = link->SelectEvent(FD_READ | FD_WRITE | FD_CLOSE) == 0;
					if (sel) {
						link->Accepted();
						ValidateOn(sock);
						InsertLink(link);
					} else
						JLink::destroy(link->ID);
					if (sel) {
						jpManager->Wakeup();
						EvLinkAccept(sock);
						EvLinkAccess(sock, false);
						EvLinkEstablished(sock);
					}
				} else {
					closesocket(sock);
				}
			}

			break;
		}

	case FD_CONNECT:
		{
			EnterCriticalSection(&m_csLinks);
			JPtr<JLink> link = JLink::get(sock);
			link->Connected();
			LeaveCriticalSection(&m_csLinks);

			if (!err) {
				EvLinkConnect(sock);
				EvLinkAccess(sock, false);
				EvLinkEstablished(sock);
			} else {
				EvLinkFail(sock, err);
			}
			break;
		}

	case FD_READ:
		{
			DoCS cs(&m_csLinks);
			JPtr<JLink> link = JLink::get(sock);
			_ASSERT(link);
			if (link) {
				// Recieve data from given socket
				if (link->RecvData()) {
					DispatchLoop(sock);
				}
			}
			break;
		}

	case FD_WRITE:
		{
			DoCS cs(&m_csLinks);
			JPtr<JLink> link = JLink::get(sock);
			_ASSERT(link);
			if (link) {
				link->m_bReadyWrite = true;
				link->SendData();
			}
			break;
		}

	case FD_CLOSE:
		{
			EvLinkClose(sock, err);
			break;
		}

	default:
		break;
	}
}

void JEngine::OnHook(JNode* src)
{
	using namespace fastdelegate;

	__super::OnHook(src);

	JNODE(JEngine, node, src);
	if (node) {
		node->EvLinkAccept += MakeDelegate(this, &JEngine::OnLinkAccept);
		node->EvLinkConnect += MakeDelegate(this, &JEngine::OnLinkConnect);
		node->EvLinkEstablished += MakeDelegate(this, &JEngine::OnLinkEstablished);
		node->EvLinkAccess += MakeDelegate(this, &JEngine::OnLinkAccess);
		node->EvLinkIdentify += MakeDelegate(this, &JEngine::OnLinkIdentify);
		node->EvLinkStart += MakeDelegate(this, &JEngine::OnLinkStart);
		node->EvLinkClose += MakeDelegate(this, &JEngine::OnLinkClose);
		node->EvLinkFail += MakeDelegate(this, &JEngine::OnLinkFail);
	}
	RegHandlers(src);
}

void JEngine::OnUnhook(JNode* src)
{
	using namespace fastdelegate;

	UnregHandlers(src);
	JNODE(JEngine, node, src);
	if (node) {
		node->EvLinkAccept -= MakeDelegate(this, &JEngine::OnLinkAccept);
		node->EvLinkConnect -= MakeDelegate(this, &JEngine::OnLinkConnect);
		node->EvLinkEstablished -= MakeDelegate(this, &JEngine::OnLinkEstablished);
		node->EvLinkAccess -= MakeDelegate(this, &JEngine::OnLinkAccess);
		node->EvLinkIdentify -= MakeDelegate(this, &JEngine::OnLinkIdentify);
		node->EvLinkStart -= MakeDelegate(this, &JEngine::OnLinkStart);
		node->EvLinkClose -= MakeDelegate(this, &JEngine::OnLinkClose);
		node->EvLinkFail -= MakeDelegate(this, &JEngine::OnLinkFail);
	}

	__super::OnUnhook(src);
}

void JEngine::OnLinkAccess(SOCKET sock, bool access)
{
	DoCS cs(&m_csLinks);
	JPtr<JLink> link = JLink::get(sock);
	_ASSERT(link);
	link->m_bAccessAllowed = access;
	if (access) {
		ValidateOff(sock);
	}
}

void JEngine::OnLinkIdentify(SOCKET sock)
{
	DoCS cs(&m_csLinks);
	JPtr<JLink> link = JLink::get(sock);
	_ASSERT(link);
	link->m_bAccessAllowed = true;
	ValidateOff(sock);
}

void JEngine::OnLinkClose(SOCKET sock, UINT err)
{
	{
		DoCS cs(&m_csClose);
		jpManager->m_aClose.insert(sock);
	}
	jpManager->Wakeup();
}

void JEngine::OnLinkFail(SOCKET sock, UINT err)
{
	{
		DoCS cs(&m_csClose);
		jpManager->m_aClose.insert(sock);
	}
	jpManager->Wakeup();
}

//-----------------------------------------------------------------------------

// The End.