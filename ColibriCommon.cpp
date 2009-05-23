
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
}

void CALLBACK io::pack(std::ostream& os, const colibrichat::Channel& data)
{
	io::pack(os, data.name);
	io::pack(os, data.opened);
	io::pack(os, data.ftCreation);
	io::pack(os, data.password);
	io::pack(os, data.topic);
	io::pack(os, data.idFounder);
	io::pack(os, data.nAutoStatus);
	io::pack(os, data.isHidden);
	io::pack(os, data.isAnonymous);
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
}

void CALLBACK io::unpack(io::mem& is, colibrichat::Channel& data)
{
	io::unpack(is, data.name);
	io::unpack(is, data.opened);
	io::unpack(is, data.ftCreation);
	io::unpack(is, data.password);
	io::unpack(is, data.topic);
	io::unpack(is, data.idFounder);
	io::unpack(is, data.nAutoStatus);
	io::unpack(is, data.isHidden);
	io::unpack(is, data.isAnonymous);
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

// The End.