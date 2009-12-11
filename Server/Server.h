/******************************************************************************
*                                                                             *
* Server.h -- Beowolf Network Protocol Engine explore sample                  *
*                                                                             *
* Copyright (c) Podobashev Dmitry / BEOWOLF, 2009. All rights reserved.       *
*                                                                             *
******************************************************************************/

#ifndef _COLIBRICHAT_SERVER_
#define _COLIBRICHAT_SERVER_

//-----------------------------------------------------------------------------

//
// Includes
//

#pragma region Includes

// Common
#include "app.h"

// Project
#include "..\ColibriCommon.h"

#pragma endregion

//-----------------------------------------------------------------------------

#define APPNAME                TEXT("Colibri Chat")
#define WC_MSG                 TEXT("BEOWOLF ColibriChat")
#define WT_MSG                 TEXT("ColibriChat Window")

//
// Register
//

// Folders
#define RF_SERVER              TEXT("Server\\")
#define RF_METRICS             RF_SERVER TEXT("Metrics\\")

// NetEngine
#define RK_COMPRESSION         TEXT("CompressionLevel")
#define RK_USEENCODING         TEXT("UseEncoding")

// Metrics
#define RK_NameMaxLength       TEXT("NameMaxLength")
#define RK_PassMaxLength       TEXT("PassMaxLength")
#define RK_StatusMsgMaxLength  TEXT("StatusMsgMaxLength")
#define RK_TopicMaxLength      TEXT("TopicMaxLength")
#define RK_MsgSpinMaxCount     TEXT("MsgSpinMaxCount")
#define RK_ChatLineMaxVolume   TEXT("ChatLineMaxVolume")
#define RK_TransmitClipboard   TEXT("TransmitClipboard")

// Listening
#define RK_PASSWORDNET         TEXT("PasswordNetEngine")
#define RK_PORTCOUNT           TEXT("PortCount")

// Cheats
#define RK_PASSWORDGOD         TEXT("PasswordGod")
#define RK_PASSWORDDEVIL       TEXT("PasswordDevil")

// Interface
#define RK_SHOWICON            TEXT("ShowTrayIcon")
#define RK_CANEDITNICK         TEXT("CanEditNick")
#define RK_CANMAKEGOD          TEXT("CanMakeGod")
#define RK_CANMAKEDEVIL        TEXT("CanMakeDevil")

//-----------------------------------------------------------------------------

namespace colibrichat
{
	class JServer;
	typedef std::map<SOCKET, std::tstring> MapLinkPassword;
	typedef std::map<SOCKET, DWORD> MapSocketId;
	typedef std::map<DWORD, SOCKET> MapIdSocket;

	class JServer : public JEngine, public JWindow
	{
	public:

		//
		// Dialogs
		//

		class JConnections : public JAttachedDialog<JServer>
		{
		public:

			CALLBACK JConnections();

		protected:

			LRESULT WINAPI DlgProc(HWND, UINT, WPARAM, LPARAM);

			int CALLBACK AddLine(SOCKET sock);
			void CALLBACK DelLine(SOCKET sock);
			void CALLBACK BuildView();

			MapUser::iterator getSelUser(int& index);

			void OnHook(JEventable* src);
			void OnUnhook(JEventable* src);

			void OnLinkEstablished(SOCKET sock);
			void OnLinkClose(SOCKET sock, UINT err);

		protected:

			JPROPERTY_R(HWND, hwndList);
			RECT rcList;
		};

		class JPasswords : public JAttachedDialog<JServer>
		{
		public:

			CALLBACK JPasswords();

		protected:

			LRESULT WINAPI DlgProc(HWND, UINT, WPARAM, LPARAM);

			void OnMetrics(const Metrics& metrics);
		};

	public:

		// Constructor
		CALLBACK JServer();
		DWORD CALLBACK getMinVersion() const {return BNP_ENGINEVERSMIN;}
		DWORD CALLBACK getCurVersion() const {return BNP_ENGINEVERSNUM;}

		void CALLBACK Init();
		void CALLBACK Done();

		void CALLBACK JobQuantum() {}

		// Create or destroy message window
		HWND CALLBACK CreateMsgWindow();
		BOOL CALLBACK DestroyMsgWindow();

	protected:

		void CALLBACK LoadState(); // can be only one call for object
		void CALLBACK SaveState(); // can be multiply calls for object
		void CALLBACK InitLogs() {}

		LRESULT WINAPI DlgProc(HWND, UINT, WPARAM, LPARAM);

		bool CALLBACK CheckAccess(const TCHAR* password, SetAccess& access) const;

		// --- CRC work ---

		bool CALLBACK hasCRC(DWORD crc) const;
		bool CALLBACK linkCRC(DWORD crc1, DWORD crc2);
		void CALLBACK unlinkCRC(DWORD crc1, DWORD crc2);

		// --- Support ---

		static bool CALLBACK CheckNick(std::tstring& nick, const TCHAR*& msg);
		std::tstring CALLBACK getNearestName(const std::tstring& nick) const;
		void CALLBACK RenameContact(DWORD idByOrSock, DWORD idOld, std::tstring newname);
		bool CALLBACK isGod(DWORD idUser) const;
		bool CALLBACK isDevil(DWORD idUser) const;
		bool CALLBACK isCheats(DWORD idUser) const;

	protected:

		int  CALLBACK BroadcastTrn(const SetId& set, bool nested, WORD message, const std::string& str, size_t ssi = 0) throw();

		// Beowolf Network Protocol Messages reciving
		void CALLBACK Recv_Cmd_NICK(SOCKET sock, WORD trnid, io::mem& is);
		void CALLBACK Recv_Quest_LIST(SOCKET sock, WORD trnid, io::mem& is);
		void CALLBACK Recv_Quest_JOIN(SOCKET sock, WORD trnid, io::mem& is);
		void CALLBACK Recv_Cmd_PART(SOCKET sock, WORD trnid, io::mem& is);
		void CALLBACK Recv_Quest_USERINFO(SOCKET sock, WORD trnid, io::mem& is);
		void CALLBACK Recv_Cmd_ONLINE(SOCKET sock, WORD trnid, io::mem& is);
		void CALLBACK Recv_Cmd_STATUS(SOCKET sock, WORD trnid, io::mem& is);
		void CALLBACK Recv_Cmd_SAY(SOCKET sock, WORD trnid, io::mem& is);
		void CALLBACK Recv_Cmd_TOPIC(SOCKET sock, WORD trnid, io::mem& is);
		void CALLBACK Recv_Cmd_CHANOPTIONS(SOCKET sock, WORD trnid, io::mem& is);
		void CALLBACK Recv_Cmd_ACCESS(SOCKET sock, WORD trnid, io::mem& is);
		void CALLBACK Recv_Cmd_BEEP(SOCKET sock, WORD trnid, io::mem& is);
		void CALLBACK Recv_Cmd_CLIPBOARD(SOCKET sock, WORD trnid, io::mem& is);
		void CALLBACK Recv_Quest_MESSAGE(SOCKET sock, WORD trnid, io::mem& is);
		void CALLBACK Recv_Cmd_SPLASHRTF(SOCKET sock, WORD trnid, io::mem& is);

		// Beowolf Network Protocol Messages sending
		void CALLBACK Send_Notify_METRICS(SOCKET sock, const Metrics& metrics);
		void CALLBACK Broadcast_Notify_NICK(const SetId& set, DWORD result, DWORD idOld, DWORD idNew, const std::tstring& newname);
		void CALLBACK Send_Reply_LIST(SOCKET sock, WORD trnid);
		void CALLBACK Send_Reply_JOIN_Result(SOCKET sock, WORD trnid, DWORD result, EContact type, DWORD id);
		void CALLBACK Send_Reply_JOIN_User(SOCKET sock, WORD trnid, DWORD id, const User& user);
		void CALLBACK Send_Reply_JOIN_Channel(SOCKET sock, WORD trnid, DWORD id, const Channel& chan);
		void CALLBACK Send_Notify_JOIN(SOCKET sock, DWORD idWho, DWORD idWhere, const User& user);
		void CALLBACK Broadcast_Notify_JOIN(const SetId& set, DWORD idWho, DWORD idWhere, const User& user);
		void CALLBACK Send_Notify_PART(SOCKET sock, DWORD idWho, DWORD idWhere, DWORD idBy);
		void CALLBACK Broadcast_Notify_PART(const SetId& set, DWORD idWho, DWORD idWhere, DWORD idBy);
		void CALLBACK Send_Reply_USERINFO(SOCKET sock, WORD trnid, const SetId& set);
		void CALLBACK Broadcast_Notify_ONLINE(const SetId& set, DWORD idWho, EOnline online, DWORD id);
		void CALLBACK Broadcast_Notify_STATUS(const SetId& set, DWORD idWho, WORD type, EUserStatus stat, const Alert& a, int img, std::tstring msg);
		void CALLBACK Broadcast_Notify_STATUS_God(const SetId& set, DWORD idWho, bool god);
		void CALLBACK Broadcast_Notify_STATUS_Devil(const SetId& set, DWORD idWho, bool devil);
		void CALLBACK Send_Notify_SAY(SOCKET sock, DWORD idWho, DWORD idWhere, UINT type, const std::string& content);
		void CALLBACK Broadcast_Notify_SAY(const SetId& set, DWORD idWho, DWORD idWhere, UINT type, const std::string& content);
		void CALLBACK Broadcast_Notify_TOPIC(const SetId& set, DWORD idWho, DWORD idWhere, const std::tstring& topic);
		void CALLBACK Send_Notify_CHANOPTIONS(SOCKET sock, DWORD idWho, DWORD idWhere, int op, DWORD val);
		void CALLBACK Broadcast_Notify_CHANOPTIONS(const SetId& set, DWORD idWho, DWORD idWhere, int op, DWORD val);
		void CALLBACK Broadcast_Notify_ACCESS(const SetId& set, DWORD idWho, DWORD idWhere, EChanStatus stat, DWORD idBy);
		void CALLBACK Send_Notify_BEEP(SOCKET sock, DWORD idBy);
		void CALLBACK Send_Notify_CLIPBOARD(SOCKET sock, DWORD idBy, const char* ptr, size_t size);
		void CALLBACK Send_Notify_MESSAGE(SOCKET sock, DWORD idBy, const FILETIME& ft, const char* ptr, size_t size);
		void CALLBACK Send_Reply_MESSAGE(SOCKET sock, WORD trnid, DWORD idWho, UINT type);
		void CALLBACK Send_Notify_SPLASHRTF(SOCKET sock, DWORD idBy, const char* ptr, size_t size);

		void OnHook(JEventable* src);
		void OnUnhook(JEventable* src);

		void OnLinkClose(SOCKET sock, UINT err);
		void OnLinkEstablished(SOCKET sock);
		void OnLinkStart(SOCKET sock);
		void OnTransactionProcess(SOCKET sock, WORD message, WORD trnid, io::mem is);

		// --- Commands ---

		void CALLBACK Connections();
		void CALLBACK Passwords();
		void CALLBACK About();

	public:

		// --- Events ---

		//fastdelegate::FastDelegateList1<const Message&>
			//EvMessage;

	protected:

		JPROPERTY_R(u_short, port);
		JPROPERTY_RREF_CONST(std::tstring, passwordNet);

		// Connections dialog
		JPtr<JAttachedDialog<JServer>> jpConnections, jpPasswords;
		// Tray icon
		JPROPERTY_R(bool, bShowIcon);

		// Chat
		JPROPERTY_RREF_CONST(MapSocketId, mSocketId);
		JPROPERTY_RREF_CONST(MapIdSocket, mIdSocket);
		JPROPERTY_RREF_CONST(MapUser, mUser);
		JPROPERTY_RREF_CONST(MapChannel, mChannel);

		JPROPERTY_RREF_CONST(std::tstring, passwordGod);
		JPROPERTY_RREF_CONST(std::tstring, passwordDevil);
		JPROPERTY_RREF_CONST(Metrics, metrics);
	};

	class JServerApp : public JApplication // Singleton
	{
	public:

		virtual void CALLBACK Init();
		virtual bool CALLBACK InitInstance();
		virtual void CALLBACK Done();

	protected:

		CALLBACK JServerApp(HINSTANCE hInstance = 0, HINSTANCE hPrevInstance = 0, LPTSTR szcl = 0, int ncs = SW_SHOWDEFAULT);

	public:

		static JPtr<JServerApp> jpApp;
		JPtr<JServer> jpServer;

	protected:

		HINSTANCE hinstRichEdit;

		JPROPERTY_R(HMENU, hmenuConnections);
	};
}; // colibrichat

//-----------------------------------------------------------------------------

#endif // _COLIBRICHAT_SERVER_