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

// Lua
#include "LuaGluer.h"

// Common
#include "patterns.h"
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
#define APPVER                 TEXT("1.2")

//
// Register
//

// Folders
#define RF_CLIENT              TEXT("Client\\")
#define RF_AUTOOPEN            RF_CLIENT TEXT("Autoopen\\")
#define RF_SOUNDS              RF_CLIENT TEXT("Sounds\\")

// NetEngine
#define RK_COMPRESSION         TEXT("CompressionLevel")
#define RK_USEENCODING         TEXT("UseEncoding")

// Server
#define RK_NICK                TEXT("Nickname")
#define RK_HOST                TEXT("Host")
#define RK_PORT                TEXT("Port")
#define RK_PASSWORDNET         TEXT("PasswordNetEngine")
#define RK_STATE               TEXT("ConnectionState")
#define RK_STATUS              TEXT("Status")
#define RK_STATUSIMG           TEXT("StatusImage")
#define RK_STATUSMSG           TEXT("StatusMessage")
#define RK_TIMERCONNECT        TEXT("TimerConnect")

// Editor
#define RK_SENDBYENTER         TEXT("SendByEnter")
#define RK_CHEATANONYMOUS      TEXT("CheatAnonymous")
#define RK_QUOTATIONBLUE       TEXT("QuotationBlue")
#define RK_QUOTATIONRED        TEXT("QuotationRed")

// Interface
#define RK_FLASHPAGENEW        TEXT("FlashPageNew")
#define RK_FLASHPAGESAYPRIVATE TEXT("FlashPageSayPrivate")
#define RK_FLASHPAGESAYCHANNEL TEXT("FlashPageSayChannel")
#define RK_FLASHPAGETOPIC      TEXT("FlashPageChangeTopic")

// Autoopen
#define RK_USEAUTOOPEN         TEXT("UseAutoopen")
#define RK_CHANCOUNT           TEXT("ContactsCount")

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
#define IML_MANVOID            12
#define IML_MANGOD             13
#define IML_MANDEVIL           14
#define IML_MAN_COUNT          15
// Status images
#define IML_STATUSIMG_COUNT    38

// Timers
#define IDT_CONNECT                    100
#define IDT_BALOONPOP                  101
#define TIMER_CONNECT                  (30*1000)
#define TIMER_BALOONPOP                (GetDoubleClickTime() * 10)

//-----------------------------------------------------------------------------

namespace colibrichat
{
	enum ETimeFormat {etimeNone, etimeHHMM, etimeHHMMSS};
	enum EAlert {eGreen, eBlue, eYellow, eRed};

	class JClient : public JEngine, public JDialog, protected initdoneable<JClient>, protected CLuaGluer<JClient>
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
		class JPageChat;
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
			virtual std::tstring getSafeName(DWORD idUser) const;

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

			void OnLinkStart(SOCKET sock);
			void OnLinkClose(SOCKET sock, UINT err);

		protected:

			JPROPERTY_R(EAlert, alert);
			JPROPERTY_R(bool, fEnabled);
		};

		class JPageLog : public JPage
		{
		public:

			CALLBACK JPageLog();

			HWND getDefFocusWnd() const {return m_hwndLog;}

			void CALLBACK AppendRtf(std::string& content, bool toascii = false) const;
			void CALLBACK AppendScript(const std::tstring& content, bool withtime = true) const;
			virtual void CALLBACK Say(DWORD idWho, std::string& content);

		protected:

			LRESULT WINAPI DlgProc(HWND, UINT, WPARAM, LPARAM);

		protected:

			JPROPERTY_RREF_CONST(std::set<EGroup>, Groups);
			JPROPERTY_R(EPriority, Priority);
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

			void OnLog(const std::tstring& str, bool withtime = true);
			void OnReport(const std::tstring& str, EGroup gr = eMessage, EPriority prior = eNormal);
			void OnMetrics(const Metrics& metrics);

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

			MapChannel::const_iterator getSelChannel() const;

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

			void OnLinkStart(SOCKET sock);
			void OnTransactionProcess(SOCKET sock, WORD message, WORD trnid, io::mem is);
			void OnMetrics(const Metrics& metrics);
			void OnTopic(DWORD idWho, DWORD idWhere, const std::tstring& topic);
			void OnNick(DWORD idOld, const std::tstring& oldname, DWORD idNew, const std::tstring& newname);

		protected:

			JPROPERTY_R(HWND, hwndList);
			JPROPERTY_R(HWND, hwndChan);
			JPROPERTY_R(HWND, hwndPass);
			RECT rcList, rcChan, rcPass;
			RECT rcStatic1, rcStatic2, rcJoin, rcRefresh;

			JPROPERTY_RREF_CONST(MapChannel, mChannel);
		};

		class JPageChat : public JPageLog, public rtf::Editor, protected initdoneable<JPageChat>
		{
		public:

			friend class JClient;

			static void initclass();
			static void doneclass();
			CALLBACK JPageChat(DWORD id);

			bool CALLBACK IsPermanent() const {return false;}
			HWND getDefFocusWnd() const {return m_hwndEdit;}

			DWORD CALLBACK getID() const {return m_ID;}

			void CALLBACK Say(DWORD idWho, std::string& content);
			virtual bool CALLBACK CanSend() const {return true;}

		protected:

			LRESULT WINAPI DlgProc(HWND, UINT, WPARAM, LPARAM);

		protected:

			DWORD m_ID;

			JPROPERTY_R(HWND, hwndMsgSpinBlue);
			JPROPERTY_R(HWND, hwndMsgSpinRed);
			JPROPERTY_R(HWND, hwndSend);
			RECT rcMsgSpinBlue, rcMsgSpinRed, rcSend;

			std::vector<std::string> vecMsgSpinBlue, vecMsgSpinRed;

			static std::map<UINT, const TCHAR*> s_mapButTips;
		};

		class JPageUser : public JPageChat
		{
		public:

			friend class JClient;

			CALLBACK JPageUser(DWORD id, const std::tstring& nick);

			int CALLBACK ImageIndex() const;
			LPCTSTR CALLBACK Template() const {return MAKEINTRESOURCE(IDD_USER);}

			EContact CALLBACK gettype() const {return eUser;}
			std::tstring CALLBACK getname() const {return m_user.name;}

			void CALLBACK Enable();
			void CALLBACK Disable();

			void CALLBACK OnSheetColor(COLORREF cr);
			bool CALLBACK CanSend() const {return true;}

			void CALLBACK setuser(const User& val) {m_user = val;}
			void CALLBACK rename(DWORD idNew, const std::tstring& newname);

		protected:

			LRESULT WINAPI DlgProc(HWND, UINT, WPARAM, LPARAM);

			void OnHook(JEventable* src);
			void OnUnhook(JEventable* src);

			void OnLinkStart(SOCKET sock);
			void OnLinkClose(SOCKET sock, UINT err);

		protected:

			JPROPERTY_RREF_CONST(User, user);
		};

		class JPageChannel : public JPageChat
		{
		public:

			friend class JClient;

			CALLBACK JPageChannel(DWORD id, const std::tstring& nick);

			int CALLBACK ImageIndex() const;
			LPCTSTR CALLBACK Template() const {return MAKEINTRESOURCE(IDD_CHANNEL);}
			std::tstring getSafeName(DWORD idUser) const;
			std::tstring gettopic() const;

			EContact CALLBACK gettype() const {return eChannel;}
			std::tstring CALLBACK getname() const {return m_channel.name;}

			void CALLBACK Enable();
			void CALLBACK Disable();

			void CALLBACK DisplayMessage(DWORD idUser, const TCHAR* msg, HICON hicon = 0, COLORREF cr = RGB(0x00, 0x00, 0x00));

			void CALLBACK OnSheetColor(COLORREF cr);
			bool CALLBACK CanSend() const;

			void CALLBACK setchannel(const Channel& val);
			void CALLBACK rename(DWORD idNew, const std::tstring& newname);
			bool CALLBACK replace(DWORD idOld, DWORD idNew);
			void CALLBACK redrawUser(DWORD idUser);

			void CALLBACK Join(DWORD idWho);
			void CALLBACK Part(DWORD idWho, DWORD idBy);

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

			void OnLinkStart(SOCKET sock);
			void OnLinkClose(SOCKET sock, UINT err);
			void OnNick(DWORD idOld, const std::tstring& oldname, DWORD idNew, const std::tstring& newname);
			void OnTopic(DWORD idWho, DWORD idWhere, const std::tstring& topic);

		protected:

			JPROPERTY_RREF_CONST(Channel, channel);

			JPROPERTY_R(HWND, hwndList);
			RECT rcList;
		};

		//
		// Dialogs
		//

		class JTopic : public JAttachedDialog<JClient>
		{
		public:

			CALLBACK JTopic(JClient* p, DWORD id, const std::tstring& n, const std::tstring& t);

		protected:

			LRESULT WINAPI DlgProc(HWND, UINT, WPARAM, LPARAM);

			void OnHook(JEventable* src);
			void OnUnhook(JEventable* src);

			void OnLinkEstablished(SOCKET sock);
			void OnLinkClose(SOCKET sock, UINT err);
			void OnPageClose(DWORD id);
			void OnMetrics(const Metrics& metrics);

		protected:

			JPROPERTY_R(DWORD, idChannel);
			JPROPERTY_RREF_CONST(std::tstring, name);
			JPROPERTY_RREF_CONST(std::tstring, topic);
		};

		class JSplashRtfEditor : public JDialog, public rtf::Editor, public JAttachment<JClient>, protected initdoneable<JSplashRtfEditor>
		{
		public:

			static void initclass();
			static void doneclass();
			CALLBACK JSplashRtfEditor(JClient* p, DWORD who);

		protected:

			LRESULT WINAPI DlgProc(HWND, UINT, WPARAM, LPARAM);

			void OnHook(JEventable* src);
			void OnUnhook(JEventable* src);

			void OnLinkEstablished(SOCKET sock);
			void OnLinkClose(SOCKET sock, UINT err);

		protected:

			DWORD idWho;
			RECT rcAutoclose, rcAutocloseSpin, rcSend, rcCancel;

			static std::map<UINT, const TCHAR*> s_mapButTips;
		};

		class JSplash : public JAttachedDialog<JClient>
		{
		public:

			CALLBACK JSplash(JClient* p);

		protected:

			LRESULT WINAPI DlgProc(HWND, UINT, WPARAM, LPARAM);

			void OnHook(JEventable* src);
			void OnUnhook(JEventable* src);

			void OnLinkClose(SOCKET sock, UINT err);

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

		class JMessageEditor : public JDialog, public rtf::Editor, public JAttachment<JClient>, protected initdoneable<JMessageEditor>
		{
		public:

			static void initclass();
			static void doneclass();
			CALLBACK JMessageEditor(JClient* p, const std::tstring& who, bool alert = false);

		protected:

			LRESULT WINAPI DlgProc(HWND, UINT, WPARAM, LPARAM);

			void OnHook(JEventable* src);
			void OnUnhook(JEventable* src);

			void OnLinkEstablished(SOCKET sock);
			void OnLinkClose(SOCKET sock, UINT err);
			void OnMetrics(const Metrics& metrics);

		protected:

			std::tstring strWho;
			bool fAlert;
			RECT rcStatic1, rcNick, rcAlert, rcSend, rcCancel;

			static std::map<UINT, const TCHAR*> s_mapButTips;
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
			void OnLinkClose(SOCKET sock, UINT err);

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
		static void initclass();
		static void doneclass();
		static const char className[];
		static CLuaGluer<JClient>::_tRegType methods[];
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

		// Autoopen
		void CALLBACK saveAutoopen() const;
		void CALLBACK openAutoopen();
		bool m_fAutoopen;

		// Contacts
		int  CALLBACK ContactAdd(const std::tstring& name, DWORD id, EContact type);
		void CALLBACK ContactDel(DWORD id);
		void CALLBACK ContactSel(int index);
		void CALLBACK ContactRename(DWORD idOld, const std::tstring& oldname, DWORD idNew, const std::tstring& newname);
		int  CALLBACK getTabIndex(DWORD id);
		JPtr<JPage> CALLBACK getPage(DWORD id);
		JPtr<JPageLog> CALLBACK getPageLog(DWORD id);
		static bool CALLBACK CheckNick(std::tstring& nick, const TCHAR*& msg);
		void CALLBACK ShowTopic(const std::tstring& topic);

		// Error provider
		void CALLBACK DisplayMessage(HWND hwndId, const TCHAR* msg, const TCHAR* title, int icon = 0, COLORREF cr = RGB(0x00, 0x00, 0x00));
		void CALLBACK ShowBaloon(const POINT& p, HWND hwndId, const TCHAR* msg, const TCHAR* title = 0, HICON hicon = 0, COLORREF cr = RGB(0x00, 0x00, 0x00));
		void CALLBACK HideBaloon(HWND hwnd = 0);
		void CALLBACK PlaySound(const TCHAR* snd);

		// Users managment
		std::tstring getSafeName(DWORD idUser) const;
		bool CALLBACK isGod(DWORD idUser = CRC_NONAME) const;
		bool CALLBACK isDevil(DWORD idUser = CRC_NONAME) const;
		bool CALLBACK isCheats(DWORD idUser = CRC_NONAME) const;
		void CALLBACK InsertUser(DWORD idUser, const User& user);
		void CALLBACK LinkUser(DWORD idUser, DWORD idLink);
		void CALLBACK UnlinkUser(DWORD idUser, DWORD idLink);

	protected:

		// Beowolf Network Protocol Messages reciving
		void CALLBACK Recv_Notify_METRICS(SOCKET sock, WORD trnid, io::mem& is);
		void CALLBACK Recv_Notify_NICK(SOCKET sock, WORD trnid, io::mem& is);
		void CALLBACK Recv_Reply_JOIN(SOCKET sock, WORD trnid, io::mem& is);
		void CALLBACK Recv_Notify_JOIN(SOCKET sock, WORD trnid, io::mem& is);
		void CALLBACK Recv_Notify_PART(SOCKET sock, WORD trnid, io::mem& is);
		void CALLBACK Recv_Reply_USERINFO(SOCKET sock, WORD trnid, io::mem& is);
		void CALLBACK Recv_Notify_ONLINE(SOCKET sock, WORD trnid, io::mem& is);
		void CALLBACK Recv_Notify_STATUS(SOCKET sock, WORD trnid, io::mem& is);
		void CALLBACK Recv_Notify_SAY(SOCKET sock, WORD trnid, io::mem& is);
		void CALLBACK Recv_Notify_TOPIC(SOCKET sock, WORD trnid, io::mem& is);
		void CALLBACK Recv_Notify_CHANOPTIONS(SOCKET sock, WORD trnid, io::mem& is);
		void CALLBACK Recv_Notify_ACCESS(SOCKET sock, WORD trnid, io::mem& is);
		void CALLBACK Recv_Reply_MESSAGE(SOCKET sock, WORD trnid, io::mem& is);
		void CALLBACK Recv_Notify_MESSAGE(SOCKET sock, WORD trnid, io::mem& is);
		void CALLBACK Recv_Notify_BEEP(SOCKET sock, WORD trnid, io::mem& is);
		void CALLBACK Recv_Notify_CLIPBOARD(SOCKET sock, WORD trnid, io::mem& is);
		void CALLBACK Recv_Notify_SPLASHRTF(SOCKET sock, WORD trnid, io::mem& is);

		// Beowolf Network Protocol Messages sending
		void CALLBACK Send_Cmd_NICK(SOCKET sock, DWORD idWho, const std::tstring& nick);
		void CALLBACK Send_Quest_JOIN(SOCKET sock, const std::tstring& name, const std::tstring& pass = TEXT(""), int type = eCheat | eUser | eChannel | eBoard);
		void CALLBACK Send_Cmd_PART(SOCKET sock, DWORD idWho, DWORD idWhere);
		void CALLBACK Send_Quest_USERINFO(SOCKET sock, const SetId& set);
		void CALLBACK Send_Cmd_ONLINE(SOCKET sock, EOnline online, DWORD id);
		void CALLBACK Send_Cmd_STATUS_Mode(SOCKET sock, EUserStatus stat, const Alert& a);
		void CALLBACK Send_Cmd_STATUS_Img(SOCKET sock, int img);
		void CALLBACK Send_Cmd_STATUS_Msg(SOCKET sock, const std::tstring& msg);
		void CALLBACK Send_Cmd_STATUS(SOCKET sock, EUserStatus stat, const Alert& a, int img, const std::tstring& msg);
		void CALLBACK Send_Cmd_SAY(SOCKET sock, DWORD idWhere, UINT type, const std::string& content);
		void CALLBACK Send_Cmd_TOPIC(SOCKET sock, DWORD idWhere, const std::tstring& topic);
		void CALLBACK Send_Cmd_CHANOPTIONS(SOCKET sock, DWORD idWhere, int op, DWORD val);
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
		void OnLinkClose(SOCKET sock, UINT err);
		void OnLinkFail(SOCKET sock, UINT err);
		void OnLinkStart(SOCKET sock);
		void OnTransactionProcess(SOCKET sock, WORD message, WORD trnid, io::mem is);
		void OnNick(DWORD idOld, const std::tstring& oldname, DWORD idNew, const std::tstring& newname);

		// Lua gluer
		int lua_regFuncs(lua_State *luaVM);
		int lua_getGlobal(lua_State *luaVM);
		int lua_getVars(lua_State *luaVM);
		int lua_setVars(lua_State *luaVM);
		int lua_PlaySound(lua_State *luaVM);
		int lua_ShowTopic(lua_State *luaVM);
		int lua_saveAutoopen(lua_State *luaVM);
		int lua_openAutoopen(lua_State *luaVM);
		int lua_Log(lua_State *luaVM);
		int lua_HideBaloon(lua_State *luaVM);
		int lua_Connect(lua_State *luaVM);
		int lua_Disconnect(lua_State *luaVM);
		int lua_getConnectCount(lua_State *luaVM);
		int lua_setConnectCount(lua_State *luaVM);
		int lua_getSocket(lua_State *luaVM);
		int lua_checkConnectionButton(lua_State *luaVM);
		int lua_WaitConnectStart(lua_State *luaVM);
		int lua_WaitConnectStop(lua_State *luaVM);
		int lua_MinimizeWindow(lua_State *luaVM);
		int lua_MaximizeWindow(lua_State *luaVM);
		int lua_RestoreWindow(lua_State *luaVM);
		int lua_FlashWindow(lua_State *luaVM);
		int lua_DestroyWindow(lua_State *luaVM);
		int lua_PageEnable(lua_State *luaVM);
		int lua_PageDisable(lua_State *luaVM);
		int lua_PageAppendScript(lua_State *luaVM);
		int lua_Say(lua_State *luaVM);
		int lua_Message(lua_State *luaVM);
		int lua_Alert(lua_State *luaVM);
		int lua_Beep(lua_State *luaVM);

	public:

		static std::map<EChanStatus, std::tstring> s_mapChanStatName;
		static std::map<UINT, std::tstring> s_mapWsaErr;

		// --- Events ---

		// Events on sockets
		fastdelegate::FastDelegateList1<DWORD>
			EvPageClose;
		// Events on transactions
		fastdelegate::FastDelegateList1<const Metrics&>
			EvMetrics;
		fastdelegate::FastDelegateList4<DWORD, const std::tstring&, DWORD, const std::tstring&>
			EvNick;
		fastdelegate::FastDelegateList3<DWORD, DWORD, const std::tstring&>
			EvTopic;

	protected:

		JPROPERTY_R(DWORD, idOwn);
		JPROPERTY_RREF_CONST(MapUser, mUser);

		JPROPERTY_R(SOCKET, clientsock);
		JPROPERTY_RREF_CONST(std::string, hostname);
		JPROPERTY_R(u_short, port);
		JPROPERTY_RREF_CONST(std::tstring, passwordNet);
		JPROPERTY_R(bool, bReconnect);
		JPROPERTY_R(int, nConnectCount);

		JPROPERTY_R(bool, bSendByEnter);
		JPROPERTY_R(bool, bCheatAnonymous);
		Alert s_mapAlert[6];
		JPROPERTY_RREF_CONST(MapAlert, mAlert);

		// Baloon tooltip
		JPROPERTY_R(HWND, isBaloon);
		JPROPERTY_R(HWND, hwndBaloon);

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

		// Lua managment
		JPROPERTY_R(lua_State*, luaEvents);
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
		JPROPERTY_R(HACCEL, haccelRichEdit);

		JPROPERTY_R(HMENU, hmenuTab);
		JPROPERTY_R(HMENU, hmenuLog);
		JPROPERTY_R(HMENU, hmenuChannel);
		JPROPERTY_R(HMENU, hmenuList);
		JPROPERTY_R(HMENU, hmenuRichEdit);
		JPROPERTY_R(HMENU, hmenuUser);
		JPROPERTY_R(HMENU, hmenuUserGod);

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