#pragma once
#ifndef __stringutil_h__
#define __stringutil_h__

//-----------------------------------------------------------------------------

#include <tchar.h>
#include <string.h>

#include <string>

namespace std {

	typedef basic_string<TCHAR> tstring;

	template<typename T1, typename T2> inline basic_string<T2> stringConvert( const basic_string<T1>& src )
	{
		basic_string<T2> res;

		int size = (int)src.size();

		res.resize( size );
		for( int i = 0; i < size; i++ )
		{
			T1 c = src[i];
			if( c >= 0 && c <= 0x7f )
				res[i] = (T2)c;
			else
				res[i] = '?';
		}

		return res;
	}

	struct striless : std::binary_function<std::string, std::string, bool>
	{
		__forceinline bool __fastcall operator()(const std::string& _X, const std::string& _Y) const
		{
			return _stricmp(_X.c_str(), _Y.c_str()) < 0;
		};
	};

	struct wstriless : std::binary_function<std::basic_string<wchar_t>, std::basic_string<wchar_t>, bool>
	{
		__forceinline bool __fastcall operator()(const std::basic_string<wchar_t>& _X, const std::basic_string<wchar_t>& _Y) const
		{
			return _wcsicmp(_X.c_str(), _Y.c_str()) < 0;
		};
	};

};

//-----------------------------------------------------------------------------

std::string __cdecl format(const char* fmt, ...);
std::wstring __cdecl wformat(const wchar_t* fmt, ...);

#ifdef UNICODE
#define tformat wformat
#else
#define tformat format
#endif

// bring filename to unix-compatible format
template<typename T>
void correctfilename(std::basic_string<T>& str) {
	T c;
	for (T* dest = (T*)str.data(); *dest; dest++) {
		c = *dest;
		if (c >= 'A' && c <= 'Z') *dest += 32;
		else if (c == '\\') *dest = '/';
		else if (c == '*' || c == '?' || c == '|' || c == '<' || c == '>') *dest = '_';
	}
}

std::string wchar_to_utf8(const wchar_t* in, size_t len);
std::wstring utf8_to_wchar(const char* in, size_t len);
inline std::string wchar_to_utf8(const std::wstring& in) { return wchar_to_utf8(in.data(), in.size()); }
inline std::wstring utf8_to_wchar(const std::string& in) { return utf8_to_wchar(in.data(), in.size()); }

#ifdef UNICODE
#define tstr_to_utf8 wchar_to_utf8
inline std::wstring tstr_to_wchar(const TCHAR* in, size_t len) { return std::wstring(in, len); }
inline std::wstring tstr_to_wchar(const std::tstring& in) { return in; }
#define utf8_to_tstr utf8_to_wchar
inline std::tstring wchar_to_tstr(const wchar_t* in, size_t len) { return std::wstring(in, len); }
inline std::tstring wchar_to_tstr(const std::wstring& in) { return in; }
#else
inline std::string tstr_to_utf8(const TCHAR* in, size_t len) { return std::string(in, len); }
inline std::string tstr_to_utf8(const std::tstring& in) { return in; }
#define tstr_to_wchar utf8_to_wchar
inline std::tstring utf8_to_tstr(const char* in, size_t len) { return std::string(in, len); }
inline std::tstring utf8_to_tstr(const std::string& in) { return in; }
#define wchar_to_tstr wchar_to_utf8
#endif

//-----------------------------------------------------------------------------

#endif // __stringutil_h__
