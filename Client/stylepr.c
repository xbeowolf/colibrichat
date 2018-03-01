//-----------------------------------------------------------------------------
// Style print.
// (C) Copyright 2005 by Podobashev D. O.
//
//   Style print for rich edit controls code.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------

//
// Includes
//

#pragma region Includes

#include "stdafx.h"

// Windows API
#include <richedit.h>

#include "stylepr.h"

#pragma endregion

//-----------------------------------------------------------------------------

#ifdef __cplusplus
extern "C" {
#endif

	//-----------------------------------------------------------------------------

	//
	// Style print
	//

	static struct {
		const TCHAR* str;
		WORD index;
	} SPI_Font[] = {
		{TEXT("MS Sans Serif"), 0},
		{TEXT("Arial"), 1},
		{TEXT("Arial Black"), 2},
		{TEXT("Comic Sans MS"), 3},
		{TEXT("Courier"), 4},
		{TEXT("Courier New"), 5},
		{TEXT("Fixedsys"), 6},
		{TEXT("Impact"), 7},
		{TEXT("Lucida Console"), 8},
		{TEXT("Marlett"), 9},
		{TEXT("MS Sans Serif"), 10},
		{TEXT("Symbol"), 11},
		{TEXT("System"), 12},
		{TEXT("Tahoma"), 13},
		{TEXT("Terminal"), 14},
		{TEXT("Times New Roman"), 15},
		{TEXT("Verdana"), 16},
		{TEXT("Webdings"), 17},
		{TEXT("Wingdings"), 18}
	};

	static int CALLBACK FindOpenBracket(const TCHAR* str)
	{
		int i = 0;
		while (*(str + i) && *(str + i) != TEXT('[')) i++;
		return i;
	}

	static int CALLBACK FindCloseBracket(const TCHAR* str)
	{
		int i = 0;
		if (*str == TEXT('[')) i++;
		while (*(str + i) && *(str + i) != TEXT(']') && *(str + i) != TEXT('[')) i++;
		return i;
	}

	static int CALLBACK SkipSpace(const TCHAR* str)
	{
		int pos = 0;
		while (*(str + pos) && *(str + pos) == TEXT(' ')) pos++;
		return pos;
	}

	UINT CALLBACK StylePrint(const TCHAR* str, STYLEFIELD* sf, const TCHAR** styledescr)
	{
		int src_pos = 0, dest_pos = 0, src_len;
		int in_pos = 0, out_pos, pos;
		int style = -1, color = -1, font = -1, size = -1, offset = -1,
			bold = -1, italic = -1, uline = -1, strout = -1, subscript = -1, superscript = -1;
		int loop, count;
		BOOL opened;

		if (IsBadStringPtr(str, 0xFFFF) || IsBadWritePtr(sf, sizeof(*sf)) ||
			IsBadWritePtr(sf->sr, sf->sr_num*sizeof(*sf->sr))) return FALSE;
		sf->sr_i = 0;
		while (sf->sr_i < sf->sr_num && dest_pos < sf->len && *(str + in_pos)) {
			opened = TRUE;
			in_pos = src_pos + FindOpenBracket(str + src_pos);
			out_pos = in_pos + FindCloseBracket(str + in_pos);
			if (*(str + in_pos) && *(str + out_pos) == TEXT(']')) {
				pos = in_pos + 1;
				pos += SkipSpace(str + pos);
				count = 0;
				src_len = min(sf->len - dest_pos - 1, in_pos - src_pos);
				if (src_len > 0) {
					MoveMemory(sf->str + dest_pos, str + src_pos, src_len*sizeof(TCHAR));
					dest_pos += src_len;
				}
				src_pos = out_pos + 1;
				do {
					loop = 0;
					// Determine tag closing
					if (*(str + pos) == TEXT('/')) {
						pos++;
						pos += SkipSpace(str + pos);
						opened = FALSE;
					}
					// Style tag
					if (loop != 1 && !_tcsnicmp(str + pos, TEXT("style"), 5) &&
						!IsCharAlphaNumeric(*(str + pos + 5))) {
							pos += 5;
							pos += SkipSpace(str + pos);
							loop = 1, count++;
							if (style >= 0) { // Close prev. style region
								sf->sr[style].end = dest_pos;
								style = -1;
							}
							if (opened) { // Read content of style open index
								int i;
								style = sf->sr_i;
								sf->sr_i++;
								sf->sr[style].start = dest_pos;
								if (*(str + pos) == TEXT('=')) {
									pos++;
									pos += SkipSpace(str + pos);
									if (styledescr) {
										for (i = 0; styledescr[i*2] && _tcsnicmp(str + pos, styledescr[i*2], min(lstrlen(styledescr[i*2]), 4)); i++);
										sf->sr[style].index = SPF_STYLE | (styledescr[i*2] ? i : min(max(_ttoi(str + pos), 0), i - 1));
									} else sf->sr[style].index = SPF_STYLE | 0;
									// Skip definition
									while (IsCharAlphaNumeric(*(str + pos))) pos++;
								} else sf->sr[style].index = SPF_STYLE | 0;
							}
							if (*(str + pos) == TEXT(',') || *(str + pos) == TEXT(';')) pos++, loop = 2;
							pos += SkipSpace(str + pos);
					}
					// Color tag
					if (loop != 1 && !_tcsnicmp(str + pos, TEXT("color"), 5) &&
						!IsCharAlphaNumeric(*(str + pos + 5))) {
							pos += 5;
							pos += SkipSpace(str + pos);
							loop = 1, count++;
							if (color >= 0) { // Close prev. color region
								sf->sr[color].end = dest_pos;
								color = -1;
							}
							if (opened) { // Read content of color open index
								static struct {
									const TCHAR* str;
									COLORREF color;
								} Color[] = {
									{TEXT("maroon"), RGB(160, 64, 64)},
									{TEXT("darkred"), RGB(192, 0, 0)},
									{TEXT("red"), RGB(255, 0, 0)},
									{TEXT("crimson"), RGB(255, 0, 128)},
									{TEXT("orange"), RGB(255, 128, 0)},
									{TEXT("yellow"), RGB(255, 255, 0)},
									{TEXT("olive"), RGB(128, 128, 0)},
									{TEXT("brown"), RGB(128, 64, 0)},
									{TEXT("green"), RGB(0, 128, 0)},
									{TEXT("lime"), RGB(0, 255, 0)},
									{TEXT("salad"), RGB(128, 255, 128)},
									{TEXT("aqua"), RGB(128, 255, 255)},
									{TEXT("cyan"), RGB(0, 255, 255)},
									{TEXT("teal"), RGB(64, 128, 128)},
									{TEXT("blue"), RGB(0, 0, 255)},
									{TEXT("navy"), RGB(0, 0, 128)},
									{TEXT("purple"), RGB(128, 0, 128)},
									{TEXT("indigo"), RGB(192, 0, 192)},
									{TEXT("lilac"), RGB(192, 0, 255)},
									{TEXT("violet"), RGB(255, 0, 255)},
									{TEXT("fuchsia"), RGB(255, 128, 255)},
									{TEXT("white"), RGB(255, 255, 255)},
									{TEXT("silver"), RGB(192, 192, 192)},
									{TEXT("grey"), RGB(128, 128, 128)},
									{TEXT("black"), RGB(0, 0, 0)}
								};
								int i;
								color = sf->sr_i;
								sf->sr_i++;
								sf->sr[color].start = dest_pos;
								if (*(str + pos) == TEXT('=')) {
									pos++;
									pos += SkipSpace(str + pos);
									if (_tcsnicmp(str + pos, TEXT("auto"), 4)) {
										for (i = 0; i < _countof(Color) && _tcsnicmp(str + pos, Color[i].str, lstrlen(Color[i].str)); i++);
										sf->sr[color].index = SPF_COLOR | (i < _countof(Color) ? Color[i].color : _ttol(str + pos) & SPF_INDEX);
									} else sf->sr[color].index = SPF_AUTOCOLOR;
									// Skip definition
									while (IsCharAlphaNumeric(*(str + pos))) pos++;
								} else sf->sr[color].index = SPF_AUTOCOLOR;
							}
							if (*(str + pos) == TEXT(',') || *(str + pos) == TEXT(';')) pos++, loop = 2;
							pos += SkipSpace(str + pos);
					}
					// Font tag
					if (loop != 1 && !_tcsnicmp(str + pos, TEXT("font"), 4) &&
						!IsCharAlphaNumeric(*(str + pos + 4))) {
							pos += 4;
							pos += SkipSpace(str + pos);
							loop = 1, count++;
							if (font >= 0) { // Close prev. font region
								sf->sr[font].end = dest_pos;
								font = -1;
							}
							if (opened) { // Read content of font open index
								int i;
								font = sf->sr_i;
								sf->sr_i++;
								sf->sr[font].start = dest_pos;
								if (*(str + pos) == TEXT('=')) {
									pos++;
									pos += SkipSpace(str + pos);
									if (_tcsnicmp(str + pos, TEXT("system"), 3)) {
										for (i = 0; i < _countof(SPI_Font) && _tcsnicmp(str + pos, SPI_Font[i].str, lstrlen(SPI_Font[i].str)); i++);
										sf->sr[font].index = SPF_FONT | (i < _countof(SPI_Font) ? SPI_Font[i].index : _ttoi(str + pos) & SPF_INDEX);
										// Skip definition
										if (i < _countof(SPI_Font)) pos += lstrlen(SPI_Font[i].str);
										else while (IsCharAlphaNumeric(*(str + pos))) pos++;
									} else {
										sf->sr[font].index = SPF_FONT | 0;
										// Skip definition
										while (IsCharAlphaNumeric(*(str + pos))) pos++;
									}
								} else sf->sr[font].index = SPF_FONT | 0;
							}
							if (*(str + pos) == TEXT(',') || *(str + pos) == TEXT(';')) pos++, loop = 2;
							pos += SkipSpace(str + pos);
					}
					// Size tag
					if (loop != 1 && !_tcsnicmp(str + pos, TEXT("size"), 4) &&
						!IsCharAlphaNumeric(*(str + pos + 4))) {
							pos += 4;
							pos += SkipSpace(str + pos);
							loop = 1, count++;
							if (size >= 0) { // Close prev. size region
								sf->sr[size].end = dest_pos;
								size = -1;
							}
							if (opened) { // Read content of size open index
								size = sf->sr_i;
								sf->sr_i++;
								sf->sr[size].start = dest_pos;
								if (*(str + pos) == TEXT('=')) {
									pos++;
									pos += SkipSpace(str + pos);
									sf->sr[size].index = SPF_SIZE | (short)_ttoi(str + pos);
									// Skip definition
									while (IsCharAlphaNumeric(*(str + pos))) pos++;
								} else sf->sr[size].index = SPF_SIZE | 10;
							}
							if (*(str + pos) == TEXT(',') || *(str + pos) == TEXT(';')) pos++, loop = 2;
							pos += SkipSpace(str + pos);
					}
					// Offset tag
					if (loop != 1 && !_tcsnicmp(str + pos, TEXT("offset"), 6) &&
						!IsCharAlphaNumeric(*(str + pos + 6))) {
							pos += 6;
							pos += SkipSpace(str + pos);
							loop = 1, count++;
							if (offset >= 0) { // Close prev. offset region
								sf->sr[offset].end = dest_pos;
								offset = -1;
							}
							if (opened) { // Read content of offset open index
								offset = sf->sr_i;
								sf->sr_i++;
								sf->sr[offset].start = dest_pos;
								if (*(str + pos) == TEXT('=')) {
									pos++;
									pos += SkipSpace(str + pos);
									sf->sr[offset].index = SPF_OFFSET | (short)_ttoi(str + pos);
									// Skip definition
									if (*(str + pos) == TEXT('-') || *(str + pos) == TEXT('+')) pos++;
								} else sf->sr[offset].index = SPF_OFFSET | 0;
								while (IsCharAlphaNumeric(*(str + pos))) pos++;
							}
							if (*(str + pos) == TEXT(',') || *(str + pos) == TEXT(';')) pos++, loop = 2;
							pos += SkipSpace(str + pos);
					}
					// Bold tag
					if (loop != 1 && !_tcsnicmp(str + pos, TEXT("b"), 1) &&
						!IsCharAlphaNumeric(*(str + pos + 1))) {
							pos += 1;
							pos += SkipSpace(str + pos);
							loop = 1, count++;
							if (bold >= 0) { // Close prev. bold region
								sf->sr[bold].end = dest_pos;
								bold = -1;
							}
							if (opened) { // Read content of bold open index
								static struct {
									const TCHAR* str;
									int value;
								} BoolStr[] = {
									{TEXT("false"), FALSE},
									{TEXT("true"), TRUE}
								};
								int i;
								bold = sf->sr_i;
								sf->sr_i++;
								sf->sr[bold].start = dest_pos;
								if (*(str + pos) == TEXT('=')) {
									pos++;
									pos += SkipSpace(str + pos);
									for (i = 0; i < _countof(BoolStr) && _tcsnicmp(str + pos, BoolStr[i].str, lstrlen(BoolStr[i].str)); i++);
									sf->sr[bold].index = SPF_BOLD | (i < _countof(BoolStr) ? BoolStr[i].value : _ttoi(str + pos) & SPF_INDEX);
									// Skip definition
									while (IsCharAlphaNumeric(*(str + pos))) pos++;
								} else sf->sr[bold].index = SPF_BOLD | TRUE;
							}
							if (*(str + pos) == TEXT(',') || *(str + pos) == TEXT(';')) pos++, loop = 2;
							pos += SkipSpace(str + pos);
					}
					// Italic tag
					if (loop != 1 && !_tcsnicmp(str + pos, TEXT("i"), 1) &&
						!IsCharAlphaNumeric(*(str + pos + 1))) {
							pos += 1;
							pos += SkipSpace(str + pos);
							loop = 1, count++;
							if (italic >= 0) { // Close prev. italic region
								sf->sr[italic].end = dest_pos;
								italic = -1;
							}
							if (opened) { // Read content of italic open index
								static struct {
									const TCHAR* str;
									int value;
								} BoolStr[] = {
									{TEXT("false"), FALSE},
									{TEXT("true"), TRUE}
								};
								int i;
								italic = sf->sr_i;
								sf->sr_i++;
								sf->sr[italic].start = dest_pos;
								if (*(str + pos) == TEXT('=')) {
									pos++;
									pos += SkipSpace(str + pos);
									for (i = 0; i < _countof(BoolStr) && _tcsnicmp(str + pos, BoolStr[i].str, lstrlen(BoolStr[i].str)); i++);
									sf->sr[italic].index = SPF_ITALIC | (i < _countof(BoolStr) ? BoolStr[i].value : _ttoi(str + pos) & SPF_INDEX);
									// Skip definition
									while (IsCharAlphaNumeric(*(str + pos))) pos++;
								} else sf->sr[italic].index = SPF_ITALIC | TRUE;
							}
							if (*(str + pos) == TEXT(',') || *(str + pos) == TEXT(';')) pos++, loop = 2;
							pos += SkipSpace(str + pos);
					}
					// Underline tag
					if (loop != 1 && !_tcsnicmp(str + pos, TEXT("u"), 1) &&
						!IsCharAlphaNumeric(*(str + pos + 1))) {
							pos += 1;
							pos += SkipSpace(str + pos);
							loop = 1, count++;
							if (uline >= 0) { // Close prev. underline region
								sf->sr[uline].end = dest_pos;
								uline = -1;
							}
							if (opened) { // Read content of underline open index
								static struct {
									const TCHAR* str;
									int value;
								} BoolStr[] = {
									{TEXT("false"), FALSE},
									{TEXT("true"), TRUE}
								};
								int i;
								uline = sf->sr_i;
								sf->sr_i++;
								sf->sr[uline].start = dest_pos;
								if (*(str + pos) == TEXT('=')) {
									pos++;
									pos += SkipSpace(str + pos);
									for (i = 0; i < _countof(BoolStr) && _tcsnicmp(str + pos, BoolStr[i].str, lstrlen(BoolStr[i].str)); i++);
									sf->sr[uline].index = SPF_ULINE | (i < _countof(BoolStr) ? BoolStr[i].value : _ttoi(str + pos) & SPF_INDEX);
									// Skip definition
									while (IsCharAlphaNumeric(*(str + pos))) pos++;
								} else sf->sr[uline].index = SPF_ULINE | TRUE;
							}
							if (*(str + pos) == TEXT(',') || *(str + pos) == TEXT(';')) pos++, loop = 2;
							pos += SkipSpace(str + pos);
					}
					// Strikeout tag
					if (loop != 1 && !_tcsnicmp(str + pos, TEXT("s"), 1) &&
						!IsCharAlphaNumeric(*(str + pos + 1))) {
							pos += 1;
							pos += SkipSpace(str + pos);
							loop = 1, count++;
							if (strout >= 0) { // Close prev. strikeout region
								sf->sr[strout].end = dest_pos;
								strout = -1;
							}
							if (opened) { // Read content of strikeout open index
								static struct {
									const TCHAR* str;
									int value;
								} BoolStr[] = {
									{TEXT("false"), FALSE},
									{TEXT("true"), TRUE}
								};
								int i;
								strout = sf->sr_i;
								sf->sr_i++;
								sf->sr[strout].start = dest_pos;
								if (*(str + pos) == TEXT('=')) {
									pos++;
									pos += SkipSpace(str + pos);
									for (i = 0; i < _countof(BoolStr) && _tcsnicmp(str + pos, BoolStr[i].str, lstrlen(BoolStr[i].str)); i++);
									sf->sr[strout].index = SPF_STROUT | (i < _countof(BoolStr) ? BoolStr[i].value : _ttoi(str + pos) & SPF_INDEX);
									// Skip definition
									while (IsCharAlphaNumeric(*(str + pos))) pos++;
								} else sf->sr[strout].index = SPF_STROUT | TRUE;
							}
							if (*(str + pos) == TEXT(',') || *(str + pos) == TEXT(';')) pos++, loop = 2;
							pos += SkipSpace(str + pos);
					}
					// Subscript tag
					if (loop != 1 && !_tcsnicmp(str + pos, TEXT("l"), 1) &&
						!IsCharAlphaNumeric(*(str + pos + 1))) {
							pos += 1;
							pos += SkipSpace(str + pos);
							loop = 1, count++;
							if (subscript >= 0) { // Close prev. subscript region
								sf->sr[subscript].end = dest_pos;
								subscript = -1;
							}
							if (opened) { // Read content of subscript open index
								static struct {
									const TCHAR* str;
									int value;
								} BoolStr[] = {
									{TEXT("false"), FALSE},
									{TEXT("true"), TRUE}
								};
								int i;
								subscript = sf->sr_i;
								sf->sr_i++;
								sf->sr[subscript].start = dest_pos;
								if (*(str + pos) == TEXT('=')) {
									pos++;
									pos += SkipSpace(str + pos);
									for (i = 0; i < _countof(BoolStr) && _tcsnicmp(str + pos, BoolStr[i].str, lstrlen(BoolStr[i].str)); i++);
									sf->sr[subscript].index = SPF_SUBSCRIPT | (i < _countof(BoolStr) ? BoolStr[i].value : _ttoi(str + pos) & SPF_INDEX);
									// Skip definition
									while (IsCharAlphaNumeric(*(str + pos))) pos++;
								} else sf->sr[subscript].index = SPF_SUBSCRIPT | TRUE;
							}
							if (*(str + pos) == TEXT(',') || *(str + pos) == TEXT(';')) pos++, loop = 2;
							pos += SkipSpace(str + pos);
					}
					// Superscript tag
					if (loop != 1 && !_tcsnicmp(str + pos, TEXT("h"), 1) &&
						!IsCharAlphaNumeric(*(str + pos + 1))) {
							pos += 1;
							pos += SkipSpace(str + pos);
							loop = 1, count++;
							if (superscript >= 0) { // Close prev. superscript region
								sf->sr[superscript].end = dest_pos;
								superscript = -1;
							}
							if (opened) { // Read content of superscript open index
								static struct {
									const TCHAR* str;
									int value;
								} BoolStr[] = {
									{TEXT("false"), FALSE},
									{TEXT("true"), TRUE}
								};
								int i;
								superscript = sf->sr_i;
								sf->sr_i++;
								sf->sr[superscript].start = dest_pos;
								if (*(str + pos) == TEXT('=')) {
									pos++;
									pos += SkipSpace(str + pos);
									for (i = 0; i < _countof(BoolStr) && _tcsnicmp(str + pos, BoolStr[i].str, lstrlen(BoolStr[i].str)); i++);
									sf->sr[superscript].index = SPF_SUPERSCRIPT | (i < _countof(BoolStr) ? BoolStr[i].value : _ttoi(str + pos) & SPF_INDEX);
									// Skip definition
									while (IsCharAlphaNumeric(*(str + pos))) pos++;
								} else sf->sr[superscript].index = SPF_SUPERSCRIPT | TRUE;
							}
							if (*(str + pos) == TEXT(',') || *(str + pos) == TEXT(';')) pos++, loop = 2;
							pos += SkipSpace(str + pos);
					}
				} while (loop > 1);
				if (!count) {
					src_len = min(sf->len - dest_pos - 1, out_pos - in_pos + 1);
					if (src_len > 0) {
						MoveMemory(sf->str + dest_pos, str + in_pos, src_len*sizeof(TCHAR));
						dest_pos += src_len;
					}
				}
			}
		}
		// Copy the end of line
		src_len = min(sf->len - dest_pos - 1, in_pos - src_pos);
		if (src_len > 0) {
			MoveMemory(sf->str + dest_pos, str + src_pos, src_len*sizeof(TCHAR));
			dest_pos += src_len;
		}
		sf->str[dest_pos] = 0;
		if (style >= 0) { // Close style region till the end of line
			sf->sr[style].end = dest_pos;
		}
		if (color >= 0) { // Close color region till the end of line
			sf->sr[color].end = dest_pos;
		}
		if (font >= 0) { // Close font region till the end of line
			sf->sr[font].end = dest_pos;
		}
		if (size >= 0) { // Close size region till the end of line
			sf->sr[size].end = dest_pos;
		}
		if (offset >= 0) { // Close offset region till the end of line
			sf->sr[offset].end = dest_pos;
		}
		if (bold >= 0) { // Close bold region till the end of line
			sf->sr[bold].end = dest_pos;
		}
		if (italic >= 0) { // Close italic region till the end of line
			sf->sr[italic].end = dest_pos;
		}
		if (uline >= 0) { // Close underline region till the end of line
			sf->sr[uline].end = dest_pos;
		}
		if (strout >= 0) { // Close strikeout region till the end of line
			sf->sr[strout].end = dest_pos;
		}
		if (subscript >= 0) { // Close strikeout region till the end of line
			sf->sr[subscript].end = dest_pos;
		}
		if (superscript >= 0) { // Close strikeout region till the end of line
			sf->sr[superscript].end = dest_pos;
		}
		return TRUE;
	}

	BOOL CALLBACK StylePrint_Apply(HWND hwndRE, STYLEFIELD* sf, const CHARFORMAT* style, UINT cfcount, int offset)
	{
		UINT i;
		static CHARFORMAT2 cf;
		cf.cbSize = sizeof(CHARFORMAT2);

		if ((style && IsBadReadPtr(style, cfcount*sizeof(*style))) || IsBadReadPtr(sf, sizeof(*sf)) ||
			IsBadReadPtr(sf->sr, sf->sr_num*sizeof(*sf->sr))) return FALSE;
		for (i = 0; i < sf->sr_i; i++) {
			if (sf->sr[i].start >= sf->sr[i].end) continue;
			SendMessage(hwndRE, EM_SETSEL, offset + sf->sr[i].start, offset + sf->sr[i].end);
			switch (sf->sr[i].index & SPF_MASK) {
			case SPF_STYLE:
				{
					if (style) {
						MoveMemory(&cf, &style[min(sf->sr[i].index & SPF_INDEX, cfcount - 1)], sizeof(*style));
					}
					break;
				}

			case SPF_COLOR:
				{
					cf.dwMask = CFM_COLOR;
					cf.crTextColor = sf->sr[i].index & SPF_INDEX;
					break;
				}

			case SPF_AUTOCOLOR:
				{
					cf.dwMask = CFM_COLOR;
					cf.dwEffects = CFE_AUTOCOLOR;
					cf.crTextColor = sf->sr[i].index & SPF_INDEX;
					break;
				}

			case SPF_FONT:
				{
					int j;
					for (j = 0; (j < _countof(SPI_Font)) && (SPI_Font[j].index != (int)(sf->sr[i].index & SPF_INDEX)); j++);
					if (j >= _countof(SPI_Font)) j = 0;
					cf.dwMask = CFM_FACE;
					lstrcpyn(cf.szFaceName, SPI_Font[j].str, _countof(cf.szFaceName));
					break;
				}

			case SPF_SIZE:
				{
					cf.dwMask = CFM_SIZE;
					cf.yHeight = (sf->sr[i].index & SPF_INDEX)*20;
					break;
				}

			case SPF_OFFSET:
				{
					cf.dwMask = CFM_OFFSET;
					cf.yOffset = (int)(sf->sr[i].index & SPF_INDEX)*20;
					break;
				}

			case SPF_BOLD:
				{
					cf.dwMask = CFM_BOLD;
					cf.dwEffects = (sf->sr[i].index & SPF_INDEX) ? CFE_BOLD : 0;
					break;
				}

			case SPF_ITALIC:
				{
					cf.dwMask = CFM_ITALIC;
					cf.dwEffects = (sf->sr[i].index & SPF_INDEX) ? CFE_ITALIC : 0;
					break;
				}

			case SPF_ULINE:
				{
					cf.dwMask = CFM_UNDERLINE;
					cf.dwEffects = (sf->sr[i].index & SPF_INDEX) ? CFE_UNDERLINE : 0;
					break;
				}

			case SPF_STROUT:
				{
					cf.dwMask = CFM_STRIKEOUT;
					cf.dwEffects = (sf->sr[i].index & SPF_INDEX) ? CFE_STRIKEOUT : 0;
					break;
				}

			case SPF_SUBSCRIPT:
				{
					cf.dwMask = CFM_SUBSCRIPT;
					cf.dwEffects = (sf->sr[i].index & SPF_INDEX) ? CFE_SUBSCRIPT : 0;
					break;
				}

			case SPF_SUPERSCRIPT:
				{
					cf.dwMask = CFM_SUPERSCRIPT;
					cf.dwEffects = (sf->sr[i].index & SPF_INDEX) ? CFE_SUPERSCRIPT : 0;
					break;
				}
			}
			SendMessage(hwndRE, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cf);
		}
		return TRUE;
	}

	BOOL CALLBACK StylePrint_Append(HWND hwndRE, const TCHAR* str, STYLEFIELD* sf,
		const TCHAR** styledescr, const CHARFORMAT* style, UINT cfcount)
	{
		long offset;
		if (IsBadStringPtr(str, 0xFFFF) ||
			(style && IsBadReadPtr(style, cfcount*sizeof(*style))) || IsBadReadPtr(sf, sizeof(*sf)) ||
			IsBadReadPtr(sf->sr, sf->sr_num*sizeof(*sf->sr))) return FALSE;
		StylePrint(str, sf, styledescr);
		SendMessage(hwndRE, EM_SETSEL, -1, -1),
			SendMessage(hwndRE, EM_GETSEL, 0, (LPARAM)&offset),
			SendMessage(hwndRE, EM_REPLACESEL, FALSE, (LPARAM)sf->str);
		StylePrint_Apply(hwndRE, sf, style, cfcount, offset);
		return TRUE;
	}

	//-----------------------------------------------------------------------------

#ifdef __cplusplus
}
#endif

//-----------------------------------------------------------------------------

// The End.