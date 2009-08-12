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
#include "netengine.h"

#pragma endregion

//-----------------------------------------------------------------------------

#define SZ_BADTRN              TEXT("bad transaction data format")

// --- Windows messages ---

#define BEM_NOTIFYICON         (WM_USER + 100)
#define BEM_NETWORK            (WM_USER + 101)
#define BEM_ADJUSTSIZE         (WM_USER + 102)
#define BEM_JOBJECT            (WM_USER + 103)

//-----------------------------------------------------------------------------

namespace colibrichat
{
	struct Contact;
	struct User;
	struct Channel;

	enum EContact {eServer = 0x01U, eList = 0x02U, eUser = 0x04U, eChannel = 0x08U, eBoard = 0x10U};
	enum EChanStatus {eOutsider, eReader, eWriter, eMember, eModerator, eAdmin, eFounder};
	enum EUserStatus {eReady, eDND, eBusy, eNA, eAway, eInvisible};
	enum EObjType {eObjCommon, eDialogRtf, eDialogImg};

	typedef std::set<DWORD> SetId;

	struct Contact
	{
		std::tstring name; // unical contact name
		SetId opened; // identifiers of opened contacts
		FILETIME ftCreation; // contact creation time
	};
	typedef std::map<DWORD, Contact> MapContact;

	struct User : Contact
	{
		in_addr IP; // IP-address of user
		bool isOnline; // true if application window is active
		DWORD idOnline; // identifier of selected contact
		EUserStatus nStatus;
		int nStatusImg;
		std::tstring strStatus;

		void CALLBACK Init();
	};
	typedef std::map<DWORD, User> MapUser;

	struct Channel : Contact
	{
		std::tstring password; // password for join into channel
		std::tstring topic; // channel topic
		DWORD idTopicWriter;
		SetId writer, member, moderator, admin, founder; // users identifiers with access rights
		EChanStatus nAutoStatus; // default access right for incomer
		UINT nLimit; // maximum users on channel
		bool isHidden, isAnonymous;

		EChanStatus CALLBACK getStatus(DWORD idUser) const;
		void CALLBACK setStatus(DWORD idUser, EChanStatus val);
	};
	typedef std::map<DWORD, Channel> MapChannel;

	struct Metrics
	{
		size_t uNickMaxLength;
		size_t uStatusMsgMaxLength;
		size_t uChanMaxLength;
		size_t uPassMaxLength;
		size_t uTopicMaxLength;
		size_t nMsgSpinMaxCount;
	};

	enum EGender {eMale, eFemale};
	struct Personal
	{
		std::tstring name;
		// personal
		EGender gender;
		UINT birthDay, birthMonth, birthYear;
		// contacts
		UINT icq;
		std::tstring email;
		std::tstring phone;
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
void FileTimeToLocalTime(const FILETIME &ft, SYSTEMTIME &st);
void CALLBACK GetSystemFileTime(FILETIME& ft);

//-----------------------------------------------------------------------------

#endif // _COLIBRICHAT_COMMON_