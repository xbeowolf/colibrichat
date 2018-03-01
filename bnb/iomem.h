/******************************************************************************
*                                                                             *
* iomem.h -- Memory data packing/unpacking class and functions.               *
*                                                                             *
* Copyright (c) Podobashev Dmitry / BEOWOLF, 2008-2010. All rights reserved.  *
*                                                                             *
******************************************************************************/

#ifndef _IOMEM_
#define _IOMEM_

//-----------------------------------------------------------------------------

//
// Includes
//

#pragma region Includes
#pragma once

#pragma endregion

//-----------------------------------------------------------------------------

namespace io
{
	class mem
	{
	public:

		template<typename T> friend void unpack(mem& is, T& data);
		friend void unpack(mem& is, std::string& data);
		friend void unpack(mem& is, std::wstring& data);
		friend void unpackptr(mem& is, const void*& ptr, size_t size);
		friend void unpackmem(mem& is, void* ptr, size_t size);
		template<typename T> friend void unpackptr(mem& is, T*& ptr);
		template<typename T> friend void unpackstr(mem& is, T*& str);
		template<typename T> friend void skip(mem& is);
		friend void skip(mem& is, size_t size);

		mem(const char* start, const char* end = 0);
		mem(const char* start, size_t size, int c = 0);

		size_t getsize() const {return m_end - m_pos;}
		bool check(size_t size) const;
		operator const char*& () {return m_pos;}

		static bool throwable;

	protected:

		JPROPERTY_R(const char*, pos);
		JPROPERTY_R(const char*, end);
		int count;
	};

	class exception : public std::exception
	{
	public:

		exception(int c = 0);

		int count;
	};

	// pack to ostream
	template<typename T> void pack(std::ostream& os, const T& data)
	{
		os.write((const char*)&data, sizeof(T));
	};
	void pack(std::ostream& os, const CHAR* data);
	void pack(std::ostream& os, const WCHAR* data);
	void pack(std::ostream& os, const std::string& data);
	void pack(std::ostream& os, const std::wstring& data);

	// unpack from istream
	template<typename T> void unpack(std::istream& is, T& data)
	{
		is.read((char*)&data, sizeof(T));
	};
	void unpack(std::istream& is, std::string& data);
	void unpack(std::istream& is, std::wstring& data);

	// unpack from mem
	template<typename T> void unpack(mem& is, T& data)
	{
		is.check(sizeof(T));
		MoveMemory(&data, is.m_pos, sizeof(T));
		is.m_pos += sizeof(T);
		is.count++;
	};
	void unpack(mem& is, std::string& data);
	void unpack(mem& is, std::wstring& data);
	template<typename T> void unpackptr(mem& is, T*& ptr)
	{
		is.check(sizeof(T));
		ptr = (T*)is.m_pos;
		is.m_pos += sizeof(T);
		is.count++;
	};
	void unpackptr(mem& is, const void*& ptr, size_t size);
	void unpackmem(mem& is, void* ptr, size_t size);
	template<typename T> void unpackstr(mem& is, T*& str)
	{
		str = (T*)is.m_pos;
		while (*(T*)is.m_pos)
		{
			is.check(sizeof(T));
			is.m_pos += sizeof(T);
		}
		is.check(sizeof(T));
		is.m_pos += sizeof(T);
		is.count++;
	};
	// skip in mem
	template<typename T> void skip(mem& is)
	{
		is.check(sizeof(T));
		is.m_pos += sizeof(T);
		is.count++;
	};
	void skip(mem& is, size_t size);

	// unpack from pointer
	template<typename T> void unpack(const char*& is, T& data)
	{
		MoveMemory(&data, is, sizeof(T));
		is += sizeof(T);
	};
	void unpack(const char*& is, std::string& data);
	void unpack(const char*& is, std::wstring& data);
	template<typename T> void unpackptr(const char*& is, T*& ptr)
	{
		ptr = (T*)is;
		is += sizeof(T);
	};
	void unpackptr(const char*& is, const void*& ptr, size_t size);
	void unpackmem(const char*& is, void* ptr, size_t size);
	template<typename T> void unpackstr(const char*& is, T*& str)
	{
		ptr = (T*)is;
		while (*(T*)is)
			is += sizeof(T);
		is += sizeof(T);
	};
	// skip in mem
	template<typename T> void skip(const char*& is)
	{
		is += sizeof(T);
	};
	void skip(const char*& is, size_t size);
}; // io

//-----------------------------------------------------------------------------

#endif // _IOMEM_