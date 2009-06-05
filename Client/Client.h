/******************************************************************************
*                                                                             *
* Client.h -- Beowolf Network Protocol Engine explore sample                  *
*                                                                             *
* Copyright (c) Podobashev Dmitry / BEOWOLF, 2009. All rights reserved.       *
*                                                                             *
******************************************************************************/

#ifndef _COLIBRICHAT_CLIENT_
#define _COLIBRICHAT_CLIENT_

//-----------------------------------------------------------------------------

//
// Includes
//

#pragma region Includes

// Common
#include "netengine.h"
#include "app.h"
#include "jrtf.h"

// Project
#include "..\ColibriCommon.h"

#pragma endregion

//-----------------------------------------------------------------------------

#define APPNAME                TEXT("Colibri Chat")
#define APPDATE                TEXT(__DATE__)
#define APPDATEW               WTEXT(__DATE__)
#define APPDATEA               ATEXT(__DATE__)
#define APPVER                 TEXT("1.0")

//
// Register
//

// Folders
#define RF_CLIENT              TEXT("Client\\")

// Server
#define RK_NICK                TEXT("Nickname")
#define RK_HOST                TEXT("Host")
#define RK_PORT                TEXT("Port")
#define RK_PASSWORD            TEXT("Password")
#define RK_STATE               TEXT("ConnectionState")
#define RK_STATUS              TEXT("Status")
#define RK_STATUSIMG           TEXT("StatusImage")
#define RK_STATUSMSG           TEXT("StatusMessage")
//#define RK_QUITPAUSE           TEXT("QuitPause")

// Interface
#define RK_SENDBYENTER         TEXT("SendByEnter")
#define RK_FLASHPAGENEW        TEXT("FlashPageNew")
#define RK_FLASHPAGESAYPRIVATE TEXT("FlashPageSayPrivate")
#define RK_FLASHPAGESAYCHANNEL TEXT("FlashPageSayChannel")

// --- Toolbar's image list indexes ---

// Rich edit commands
#define IML_BOLD               0
#define IML_ITALIC             1
#define IML_ULINE              2
#define IML_SUBSCRIPT          3
#define IML_SUPERSCRIPT        4
#define IML_FONT               5
#define IML_FRCOL              6
#define IML_BGCOL              7
#define IML_SHEETCOL           8
#define IML_LEFT               9
#define IML_CENTER             10
#define IML_RIGHT              11
#define IML_JUSTIFY            12
#define IML_MARKS_BULLET       13
#define IML_MARKS_ARABIC       14
#define IML_STARTINDENTINC     15
#define IML_STARTINDENTDEC     16
#define IML_BKMODE             17
// Tab icons
#define IML_CHANNELVOID        0
#define IML_CHANNELRED         1
#define IML_CHANNELYELLOW      2
#define IML_CHANNELGREEN       3
#define IML_CHANNELCYAN        4
#define IML_CHANNELBLUE        5
#define IML_CHANNELMAGENTA     6
#define IML_PRIVATEGREEN       7
#define IML_PRIVATEBLUE        8
#define IML_PRIVATERED         9
#define IML_SERVERGREEN        10
#define IML_SERVERBLUE         11
#define IML_SERVERRED          12
// Man icons
#define IML_MANREDON           0
#define IML_MANYELLOWON        1
#define IML_MANGREENON         2
#define IML_MANCYANON          3
#define IML_MANBLUEON          4
#define IML_MANMAGENTAON       5
#define IML_MANREDOFF          6
#define IML_MANYELLOWOFF       7
#define IML_MANGREENOFF        8
#define IML_MANCYANOFF         9
#define IML_MANBLUEOFF         10
#define IML_MANMAGENTAOFF      11
#define IML_MANVOID            60
#define IML_MAN_COUNT          61
// Status images
#define IML_STATUSIMG_COUNT    38

//-----------------------------------------------------------------------------

namespace colibrichat
{
	enum ETimeFormat {etimeNone, etimeHHMM, etimeHHMMSS};
	struct Tab
	{
		std::tstring name;
		EContact type;
		HWND hwnd;
	};
	typedef std::map<DWORD, Tab> MapTab;

	class JClient : public netengine::JEngine, public JDialog
	{
	public:

		//
		// Predefinitions
		//

		class JPage;
		typedef std::map<DWORD, JPtr<JPage>> MapPage;
		class JPageLog;
		class JPageServer;
		class JPageList;
		class JPageUser;
		typedef std::map<DWORD, JPtr<JPageUser>> MapPageUser;
		class JPageChannel;
		typedef std::map<DWORD, JPtr<JPageChannel>> MapPageChannel;
		class JPageBoard;
		typedef std::map<DWORD, JPtr<JPageBoard>> MapPageBoard;

		//
		// Pages
		//

		class JPage : public JAttachedDialog<JClient>
		{
		public:

			static bool CALLBACK Write(HWND hwnd, const TCHAR* str);

			CALLBACK JPage();

			virtual bool CALLBACK MinTrackSize(SIZE& size) const {return false;}
			virtual int CALLBACK ImageIndex() const = 0;
			virtual LPCTSTR CALLBACK Template() const = 0;
			virtual bool CALLBACK IsPermanent() const = 0;

			virtual EContact CALLBACK gettype() const = 0;
			virtual DWORD CALLBACK getID() const = 0;
			virtual std::tstring CALLBACK getname() const = 0;

		protected:

			//LRESULT WINAPI DlgProc(HWND, UINT, WPARAM, LPARAM);

			void OnHook(JEventable* src);
			void OnUnhook(JEventable* src);
		};

		class JPageLog : public JPage
		{
		public:

			CALLBACK JPageLog();

			void CALLBACK AppendRtf(const std::string& content) const;
			void CALLBACK AppendScript(const std::tstring& content, bool withtime = true) const;

		protected:

			LRESULT WINAPI DlgProc(HWND, UINT, WPARAM, LPARAM);

		protected:

			JPROPERTY_RREF_CONST(std::set<netengine::EGroup>, Groups);
			JPROPERTY_R(netengine::EPriority, Priority);
			ETimeFormat etimeFormat;

			JPROPERTY_R(HWND, hwndLog);
			RECT rcLog;
		};

		class JPageServer : public JPageLog
		{
		public:

			CALLBACK JPageServer();

			int CALLBACK ImageIndex() const {return IML_SERVERGREEN;}
			LPCTSTR CALLBACK Template() const {return MAKEINTRESOURCE(IDD_SERVER);}
			bool CALLBACK IsPermanent() const {return true;}

			EContact CALLBACK gettype() const {return eServer;}
			DWORD CALLBACK getID() const {return CRC_SERVER;}
			std::tstring CALLBACK getname() const {return NAME_SERVER;}

		protected:

			LRESULT WINAPI DlgProc(HWND, UINT, WPARAM, LPARAM);

			void OnHook(JEventable* src);
			void OnUnhook(JEventable* src);

			void OnLinkConnecting(SOCKET sock);
			void OnLinkConnect(SOCKET sock);
			void OnLinkDestroy(SOCKET sock);
			void OnLog(const std::tstring& str, bool withtime = true);
			void OnReport(const std::tstring& str, netengine::EGroup gr = netengine::eMessage, netengine::EPriority prior = netengine::eNormal);

		protected:

			JPROPERTY_R(HWND, hwndHost);
			JPROPERTY_R(HWND, hwndPort);
			JPROPERTY_R(HWND, hwndPass);
			JPROPERTY_R(HWND, hwndNick);
			JPROPERTY_R(HWND, hwndStatus);
			JPROPERTY_R(HWND, hwndStatusImg);
			JPROPERTY_R(HWND, hwndStatusMsg);
		};

		class JPageList : public JPage
		{
		public:

			CALLBACK JPageList();

			int CALLBACK ImageIndex() const {return IML_CHANNELGREEN;}
			LPCTSTR CALLBACK Template() const {return MAKEINTRESOURCE(IDD_LIST);}
			bool CALLBACK IsPermanent() const {return true;}

			EContact CALLBACK gettype() const {return eList;}
			DWORD CALLBACK getID() const {return CRC_LIST;}
			std::tstring CALLBACK getname() const {return NAME_LIST;}

		protected:

			LRESULT WINAPI DlgProc(HWND, UINT, WPARAM, LPARAM);

			void CALLBACK BuildView();
			void CALLBACK ClearView();
			int CALLBACK AddLine(DWORD id);

		protected:

			// Beowolf Network Protocol Messages reciving
			void CALLBACK Recv_Reply_LIST(SOCKET sock, WORD trnid, io::mem& is);

			// Beowolf Network Protocol Messages sending
			void CALLBACK Send_Quest_LIST(SOCKET sock);

			void OnHook(JEventable* src);
			void OnUnhook(JEventable* src);

			void OnLinkConnect(SOCKET sock);
			void OnLinkDestroy(SOCKET sock);
			void OnLinkIdentify(SOCKET sock, const netengine::SetAccess& access);
			void OnTransactionProcess(SOCKET sock, WORD message, WORD trnid, io::mem is);

		protected:

			JPROPERTY_R(HWND, hwndList);
			JPROPERTY_R(HWND, hwndChan);
			JPROPERTY_R(HWND, hwndPass);
			RECT rcList;

			JPROPERTY_RREF_CONST(MapChannel, mChannel);
		};

		class JPageUser : public JPageLog, public rtf::Editor
		{
		public:

			CALLBACK JPageUser(DWORD id, const std::tstring& nick);

			int CALLBACK ImageIndex() const {return IML_PRIVATEGREEN;}
			LPCTSTR CALLBACK Template() const {return MAKEINTRESOURCE(IDD_USER);}
			bool CALLBACK IsPermanent() const {return false;}

			EContact CALLBACK gettype() const {return eUser;}
			DWORD CALLBACK getID() const {return m_ID;}
			std::tstring CALLBACK getname() const {return m_user.name;}

			void CALLBACK setuser(const User& val) {m_user = val;}
			void CALLBACK rename(DWORD idNew, const std::tstring& newname);

		protected:

			LRESULT WINAPI DlgProc(HWND, UINT, WPARAM, LPARAM);

			void OnHook(JEventable* src);
			void OnUnhook(JEventable* src);

			void OnLinkConnect(SOCKET sock);
			void OnLinkDestroy(SOCKET sock);

		protected:

			DWORD m_ID;
			JPROPERTY_RREF_CONST(User, user);

			JPROPERTY_R(HWND, hwndMsgSpin);
			JPROPERTY_R(HWND, hwndSend);
			RECT rcSend;

			std::vector<std::string> vecMsgSpin;
		};

		class JPageChannel : public JPageLog, public rtf::Editor
		{
		public:

			CALLBACK JPageChannel(DWORD id, const std::tstring& nick);

			int CALLBACK ImageIndex() const {return IML_CHANNELGREEN;}
			LPCTSTR CALLBACK Template() const {return MAKEINTRESOURCE(IDD_CHANNEL);}
			bool CALLBACK IsPermanent() const {return false;}

			EContact CALLBACK gettype() const {return eChannel;}
			DWORD CALLBACK getID() const {return m_ID;}
			std::tstring CALLBACK getname() const {return m_channel.name;}

			void CALLBACK setchannel(const Channel& val);
			void CALLBACK rename(DWORD idNew, const std::tstring& newname);
			bool CALLBACK replace(DWORD idOld, DWORD idNew);

			void CALLBACK Join(DWORD idUser);
			void CALLBACK Part(DWORD idUser, DWORD reason);

			int  CALLBACK indexIcon(DWORD idUser) const;

		protected:

			LRESULT WINAPI DlgProc(HWND, UINT, WPARAM, LPARAM);

			void CALLBACK BuildView();
			void CALLBACK ClearView();
			int  CALLBACK AddLine(DWORD id);
			void CALLBACK DelLine(DWORD id);

			void OnHook(JEventable* src);
			void OnUnhook(JEventable* src);

			void OnLinkConnect(SOCKET sock);
			void OnLinkDestroy(SOCKET sock);

		protected:

			DWORD m_ID;
			JPROPERTY_RREF_CONST(Channel, channel);

			JPROPERTY_R(HWND, hwndList);
			JPROPERTY_R(HWND, hwndMsgSpin);
			JPROPERTY_R(HWND, hwndSend);
			RECT rcList, rcSend;

			std::vector<std::string> vecMsgSpin;
		};

	public:

		// Constructor
		CALLBACK JClient();
		DWORD CALLBACK getMinVersion() const {return BNP_ENGINEVERSMIN;}
		DWORD CALLBACK getCurVersion() const {return BNP_ENGINEVERSNUM;}

		void CALLBACK Init();
		void CALLBACK Done();
		int  CALLBACK Run();

		void CALLBACK JobQuantum() {}

	protected:

		void CALLBACK LoadState(); // can be only one call for object
		void CALLBACK SaveState(); // can be multiply calls for object
		void CALLBACK InitLogs() {}

		LRESULT WINAPI DlgProc(HWND, UINT, WPARAM, LPARAM);

		// Connecting/disconecting to server
		void CALLBACK Connect(bool getsetting = false);
		void CALLBACK Disconnect();

		// Contacts
		void CALLBACK ContactAdd(const std::tstring& name, DWORD id, EContact type);
		void CALLBACK ContactDel(DWORD id);
		void CALLBACK ContactSel(int index);
		void CALLBACK ContactRename(DWORD idOld, DWORD idNew, const std::tstring& newname);
		int  CALLBACK getTabIndex(DWORD id);
		JPtr<JPage> CALLBACK getPage(DWORD id);
		bool CheckNick(std::tstring& nick, std::tstring& msg);

		// Error provider
		void ShowErrorMessage(HWND hwnd, const std::tstring& msg);

		// Users managment
		void CALLBACK InsertUser(DWORD idUser, const User& user);
		void CALLBACK LinkUser(DWORD idUser, DWORD idLink);
		void CALLBACK UnlinkUser(DWORD idUser, DWORD idLink);

	protected:

		// Beowolf Network Protocol Messages reciving
		void CALLBACK Recv_Reply_NICK(SOCKET sock, WORD trnid, io::mem& is);
		void CALLBACK Recv_Notify_RENAME(SOCKET sock, WORD trnid, io::mem& is);
		void CALLBACK Recv_Reply_JOIN(SOCKET sock, WORD trnid, io::mem& is);
		void CALLBACK Recv_Notify_JOIN(SOCKET sock, WORD trnid, io::mem& is);
		void CALLBACK Recv_Notify_PART(SOCKET sock, WORD trnid, io::mem& is);
		void CALLBACK Recv_Reply_USERINFO(SOCKET sock, WORD trnid, io::mem& is);
		void CALLBACK Recv_Notify_ONLINE(SOCKET sock, WORD trnid, io::mem& is);
		void CALLBACK Recv_Notify_STATUS(SOCKET sock, WORD trnid, io::mem& is);
		void CALLBACK Recv_Notify_SAY(SOCKET sock, WORD trnid, io::mem& is);

		// Beowolf Network Protocol Messages sending
		void CALLBACK Send_Quest_NICK(SOCKET sock, const std::tstring& nick);
		void CALLBACK Send_Quest_JOIN(SOCKET sock, const std::tstring& name, const std::tstring& pass = TEXT(""));
		void CALLBACK Send_Cmd_PART(SOCKET sock, DWORD id, DWORD reason);
		void CALLBACK Send_Quest_USERINFO(SOCKET sock, const SetId& set);
		void CALLBACK Send_Cmd_ONLINE(SOCKET sock, bool on, DWORD id);
		void CALLBACK Send_Cmd_STATUS_Mode(SOCKET sock, EUserStatus stat);
		void CALLBACK Send_Cmd_STATUS_Img(SOCKET sock, int img);
		void CALLBACK Send_Cmd_STATUS_Msg(SOCKET sock, const std::tstring& msg);
		void CALLBACK Send_Cmd_STATUS(SOCKET sock, EUserStatus stat, int img, const std::tstring& msg);
		void CALLBACK Send_Cmd_SAY(SOCKET sock, DWORD idWhere, UINT type, const std::string& content);

		void OnHook(JEventable* src);
		void OnUnhook(JEventable* src);

		void OnLinkConnect(SOCKET sock);
		void OnLinkClose(SOCKET sock);
		void OnLinkDestroy(SOCKET sock);
		void OnLinkIdentify(SOCKET sock, const netengine::SetAccess& access);
		void OnTransactionProcess(SOCKET sock, WORD message, WORD trnid, io::mem is);

	public:

		// --- Events ---

		fastdelegate::FastDelegateList1<const Message&>
			EvMessage;
		fastdelegate::FastDelegateList1<SOCKET>
			EvLinkConnecting;

	protected:

		JPROPERTY_R(DWORD, idOwn);
		JPROPERTY_RREF_CONST(MapUser, mUser);

		JPROPERTY_R(SOCKET, clientsock);
		JPROPERTY_RREF_CONST(std::string, hostname);
		JPROPERTY_R(u_short, port);
		JPROPERTY_RREF_CONST(std::tstring, password);
		JPROPERTY_R(bool, bReconnect);
		JPROPERTY_R(bool, bSendByEnter);

		// Pages
		JPtr<JPage> jpOnline;
		JPtr<JPageServer> jpPageServer;
		JPtr<JPageList> jpPageList;
		MapPageUser mPageUser;
		MapPageChannel mPageChannel;

		// HWND tabs
		HWND hwndTab;

		JPROPERTY_RREF_CONST(Metrics, metrics);
	};

	class JClientApp : public JApplication // Singleton
	{
	public:

		virtual void CALLBACK Init();
		virtual bool CALLBACK InitInstance();
		virtual void CALLBACK Done();

	protected:

		CALLBACK JClientApp(HINSTANCE hInstance = 0, HINSTANCE hPrevInstance = 0, LPTSTR szcl = 0, int ncs = SW_SHOWDEFAULT);

	public:

		static JPtr<JClientApp> jpApp;
		JPtr<JClient> jpClient;

	protected:

		HINSTANCE hinstRichEdit;

		JPROPERTY_R(HACCEL, haccelMain);

		JPROPERTY_R(HMENU, hmenuTab);
		JPROPERTY_R(HMENU, hmenuLog);
		JPROPERTY_R(HMENU, hmenuRichEdit);

		JPROPERTY_R(HIMAGELIST, himlEdit);
		JPROPERTY_R(HIMAGELIST, himlTab);
		JPROPERTY_R(HIMAGELIST, himlMan);
		JPROPERTY_R(HIMAGELIST, himlStatus);
		JPROPERTY_R(HIMAGELIST, himlStatusImg);
		JPROPERTY_R(HANDLE, himgSend);
		JPROPERTY_R(HANDLE, himgULBG);
		JPROPERTY_R(HANDLE, himgULFoc);
		JPROPERTY_R(HANDLE, himgULSel);
		JPROPERTY_R(HANDLE, himgULHot);
	};
}; // colibrichat

//-----------------------------------------------------------------------------

#endif // _COLIBRICHAT_CLIENT_