// jrtf.cpp
//

//-----------------------------------------------------------------------------

//
// Includes
//

#pragma region Includes

#include "stdafx.h"

// Windows API
#include <strsafe.h>
#include <richedit.h>

// Common
#include "jrtf.h"

#pragma endregion

using namespace rtf;

//-----------------------------------------------------------------------------

//
// Editor dialog
//

COLORREF Editor::rgbCustColors[16] =
{
	0x010101L, 0x101010L, 0x202020L, 0x303030L,
	0x404040L, 0x505050L, 0x606060L, 0x707070L,
	0x808080L, 0x909090L, 0xA0A0A0L, 0xB0B0B0L,
	0xC0C0C0L, 0xD0D0D0L, 0xE0E0E0L, 0xF0F0F0L
};

// Message handler for rich text editor dialog box.
LRESULT WINAPI Editor::DlgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	LRESULT retval = TRUE;

	switch (message)
	{
	case WM_COMMAND:
		{
			switch (LOWORD(wParam))
			{
			case idcBold:
				{
					CHARFORMAT cf;
					cf.cbSize = sizeof(cf);
					cf.dwMask = CFM_BOLD;
					SendMessage(m_hwndEdit, EM_GETCHARFORMAT, TRUE, (LPARAM)&cf);
					cf.dwEffects ^= CFE_BOLD;
					OnCharFormat(cf, wCharFormatting);
					break;
				}

			case idcItalic:
				{
					CHARFORMAT cf;
					cf.cbSize = sizeof(cf);
					cf.dwMask = CFM_ITALIC;
					SendMessage(m_hwndEdit, EM_GETCHARFORMAT, TRUE, (LPARAM)&cf);
					cf.dwEffects ^= CFE_ITALIC;
					OnCharFormat(cf, wCharFormatting);
					break;
				}

			case idcUnderline:
				{
					CHARFORMAT cf;
					cf.cbSize = sizeof(cf);
					cf.dwMask = CFM_UNDERLINE;
					SendMessage(m_hwndEdit, EM_GETCHARFORMAT, TRUE, (LPARAM)&cf);
					cf.dwEffects ^= CFE_UNDERLINE;
					OnCharFormat(cf, wCharFormatting);
					break;
				}

			case idcSubscript:
				{
					CHARFORMAT2 cf2;
					cf2.cbSize = sizeof(cf2);
					cf2.dwMask = CFM_SUBSCRIPT;
					SendMessage(m_hwndEdit, EM_GETCHARFORMAT, TRUE, (LPARAM)&cf2);
					cf2.dwEffects ^= CFE_SUBSCRIPT;
					cf2.dwEffects &= ~CFE_SUPERSCRIPT;
					OnCharFormat(cf2, wCharFormatting);
					break;
				}

			case idcSuperscript:
				{
					CHARFORMAT2 cf2;
					cf2.cbSize = sizeof(cf2);
					cf2.dwMask = CFM_SUPERSCRIPT;
					SendMessage(m_hwndEdit, EM_GETCHARFORMAT, TRUE, (LPARAM)&cf2);
					cf2.dwEffects ^= CFE_SUPERSCRIPT;
					cf2.dwEffects &= ~CFE_SUBSCRIPT;
					OnCharFormat(cf2, wCharFormatting);
					break;
				}

			case idcFont:
				{
					CHARFORMAT cf;
					cf.cbSize = sizeof(cf);
					cf.dwMask = CFM_BOLD | CFM_COLOR | CFM_FACE | CFM_ITALIC | CFM_OFFSET | CFM_SIZE | CFM_UNDERLINE;
					SendMessage(m_hwndEdit, EM_GETCHARFORMAT, TRUE, (LPARAM)&cf);

					CHARRANGE cr;
					SendMessage(m_hwndEdit, EM_EXGETSEL, 0, (LPARAM)&cr);
					BOOL focus = GetFocus() == m_hwndEdit;

					HDC hDC = GetDC(hWnd);
					LOGFONT lf;
					static TCHAR buffer[32];
					CHOOSEFONT cfnt =
					{
						sizeof(CHOOSEFONT), // lStructSize
						0, // hwndOwner
						0, // hDC
						&lf, // lpLogFont
						120, // iPointSize
						CF_EFFECTS | CF_FORCEFONTEXIST | CF_INITTOLOGFONTSTRUCT |
						CF_SCREENFONTS | CF_USESTYLE, // Flags
						RGB(0, 0, 0), // rgbColors
						0, // lCustData
						0, // lpfnHook
						0, // lpTemplateName
						0, // hInstance
						buffer, // lpszStyle
						0, // nFontType
						0, // nSizeMin
						0 // nSizeMax
					};
					cfnt.hwndOwner = hWnd;
					cfnt.rgbColors = cf.crTextColor;
					lf.lfHeight = -MulDiv(cf.yHeight/20, GetDeviceCaps(hDC, LOGPIXELSY), 72);
					lf.lfWidth = 0;
					lf.lfEscapement = 0;
					lf.lfOrientation = 0;
					lf.lfWeight = (cf.dwEffects & CFE_BOLD) ? FW_BOLD : FW_NORMAL;
					lf.lfItalic = (cf.dwEffects & CFE_ITALIC) != 0;
					lf.lfUnderline = (cf.dwEffects & CFE_UNDERLINE) != 0;
					lf.lfStrikeOut = (cf.dwEffects & CFE_STRIKEOUT) != 0;
					lf.lfCharSet = cf.bCharSet;
					lf.lfOutPrecision = OUT_DEFAULT_PRECIS;
					lf.lfClipPrecision = CLIP_DEFAULT_PRECIS;
					lf.lfQuality = DEFAULT_QUALITY;
					lf.lfPitchAndFamily = cf.bPitchAndFamily;
					StringCchCopy(lf.lfFaceName, sizeof(lf.lfFaceName), cf.szFaceName);
					BOOL set = ChooseFont(&cfnt);
					if (focus) {
						SetFocus(m_hwndEdit);
						SendMessage(m_hwndEdit, EM_EXSETSEL, 0, (LPARAM)&cr);
					}
					if (set)
					{
						cf.dwMask = CFM_BOLD | CFM_COLOR | CFM_FACE | CFM_ITALIC |
							(cf.dwMask & CFM_OFFSET) | CFM_SIZE | CFM_STRIKEOUT | CFM_UNDERLINE;
						cf.dwEffects =
							(lf.lfWeight >= FW_BOLD ? CFE_BOLD : 0) |
							(lf.lfItalic ? CFE_ITALIC : 0) |
							(lf.lfStrikeOut ? CFE_STRIKEOUT : 0) |
							(lf.lfUnderline ? CFE_UNDERLINE : 0);
						cf.yHeight = -MulDiv(lf.lfHeight, 72, GetDeviceCaps(hDC, LOGPIXELSY))*20;
						cf.crTextColor = cfnt.rgbColors;
						StringCchCopy(cf.szFaceName, sizeof(cf.szFaceName), lf.lfFaceName);
						OnCharFormat(cf, wCharFormatting);
					}
					ReleaseDC(hWnd, hDC);
					break;
				}

			case idcFgColor:
				{
					CHOOSECOLOR cc =
					{
						sizeof(CHOOSECOLOR), // lStructSize
						0, // hwndOwner
						0, // hInstance
						RGB(255, 0, 0), // rgbResult
						rgbCustColors, // lpCustColors
						CC_RGBINIT, // Flags
						0, // lCustomData
						0, // lpfnHook
						0 // lpTemplateName
					};
					CHARFORMAT cf;
					cf.cbSize = sizeof(cf);
					cf.dwMask = CFM_COLOR;
					SendMessage(m_hwndEdit, EM_GETCHARFORMAT, TRUE, (LPARAM)&cf);

					CHARRANGE cr;
					SendMessage(m_hwndEdit, EM_EXGETSEL, 0, (LPARAM)&cr);
					BOOL focus = GetFocus() == m_hwndEdit;

					cc.hwndOwner = hWnd;
					cc.rgbResult = cf.crTextColor;
					BOOL set = ChooseColor(&cc);
					if (focus) {
						SetFocus(m_hwndEdit);
						SendMessage(m_hwndEdit, EM_EXSETSEL, 0, (LPARAM)&cr);
					}
					if (set)
					{
						cf.dwMask = CFM_COLOR;
						cf.crTextColor = cc.rgbResult;
						cf.dwEffects = 0;
						OnCharFormat(cf, wCharFormatting);
					}
					break;
				}

			case idcSheetColor:
				{
					CHOOSECOLOR cc =
					{
						sizeof(CHOOSECOLOR), // lStructSize
						0, // hwndOwner
						0, // hInstance
						RGB(255, 255, 255), // rgbResult
						rgbCustColors, // lpCustColors
						CC_RGBINIT, // Flags
						0, // lCustomData
						0, // lpfnHook
						0 // lpTemplateName
					};
					cc.hwndOwner = hWnd;
					cc.rgbResult = m_crSheet;
					if (ChooseColor(&cc)) {
						OnSheetColor(cc.rgbResult);
					}
					break;
				}

			case idcAlignLeft:
				{
					PARAFORMAT pf;
					pf.cbSize = sizeof(pf);
					pf.dwMask = PFM_ALIGNMENT;
					SendMessage(m_hwndEdit, EM_GETPARAFORMAT, 0, (LPARAM)&pf);
					pf.dwMask = PFM_ALIGNMENT;
					pf.wAlignment = PFA_LEFT;
					OnParaFormat(pf);
					break;
				}

			case idcAlignCenter:
				{
					PARAFORMAT pf;
					pf.cbSize = sizeof(pf);
					pf.dwMask = PFM_ALIGNMENT;
					SendMessage(m_hwndEdit, EM_GETPARAFORMAT, 0, (LPARAM)&pf);
					pf.dwMask = PFM_ALIGNMENT;
					pf.wAlignment = PFA_CENTER;
					OnParaFormat(pf);
					break;
				}

			case idcAlignRight:
				{
					PARAFORMAT pf;
					pf.cbSize = sizeof(pf);
					pf.dwMask = PFM_ALIGNMENT;
					SendMessage(m_hwndEdit, EM_GETPARAFORMAT, 0, (LPARAM)&pf);
					pf.dwMask = PFM_ALIGNMENT;
					pf.wAlignment = PFA_RIGHT;
					OnParaFormat(pf);
					break;
				}

			case idcAlignJustify:
				{
					PARAFORMAT pf;
					pf.cbSize = sizeof(pf);
					pf.dwMask = PFM_ALIGNMENT;
					SendMessage(m_hwndEdit, EM_GETPARAFORMAT, 0, (LPARAM)&pf);
					pf.dwMask = PFM_ALIGNMENT;
					pf.wAlignment = PFA_JUSTIFY;
					OnParaFormat(pf);
					break;
				}

			case idcMarksBullet:
				{
					PARAFORMAT pf;
					pf.cbSize = sizeof(pf);
					pf.dwMask = PFM_NUMBERING;
					SendMessage(m_hwndEdit, EM_GETPARAFORMAT, 0, (LPARAM)&pf);
					pf.dwMask = PFM_NUMBERING;
					pf.wNumbering = pf.wNumbering != PFN_BULLET ? PFN_BULLET : 0;
					OnParaFormat(pf);
					break;
				}

			case idcMarksArabic:
				{
					PARAFORMAT2 pf2;
					WORD style[] = {PFNS_PAREN, PFNS_PARENS, PFNS_PERIOD, PFNS_PLAIN, PFNS_NONUMBER, PFNS_NEWNUMBER};
					int i;
					pf2.cbSize = sizeof(pf2);
					pf2.dwMask = PFM_NUMBERING | PFM_NUMBERINGSTART | PFM_NUMBERINGSTYLE;
					SendMessage(m_hwndEdit, EM_GETPARAFORMAT, 0, (LPARAM)&pf2);
					if (pf2.wNumbering == PFN_ARABIC) {
						for (i = 0; i < _countof(style) && pf2.wNumberingStyle != style[i]; i++) {}
						i++;
					} else i = 0;
					pf2.dwMask = PFM_NUMBERING | PFM_NUMBERINGSTART | PFM_NUMBERINGSTYLE;
					pf2.wNumbering = pf2.wNumbering != PFN_ARABIC || i < _countof(style) ? PFN_ARABIC : 0;
					pf2.wNumberingStart = 1;
					pf2.wNumberingStyle = i < _countof(style) ? style[i] : 0;
					OnParaFormat(pf2);
					break;
				}

			case idcStartIndentInc:
				{
					CHARRANGE cr0, cr;
					SendMessage(m_hwndEdit, EM_EXGETSEL, 0, (LPARAM)&cr0);
					LONG posb = (LONG)SendMessage(m_hwndEdit, EM_LINEFROMCHAR, cr0.cpMin, 0);
					LONG pose = (LONG)SendMessage(m_hwndEdit, EM_LINEFROMCHAR, cr0.cpMax, 0);
					for (LONG i = posb; i <= pose; i++) {
						cr.cpMin = cr.cpMax = (LONG)SendMessage(m_hwndEdit, EM_LINEINDEX, i, 0);
						SendMessage(m_hwndEdit, EM_EXSETSEL, 0, (LPARAM)&cr);
						PARAFORMAT2 pf2;
						pf2.cbSize = sizeof(pf2);
						pf2.dwMask = PFM_STARTINDENT;
						SendMessage(m_hwndEdit, EM_GETPARAFORMAT, 0, (LPARAM)&pf2);
						pf2.dwMask = PFM_STARTINDENT;
						pf2.dxStartIndent += twipIndentStep;
						pf2.dxStartIndent %= 12000;
						pf2.dxStartIndent = (pf2.dxStartIndent/twipIndentStep)*twipIndentStep;
						SendMessage(m_hwndEdit, EM_SETPARAFORMAT, 0, (LPARAM)&pf2);
					}
					SendMessage(m_hwndEdit, EM_EXSETSEL, 0, (LPARAM)&cr0);
					break;
				}

			case idcStartIndentDec:
				{
					CHARRANGE cr0, cr;
					SendMessage(m_hwndEdit, EM_EXGETSEL, 0, (LPARAM)&cr0);
					LONG posb = (LONG)SendMessage(m_hwndEdit, EM_LINEFROMCHAR, cr0.cpMin, 0);
					LONG pose = (LONG)SendMessage(m_hwndEdit, EM_LINEFROMCHAR, cr0.cpMax, 0);
					for (LONG i = posb; i <= pose; i++) {
						cr.cpMin = cr.cpMax = (LONG)SendMessage(m_hwndEdit, EM_LINEINDEX, i, 0);
						SendMessage(m_hwndEdit, EM_EXSETSEL, 0, (LPARAM)&cr);
						PARAFORMAT2 pf2;
						pf2.cbSize = sizeof(pf2);
						pf2.dwMask = PFM_STARTINDENT;
						SendMessage(m_hwndEdit, EM_GETPARAFORMAT, 0, (LPARAM)&pf2);
						pf2.dwMask = PFM_STARTINDENT;
						pf2.dxStartIndent -= twipIndentStep;
						if (pf2.dxStartIndent < 0) pf2.dxStartIndent = 0;
						pf2.dxStartIndent = (pf2.dxStartIndent/twipIndentStep)*twipIndentStep;
						SendMessage(m_hwndEdit, EM_SETPARAFORMAT, 0, (LPARAM)&pf2);
					}
					SendMessage(m_hwndEdit, EM_EXSETSEL, 0, (LPARAM)&cr0);
					break;
				}

			case idcBkMode:
				{
					m_fTransparent = !m_fTransparent;
					break;
				}

			default: retval = FALSE;
			}
			break;
		}

	case WM_NOTIFY:
		{
			NMHDR* pnmh = (NMHDR*)lParam;
			switch (pnmh->code)
			{
			case EN_DRAGDROPDONE:
			case EN_SELCHANGE:
				{
					if (pnmh->hwndFrom == m_hwndEdit)
					{
						UpdateCharacterButtons();
						UpdateParagraphButtons();
					} else retval = FALSE;
					break;
				}

			default: retval = FALSE;
			}
			break;
		}

	default: retval = FALSE;
	}
	return retval;
}

CALLBACK Editor::Editor()
{
	m_fTransparent = false;
	m_crSheet = GetSysColor(COLOR_WINDOW);

	wCharFormatting = SCF_SELECTION;
	twipIndentStep = 120*3; // 3 chars
}

DWORD CALLBACK rtf::StreamToHandle(DWORD_PTR dwCookie, LPBYTE pbBuff, LONG cb, LONG *pcb)
{
	_VERIFY(WriteFile((HANDLE)dwCookie, pbBuff, cb, (LPDWORD)pcb, 0));
	return cb == *pcb;
}

DWORD CALLBACK rtf::StreamToStringA(DWORD_PTR dwCookie, LPBYTE pbBuff, LONG cb, LONG *pcb)
{
	((std::string*)dwCookie)->append((const char*)pbBuff, cb/sizeof(char));
	*pcb = cb;
	return 0;
}

DWORD CALLBACK rtf::StreamToStringW(DWORD_PTR dwCookie, LPBYTE pbBuff, LONG cb, LONG *pcb)
{
	((std::wstring*)dwCookie)->append((const wchar_t*)pbBuff, cb/sizeof(wchar_t));
	*pcb = cb;
	return 0;
}

void Editor::getContent(std::string& content,  UINT mode)
{
	EDITSTREAM es;
	es.dwCookie = (DWORD_PTR)&content;
	es.dwError = 0;
	es.pfnCallback = rtf::StreamToStringA;
	SendMessage(m_hwndEdit, EM_STREAMOUT, mode, (LPARAM)&es);
}

void Editor::getContent(std::wstring& content, UINT mode)
{
	EDITSTREAM es;
	es.dwCookie = (DWORD_PTR)&content;
	es.dwError = 0;
	es.pfnCallback = rtf::StreamToStringW;
	SendMessage(m_hwndEdit, EM_STREAMOUT, mode, (LPARAM)&es);
}

void Editor::UpdateCharacterButtons()
{
	CHARFORMAT2 cf2;
	cf2.cbSize = sizeof(cf2);
	cf2.dwMask = CFM_BOLD | CFM_COLOR | CFM_FACE | CFM_ITALIC | CFM_OFFSET | CFM_SIZE | CFM_STRIKEOUT | CFM_SUBSCRIPT | CFM_SUPERSCRIPT | CFM_UNDERLINE;
	SendMessage(m_hwndEdit, EM_GETCHARFORMAT, TRUE, (LPARAM)&cf2);

	SendMessage(m_hwndTB, TB_CHECKBUTTON, idcBold, MAKELONG((cf2.dwEffects & CFE_BOLD) != 0, 0));
	SendMessage(m_hwndTB, TB_CHECKBUTTON, idcItalic, MAKELONG((cf2.dwEffects & CFE_ITALIC) != 0, 0));
	SendMessage(m_hwndTB, TB_CHECKBUTTON, idcUnderline, MAKELONG((cf2.dwEffects & CFE_UNDERLINE) != 0, 0));
	SendMessage(m_hwndTB, TB_CHECKBUTTON, idcSubscript, MAKELONG((cf2.dwEffects & CFE_SUBSCRIPT) != 0, 0));
	SendMessage(m_hwndTB, TB_CHECKBUTTON, idcSuperscript, MAKELONG((cf2.dwEffects & CFE_SUPERSCRIPT) != 0, 0));
}

void Editor::UpdateParagraphButtons()
{
	PARAFORMAT pf2;
	pf2.cbSize = sizeof(pf2);
	pf2.dwMask = PFM_ALIGNMENT | PFM_NUMBERING;
	SendMessage(m_hwndEdit, EM_GETPARAFORMAT, 0, (LPARAM)&pf2);

	SendMessage(m_hwndTB, TB_CHECKBUTTON, idcAlignLeft, MAKELONG(pf2.wAlignment == PFA_LEFT, 0));
	SendMessage(m_hwndTB, TB_CHECKBUTTON, idcAlignCenter, MAKELONG(pf2.wAlignment == PFA_CENTER, 0));
	SendMessage(m_hwndTB, TB_CHECKBUTTON, idcAlignRight, MAKELONG(pf2.wAlignment == PFA_RIGHT, 0));
	SendMessage(m_hwndTB, TB_CHECKBUTTON, idcAlignJustify, MAKELONG(pf2.wAlignment == PFA_JUSTIFY, 0));
	SendMessage(m_hwndTB, TB_CHECKBUTTON, idcMarksBullet, MAKELONG(pf2.wNumbering == PFN_BULLET, 0));
	SendMessage(m_hwndTB, TB_CHECKBUTTON, idcMarksArabic, MAKELONG(pf2.wNumbering == PFN_ARABIC, 0));
}

void Editor::OnSheetColor(COLORREF cr)
{
	m_crSheet = cr;
	SendMessage(m_hwndEdit, EM_SETBKGNDCOLOR, FALSE, (LPARAM)cr);
}

void Editor::OnCharFormat(const CHARFORMAT& cf, UINT mode)
{
	SendMessage(m_hwndEdit, EM_SETCHARFORMAT, mode, (LPARAM)&cf);
	UpdateCharacterButtons();
}

void Editor::OnParaFormat(const PARAFORMAT& pf)
{
	SendMessage(m_hwndEdit, EM_SETPARAFORMAT, 0, (LPARAM)&pf);
	UpdateParagraphButtons();
}

//-----------------------------------------------------------------------------

// The End.