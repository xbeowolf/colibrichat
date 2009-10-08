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

// Windows
#include <Mmsystem.h>

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
#define RF_SOUNDS              RF_CLIENT TEXT("Sounds\\")

// Server
#define RK_NICK                TEXT("Nickname")
#define RK_HOST                TEXT("Host")
#define RK_PORT                TEXT("Port")
#define RK_PASSWORD            TEXT("Password")
#define RK_STATE               TEXT("ConnectionState")
#define RK_STATUS              TEXT("Status")
#define RK_STATUSIMG           TEXT("StatusImage")
#define RK_STATUSMSG           TEXT("StatusMessage")

// Interface
#define RK_SENDBYENTER         TEXT("SendByEnter")
#define RK_FLASHPAGENEW        TEXT("FlashPageNew")
#define RK_FLASHPAGESAYPRIVATE TEXT("FlashPageSayPrivate")
#define RK_FLASHPAGESAYCHANNEL TEXT("FlashPageSayChannel")
#define RK_FLASHPAGETOPIC      TEXT("FlashPageChangeTopic")

// Sounds
#define RK_WAVMELINE           TEXT("MeLine")
#define RK_WAVCHATLINE         TEXT("ChatLine")
#define RK_WAVCONFIRM          TEXT("Confirm")
#define RK_WAVPRIVATELINE      TEXT("PrivateLine")
#define RK_WAVTOPIC            TEXT("Topic")
#define RK_WAVJOIN             TEXT("Join")
#define RK_WAVPART             TEXT("Part")
#define RK_WAVPRIVATE          TEXT("Private")
#define RK_WAVALERT            TEXT("Alert")
#define RK_WAVMESSAGE          TEXT("Message")
#define RK_WAVBEEP             TEXT("Beep")
#define RK_WAVCLIPBOARD        TEXT("Clipboard")

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
#define IML_PRIVATEYELLOW      8
#define IML_PRIVATERED         9
#define IML_SERVERGREEN        10
#define IML_SERVERBLUE         11
#define IML_SERVERYELLOW       12
#define IML_SERVERRED          13
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
	enum EAlert {eGreen, eBlue, eYellow, eRed};

#pragma pack(push, 1)

	struct Alert {
		bool fFlashPageNew, fFlashPageSayPrivate, fFlahPageSayChannel, fFlashPageChangeTopic;
		bool fCanOpenPrivate, fCanAlert, fCanMessage, fCanSignal, fCanRecvClipboard;
		bool fPlayChatSounds, fPlayPrivateSounds, fPlayAlert, fPlayMessage, fPlayBeep, fPlayClipboard;
	};
	typedef std::map<EUserStatus, Alert> MapAlert;

#pragma pack(pop)

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

		class JTopic;
		class JSplashRtfEditor;
		class JSplash;
		class JSplashRtf;
		class JMessageEditor;
		class JMessage;

		//
		// Pages
		//

		class JPage : public JAttachedDialog<JClient>
		{
		public:

			static bool CALLBACK Write(HWND hwnd, const TCHAR* str);

			CALLBACK JPage();

			virtual int CALLBACK ImageIndex() const = 0;
			virtual LPCTSTR CALLBACK Template() const = 0;
			virtual bool CALLBACK IsPermanent() const = 0;
			virtual std::tstring gettopic() const {return getname();}
			virtual HWND getDefFocusWnd() const = 0;

			virtual EContact CALLBACK gettype() const = 0;
			virtual DWORD CALLBACK getID() const = 0;
			virtual std::tstring CALLBACK getname() const = 0;

			void CALLBACK activate();
			virtual void CALLBACK setAlert(EAlert a);

			virtual void CALLBACK Enable() = 0;
			virtual void CALLBACK Disable() = 0;

		protected:

			//LRESULT WINAPI DlgProc(HWND, UINT, WPARAM, LPARAM);

			void OnHook(JEventable* src);
			void OnUnhook(JEventable* src);

			void OnLinkConnect(SOCKET sock);
			void OnLinkDestroy(SOCKET sock);

		protected:

			JPROPERTY_R(EAlert, alert);
			JPROPERTY_R(bool, fEnabled);
		};

		class JPageLog : public JPage
		{
		public:

			CALLBACK JPageLog();

			HWND getDefFocusWnd() const {return m_hwndLog;}

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

			int CALLBACK ImageIndex() const;
			LPCTSTR CALLBACK Template() const {return MAKEINTRESOURCE(IDD_SERVER);}
			bool CALLBACK IsPermanent() const {return true;}
			HWND getDefFocusWnd() const {return m_hwndNick;}

			EContact CALLBACK gettype() const {return eServer;}
			DWORD CALLBACK getID() const {return CRC_SERVER;}
			std::tstring CALLBACK getname() const {return NAME_SERVER;}

			void CALLBACK Enable();
			void CALLBACK Disable();

		protected:

			LRESULT WINAPI DlgProc(HWND, UINT, WPARAM, LPARAM);

			void OnHook(JEventable* src);
			void OnUnhook(JEventable* src);

			void OnLinkConnecting(SOCKET sock);
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

			RECT rcHost, rcPort, rcPass, rcNick, rcStatus, rcStatusImg, rcStatusMsg;
			RECT rcStatic1, rcStatic2, rcStatic3, rcStatic4, rcStatic5, rcConnect;
		};

		class JPageList : public JPage
		{
		public:

			CALLBACK JPageList();

			int CALLBACK ImageIndex() const {return IML_CHANNELGREEN;}
			LPCTSTR CALLBACK Template() const {return MAKEINTRESOURCE(IDD_LIST);}
			bool CALLBACK IsPermanent() const {return true;}
			HWND getDefFocusWnd() const {return m_hwndChan;}

			EContact CALLBACK gettype() const {return eList;}
			DWORD CALLBACK getID() const {return CRC_LIST;}
			std::tstring CALLBACK getname() const {return NAME_LIST;}

			void CALLBACK Enable();
			void CALLBACK Disable();

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

			void OnLinkIdentify(SOCKET sock, const netengine::SetAccess& access);
			void OnTransactionProcess(SOCKET sock, WORD message, WORD trnid, io::mem is);

		protected:

			JPROPERTY_R(HWND, hwndList);
			JPROPERTY_R(HWND, hwndChan);
			JPROPERTY_R(HWND, hwndPass);
			RECT rcList, rcChan, rcPass;
			RECT rcStatic1, rcStatic2, rcJoin, rcRefresh;

			JPROPERTY_RREF_CONST(MapChannel, mChannel);
		};

		class JPageUser : public JPageLog, public rtf::Editor
		{
		public:

			friend class JClient;

			CALLBACK JPageUser(DWORD id, const std::tstring& nick);

			int CALLBACK ImageIndex() const;
			LPCTSTR CALLBACK Template() const {return MAKEINTRESOURCE(IDD_USER);}
			bool CALLBACK IsPermanent() const {return false;}
			HWND getDefFocusWnd() const {return hwndEdit;}

			EContact CALLBACK gettype() const {return eUser;}
			DWORD CALLBACK getID() const {return m_ID;}
			std::tstring CALLBACK getname() const {return m_user.name;}

			void CALLBACK Enable();
			void CALLBACK Disable();

			void CALLBACK OnSheetColor(COLORREF cr);

			void CALLBACK setuser(const User& val) {m_user = val;}
			void CALLBACK rename(DWORD idNew, const std::tstring& newname);

		protected:

			LRESULT WINAPI DlgProc(HWND, UINT, WPARAM, LPARAM);

			void OnHook(JEventable* src);
			void OnUnhook(JEventable* src);

			void OnLinkDestroy(SOCKET sock);

		protected:

			DWORD m_ID;
			JPROPERTY_RREF_CONST(User, user);

			JPROPERTY_R(HWND, hwndMsgSpin);
			JPROPERTY_R(HWND, hwndSend);
			RECT rcMsgSpin, rcSend;

			std::vector<std::string> vecMsgSpin;
		};

		class JPageChannel : public JPageLog, public rtf::Editor
		{
		public:

			friend class JClient;

			CALLBACK JPageChannel(DWORD id, const std::tstring& nick);

			int CALLBACK ImageIndex() const;
			LPCTSTR CALLBACK Template() const {return MAKEINTRESOURCE(IDD_CHANNEL);}
			bool CALLBACK IsPermanent() const {return false;}
			std::tstring gettopic() const;
			HWND getDefFocusWnd() const {return hwndEdit;}

			EContact CALLBACK gettype() const {return eChannel;}
			DWORD CALLBACK getID() const {return m_ID;}
			std::tstring CALLBACK getname() const {return m_channel.name;}

			void CALLBACK Enable();
			void CALLBACK Disable();

			void CALLBACK OnSheetColor(COLORREF cr);

			void CALLBACK setchannel(const Channel& val);
			void CALLBACK rename(DWORD idNew, const std::tstring& newname);
			bool CALLBACK replace(DWORD idOld, DWORD idNew);
			void CALLBACK redrawUser(DWORD idUser);

			void CALLBACK Join(DWORD idUser);
			void CALLBACK Part(DWORD idUser, DWORD idBy);

			int  CALLBACK indexIcon(DWORD idUser) const;
			MapUser::const_iterator getSelUser() const;

		protected:

			LRESULT WINAPI DlgProc(HWND, UINT, WPARAM, LPARAM);

			void CALLBACK BuildView();
			void CALLBACK ClearView();
			int  CALLBACK AddLine(DWORD id);
			void CALLBACK DelLine(DWORD id);

			void OnHook(JEventable* src);
			void OnUnhook(JEventable* src);

			void OnLinkDestroy(SOCKET sock);

		protected:

			DWORD m_ID;
			JPROPERTY_RREF_CONST(Channel, channel);

			JPROPERTY_R(HWND, hwndList);
			JPROPERTY_R(HWND, hwndMsgSpin);
			JPROPERTY_R(HWND, hwndSend);
			RECT rcList, rcMsgSpin, rcSend;

			std::vector<std::string> vecMsgSpin;
		};

		//
		// Dialogs
		//

		class JTopic : public JAttachedDialog<JClient>
		{
		public:

			CALLBACK JTopic(JClient* p, JPageChannel* chan);

		protected:

			LRESULT WINAPI DlgProc(HWND, UINT, WPARAM, LPARAM);

			void OnHook(JEventable* src);
			void OnUnhook(JEventable* src);

			void OnLinkEstablished(SOCKET sock);
			void OnLinkDestroy(SOCKET sock);
			void OnPageClose(DWORD id);

		protected:

			JPtr<JPageChannel> jpChannel;
		};

		class JSplashRtfEditor : public JDialog, public rtf::Editor, public JAttachment<JClient>
		{
		public:

			CALLBACK JSplashRtfEditor(JClient* p, DWORD who);

		protected:

			LRESULT WINAPI DlgProc(HWND, UINT, WPARAM, LPARAM);

			void OnHook(JEventable* src);
			void OnUnhook(JEventable* src);

			void OnLinkEstablished(SOCKET sock);
			void OnLinkDestroy(SOCKET sock);

		protected:

			DWORD idWho;
			RECT rcAutoclose, rcAutocloseSpin, rcSend, rcCancel;
		};

		class JSplash : public JAttachedDialog<JClient>
		{
		public:

			CALLBACK JSplash(JClient* p);

		protected:

			LRESULT WINAPI DlgProc(HWND, UINT, WPARAM, LPARAM);

			void OnHook(JEventable* src);
			void OnUnhook(JEventable* src);

			void OnLinkDestroy(SOCKET sock);

		protected:

			JPROPERTY_RW(bool, bCloseOnDisconnect);
			JPROPERTY_RW(DWORD, dwCanclose);
			JPROPERTY_RW(DWORD, dwAutoclose);

			JPROPERTY_RW(bool, fTransparent);
			JPROPERTY_RW(COLORREF, crSheet);

			JPROPERTY_RW(WORD, trnid);
			JPROPERTY_R(int, result);
			JPROPERTY_RREF(RECT, rcPos);

		protected:

			DWORD dwStarted;
		};

		class JSplashRtf : public JSplash
		{
		public:

			CALLBACK JSplashRtf(JClient* p, const char* text, size_t size);

		protected:

			LRESULT WINAPI DlgProc(HWND, UINT, WPARAM, LPARAM);

		protected:

			HWND hwndRtf;
			RECT rcRtf;

		private:

			std::string content;
		};

		class JMessageEditor : public JDialog, public rtf::Editor, public JAttachment<JClient>
		{
		public:

			CALLBACK JMessageEditor(JClient* p, const std::tstring& who, bool alert = false);

		protected:

			LRESULT WINAPI DlgProc(HWND, UINT, WPARAM, LPARAM);

			void OnHook(JEventable* src);
			void OnUnhook(JEventable* src);

			void OnLinkEstablished(SOCKET sock);
			void OnLinkDestroy(SOCKET sock);

		protected:

			std::tstring strWho;
			bool fAlert;
			RECT rcStatic1, rcNick, rcAlert, rcSend, rcCancel;
		};

		class JMessage : public JAttachedDialog<JClient>
		{
		public:

			CALLBACK JMessage(JClient* p);

		protected:

			LRESULT WINAPI DlgProc(HWND, UINT, WPARAM, LPARAM);

			void OnHook(JEventable* src);
			void OnUnhook(JEventable* src);

			void OnLinkEstablished(SOCKET sock);
			void OnLinkDestroy(SOCKET sock);

		protected:

			JPROPERTY_RW(DWORD, idWho);
			JPROPERTY_RW(std::string, content);

			JPROPERTY_RW(bool, bCloseOnDisconnect);
			JPROPERTY_RW(bool, fAlert);
			JPROPERTY_RW(COLORREF, crSheet);
			JPROPERTY_RW(FILETIME, ftRecv);

			RECT rcText, rcTime, rcReply, rcPrivate, rcCancel;
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
		int  CALLBACK ContactAdd(const std::tstring& name, DWORD id, EContact type);
		void CALLBACK ContactDel(DWORD id);
		void CALLBACK ContactSel(int index);
		void CALLBACK ContactRename(DWORD idOld, DWORD idNew, const std::tstring& newname);
		int  CALLBACK getTabIndex(DWORD id);
		JPtr<JPage> CALLBACK getPage(DWORD id);
		bool CheckNick(std::tstring& nick, std::tstring& msg);
		void CALLBACK ShowTopic(const std::tstring& topic);

		// Error provider
		void DisplayMessage(HWND hwnd, const std::tstring& msg);
		void CALLBACK PlaySound(const TCHAR* snd);

		// Users managment
		std::tstring getSafeName(DWORD idUser) const;
		void CALLBACK InsertUser(DWORD idUser, const User& user);
		void CALLBACK LinkUser(DWORD idUser, DWORD idLink);
		void CALLBACK UnlinkUser(DWORD idUser, DWORD idLink);

	protected:

		// Beowolf Network Protocol Messages reciving
		void CALLBACK Recv_Notify_NICK(SOCKET sock, WORD trnid, io::mem& is);
		void CALLBACK Recv_Reply_JOIN(SOCKET sock, WORD trnid, io::mem& is);
		void CALLBACK Recv_Notify_JOIN(SOCKET sock, WORD trnid, io::mem& is);
		void CALLBACK Recv_Notify_PART(SOCKET sock, WORD trnid, io::mem& is);
		void CALLBACK Recv_Reply_USERINFO(SOCKET sock, WORD trnid, io::mem& is);
		void CALLBACK Recv_Notify_ONLINE(SOCKET sock, WORD trnid, io::mem& is);
		void CALLBACK Recv_Notify_STATUS(SOCKET sock, WORD trnid, io::mem& is);
		void CALLBACK Recv_Notify_SAY(SOCKET sock, WORD trnid, io::mem& is);
		void CALLBACK Recv_Notify_TOPIC(SOCKET sock, WORD trnid, io::mem& is);
		void CALLBACK Recv_Notify_BACKGROUND(SOCKET sock, WORD trnid, io::mem& is);
		void CALLBACK Recv_Notify_ACCESS(SOCKET sock, WORD trnid, io::mem& is);
		void CALLBACK Recv_Reply_MESSAGE(SOCKET sock, WORD trnid, io::mem& is);
		void CALLBACK Recv_Notify_MESSAGE(SOCKET sock, WORD trnid, io::mem& is);
		void CALLBACK Recv_Notify_BEEP(SOCKET sock, WORD trnid, io::mem& is);
		void CALLBACK Recv_Notify_CLIPBOARD(SOCKET sock, WORD trnid, io::mem& is);
		void CALLBACK Recv_Notify_SPLASHRTF(SOCKET sock, WORD trnid, io::mem& is);

		// Beowolf Network Protocol Messages sending
		void CALLBACK Send_Cmd_NICK(SOCKET sock, const std::tstring& nick);
		void CALLBACK Send_Quest_JOIN(SOCKET sock, const std::tstring& name, const std::tstring& pass = TEXT(""));
		void CALLBACK Send_Cmd_PART(SOCKET sock, DWORD idWho, DWORD idWhere);
		void CALLBACK Send_Quest_USERINFO(SOCKET sock, const SetId& set);
		void CALLBACK Send_Cmd_ONLINE(SOCKET sock, bool on, DWORD id);
		void CALLBACK Send_Cmd_STATUS_Mode(SOCKET sock, EUserStatus stat);
		void CALLBACK Send_Cmd_STATUS_Img(SOCKET sock, int img);
		void CALLBACK Send_Cmd_STATUS_Msg(SOCKET sock, const std::tstring& msg);
		void CALLBACK Send_Cmd_STATUS(SOCKET sock, EUserStatus stat, int img, const std::tstring& msg);
		void CALLBACK Send_Cmd_SAY(SOCKET sock, DWORD idWhere, UINT type, const std::string& content);
		void CALLBACK Send_Cmd_TOPIC(SOCKET sock, DWORD idWhere, const std::tstring& topic);
		void CALLBACK Send_Cmd_BACKGROUND(SOCKET sock, DWORD idWhere, COLORREF cr);
		void CALLBACK Send_Cmd_ACCESS(SOCKET sock, DWORD idWho, DWORD idWhere, EChanStatus stat);
		void CALLBACK Send_Quest_MESSAGE(SOCKET sock, DWORD idWho, const std::string& text, bool fAlert, COLORREF crSheet);
		void CALLBACK Send_Cmd_BEEP(SOCKET sock, DWORD idWho);
		void CALLBACK Send_Cmd_CLIPBOARD(SOCKET sock, DWORD idWho);
		void CALLBACK Send_Cmd_SPLASHRTF(SOCKET sock, DWORD idWho, const std::string& text,
			const RECT& rcPos, bool bCloseOnDisconnect = true, DWORD dwCanclose = 2500, DWORD dwAutoclose = 30000,
			bool fTransparent = true, COLORREF crSheet = RGB(255, 255, 255));

		void OnHook(JEventable* src);
		void OnUnhook(JEventable* src);

		void OnLinkConnect(SOCKET sock);
		void OnLinkClose(SOCKET sock);
		void OnLinkDestroy(SOCKET sock);
		void OnLinkIdentify(SOCKET sock, const netengine::SetAccess& access);
		void OnTransactionProcess(SOCKET sock, WORD message, WORD trnid, io::mem is);

	public:

		static std::map<EChanStatus, std::tstring> s_mapChanStatName;
		static Alert s_mapAlert[];

		// --- Events ---

		fastdelegate::FastDelegateList1<DWORD>
			EvPageClose;
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
		JPROPERTY_RREF_CONST(MapAlert, mAlert);

		// Pages
		JPtr<JPage> jpOnline;
		JPtr<JPageServer> jpPageServer;
		JPtr<JPageList> jpPageList;
		MapPageUser mPageUser;
		MapPageChannel mPageChannel;

		// HWND tabs
		JPROPERTY_R(HWND, hwndTab);
		RECT rcTab;

		// Sounds ids
		JPROPERTY_RREF_CONST(std::set<MCIDEVICEID>, wDeviceID);

		JPROPERTY_RREF_CONST(Metrics, metrics);
	};

	class JClientApp : public JApplication // Singleton
	{
	public:

		static void CALLBACK s_Init();
		static void CALLBACK s_Done();

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
		JPROPERTY_R(HACCEL, haccelRichEdit);

		JPROPERTY_R(HMENU, hmenuTab);
		JPROPERTY_R(HMENU, hmenuLog);
		JPROPERTY_R(HMENU, hmenuChannel);
		JPROPERTY_R(HMENU, hmenuRichEdit);
		JPROPERTY_R(HMENU, hmenuUser);

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

		// Waves
		JPROPERTY_R(std::tstring, strWavMeline);
		JPROPERTY_R(std::tstring, strWavChatline);
		JPROPERTY_R(std::tstring, strWavConfirm);
		JPROPERTY_R(std::tstring, strWavPrivateline);
		JPROPERTY_R(std::tstring, strWavTopic);
		JPROPERTY_R(std::tstring, strWavJoin);
		JPROPERTY_R(std::tstring, strWavPart);
		JPROPERTY_R(std::tstring, strWavPrivate);
		JPROPERTY_R(std::tstring, strWavAlert);
		JPROPERTY_R(std::tstring, strWavMessage);
		JPROPERTY_R(std::tstring, strWavBeep);
		JPROPERTY_R(std::tstring, strWavClipboard);
	};
}; // colibrichat

//-----------------------------------------------------------------------------

#endif // _COLIBRICHAT_CLIENT_