/******************************************************************************
*                                                                             *
* jrtf.h -- Editor with rich edit formatting commands class definition       *
*                                                                             *
* Copyright (c) Podobashev Dmitry / BEOWOLF, 2009. All rights reserved.       *
*                                                                             *
******************************************************************************/

#ifndef _JRTF_
#define _JRTF_

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

namespace rtf
{
	// Commands identifiers for toolbar
	enum {
		idcRtfCmd = 200,
		idcBold,
		idcItalic,
		idcUnderline,
		idcSubscript,
		idcSuperscript,
		idcFont,
		idcFgColor,
		idcBgColor,
		idcSheetColor,
		idcAlignLeft,
		idcAlignRight,
		idcAlignCenter,
		idcAlignJustify,
		idcMarksBullet,
		idcMarksArabic,
		idcStartIndentInc,
		idcStartIndentDec,
		idcBkMode,
	};

	// Callbacks for RTF streaming to std::strings
	DWORD CALLBACK StreamToHandle(DWORD_PTR dwCookie, LPBYTE pbBuff, LONG cb, LONG *pcb);
	DWORD CALLBACK StreamToStringA(DWORD_PTR dwCookie, LPBYTE pbBuff, LONG cb, LONG *pcb);
	DWORD CALLBACK StreamToStringW(DWORD_PTR dwCookie, LPBYTE pbBuff, LONG cb, LONG *pcb);

	class Editor
	{
	public:

		static COLORREF rgbCustColors[16];

		CALLBACK Editor();

	protected:

		LRESULT WINAPI DlgProc(HWND, UINT, WPARAM, LPARAM);

		void getContent(std::string& content,  UINT mode);
		void getContent(std::wstring& content, UINT mode);

		void UpdateCharacterButtons();
		void UpdateParagraphButtons();

		virtual void OnSheetColor(COLORREF cr);
		virtual void OnCharFormat(const CHARFORMAT& cf, UINT mode);
		virtual void OnParaFormat(const PARAFORMAT& pf);

	protected:

		JPROPERTY_R(bool, fTransparent);
		JPROPERTY_R(COLORREF, crSheet);

		WORD wCharFormatting;
		int twipIndentStep;

		JPROPERTY_R(HWND, hwndTB);
		JPROPERTY_R(HWND, hwndEdit);
		RECT rcEdit;
	};
}; // namespace rtf

//-----------------------------------------------------------------------------

#endif // _JRTF_