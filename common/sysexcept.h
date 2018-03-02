// stdexcept standard header
#pragma once
#ifndef _SYSEXCEPT_
#define _SYSEXCEPT_
#ifndef RC_INVOKED
#include <exception>
#include <xstring>
#ifndef _WINDOWS_
#include <windows.h>
#endif
#include "stringutil.h"

#ifdef  _MSC_VER
#pragma pack(push,_CRT_PACKING)
#pragma warning(push,3)
#endif  /* _MSC_VER */

// CLASS system_error
class system_error
	: public std::exception
{	// base of all sysyem-error exceptions
public:
	static std::string getmessage(DWORD err)
	{
		LPVOID lpMsgBuf;
		FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
			NULL, err, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPWSTR)&lpMsgBuf, 0, NULL);
		std::string res = wchar_to_utf8((LPCWSTR)lpMsgBuf);
		LocalFree(lpMsgBuf);
		return res;
	}

	explicit __CLR_OR_THIS_CALL system_error()
		: _Str(getmessage(GetLastError()))
	{	// construct from message string
	}

	__CLR_OR_THIS_CALL system_error(DWORD _Err)
		: _Str(getmessage(_Err))
	{	// construct from message string
	}

	__CLR_OR_THIS_CALL system_error(const std::string& _Message)
		: _Str(format(_Message.c_str(), getmessage(GetLastError()).c_str()))
	{	// construct from message string
	}

	__CLR_OR_THIS_CALL system_error(const std::string& _Message, DWORD _Err)
		: _Str(format(_Message.c_str(), getmessage(_Err).c_str()))
	{	// construct from message string
	}

	virtual __CLR_OR_THIS_CALL ~system_error() throw()
	{	// destroy the object
	}

	virtual const char *__CLR_OR_THIS_CALL what() const throw()
	{	// return pointer to message string
		return (_Str.c_str());
	}

#if !_HAS_EXCEPTIONS
protected:
	virtual void __CLR_OR_THIS_CALL _Doraise() const
	{	// perform class-specific exception handling
		_RAISE(*this);
	}
#endif /* _HAS_EXCEPTIONS */

private:
	std::string _Str;	// the stored message string
};

#ifdef  _MSC_VER
#pragma warning(pop)
#pragma pack(pop)
#endif  /* _MSC_VER */

#endif /* RC_INVOKED */
#endif /* _SYSEXCEPT_ */
