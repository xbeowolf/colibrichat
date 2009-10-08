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
#include "netengine.h"
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

//-----------------------------------------------------------------------------

namespace colibrichat
{
	class JServer;
	typedef std::map<SOCKET, std::tstring> MapLinkPassword;
	typedef std::map<SOCKET, DWORD> MapSocketId;
	typedef std::map<DWORD, SOCKET> MapIdSocket;

	class JServer : public netengine::JEngine, public JWindow
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

			void OnHook(JEventable* src);
			void OnUnhook(JEventable* src);

			void OnLinkEstablished(SOCKET sock);
			void OnLinkDestroy(SOCKET sock);

		protected:

			JPROPERTY_R(HWND, hwndList);
			RECT rcList;
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

		bool CALLBACK CheckAccess(const TCHAR* password, netengine::SetAccess& access) const;

		// --- CRC work ---

		bool CALLBACK hasCRC(DWORD crc) const;
		bool CALLBACK linkCRC(DWORD crc1, DWORD crc2);
		void CALLBACK unlinkCRC(DWORD crc1, DWORD crc2);

		// --- Support ---

		std::tstring CALLBACK getNearestName(const std::tstring& nick) const;
		void CALLBACK RenameContact(SOCKET sock, DWORD result, DWORD idOld, DWORD idNew, const std::tstring& newname);

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
		void CALLBACK Recv_Cmd_BACKGROUND(SOCKET sock, WORD trnid, io::mem& is);
		void CALLBACK Recv_Cmd_ACCESS(SOCKET sock, WORD trnid, io::mem& is);
		void CALLBACK Recv_Cmd_BEEP(SOCKET sock, WORD trnid, io::mem& is);
		void CALLBACK Recv_Cmd_CLIPBOARD(SOCKET sock, WORD trnid, io::mem& is);
		void CALLBACK Recv_Quest_MESSAGE(SOCKET sock, WORD trnid, io::mem& is);
		void CALLBACK Recv_Cmd_SPLASHRTF(SOCKET sock, WORD trnid, io::mem& is);

		// Beowolf Network Protocol Messages sending
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
		void CALLBACK Broadcast_Notify_ONLINE(const SetId& set, DWORD idWho, bool on, DWORD id);
		void CALLBACK Broadcast_Notify_STATUS(const SetId& set, DWORD idWho, WORD type, EUserStatus stat, int img, std::tstring msg);
		void CALLBACK Send_Notify_SAY(SOCKET sock, DWORD idWho, DWORD idWhere, UINT type, const std::string& content);
		void CALLBACK Broadcast_Notify_SAY(const SetId& set, DWORD idWho, DWORD idWhere, UINT type, const std::string& content);
		void CALLBACK Broadcast_Notify_TOPIC(const SetId& set, DWORD idWho, DWORD idWhere, const std::tstring& topic);
		void CALLBACK Send_Notify_BACKGROUND(SOCKET sock, DWORD idWho, DWORD idWhere, COLORREF cr);
		void CALLBACK Broadcast_Notify_BACKGROUND(const SetId& set, DWORD idWho, DWORD idWhere, COLORREF cr);
		void CALLBACK Broadcast_Notify_ACCESS(const SetId& set, DWORD idWho, DWORD idWhere, EChanStatus stat, DWORD idBy);
		void CALLBACK Send_Notify_BEEP(SOCKET sock, DWORD idBy);
		void CALLBACK Send_Notify_CLIPBOARD(SOCKET sock, DWORD idBy, const char* ptr, size_t size);
		void CALLBACK Send_Notify_MESSAGE(SOCKET sock, DWORD idBy, const FILETIME& ft, const char* ptr, size_t size);
		void CALLBACK Send_Reply_MESSAGE(SOCKET sock, WORD trnid, DWORD idWho, UINT type);
		void CALLBACK Send_Notify_SPLASHRTF(SOCKET sock, DWORD idBy, const char* ptr, size_t size);

		void OnHook(JEventable* src);
		void OnUnhook(JEventable* src);

		void OnLinkDestroy(SOCKET sock);
		void OnLinkEstablished(SOCKET sock);
		void OnTransactionProcess(SOCKET sock, WORD message, WORD trnid, io::mem is);

		// --- Commands ---

		void CALLBACK Connections();
		void CALLBACK About();

	public:

		// --- Events ---

		//fastdelegate::FastDelegateList1<const Message&>
			//EvMessage;

	protected:

		JPROPERTY_R(u_long, IP);
		JPROPERTY_R(u_short, port);
		JPROPERTY_R(std::tstring, password);

		// Connections dialog
		JPtr<JAttachedDialog<JServer>> jpConnections;
		// Tray icon
		JPROPERTY_R(bool, bShowIcon);

		// Chat
		JPROPERTY_RREF_CONST(MapSocketId, mSocketId);
		JPROPERTY_RREF_CONST(MapIdSocket, mIdSocket);
		JPROPERTY_RREF_CONST(MapUser, mUser);
		JPROPERTY_RREF_CONST(MapChannel, mChannel);

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
	};
}; // colibrichat

//-----------------------------------------------------------------------------

#endif // _COLIBRICHAT_SERVER_