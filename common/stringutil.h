#pragma once

#include <string>
#include <functional>

#if _MSC_VER < 1300

#include <afx.h>

inline CString __cdecl Format(LPCTSTR lpszFormat, ...)
{
	CString res;
	va_list argList;
	va_start(argList, lpszFormat);
	res.FormatV(lpszFormat, argList);
	va_end(argList);

	return res;
};

#else

#include <malloc.h>
#include <stdio.h>
#include <stdarg.h>
#include <windows.h>
#include <string.h>

inline std::string __cdecl format(LPCSTR lpszFormat, ...)
{
	va_list argList;
	va_start(argList, lpszFormat);

	std::string res(_vscprintf(lpszFormat, argList), 0);
	vsprintf_s((std::string::value_type*)res.data(), res.size()+1, lpszFormat, argList);

	return res;
};

inline std::wstring __cdecl wformat(LPCWSTR lpszFormat, ...)
{
	va_list argList;
	va_start(argList, lpszFormat);

	std::wstring res(_vscwprintf(lpszFormat, argList), 0);
	vswprintf_s((std::wstring::value_type*)res.data(), res.size()+1, lpszFormat, argList);

	return res;
};

inline std::string correctSlashes(const std::string& inStr)
{
	std::string buf = inStr;
	for (char* dest = &buf[0]; *dest; dest++) {
		if (*dest == '/') *dest = '\\';
	}
	return buf;
}

#ifdef UNICODE
#define tformat wformat
#else
#define tformat format
#endif

#endif

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

#ifdef FASTSTRINGCONVERT
#define ANSIToUnicode(src) std::stringConvert<CHAR, WCHAR>(src)
#define UnicodeToANSI(src) std::stringConvert<WCHAR, CHAR>(src)
#else
inline std::wstring ANSIToUnicode(const std::string& src, UINT CodePage = CP_ACP)
{
	std::wstring res((std::wstring::size_type)MultiByteToWideChar(CodePage, 0, src.c_str(), (int)src.size(), 0, 0), 0);
	MultiByteToWideChar(CodePage, 0, src.c_str(), (int)src.size(), (LPWSTR)res.data(), (int)res.size() + 1);
	return res;
}
inline std::string UnicodeToANSI(const std::wstring& src, UINT CodePage = CP_ACP)
{
	std::string res((std::string::size_type)WideCharToMultiByte(CodePage, 0, src.c_str(), (int)src.size(), 0, 0, 0, 0), 0);
	WideCharToMultiByte(CodePage, 0, src.c_str(), (int)src.size(), (LPSTR)res.data(), (int)res.size() + 1, 0, 0);
	return res;
}
#define MbToWide(src) ANSIToUnicode(src, CP_UTF8)
#define WideToMb(src) UnicodeToANSI(src, CP_UTF8)
#define CpToCp(src, from, to) UnicodeToANSI(ANSIToUnicode(src, from), to)
#endif

#ifdef UNICODE
#define TstrToUnicode(src) ((const std::wstring&)src)
#define TstrToANSI(src) UnicodeToANSI(src)
#define UnicodeToTstr(src) ((const std::tstring&)src)
#define ANSIToTstr(src) ANSIToUnicode(src)
#else
#define TstrToUnicode(src) ANSIToUnicode(src)
#define TstrToANSI(src) ((const std::string&)src)
#define UnicodeToTstr(src) UnicodeToANSI(src)
#define ANSIToTstr(src) ((const std::tstring&)src)
#endif
