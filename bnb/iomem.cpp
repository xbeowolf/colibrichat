
//-----------------------------------------------------------------------------

//
// Includes
//

#pragma region Includes

#include "stdafx.h"

// Common
#include "iomem.h"

#pragma endregion

//-----------------------------------------------------------------------------

using namespace io;

//-----------------------------------------------------------------------------

bool mem::throwable = true;

mem::mem(const char* start, const char* end)
{
	m_pos = start;
	this->m_end = end;
	count = 0;
}

mem::mem(const char* start, size_t size, int c)
{
	m_pos = start;
	m_end = start + size;
	count = c;
}

bool mem::check(size_t size) const
{
	if (m_end && size > getsize())
	{
		if (throwable) throw exception(count);
		else return false;
	}
	else return true;
}

exception::exception(int c)
: std::exception("Argument streaming failed")
{
	count = c;
}

void io::pack(std::ostream& os, const std::string& data)
{
	os.write((const char*)data.c_str(), sizeof(char)*((int)data.length() + 1));
};

void io::pack(std::ostream& os, const std::wstring& data)
{
	os.write((const char*)data.c_str(), sizeof(wchar_t)*((int)data.length() + 1));
};

void io::pack(std::ostream& os, const CHAR* data)
{
	os.write((const char*)data, sizeof(CHAR)*(lstrlenA(data) + 1));
};

void io::pack(std::ostream& os, const WCHAR* data)
{
	os.write((const char*)data, sizeof(WCHAR)*(lstrlenW(data) + 1));
};

void io::unpack(std::istream& is, std::string& data)
{
	data.clear();
	char c;
	is.read((char*)&c, sizeof(c));
	while (c)
	{
		data.push_back(c);
		c = 0;
		is.read((char*)&c, sizeof(c));
	}
}

void io::unpack(std::istream& is, std::wstring& data)
{
	data.clear();
	wchar_t c;
	is.read((char*)&c, sizeof(c));
	while (c)
	{
		data.push_back(c);
		c = 0;
		is.read((char*)&c, sizeof(c));
	}
}

void io::unpack(mem& is, std::string& data)
{
	bool c;
	const char* tmp = is.m_pos;
	do
	{
		is.check(sizeof(char));
		c = *(char*)is.m_pos != 0;
		is.m_pos += sizeof(char);
	} while (c);
	data = (const char*)tmp;
	is.count++;
}

void io::unpack(mem& is, std::wstring& data)
{
	bool c;
	const char* tmp = is.m_pos;
	do
	{
		is.check(sizeof(wchar_t));
		c = *(wchar_t*)is.m_pos != 0;
		is.m_pos += sizeof(wchar_t);
	} while (c);
	data = (const wchar_t*)tmp;
	is.count++;
}

void io::unpackptr(mem& is, const void*& ptr, size_t size)
{
	is.check(size);
	ptr = is.m_pos;
	is.m_pos += size;
	is.count++;
}

void io::unpackmem(mem& is, void* ptr, size_t size)
{
	is.check(size);
	MoveMemory(ptr, is.m_pos, size);
	is.m_pos += size;
	is.count++;
}

void io::unpack(const char*& is, std::string& data)
{
	data = (const char*)is;
	is += sizeof(char)*(data.length() + 1);
}

void io::unpack(const char*& is, std::wstring& data)
{
	data = (const wchar_t*)is;
	is += sizeof(wchar_t)*(data.length() + 1);
}

void io::skip(mem& is, size_t size)
{
	is.check(size);
	is.m_pos += size;
	is.count++;
}

void io::unpackptr(const char*& is, const void*& ptr, size_t size)
{
	ptr = is;
	is += size;
}

void io::unpackmem(const char*& is, void* ptr, size_t size)
{
	MoveMemory(ptr, is, size);
	is += size;
}

void io::skip(const char*& is, size_t size)
{
	is += size;
}

//-----------------------------------------------------------------------------

// The End.