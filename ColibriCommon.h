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
	struct Message;

	enum EContact {eServer, eList, eUser, eChannel, eBoard};
	enum EChanStatus {eReader, eWriter, eMember, eModerator, eAdmin, eFounder};

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
	};
	typedef std::map<DWORD, User> MapUser;

	struct Channel : Contact
	{
		std::tstring password; // password for join into channel
		std::tstring topic; // channel topic
		SetId writer, member, moderator, admin; // users identifiers with access rights
		DWORD idFounder; // the founder of channel
		EChanStatus nAutoStatus; // default access right for incomer
		bool isHidden, isAnonymous;
	};
	typedef std::map<DWORD, Channel> MapChannel;

	struct Metrics
	{
		size_t uNickMaxLength;
		size_t uChanMaxLength;
		size_t uPassMaxLength;
	};

	struct Message
	{
		FILETIME time;
		DWORD idFrom, idTo;
		std::string text;
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