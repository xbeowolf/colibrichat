#pragma once

#include "windows.h"
#include "stringutil.h"
#include "crtdbg.h"

#ifdef _DEBUG

#ifndef ASSERT
#define ASSERT(expr) _ASSERT(expr)
#define VERIFY(expr) _ASSERT(expr)
#endif

#define ASSERT_MSG(expr, msg) \
		{ if ((!(expr)) && \
			(1 == _CrtDbgReport(_CRT_ERROR, __FILE__, __LINE__, NULL, (msg)))) \
			_CrtDbgBreak(); }
#define VERIFY_MSG(expr, msg) ASSERT_MSG(expr, msg)

#if _MSC_VER < 1300
#define ASSERT_VCLASS_PTR(ptr) \
{ \
	const FARPROC** vfptr = (const FARPROC**)(const void*)ptr; \
	ASSERT_MSG(!IsBadReadPtr(vfptr, 4), Format("Bad vclass ptr - %08X", vfptr)); \
	const FARPROC* vtbl = *vfptr; \
	ASSERT_MSG(!IsBadReadPtr(vtbl, 4), Format("Bad vtbl ptr - %08X", vtbl)); \
	FARPROC vDestructor = *vtbl; \
	ASSERT_MSG(!IsBadCodePtr(vDestructor), Format("Bad vtbl[0] - %08X", vDestructor)); \
}
#else
#define ASSERT_VCLASS_PTR(ptr) \
{ \
	const FARPROC** vfptr = (const FARPROC**)(const void*)ptr; \
	ASSERT_MSG(!IsBadReadPtr(vfptr, 4), format("Bad vclass ptr - %08X", vfptr).c_str()); \
	const FARPROC* vtbl = *vfptr; \
	ASSERT_MSG(!IsBadReadPtr(vtbl, 4), format("Bad vtbl ptr - %08X", vtbl).c_str()); \
	FARPROC vDestructor = *vtbl; \
	ASSERT_MSG(!IsBadCodePtr(vDestructor), format("Bad vtbl[0] - %08X", vDestructor).c_str()); \
}
#endif

#else

#ifndef ASSERT
#define ASSERT(expr) _ASSERT(expr)
#define VERIFY(expr) (expr)
#endif

#define ASSERT_MSG(expr, msg) ((void)0)
#define VERIFY_MSG(expr, msg) \
{ if (!(expr)) { MessageBoxA(0, (msg), "Fatal application error!", MB_ICONHAND | MB_OK); exit(3);} }

#define ASSERT_VCLASS_PTR(ptr) ((void)0)

#define CH_STR(x)	   #x
#define CH_STR2(x)	CH_STR(x)
#define CH_MSG(desc) message(__FILE__ "(" CH_STR2(__LINE__) "):" #desc)

#endif
