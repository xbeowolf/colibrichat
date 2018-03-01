//-----------------------------------------------------------------------------
// Style print.
// (C) Copyright 2005 by Podobashev D. O.
//
//   Style print for rich edit controls code.
//-----------------------------------------------------------------------------

#ifndef __STYLEPRINT_
#define __STYLEPRINT_

//-----------------------------------------------------------------------------

//
// Includes
//

#pragma region Includes
#pragma once

// Windows API
#include <richedit.h>

#pragma endregion

//-----------------------------------------------------------------------------

#ifdef __cplusplus
extern "C" {
#endif

	//-----------------------------------------------------------------------------

	//
	// Style print
	//

	// Style print formats
#define SPF_STYLE              0x00000000UL
#define SPF_COLOR              0x01000000UL
#define SPF_SIZE               0x02000000UL
#define SPF_OFFSET             0x03000000UL
#define SPF_BOLD               0x04000000UL
#define SPF_ITALIC             0x05000000UL
#define SPF_ULINE              0x06000000UL
#define SPF_STROUT             0x07000000UL
#define SPF_SUBSCRIPT          0x08000000UL
#define SPF_SUPERSCRIPT        0x09000000UL
#define SPF_FONT               0x0E000000UL
#define SPF_AUTOCOLOR          0x0F000000UL
#define SPF_MASK               0xFF000000UL
#define SPF_INDEX              0x00FFFFFFUL

	typedef struct {
		DWORD index;
		DWORD start, end;
	} STYLERANGE;

	typedef struct {
		TCHAR* str;
		int len;
		STYLERANGE* sr;
		UINT sr_num;
		UINT sr_i;
	} STYLEFIELD;

	extern UINT CALLBACK StylePrint(const TCHAR* str, STYLEFIELD* sf, const TCHAR** styledescr);

	extern BOOL CALLBACK StylePrint_Apply(HWND hwndRE, STYLEFIELD* sf, const CHARFORMAT* style, UINT cfcount, int offset);

	extern BOOL CALLBACK StylePrint_Append(HWND hwndRE, const TCHAR* str, STYLEFIELD* sf, const TCHAR** styledescr, const CHARFORMAT* style, UINT cfcount);

	//-----------------------------------------------------------------------------

#ifdef __cplusplus
}
#endif

//-----------------------------------------------------------------------------

#endif  // __STYLEPRINT_
