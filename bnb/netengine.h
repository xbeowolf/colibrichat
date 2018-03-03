/******************************************************************************
*                                                                             *
* netengine.h -- Beowolf Network Engine. Sockets JEngine class definition     *
*                                                                             *
* Copyright (c) Podobashev Dmitry / BEOWOLF, 2006-2010. All rights reserved.  *
*                                                                             *
******************************************************************************/

#ifndef _NETENGINE_
#define _NETENGINE_

//-----------------------------------------------------------------------------

//
// Includes
//

#pragma region Includes
#pragma once

// Windows API
#ifndef _WINSOCK2API_
#include <winsock2.h>
#endif

// Common
#include "JService.h"

#pragma endregion

//-----------------------------------------------------------------------------

// Engine-depended WSA error codes
#define WSAVALIDATETIME        1
#define WSABADCRC              2
#define WSABANNED              3

#define MAKEVERSIONNUM(b1,b2,b3,b4) ((LPARAM)(((DWORD)(b1)<<24)+((DWORD)(b2)<<16)+((DWORD)(b3)<<8)+((DWORD)(b4))))
#define MAKEVERSIONSTR(b1,b2,b3,b4) TEXT(#b1)TEXT(".")TEXT(#b2)TEXT(".")TEXT(#b3)TEXT(".")TEXT(#b4)
#define MAKEVERSIONSTRW(b1,b2,b3,b4) L##b1 L"." L##b2 L"." L##b3 L"." L##b4
#define MAKEVERSIONSTRA(b1,b2,b3,b4) #b1 "." #b2 "." #b3 "." #b4

// default time for identification waiting, ms
#define VALIDATETIME           5000

//-----------------------------------------------------------------------------

namespace netengine
{
	using namespace attachment;
	//
	// Predefinitions
	//

	class JTransaction;
	class JLink;
	class JEngine;

	//
	// Enumerations
	//

	enum ELog {
		elogDef   = 0,
		elogError = 1,
		elogWarn  = 2,
		elogIgnor = 3,
		elogInfo  = 4,
		elogMsg   = 5,
		elogDescr = 6,
		elogItrn  = 7,
		elogOtrn  = 8,
	};
	enum EMode {
		eNosock, eBlock, eEvent, eAsync, eIocp,
	};
	enum EConnection {
		eEmpty, eClosed, eListening, eConnecting, eConnected, eAccepted,
	};
	enum EIOCP {
		eRecv = 1, eSend = 2,
	};

	struct BOVERLAPPED : WSAOVERLAPPED
	{
		EIOCP op;
		WSABUF buf;
	};

	typedef std::map<SOCKET, HANDLE> MapListen;
	typedef std::map<SOCKET, DWORD> MapValidate;
	typedef std::set<SOCKET> SetSock;

	//
	// Interface
	//

	__interface ITransaction
	{
		// Checks transaction content on possibility processing without identification
		bool isPrimary() throw();

		// stream IO
		void serialize(JPtr<JLink> link); // write to send buffer
		bool unserialize(JPtr<JLink> link); // read from recv buffer
	};

	class JTransaction : public ITransaction, public JClass {};

	class JLink : public JIDClass<JLink, SOCKET>
	{
	public:

		friend class JEngine;

		static bool CALLBACK MakeProxy(JPtr<JLink> link1, JPtr<JLink> link2);
		static void CALLBACK BreakProxy(JPtr<JLink> link1, JPtr<JLink> link2);

		// Rerurns next valid QUEST transaction identifier for this connection
		static WORD getNextQuestId();

		explicit JLink();
		JLink(SOCKET sock, const sockaddr_in& addr);
		void beforeDestruct();

		// --- Working functions ---

		// Returns true if accepted or connected
		bool isEstablished() const;
		// Returns error on socket
		int  getError() const;

		// Sending queues
		__declspec(property(get=getSendStreamCount,put=setSendStreamCount)) size_t SendStreamCount;
		size_t getSendStreamCount() const;
		void setSendStreamCount(size_t val);

		// --- Creation functions ---

		// First call to structure
		// is address assignment

		// Second call to structure
		// and also can be used for reselections
		int  SelectEvent(long ne, HANDLE hevent = 0);
		int  SelectWindow(long ne, HWND hwnd, WORD msg); // hwnd and msg can not be 0
		bool SelectIocp(HANDLE iocp);

		// Third call to structure
		void Listen();
		void Connect();
		void Connected();
		void Accepted();

		// Final call
		void Close();

		// Freeze/unfreeze data sending
		int Freeze();
		int Unfreeze();

		void Clear(); // inits all data members

		// Add/sub data to queue
		void AddRecv(const char* data, size_t len) throw();
		void AddSend(JPtr<JTransaction> jpTrn, size_t ssi = 0) throw();
		void SubRecv(size_t len) throw();
		void SubSend(size_t len) throw();

		// Data transfer, for network events responses
		size_t RecvData() throw();
		size_t SendData() throw();

	public:

		// --- Data ---

		// Connection protocol version
		DWORD m_dwProtocolVers;

		sockaddr_in m_saAddr;

	protected:

		JPROPERTY_R(EMode, Mode);
		// Sockets handles
		JPROPERTY_R(SOCKET, proxyID); // reserved for future use, ref to other link
		JPROPERTY_R(HANDLE, Event);
		JPROPERTY_R(bool, bAutocloseHandle);
		// Addition connections information
		JPROPERTY_R(WORD, wMsg);
		JPROPERTY_R(long, lNetworkEvents);

		// Socket state
		JPROPERTY_R(EConnection, State);
		JPROPERTY_RREF_CONST(FILETIME, ftTime);
		JPROPERTY_RW(bool, bReadyWrite);
		JPROPERTY_R(bool, bAccessAllowed);
		JPROPERTY_R(DWORD, dwLastTransmission);

		// Streaming buffers
		JPROPERTY_R(int, nFreezeCount); // do not send data if value > 0
		JPROPERTY_RW(size_t, nSreamable); // send transactions as stream or by single transactions
		JPROPERTY_RREF(std::string, aBufferRecv);
		JPROPERTY_RREF(std::string, aBufferSend);
		JPROPERTY_RREF_CONST(std::vector<std::list<JPtr<JTransaction>>>, aStorage); // send storage
		JPROPERTY_R(size_t, nStorageIndex);
		JPROPERTY_RW(bool, bHalfTrnRecv); // recieved a half of transaction

		BOVERLAPPED ioRecv, ioSend;
		std::string recvbuf;

		static WORD s_wQuestCount;
	};

	class JEngine : public JService
	{
	public:

		//
		// Threads
		//

		class JEventSock : public JThread
		{
		public:

			friend class JEngine;
			GETJNODE(JEngine);

			JEventSock(JEngine* p);

		protected:

			DWORD ThreadProc();

			// Links with connections
			JPROPERTY_RREF_CONST(SetSock, aLinks);
		};
		typedef std::vector<JPtr<JEventSock>> VecEventSock;

		class JManager : public JThread
		{
		public:

			friend class JEngine;
			GETJNODE(JEngine);

			JManager(JEngine* p);

		protected:

			DWORD ThreadProc();

			// Links to close
			JPROPERTY_RREF_CONST(SetSock, aClose);
		};

		class JIocpListener : public JThread
		{
		public:

			friend class JEngine;
			GETJNODE(JEngine);

			JIocpListener(JEngine* p);

		protected:

			DWORD ThreadProc();

			// Links to close
			JPROPERTY_RREF_CONST(SetSock, aClose);
		};

		class JIocpSock : public JThread
		{
		public:

			friend class JEngine;
			GETJNODE(JEngine);

			JIocpSock(JEngine* p);

		protected:

			DWORD ThreadProc();
		};
		typedef std::vector<JPtr<JIocpSock>> VecIocpSock;

		//
		// Types
		//

		struct Statistics
		{
			// Total received and sent bytes
			unsigned __int64 dlRecvBytes, dlSentBytes;
			// Total tranfered by proxy
			unsigned __int64 dlProxiedBytes;
			// Received and sent transactions
			unsigned __int64 dlRecvTrn, dlSentTrn;
			// Received transactions that were processed and ignored
			unsigned __int64 dlRecvTrnProcessed, dlRecvTrnIgnored;
		};

		struct IPFilter
		{
			bool operator==(const IPFilter& r) const;
			bool operator<(const IPFilter& r) const;
			bool operator>(const IPFilter& r) const;

			void setCIDR(in_addr sa, u_short m = 32);
			void setCIDR(const std::string& cidr);
			bool contain(in_addr sa) const;

			JPROPERTY_R(in_addr, addr);
			JPROPERTY_R(in_addr, mask);
		};
		typedef std::set<IPFilter> SetIPFilter;

	public:

		JEngine();
		~JEngine();

		// --- Service ---

		void Init();
		void Done();

	public:

		// --- Links work ---

		// Checkup link existance
		bool HasLink(SOCKET sock) const;
		// Encounts sockets by it's mode
		size_t countEstablished() const;
		// Timeout to validate the link, ms
		virtual DWORD ValidateTimeout(SOCKET sock) {
#ifndef _DEBUG
			return 0;}
#else
			return VALIDATETIME;}
#endif
		void ValidateOn(SOCKET sock);
		void ValidateOff(SOCKET sock);

		// Ban functions
		bool isBanned(in_addr sa) const; // checkup IP-address if it in banned
		bool setBan(const IPFilter& ipf);
		bool setBan(const std::string& cidr);
		int  resBan(in_addr sa);

		void InsertLink(JPtr<JLink> link);

	protected:

		void DeleteLink(SOCKET sock);

	public:

		// --- Sending ---

		// Sends transaction with given message and data
		bool PushTrn(SOCKET sock, JTransaction* jpTrn, size_t ssi = 0) throw(); // push only prepared valid data
		// Broadcast transaction to all given sockets
		int  BroadcastTrn(const SetSock& set, JTransaction* jpTrn, size_t ssi = 0) throw();

	protected:

		// --- Transactions work ---

		SOCKET MakeListener(const sockaddr_in& service);
		// Create link object for this engine
		virtual JPtr<JLink> createLink() const;
		virtual JPtr<JLink> createLink(SOCKET sock, const sockaddr_in& addr) const;
		// Creates transaction for this protocol
		virtual JPtr<JTransaction> createTrn() const = 0;
		// Extracts and dispatch transactions from stream to objects
		virtual bool DispatchTrn(SOCKET sock, JTransaction* jpTrn) = 0;
		int DispatchLoop(SOCKET sock);

		// --- Events responders ---

		void EventSelector(SOCKET sock, UINT ev, UINT err) throw();

		void OnHook(JNode* src);
		void OnUnhook(JNode* src);
		// Register/unregister transactions parsers
		virtual void RegHandlers(JNode* src) {};
		virtual void UnregHandlers(JNode* src) {};

		virtual void OnLinkAccept(SOCKET sock) {};
		virtual void OnLinkConnect(SOCKET sock) {};
		virtual void OnLinkEstablished(SOCKET sock) {};
		virtual void OnLinkAccess(SOCKET sock, bool access);
		virtual void OnLinkIdentify(SOCKET sock);
		virtual void OnLinkStart(SOCKET sock) {};
		virtual void OnLinkClose(SOCKET sock, UINT err);
		virtual void OnLinkFail(SOCKET sock, UINT err);

	public:

		// --- Socket events ---

		fastdelegate::FastDelegateList1<SOCKET>
			EvLinkAccept; // sent for server socket
		fastdelegate::FastDelegateList1<SOCKET>
			EvLinkConnect;
		fastdelegate::FastDelegateList1<SOCKET>
			EvLinkEstablished; // already accepted or connected
		fastdelegate::FastDelegateList2<SOCKET, bool>
			EvLinkAccess; // recieve password identification
		fastdelegate::FastDelegateList1<SOCKET>
			EvLinkIdentify; // give opportunity for actions on valid access
		fastdelegate::FastDelegateList1<SOCKET>
			EvLinkStart; // successful identification to start conversation
		fastdelegate::FastDelegateList2<SOCKET, UINT>
			EvLinkClose; // sent at all cases for lost connection
		fastdelegate::FastDelegateList2<SOCKET, UINT>
			EvLinkFail; // sent at all cases of failures on sockets during connection

		fastdelegate::FastDelegateList2<const std::string&, ELog>
			EvLog; // write string to log

		// --- Data ---

		static WSADATA wsaData;
		static Statistics Stat;
		static HANDLE s_hErrlog;

	protected:

		JPROPERTY_R(HANDLE, hCompPort);

		// Links with connections
		JPROPERTY_RREF_CONST(SetSock, aLinksEvent);
		JPROPERTY_RREF_CONST(SetSock, aLinksAsync);
		JPROPERTY_RREF_CONST(SetSock, aLinksIocp);
		JPROPERTY_RREF_CONST(MapListen, mListen);
		JPROPERTY_RREF(MapValidate, mValidate);

		// Ban-list
		JPROPERTY_RREF_CONST(SetIPFilter, aBanList);

	protected:

		JPtr<JManager> jpManager;
		JPtr<JIocpListener> jpIocpManager;

		// Threads for nonblocking sockets processing
		JPROPERTY_RREF_CONST(VecIocpSock, aIocpSock);

		// Threads for nonblocking sockets processing
		JPROPERTY_RREF_CONST(VecEventSock, aEventSock);

		mutable CRITICAL_SECTION m_csLinks, m_csValidate, m_csClose;
	};
}; // netengine

//-----------------------------------------------------------------------------

#endif // _NETENGINE_