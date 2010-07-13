/******************************************************************************
*                                                                             *
* ColibriCommon.h -- Beowolf Network Protocol Engine explore sample           *
*                                                                             *
* Copyright (c) Podobashev Dmitry / BEOWOLF, 2009. All rights reserved.       *
*                                                                             *
******************************************************************************/

#ifndef _COLIBRICHAT_COMMON_
#define _COLIBRICHAT_COMMON_

//-----------------------------------------------------------------------------

//
// Includes
//

#pragma region Includes

// Common
#include "bnb.h"

#pragma endregion

//-----------------------------------------------------------------------------

#define SZ_BADTRN              TEXT("bad transaction data format")

// --- Windows messages ---

#define BEM_NOTIFYICON         (WM_USER + 100)
#define BEM_NETWORK            (WM_USER + 101)
#define BEM_ADJUSTSIZE         (WM_USER + 102)
#define BEM_JDIALOG            (WM_USER + 103)

//-----------------------------------------------------------------------------

namespace colibrichat
{
	using namespace netengine;

	struct Contact;
	struct User;
	struct Channel;

	enum EContact {eCheat = 0x01U, eServer = 0x02U, eList = 0x04U, eUser = 0x08U, eChannel = 0x10U, eBoard = 0x20U};
	enum EChanStatus {eOutsider, eReader, eWriter, eMember, eModerator, eAdmin, eFounder};
	enum EUserStatus {eReady, eDND, eBusy, eNA, eAway, eInvisible};
	enum EOnline {eOffline, eOnline, eTyping};

	typedef std::set<DWORD> SetId;

#pragma pack(push, 1)

	struct Alert {
		bool fFlashPageNew : 1, fFlashPageSayPrivate : 1, fFlahPageSayChannel : 1, fFlashPageChangeTopic : 1;
		bool fCanOpenPrivate : 1, fCanAlert : 1, fCanMessage : 1, fCanSplash : 1, fCanSignal : 1, fCanRecvClipboard : 1;
		bool fPlayChatSounds : 1, fPlayPrivateSounds : 1, fPlayAlert : 1, fPlayMessage : 1, fPlayBeep : 1, fPlayClipboard : 1;
	};
	typedef std::map<EUserStatus, Alert> MapAlert;

#pragma pack(pop)

	struct Contact
	{
		std::tstring name; // unical contact name
		std::tstring password; // password for join into channel
		SetId opened; // identifiers of opened contacts
		FILETIME ftCreation; // contact creation time

		bool CALLBACK isOpened(DWORD id) const;
	};
	typedef std::map<DWORD, Contact> MapContact;

	struct User : Contact
	{
		in_addr IP; // IP-address of user
		EOnline isOnline; // indicate application window activity
		DWORD idOnline; // identifier of selected contact
		EUserStatus nStatus;
		Alert accessibility;
		int nStatusImg;
		std::tstring strStatus;
		struct {
			bool isGod : 1, isDevil : 1;
		} cheat;

		CALLBACK User();
	};
	typedef std::map<DWORD, User> MapUser;

	struct Channel : Contact
	{
		std::tstring topic; // channel topic
		DWORD idTopicWriter;
		SetId reader, writer, member, moderator, admin, founder; // users identifiers with access rights
		EChanStatus nAutoStatus; // default access right for incomer
		UINT nLimit; // maximum users on channel
		bool isHidden, isAnonymous;
		COLORREF crBackground; // color of editor, log and list sheet

		EChanStatus CALLBACK getStatus(DWORD idUser) const;
		void CALLBACK setStatus(DWORD idUser, EChanStatus val);
		bool isPrivate() const {return nAutoStatus == eOutsider;}
	};
	typedef std::map<DWORD, Channel> MapChannel;

	struct Metrics
	{
		size_t uNameMaxLength;
		size_t uPassMaxLength;
		size_t uStatusMsgMaxLength;
		size_t uTopicMaxLength;
		size_t nMsgSpinMaxCount;
		size_t uChatLineMaxVolume;
		struct {
			bool bTransmitClipboard : 1;
		} flags;
	};

	struct Personal
	{
		enum EGender {eMale, eFemale};

		std::tstring name;
		// personal
		EGender gender;
		SYSTEMTIME birthDay;
		// contacts
		UINT icq;
		std::tstring email;
		std::tstring phone;
		std::tstring about;
	};
}; // colibrichat

//-----------------------------------------------------------------------------

namespace io
{
	void CALLBACK pack(std::ostream& os, const colibrichat::SetId& data);
	void CALLBACK pack(std::ostream& os, const colibrichat::User& data);
	void CALLBACK pack(std::ostream& os, const colibrichat::Channel& data);
	void CALLBACK unpack(mem& is, colibrichat::SetId& data);
	void CALLBACK unpack(mem& is, colibrichat::User& data);
	void CALLBACK unpack(mem& is, colibrichat::Channel& data);
}; // io

//-----------------------------------------------------------------------------

// Time funstions
void CALLBACK FileTimeToLocalTime(const FILETIME &ft, SYSTEMTIME &st);
void CALLBACK GetSystemFileTime(FILETIME& ft);

//-----------------------------------------------------------------------------

#endif // _COLIBRICHAT_COMMON_