
//-----------------------------------------------------------------------------

//
// Includes
//

#pragma region Includes

#include "stdafx.h"

// Windows API
#include <strsafe.h>

// Project
#include "ColibriCommon.h"

#pragma endregion

using namespace colibrichat;

//-----------------------------------------------------------------------------

void CALLBACK io::pack(std::ostream& os, const colibrichat::SetId& data)
{
	io::pack(os, data.size());
	for each (colibrichat::SetId::value_type const& v in data)
		io::pack(os, v);
}

void CALLBACK io::pack(std::ostream& os, const colibrichat::User& data)
{
	io::pack(os, data.name);
	io::pack(os, data.ftCreation);
	io::pack(os, data.IP);
	io::pack(os, data.isOnline);
	io::pack(os, data.idOnline);
	io::pack(os, data.nStatus);
	io::pack(os, data.nStatusImg);
	io::pack(os, data.strStatus);
}

void CALLBACK io::pack(std::ostream& os, const colibrichat::Channel& data)
{
	io::pack(os, data.name);
	io::pack(os, data.password);
	io::pack(os, data.opened);
	io::pack(os, data.ftCreation);
	io::pack(os, data.topic);
	io::pack(os, data.idTopicWriter);
	io::pack(os, data.writer);
	io::pack(os, data.member);
	io::pack(os, data.moderator);
	io::pack(os, data.admin);
	io::pack(os, data.founder);
	io::pack(os, data.nAutoStatus);
	io::pack(os, data.nLimit);
	io::pack(os, data.isHidden);
	io::pack(os, data.isAnonymous);
	io::pack(os, data.crBackground);
}

void CALLBACK io::unpack(io::mem& is, colibrichat::SetId& data)
{
	size_t count;
	DWORD id;

	io::unpack(is, count);
	while (count--) {
		io::unpack(is, id);
		data.insert(id);
	}
}

void CALLBACK io::unpack(io::mem& is, colibrichat::User& data)
{
	io::unpack(is, data.name);
	io::unpack(is, data.ftCreation);
	io::unpack(is, data.IP);
	io::unpack(is, data.isOnline);
	io::unpack(is, data.idOnline);
	io::unpack(is, data.nStatus);
	io::unpack(is, data.nStatusImg);
	io::unpack(is, data.strStatus);
}

void CALLBACK io::unpack(io::mem& is, colibrichat::Channel& data)
{
	io::unpack(is, data.name);
	io::unpack(is, data.password);
	io::unpack(is, data.opened);
	io::unpack(is, data.ftCreation);
	io::unpack(is, data.topic);
	io::unpack(is, data.idTopicWriter);
	io::unpack(is, data.writer);
	io::unpack(is, data.member);
	io::unpack(is, data.moderator);
	io::unpack(is, data.admin);
	io::unpack(is, data.founder);
	io::unpack(is, data.nAutoStatus);
	io::unpack(is, data.nLimit);
	io::unpack(is, data.isHidden);
	io::unpack(is, data.isAnonymous);
	io::unpack(is, data.crBackground);
}

//-----------------------------------------------------------------------------

// Time funstions
void FileTimeToLocalTime(const FILETIME &ft, SYSTEMTIME &st)
{
	FILETIME temp;
	FileTimeToLocalFileTime(&ft, &temp);
	FileTimeToSystemTime(&temp, &st);
}

void CALLBACK GetSystemFileTime(FILETIME& ft)
{
	SYSTEMTIME st;
	GetSystemTime(&st);
	SystemTimeToFileTime(&st, &ft);
}

//-----------------------------------------------------------------------------

void CALLBACK User::Init()
{
	opened.clear();
	GetSystemFileTime(ftCreation);

	IP.S_un.S_addr = 0;
	isOnline = eOffline;
	idOnline = 0;
	nStatus = eReady;
	nStatusImg = 0;
	strStatus = TEXT("");
}

EChanStatus CALLBACK Channel::getStatus(DWORD idUser) const
{
	if (founder.find(idUser) != founder.end())
		return eFounder;
	else if (admin.find(idUser) != admin.end())
		return eAdmin;
	else if (moderator.find(idUser) != moderator.end())
		return eModerator;
	else if (member.find(idUser) != member.end())
		return eMember;
	else if (writer.find(idUser) != writer.end())
		return eWriter;
	else if (opened.find(idUser) != opened.end())
		return eReader;
	else return eOutsider;
}

void CALLBACK Channel::setStatus(DWORD idUser, EChanStatus val)
{
	// Delete previous state if it's not founder
	if (founder.find(idUser) != founder.end())
		founder.erase(idUser);
	else if (admin.find(idUser) != admin.end())
		admin.erase(idUser);
	else if (moderator.find(idUser) != moderator.end())
		moderator.erase(idUser);
	else if (member.find(idUser) != member.end())
		member.erase(idUser);
	else if (writer.find(idUser) != writer.end())
		writer.erase(idUser);
	// Set new state
	switch (val)
	{
	case eFounder:
		founder.insert(idUser);
		break;
	case eAdmin:
		admin.insert(idUser);
		break;
	case eModerator:
		moderator.insert(idUser);
		break;
	case eMember:
		member.insert(idUser);
		break;
	case eWriter:
		writer.insert(idUser);
		break;
	case eReader:
		// State must be assigned without inviting
		break;
	case eOutsider:
		// No actions
		break;
	}
}

//-----------------------------------------------------------------------------

// The End.