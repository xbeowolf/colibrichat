
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
#include "stylepr.h"
#include "dCRC.h"
#include "Profile.h"

// Project
#include "..\ColibriProtocol.h"
#include "resource.h"
#include "client.h"

#pragma endregion

using namespace colibrichat;

//-----------------------------------------------------------------------------

//
// JClient::JTopic dialog
//

CALLBACK JClient::JTopic::JTopic(JClient* p, JPageChannel* chan)
: JAttachedDialog<JClient>(p)
{
	ASSERT(p);
	ASSERT(chan);

	jpChannel = chan;

	SetupHooks();
}

LRESULT WINAPI JClient::JTopic::DlgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	LRESULT retval = TRUE;

	switch (message)
	{
	case WM_INITDIALOG:
		{
			if (!pSource)
			{
				retval = FALSE;
				break;
			}

			TCHAR buffer[40];
			GetWindowText(hWnd, buffer, _countof(buffer));
			SetWindowText(hWnd, tformat(TEXT("%s: #%s"), buffer, jpChannel->channel.name.c_str()).c_str());

			SendDlgItemMessage(hWnd, IDC_TOPICTEXT, EM_LIMITTEXT, pSource->m_metrics.uTopicMaxLength, 0);
			SetDlgItemText(hWnd, IDC_TOPICTEXT, jpChannel->channel.topic.c_str());

			retval = TRUE;
			break;
		}

	case WM_DESTROY:
		{
			ResetHooks();
			break;
		}

	case WM_ACTIVATE:
		JClientApp::jpApp->hdlgCurrent = wParam ? hWnd : 0;
		JClientApp::jpApp->haccelCurrent = 0;
		break;

	case WM_COMMAND:
		{
			switch (LOWORD(wParam))
			{
			case IDOK:
				{
					std::tstring topic(pSource->m_metrics.uTopicMaxLength, 0);
					GetDlgItemText(hWnd, IDC_TOPICTEXT, &topic[0], (int)topic.size()+1);
					pSource->Send_Cmd_TOPIC(pSource->m_clientsock, jpChannel->getID(), topic);
					DestroyWindow(hWnd);
					break;
				}

			case IDCANCEL:
				DestroyWindow(hWnd);
				break;

			default: retval = __super::DlgProc(hWnd, message, wParam, lParam);
			}
			break;
		}

		default: retval = __super::DlgProc(hWnd, message, wParam, lParam);
	}
	return retval;
}

void JClient::JTopic::OnHook(JEventable* src)
{
	using namespace fastdelegate;

	__super::OnHook(src);

	pSource->EvLinkEstablished += MakeDelegate(this, &JClient::JTopic::OnLinkEstablished);
	pSource->EvLinkClose += MakeDelegate(this, &JClient::JTopic::OnLinkClose);
	pSource->EvPageClose += MakeDelegate(this, &JClient::JTopic::OnPageClose);
}

void JClient::JTopic::OnUnhook(JEventable* src)
{
	using namespace fastdelegate;

	pSource->EvLinkEstablished -= MakeDelegate(this, &JClient::JTopic::OnLinkEstablished);
	pSource->EvLinkClose -= MakeDelegate(this, &JClient::JTopic::OnLinkClose);
	pSource->EvPageClose -= MakeDelegate(this, &JClient::JTopic::OnPageClose);

	__super::OnUnhook(src);
}

void JClient::JTopic::OnLinkEstablished(SOCKET sock)
{
	ASSERT(pSource);
}

void JClient::JTopic::OnLinkClose(SOCKET sock, UINT err)
{
	ASSERT(pSource);
}

void JClient::JTopic::OnPageClose(DWORD id)
{
	ASSERT(pSource);

	if (m_hwndPage && id == jpChannel->getID())
		SendMessage(m_hwndPage, WM_COMMAND, IDCANCEL, 0);
}

//
// JClient::JSplashRtfEditor dialog
//

// Message handler for "Splash with rich text ..." dialog box.
LRESULT WINAPI JClient::JSplashRtfEditor::DlgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	LRESULT retval = TRUE;

	switch (message)
	{
	case WM_INITDIALOG:
		{
			static const TBBUTTON tbButtons[] =
			{
				{IML_BOLD, rtf::idcBold,
				TBSTATE_ENABLED, TBSTYLE_CHECK,
				{0, 0}, 0, 0},
				{IML_ITALIC, rtf::idcItalic,
				TBSTATE_ENABLED, TBSTYLE_CHECK,
				{0, 0}, 0, 0},
				{IML_ULINE, rtf::idcUnderline,
				TBSTATE_ENABLED, TBSTYLE_CHECK,
				{0, 0}, 0, 0},
				{IML_SUBSCRIPT, rtf::idcSubscript,
				TBSTATE_ENABLED, TBSTYLE_CHECK,
				{0, 0}, 0, 0},
				{IML_SUPERSCRIPT, rtf::idcSuperscript,
				TBSTATE_ENABLED, TBSTYLE_CHECK,
				{0, 0}, 0, 0},
				{IML_FONT, rtf::idcFont,
				TBSTATE_ENABLED, TBSTYLE_BUTTON,
				{0, 0}, 0, 0},
				{0, 0,
				0, TBSTYLE_SEP,
				{0, 0}, 0, 0},
				{IML_FRCOL, rtf::idcFgColor,
				TBSTATE_ENABLED, TBSTYLE_BUTTON,
				{0, 0}, 0, 0},
				{IML_BGCOL, rtf::idcBgColor,
				TBSTATE_ENABLED, TBSTYLE_BUTTON,
				{0, 0}, 0, 0},
				{IML_SHEETCOL, rtf::idcSheetColor,
				TBSTATE_ENABLED, TBSTYLE_BUTTON,
				{0, 0}, 0, 0},
				{0, 0,
				0, TBSTYLE_SEP,
				{0, 0}, 0, 0},
				{IML_LEFT, rtf::idcAlignLeft,
				TBSTATE_ENABLED, TBSTYLE_CHECK,
				{0, 0}, 0, 0},
				{IML_CENTER, rtf::idcAlignCenter,
				TBSTATE_ENABLED, TBSTYLE_CHECK,
				{0, 0}, 0, 0},
				{IML_RIGHT, rtf::idcAlignRight,
				TBSTATE_ENABLED, TBSTYLE_CHECK,
				{0, 0}, 0, 0},
				{IML_JUSTIFY, rtf::idcAlignJustify,
				TBSTATE_ENABLED, TBSTYLE_CHECK,
				{0, 0}, 0, 0},
				{IML_MARKS_BULLET, rtf::idcMarksBullet,
				TBSTATE_ENABLED, TBSTYLE_CHECK,
				{0, 0}, 0, 0},
				{IML_MARKS_ARABIC, rtf::idcMarksArabic,
				TBSTATE_ENABLED, TBSTYLE_CHECK,
				{0, 0}, 0, 0},
				{IML_STARTINDENTINC, rtf::idcStartIndentInc,
				TBSTATE_ENABLED, TBSTYLE_BUTTON,
				{0, 0}, 0, 0},
				{IML_STARTINDENTDEC, rtf::idcStartIndentDec,
				TBSTATE_ENABLED, TBSTYLE_BUTTON,
				{0, 0}, 0, 0},
				{IML_BKMODE, rtf::idcBkMode,
				TBSTATE_ENABLED, TBSTYLE_CHECK,
				{0, 0}, 0, 0},
			};

			m_hwndTB = GetDlgItem(hWnd, IDC_TOOLBAR);
			m_hwndEdit = GetDlgItem(hWnd, IDC_SPLASHRTF);

			// Get initial windows sizes
			MapControl(m_hwndEdit, rcEdit);
			MapControl(IDC_AUTOCLOSE, rcAutoclose);
			MapControl(IDC_AUTOCLOSESPIN, rcAutocloseSpin);
			MapControl(IDOK, rcSend);
			MapControl(IDCANCEL, rcCancel);

			m_fTransparent = true;
			wCharFormatting = SCF_SELECTION;

			if (!pSource) {
				retval = FALSE;
				break;
			}

			// Set main window icons
			SNDMSG(hWnd, WM_SETICON, ICON_SMALL, (LPARAM)JClientApp::jpApp->hiMain16);
			SNDMSG(hWnd, WM_SETICON, ICON_BIG, (LPARAM)JClientApp::jpApp->hiMain32);

			// Inits tool bar
			SendMessage(m_hwndTB, TB_BUTTONSTRUCTSIZE, (WPARAM)sizeof(TBBUTTON), 0);
			SendMessage(m_hwndTB, TB_SETBITMAPSIZE, 0, MAKELONG(16, 16));
			SendMessage(m_hwndTB, TB_SETIMAGELIST, 0, (LPARAM)JClientApp::jpApp->himlEdit);
			SendMessage(m_hwndTB, TB_ADDBUTTONS, _countof(tbButtons), (LPARAM)&tbButtons);
			// Setup font and checks buttons
			CHARFORMAT cf =
			{
				sizeof(cf), // cbSize
				CFM_BOLD | CFM_COLOR | CFM_FACE | CFM_ITALIC | CFM_OFFSET | CFM_SIZE | CFM_UNDERLINE, // dwMask
				0, // dwEffects
				24*20, // yHeight
				0, // yOffset
				RGB(255, 0, 0), // crTextColor
				DEFAULT_CHARSET, // bCharSet
				DEFAULT_PITCH | FF_ROMAN, // bPitchAndFamily
				TEXT("Times New Roman") // szFaceName
			};
			SendMessage(m_hwndEdit, EM_SETCHARFORMAT, SCF_ALL, (LPARAM)&cf);
			UpdateCharacterButtons();
			UpdateParagraphButtons();
			SendMessage(m_hwndTB, TB_CHECKBUTTON, rtf::idcBkMode, MAKELONG(m_fTransparent, 0));

			SendMessage(m_hwndEdit, EM_SETEVENTMASK, 0, EN_DRAGDROPDONE | ENM_SELCHANGE);
			SendMessage(m_hwndEdit, EM_SETBKGNDCOLOR, FALSE, (LPARAM)m_crSheet);

			SendDlgItemMessage(hWnd, IDC_AUTOCLOSE, EM_LIMITTEXT, 3, 0);
			SendDlgItemMessage(hWnd, IDC_AUTOCLOSESPIN, UDM_SETRANGE, 0, MAKELONG(600, 3));
			SetDlgItemInt(hWnd, IDC_AUTOCLOSE, 30, FALSE);

			EnableWindow(GetDlgItem(m_hwndPage, IDOK), pSource->m_clientsock != 0);

			retval = TRUE;
			break;
		}

	case WM_DESTROY:
		{
			ResetHooks();
			break;
		}

	case WM_SIZE:
		{
			if (wParam == SIZE_MINIMIZED) break;
			SendDlgItemMessage(hWnd, IDC_TOOLBAR, TB_AUTOSIZE, 0, 0);
			SendMessage(hWnd, BEM_ADJUSTSIZE, wParam, lParam);
		}

	case BEM_ADJUSTSIZE:
		{
			RECT rc;
			int cx = LOWORD(lParam), cy = HIWORD(lParam);
			HDWP hdwp = BeginDeferWindowPos(5);
			SetRect(&rc, rcEdit.left, rcEdit.top,
				cx - rcPage.right + rcEdit.right,
				cy - rcPage.bottom + rcEdit.bottom);
			DeferWindowPos(hdwp, m_hwndEdit, 0, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, SWP_NOREPOSITION | SWP_NOMOVE | SWP_NOZORDER);
			SetRect(&rc, rcAutoclose.left, cy - rcPage.bottom + rcAutoclose.top,
				rcAutoclose.right,
				cy - rcPage.bottom + rcAutoclose.bottom);
			DeferWindowPos(hdwp, GetDlgItem(hWnd, IDC_AUTOCLOSE), 0, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, SWP_NOREPOSITION | SWP_NOSIZE | SWP_NOZORDER);
			SetRect(&rc, rcAutocloseSpin.left, cy - rcPage.bottom + rcAutocloseSpin.top,
				rcAutocloseSpin.right,
				cy - rcPage.bottom + rcAutocloseSpin.bottom);
			DeferWindowPos(hdwp, GetDlgItem(hWnd, IDC_AUTOCLOSESPIN), 0, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, SWP_NOREPOSITION | SWP_NOSIZE | SWP_NOZORDER);
			SetRect(&rc, cx - rcPage.right + rcSend.left, cy - rcPage.bottom + rcSend.top,
				cx - rcPage.right + rcSend.right,
				cy - rcPage.bottom + rcSend.bottom);
			DeferWindowPos(hdwp, GetDlgItem(hWnd, IDOK), 0, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, SWP_NOREPOSITION | SWP_NOSIZE | SWP_NOZORDER);
			SetRect(&rc, cx - rcPage.right + rcCancel.left, cy - rcPage.bottom + rcCancel.top,
				cx - rcPage.right + rcCancel.right,
				cy - rcPage.bottom + rcCancel.bottom);
			DeferWindowPos(hdwp, GetDlgItem(hWnd, IDCANCEL), 0, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, SWP_NOREPOSITION | SWP_NOSIZE | SWP_NOZORDER);
			EndDeferWindowPos(hdwp);
			break;
		}

	case WM_GETMINMAXINFO:
		{
			MINMAXINFO* mmi = (MINMAXINFO*)lParam;
			mmi->ptMinTrackSize.x = rcPageNC.right - rcPageNC.left - rcSend.left + rcAutocloseSpin.right;
			mmi->ptMinTrackSize.y = rcPageNC.bottom - rcPageNC.top - rcEdit.bottom + rcEdit.top + 20;
			break;
		}

	case WM_ACTIVATE:
		JClientApp::jpApp->hdlgCurrent = wParam ? hWnd : 0;
		JClientApp::jpApp->haccelCurrent = JClientApp::jpApp->haccelRichEdit;
		break;

	case WM_COMMAND:
		{
			switch (LOWORD(wParam))
			{
			case IDOK:
				{
					if (!pSource->m_clientsock) break;
					// Get RTF content
					std::string content;
					getContent(content, SF_RTF);
					// Get window rect
					RECT rect;
					GetClientRect(m_hwndEdit, &rect);
					MapWindowPoints(m_hwndEdit, 0, (LPPOINT)&rect, 2);
					BOOL bTranslated;
					DWORD dwCanclose = 2500;
					DWORD dwAutoclose = GetDlgItemInt(hWnd, IDC_AUTOCLOSE, &bTranslated, FALSE)*1000;
					if (!bTranslated || dwCanclose > dwAutoclose) dwAutoclose = dwCanclose;

					SetWindowText(m_hwndEdit, TEXT(""));

					pSource->Send_Cmd_SPLASHRTF(pSource->m_clientsock, idWho,
						content.c_str(), rect,
						true, dwCanclose, dwAutoclose, m_fTransparent, crSheet);
					break;
				}

			case IDCANCEL:
				DestroyWindow(hWnd);
				break;

			default:
				retval =
					JDialog::DlgProc(hWnd, message, wParam, lParam) ||
					rtf::Editor::DlgProc(hWnd, message, wParam, lParam);
			}
			break;
		}

	case WM_NOTIFY:
		{
			NMHDR* pnmh = (NMHDR*)lParam;
			switch (pnmh->code)
			{
			case TTN_GETDISPINFO:
				{
					TOOLTIPTEXT* lpttt = (TOOLTIPTEXT*)lParam;
					lpttt->hinst = JClientApp::jpApp->hinstApp;
					lpttt->lpszText = (LPTSTR)s_mapButTips[lpttt->hdr.idFrom];
					break;
				}

			default:
				retval =
					JDialog::DlgProc(hWnd, message, wParam, lParam) ||
					rtf::Editor::DlgProc(hWnd, message, wParam, lParam);
			}
			break;
		}

	case WM_INITMENUPOPUP:
		{
			if ((HMENU)wParam == GetSystemMenu(hWnd, FALSE))
			{
				EnableMenuItem((HMENU)wParam, SC_MAXIMIZE, (IsZoomed(hWnd) ? MF_GRAYED : MF_ENABLED) | MF_BYCOMMAND);
				EnableMenuItem((HMENU)wParam, SC_RESTORE, ((!IsIconic(hWnd) && !IsZoomed(hWnd)) ? MF_GRAYED : MF_ENABLED) | MF_BYCOMMAND);
				break;
			}
			retval = FALSE;
			break;
		}

	default:
		retval =
			JDialog::DlgProc(hWnd, message, wParam, lParam) ||
			rtf::Editor::DlgProc(hWnd, message, wParam, lParam);
	}
	return retval;
}

std::map<UINT, const TCHAR*> JClient::JSplashRtfEditor::s_mapButTips;

void JClient::JSplashRtfEditor::initclass()
{
	// Editor toolbar buttons tips
	{
		using namespace rtf;
		s_mapButTips[idcRtfCmd] = MAKEINTRESOURCE(idsRtfCmd);
		s_mapButTips[idcBold] = MAKEINTRESOURCE(idsBold);
		s_mapButTips[idcItalic] = MAKEINTRESOURCE(idsItalic);
		s_mapButTips[idcUnderline] = MAKEINTRESOURCE(idsUnderline);
		s_mapButTips[idcSubscript] = MAKEINTRESOURCE(idsSubscript);
		s_mapButTips[idcSuperscript] = MAKEINTRESOURCE(idsSuperscript);
		s_mapButTips[idcFont] = MAKEINTRESOURCE(idsFont);
		s_mapButTips[idcFgColor] = MAKEINTRESOURCE(idsFgColor);
		s_mapButTips[idcBgColor] = MAKEINTRESOURCE(idsBgColor);
		s_mapButTips[idcSheetColor] = MAKEINTRESOURCE(idsSheetColor);
		s_mapButTips[idcAlignLeft] = MAKEINTRESOURCE(idsAlignLeft);
		s_mapButTips[idcAlignRight] = MAKEINTRESOURCE(idsAlignRight);
		s_mapButTips[idcAlignCenter] = MAKEINTRESOURCE(idsAlignCenter);
		s_mapButTips[idcAlignJustify] = MAKEINTRESOURCE(idsAlignJustify);
		s_mapButTips[idcMarksBullet] = MAKEINTRESOURCE(idsMarksBullet);
		s_mapButTips[idcMarksArabic] = MAKEINTRESOURCE(idsMarksArabic);
		s_mapButTips[idcStartIndentInc] = MAKEINTRESOURCE(idsStartIndentInc);
		s_mapButTips[idcStartIndentDec] = MAKEINTRESOURCE(idsStartIndentDec);
		s_mapButTips[idcBkMode] = MAKEINTRESOURCE(idsBkMode);
	}
}

void JClient::JSplashRtfEditor::doneclass()
{
	s_mapButTips.clear();
}

CALLBACK JClient::JSplashRtfEditor::JSplashRtfEditor(JClient* p, DWORD who)
: JDialog(), rtf::Editor(), JAttachment<JClient>(p)
{
	idWho = who;
}

void JClient::JSplashRtfEditor::OnHook(JEventable* src)
{
	using namespace fastdelegate;

	__super::OnHook(src);

	pSource->EvLinkEstablished += MakeDelegate(this, &JClient::JSplashRtfEditor::OnLinkEstablished);
	pSource->EvLinkClose += MakeDelegate(this, &JClient::JSplashRtfEditor::OnLinkClose);
}

void JClient::JSplashRtfEditor::OnUnhook(JEventable* src)
{
	using namespace fastdelegate;

	pSource->EvLinkEstablished -= MakeDelegate(this, &JClient::JSplashRtfEditor::OnLinkEstablished);
	pSource->EvLinkClose -= MakeDelegate(this, &JClient::JSplashRtfEditor::OnLinkClose);

	__super::OnUnhook(src);
}

void JClient::JSplashRtfEditor::OnLinkEstablished(SOCKET sock)
{
	ASSERT(pSource);
	EnableWindow(GetDlgItem(m_hwndPage, IDOK), TRUE);
}

void JClient::JSplashRtfEditor::OnLinkClose(SOCKET sock, UINT err)
{
	ASSERT(pSource);
	EnableWindow(GetDlgItem(m_hwndPage, IDOK), FALSE);
}

//
// JSplash dialog
//

LRESULT WINAPI JClient::JSplash::DlgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	LRESULT retval = TRUE;
	switch (message)
	{
	case WM_INITDIALOG:
		{
			SetLayeredWindowAttributes(hWnd, GetSysColor(COLOR_BTNFACE), 0, LWA_COLORKEY);

			if (!pSource)
			{
				retval = FALSE;
				break;
			}

			dwStarted = GetTickCount();
			if (m_dwAutoclose != INFINITE)
				VERIFY(SetTimer(hWnd, 100, m_dwAutoclose, 0));

			retval = TRUE;
			break;
		}

	case WM_DESTROY:
		{
			if (m_dwAutoclose != INFINITE)
				VERIFY(KillTimer(hWnd, 100));
			break;
		}

	case WM_SIZE:
		{
			if (wParam == SIZE_MINIMIZED) break;
			SendMessage(hWnd, BEM_ADJUSTSIZE, wParam, lParam);
			break;
		}

	case WM_ACTIVATE:
		JClientApp::jpApp->hdlgCurrent = wParam ? hWnd : 0;
		JClientApp::jpApp->haccelCurrent = 0;
		break;

	case WM_LBUTTONDOWN:
		{
			SNDMSG(hWnd, WM_COMMAND, IDCANCEL, 0);
			break;
		}

	case WM_RBUTTONDOWN:
		{
			SNDMSG(hWnd, WM_COMMAND, IDCANCEL, 0);
			break;
		}

	case WM_COMMAND:
		{
			DWORD time = GetTickCount();
			switch (LOWORD(wParam))
			{
			case IDCANCEL:
				if (time-dwStarted > m_dwCanclose)
				{
					m_result = 0;
					DestroyWindow(hWnd);
				}
				break;

			default: retval = FALSE;
			}
			break;
		}

	case WM_TIMER:
		{
			switch (wParam)
			{
			case 100:
				{
					m_result = 1;
					DestroyWindow(hWnd);
					break;
				}
			}
			break;
		}

	case WM_HELP:
		{
			//WinHelp(hWnd, szHelpFile, HELP_CONTEXTPOPUP, ((LPHELPINFO)lParam)->dwContextId);
			break;
		}

	default: retval = __super::DlgProc(hWnd, message, wParam, lParam);
	}
	return retval;
}

CALLBACK JClient::JSplash::JSplash(JClient* p)
: JAttachedDialog<JClient>(p, false)
{
	m_trnid = 0;
	m_result = 0;

	m_bCloseOnDisconnect = true;
	m_dwCanclose = 2500;
	m_dwAutoclose = 30000;

	dwStarted = GetTickCount();
}

void JClient::JSplash::OnHook(JEventable* src)
{
	using namespace fastdelegate;

	__super::OnHook(src);

	pSource->EvLinkClose += MakeDelegate(this, &JClient::JSplash::OnLinkClose);
}

void JClient::JSplash::OnUnhook(JEventable* src)
{
	using namespace fastdelegate;

	pSource->EvLinkClose -= MakeDelegate(this, &JClient::JSplash::OnLinkClose);

	__super::OnUnhook(src);
}

void JClient::JSplash::OnLinkClose(SOCKET sock, UINT err)
{
	ASSERT(pSource);
	if (m_hwndPage && m_bCloseOnDisconnect) DestroyWindow(m_hwndPage);
}

//
// JSplashRtf dialog
//

LRESULT WINAPI JClient::JSplashRtf::DlgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	LRESULT retval = TRUE;
	switch (message)
	{
	case WM_INITDIALOG:
		{
			hwndRtf = GetDlgItem(hWnd, IDC_RICHEDIT);

			// Get initial windows sizes
			MapControl(hwndRtf, rcRtf);

			if (!__super::DlgProc(hWnd, message, wParam, lParam))
			{
				retval = FALSE;
				break;
			}

			if (!IsRectEmpty(&rcPos))
			{
				MoveWindow(hWnd,
					rcPos.left, rcPos.top,
					rcPos.right - rcPos.left, rcPos.bottom - rcPos.top,
					FALSE);
			}

			if (m_fTransparent)
				SetWindowLong(hwndRtf, GWL_EXSTYLE, GetWindowLong(hwndRtf, GWL_EXSTYLE) | WS_EX_TRANSPARENT);
			else
				SendMessage(hwndRtf, EM_SETBKGNDCOLOR, FALSE, (LPARAM)m_crSheet);
			SETTEXTEX ste = {ST_DEFAULT, CP_ACP};
			SendMessage(hwndRtf, EM_SETTEXTEX, (WPARAM)&ste, (LPARAM)content.c_str());
			PostMessage(hwndRtf, EM_HIDESELECTION, TRUE, 0);

			retval = TRUE;
			break;
		}

	case BEM_ADJUSTSIZE:
		{
			RECT rc;
			int cx = LOWORD(lParam), cy = HIWORD(lParam);
			HDWP hdwp = BeginDeferWindowPos(1);
			SetRect(&rc, rcRtf.left, rcRtf.top,
				cx - rcPage.right + rcRtf.right,
				cy - rcPage.bottom + rcRtf.bottom);
			DeferWindowPos(hdwp, hwndRtf, 0, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, SWP_NOMOVE | SWP_NOREPOSITION | SWP_NOZORDER);
			EndDeferWindowPos(hdwp);
			break;
		}

	default: retval = __super::DlgProc(hWnd, message, wParam, lParam);
	}
	return retval;
}

CALLBACK JClient::JSplashRtf::JSplashRtf(JClient* p, const char* text, size_t size)
: JClient::JSplash(p), content(text, size)
{
}

//
// JClient::JMessageEditor dialog
//

// Message handler for "Send message" dialog box.
LRESULT WINAPI JClient::JMessageEditor::DlgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	LRESULT retval = TRUE;

	switch (message)
	{
	case WM_INITDIALOG:
		{
			static const TBBUTTON tbButtons[] =
			{
				{IML_BOLD, rtf::idcBold,
				TBSTATE_ENABLED, TBSTYLE_CHECK,
				{0, 0}, 0, 0},
				{IML_ITALIC, rtf::idcItalic,
				TBSTATE_ENABLED, TBSTYLE_CHECK,
				{0, 0}, 0, 0},
				{IML_ULINE, rtf::idcUnderline,
				TBSTATE_ENABLED, TBSTYLE_CHECK,
				{0, 0}, 0, 0},
				{IML_SUBSCRIPT, rtf::idcSubscript,
				TBSTATE_ENABLED, TBSTYLE_CHECK,
				{0, 0}, 0, 0},
				{IML_SUPERSCRIPT, rtf::idcSuperscript,
				TBSTATE_ENABLED, TBSTYLE_CHECK,
				{0, 0}, 0, 0},
				{IML_FONT, rtf::idcFont,
				TBSTATE_ENABLED, TBSTYLE_BUTTON,
				{0, 0}, 0, 0},
				{0, 0,
				0, TBSTYLE_SEP,
				{0, 0}, 0, 0},
				{IML_FRCOL, rtf::idcFgColor,
				TBSTATE_ENABLED, TBSTYLE_BUTTON,
				{0, 0}, 0, 0},
				{IML_BGCOL, rtf::idcBgColor,
				TBSTATE_ENABLED, TBSTYLE_BUTTON,
				{0, 0}, 0, 0},
				{IML_SHEETCOL, rtf::idcSheetColor,
				TBSTATE_ENABLED, TBSTYLE_BUTTON,
				{0, 0}, 0, 0},
				{0, 0,
				0, TBSTYLE_SEP,
				{0, 0}, 0, 0},
				{IML_LEFT, rtf::idcAlignLeft,
				TBSTATE_ENABLED, TBSTYLE_CHECK,
				{0, 0}, 0, 0},
				{IML_CENTER, rtf::idcAlignCenter,
				TBSTATE_ENABLED, TBSTYLE_CHECK,
				{0, 0}, 0, 0},
				{IML_RIGHT, rtf::idcAlignRight,
				TBSTATE_ENABLED, TBSTYLE_CHECK,
				{0, 0}, 0, 0},
				{IML_JUSTIFY, rtf::idcAlignJustify,
				TBSTATE_ENABLED, TBSTYLE_CHECK,
				{0, 0}, 0, 0},
				{IML_MARKS_BULLET, rtf::idcMarksBullet,
				TBSTATE_ENABLED, TBSTYLE_CHECK,
				{0, 0}, 0, 0},
				{IML_MARKS_ARABIC, rtf::idcMarksArabic,
				TBSTATE_ENABLED, TBSTYLE_CHECK,
				{0, 0}, 0, 0},
				{IML_STARTINDENTINC, rtf::idcStartIndentInc,
				TBSTATE_ENABLED, TBSTYLE_BUTTON,
				{0, 0}, 0, 0},
				{IML_STARTINDENTDEC, rtf::idcStartIndentDec,
				TBSTATE_ENABLED, TBSTYLE_BUTTON,
				{0, 0}, 0, 0},
			};

			m_hwndTB = GetDlgItem(hWnd, IDC_TOOLBAR);
			m_hwndEdit = GetDlgItem(hWnd, IDC_RICHEDIT);

			// Get initial windows sizes
			MapControl(m_hwndEdit, rcEdit);
			MapControl(IDC_STATIC1, rcStatic1);
			MapControl(IDC_NICK, rcNick);
			MapControl(IDC_ALERT, rcAlert);
			MapControl(IDOK, rcSend);
			MapControl(IDCANCEL, rcCancel);

			if (!pSource) {
				retval = FALSE;
				break;
			}

			// Set main window icons
			SNDMSG(hWnd, WM_SETICON, ICON_SMALL, (LPARAM)JClientApp::jpApp->hiMain16);
			SNDMSG(hWnd, WM_SETICON, ICON_BIG, (LPARAM)JClientApp::jpApp->hiMain32);

			// Inits tool bar
			SendMessage(m_hwndTB, TB_BUTTONSTRUCTSIZE, (WPARAM)sizeof(TBBUTTON), 0);
			SendMessage(m_hwndTB, TB_SETBITMAPSIZE, 0, MAKELONG(16, 16));
			SendMessage(m_hwndTB, TB_SETIMAGELIST, 0, (LPARAM)JClientApp::jpApp->himlEdit);
			SendMessage(m_hwndTB, TB_ADDBUTTONS, _countof(tbButtons), (LPARAM)&tbButtons);
			// Setup font and checks buttons
			CHARFORMAT cf =
			{
				sizeof(cf), // cbSize
				CFM_BOLD | CFM_COLOR | CFM_FACE | CFM_ITALIC | CFM_OFFSET | CFM_SIZE | CFM_UNDERLINE, // dwMask
				0, // dwEffects
				20*20, // yHeight
				0, // yOffset
				RGB(0, 0, 128), // crTextColor
				DEFAULT_CHARSET, // bCharSet
				DEFAULT_PITCH | FF_ROMAN, // bPitchAndFamily
				TEXT("Times New Roman") // szFaceName
			};
			SendMessage(m_hwndEdit, EM_SETCHARFORMAT, SCF_ALL, (LPARAM)&cf);
			UpdateCharacterButtons();
			UpdateParagraphButtons();
			SendMessage(m_hwndTB, TB_CHECKBUTTON, rtf::idcBkMode, MAKELONG(m_fTransparent, 0));

			SendMessage(m_hwndEdit, EM_SETEVENTMASK, 0, EN_DRAGDROPDONE | ENM_SELCHANGE);
			SendMessage(m_hwndEdit, EM_SETBKGNDCOLOR, FALSE, (LPARAM)m_crSheet);

			EnableWindow(GetDlgItem(m_hwndPage, IDOK), pSource->m_clientsock != 0);

			SendDlgItemMessage(hWnd, IDC_NICK, EM_LIMITTEXT, pSource->metrics.uNickMaxLength, 0);
			SetDlgItemText(hWnd, IDC_NICK, strWho.c_str());

			CheckDlgButton(hWnd, IDC_ALERT, fAlert ? BST_CHECKED : BST_UNCHECKED);

			retval = TRUE;
			break;
		}

	case WM_DESTROY:
		{
			ResetHooks();
			break;
		}

	case WM_SIZE:
		{
			if (wParam == SIZE_MINIMIZED) break;
			SendDlgItemMessage(hWnd, IDC_TOOLBAR, TB_AUTOSIZE, 0, 0);
			SendMessage(hWnd, BEM_ADJUSTSIZE, wParam, lParam);
		}

	case BEM_ADJUSTSIZE:
		{
			RECT rc;
			int cx = LOWORD(lParam), cy = HIWORD(lParam);
			HDWP hdwp = BeginDeferWindowPos(5);
			SetRect(&rc, rcEdit.left, rcEdit.top,
				cx - rcPage.right + rcEdit.right,
				cy - rcPage.bottom + rcEdit.bottom);
			DeferWindowPos(hdwp, m_hwndEdit, 0, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, SWP_NOREPOSITION | SWP_NOMOVE | SWP_NOZORDER);
			SetRect(&rc, rcStatic1.left, cy - rcPage.bottom + rcStatic1.top,
				rcStatic1.right,
				cy - rcPage.bottom + rcStatic1.bottom);
			DeferWindowPos(hdwp, GetDlgItem(hWnd, IDC_STATIC1), 0, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, SWP_NOREPOSITION | SWP_NOSIZE | SWP_NOZORDER);
			SetRect(&rc, rcNick.left, cy - rcPage.bottom + rcNick.top,
				rcNick.right,
				cy - rcPage.bottom + rcNick.bottom);
			DeferWindowPos(hdwp, GetDlgItem(hWnd, IDC_NICK), 0, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, SWP_NOREPOSITION | SWP_NOSIZE | SWP_NOZORDER);
			SetRect(&rc, rcAlert.left, cy - rcPage.bottom + rcAlert.top,
				rcAlert.right,
				cy - rcPage.bottom + rcAlert.bottom);
			DeferWindowPos(hdwp, GetDlgItem(hWnd, IDC_ALERT), 0, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, SWP_NOREPOSITION | SWP_NOSIZE | SWP_NOZORDER);
			SetRect(&rc, cx - rcPage.right + rcSend.left, cy - rcPage.bottom + rcSend.top,
				cx - rcPage.right + rcSend.right,
				cy - rcPage.bottom + rcSend.bottom);
			DeferWindowPos(hdwp, GetDlgItem(hWnd, IDOK), 0, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, SWP_NOREPOSITION | SWP_NOSIZE | SWP_NOZORDER);
			SetRect(&rc, cx - rcPage.right + rcCancel.left, cy - rcPage.bottom + rcCancel.top,
				cx - rcPage.right + rcCancel.right,
				cy - rcPage.bottom + rcCancel.bottom);
			DeferWindowPos(hdwp, GetDlgItem(hWnd, IDCANCEL), 0, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, SWP_NOREPOSITION | SWP_NOSIZE | SWP_NOZORDER);
			EndDeferWindowPos(hdwp);
			break;
		}

	case WM_GETMINMAXINFO:
		{
			MINMAXINFO* mmi = (MINMAXINFO*)lParam;
			mmi->ptMinTrackSize.x = rcPageNC.right - rcPageNC.left - rcSend.left + rcAlert.right;
			mmi->ptMinTrackSize.y = rcPageNC.bottom - rcPageNC.top - rcEdit.bottom + rcEdit.top + 20;
			break;
		}

	case WM_ACTIVATE:
		JClientApp::jpApp->hdlgCurrent = wParam ? hWnd : 0;
		JClientApp::jpApp->haccelCurrent = JClientApp::jpApp->haccelRichEdit;
		break;

	case WM_COMMAND:
		{
			switch (LOWORD(wParam))
			{
			case IDOK:
				{
					if (!pSource->m_clientsock) break;
					// Get RTF content
					std::string content;
					getContent(content, SF_RTF);

					std::tstring nickbuf(pSource->m_metrics.uNickMaxLength, 0), nick;
					const TCHAR* msg;
					GetDlgItemText(hWnd, IDC_NICK, &nickbuf[0], (int)nickbuf.size()+1);
					nick = nickbuf.c_str();
					if (JClient::CheckNick(nick, msg)) { // check content
						pSource->Send_Quest_MESSAGE(pSource->m_clientsock, dCRC(nick.c_str()),
							content.c_str(), fAlert, crSheet);
						DestroyWindow(hWnd);
					} else {
						pSource->DisplayMessage(pSource->jpPageServer->hwndNick, msg, strWho.c_str(), 2);
					}
					break;
				}

			case IDCANCEL:
				DestroyWindow(hWnd);
				break;

			default:
				retval =
					JDialog::DlgProc(hWnd, message, wParam, lParam) ||
					rtf::Editor::DlgProc(hWnd, message, wParam, lParam);
			}
			break;
		}

	case WM_NOTIFY:
		{
			NMHDR* pnmh = (NMHDR*)lParam;
			switch (pnmh->code)
			{
			case TTN_GETDISPINFO:
				{
					TOOLTIPTEXT* lpttt = (TOOLTIPTEXT*)lParam;
					lpttt->hinst = JClientApp::jpApp->hinstApp;
					lpttt->lpszText = (LPTSTR)s_mapButTips[lpttt->hdr.idFrom];
					break;
				}

			default:
				retval =
					JDialog::DlgProc(hWnd, message, wParam, lParam) ||
					rtf::Editor::DlgProc(hWnd, message, wParam, lParam);
			}
			break;
		}

	default:
		retval =
			JDialog::DlgProc(hWnd, message, wParam, lParam) ||
			rtf::Editor::DlgProc(hWnd, message, wParam, lParam);
	}
	return retval;
}

std::map<UINT, const TCHAR*> JClient::JMessageEditor::s_mapButTips;

void JClient::JMessageEditor::initclass()
{
	// Editor toolbar buttons tips
	{
		using namespace rtf;
		s_mapButTips[idcRtfCmd] = MAKEINTRESOURCE(idsRtfCmd);
		s_mapButTips[idcBold] = MAKEINTRESOURCE(idsBold);
		s_mapButTips[idcItalic] = MAKEINTRESOURCE(idsItalic);
		s_mapButTips[idcUnderline] = MAKEINTRESOURCE(idsUnderline);
		s_mapButTips[idcSubscript] = MAKEINTRESOURCE(idsSubscript);
		s_mapButTips[idcSuperscript] = MAKEINTRESOURCE(idsSuperscript);
		s_mapButTips[idcFont] = MAKEINTRESOURCE(idsFont);
		s_mapButTips[idcFgColor] = MAKEINTRESOURCE(idsFgColor);
		s_mapButTips[idcBgColor] = MAKEINTRESOURCE(idsBgColor);
		s_mapButTips[idcSheetColor] = MAKEINTRESOURCE(idsSheetColor);
		s_mapButTips[idcAlignLeft] = MAKEINTRESOURCE(idsAlignLeft);
		s_mapButTips[idcAlignRight] = MAKEINTRESOURCE(idsAlignRight);
		s_mapButTips[idcAlignCenter] = MAKEINTRESOURCE(idsAlignCenter);
		s_mapButTips[idcAlignJustify] = MAKEINTRESOURCE(idsAlignJustify);
		s_mapButTips[idcMarksBullet] = MAKEINTRESOURCE(idsMarksBullet);
		s_mapButTips[idcMarksArabic] = MAKEINTRESOURCE(idsMarksArabic);
		s_mapButTips[idcStartIndentInc] = MAKEINTRESOURCE(idsStartIndentInc);
		s_mapButTips[idcStartIndentDec] = MAKEINTRESOURCE(idsStartIndentDec);
		s_mapButTips[idcBkMode] = MAKEINTRESOURCE(idsBkMode);
	}
}

void JClient::JMessageEditor::doneclass()
{
	s_mapButTips.clear();
}

CALLBACK JClient::JMessageEditor::JMessageEditor(JClient* p, const std::tstring& who, bool alert)
: JDialog(), rtf::Editor(), JAttachment<JClient>(p)
{
	strWho = who;
	fAlert = alert;
}

void JClient::JMessageEditor::OnHook(JEventable* src)
{
	using namespace fastdelegate;

	__super::OnHook(src);

	pSource->EvLinkEstablished += MakeDelegate(this, &JClient::JMessageEditor::OnLinkEstablished);
	pSource->EvLinkClose += MakeDelegate(this, &JClient::JMessageEditor::OnLinkClose);
}

void JClient::JMessageEditor::OnUnhook(JEventable* src)
{
	using namespace fastdelegate;

	pSource->EvLinkEstablished -= MakeDelegate(this, &JClient::JMessageEditor::OnLinkEstablished);
	pSource->EvLinkClose -= MakeDelegate(this, &JClient::JMessageEditor::OnLinkClose);

	__super::OnUnhook(src);
}

void JClient::JMessageEditor::OnLinkEstablished(SOCKET sock)
{
	ASSERT(pSource);
	EnableWindow(GetDlgItem(m_hwndPage, IDOK), TRUE);
}

void JClient::JMessageEditor::OnLinkClose(SOCKET sock, UINT err)
{
	ASSERT(pSource);
	EnableWindow(GetDlgItem(m_hwndPage, IDOK), FALSE);
}

//
// JClient::JMessage dialog
//

// Message handler for "Recieved message" dialog box.
LRESULT WINAPI JClient::JMessage::DlgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	LRESULT retval = TRUE;

	switch (message)
	{
	case WM_INITDIALOG:
		{
			// Get initial windows sizes
			MapControl(IDC_RICHEDIT, rcText);
			MapControl(IDC_RECVTIME, rcTime);
			MapControl(IDOK, rcReply);
			MapControl(IDC_PRIVATETALK, rcPrivate);
			MapControl(IDCANCEL, rcCancel);

			if (!pSource) {
				retval = FALSE;
				break;
			}

			// Set main window icons
			SNDMSG(hWnd, WM_SETICON, ICON_SMALL, (LPARAM)JClientApp::jpApp->hiMain16);
			SNDMSG(hWnd, WM_SETICON, ICON_BIG, (LPARAM)JClientApp::jpApp->hiMain32);

			SetWindowText(hWnd, pSource->getSafeName(m_idWho).c_str());

			SendDlgItemMessage(hWnd, IDC_RICHEDIT, EM_SETBKGNDCOLOR, FALSE, (LPARAM)m_crSheet);
			SetDlgItemTextA(hWnd, IDC_RICHEDIT, m_content.c_str());

			SYSTEMTIME st;
			FileTimeToLocalTime(m_ftRecv, st);
			SendDlgItemMessage(hWnd, IDC_RECVTIME, DTM_SETSYSTEMTIME, GDT_VALID, (LPARAM)&st);

			EnableWindow(GetDlgItem(m_hwndPage, IDOK), pSource->m_clientsock != 0);
			EnableWindow(GetDlgItem(m_hwndPage, IDC_PRIVATETALK), pSource->m_clientsock != 0);

			if (m_fAlert) {
				SetWindowPos(hWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
				if (JClient::s_mapAlert[pSource->m_mUser[pSource->m_idOwn].nStatus].fPlayAlert)
					pSource->PlaySound(JClientApp::jpApp->strWavAlert.c_str());
				if (JClient::s_mapAlert[pSource->m_mUser[pSource->m_idOwn].nStatus].fFlashPageSayPrivate
					&& Profile::GetInt(RF_CLIENT, RK_FLASHPAGESAYPRIVATE, TRUE)
					&& !pSource->m_mUser[pSource->m_idOwn].isOnline)
					FlashWindow(m_hwndPage, TRUE);
			}

			retval = TRUE;
			break;
		}

	case WM_DESTROY:
		{
			ResetHooks();
			break;
		}

	case WM_SIZE:
		{
			if (wParam == SIZE_MINIMIZED) break;
			SendDlgItemMessage(hWnd, IDC_TOOLBAR, TB_AUTOSIZE, 0, 0);
			SendMessage(hWnd, BEM_ADJUSTSIZE, wParam, lParam);
		}

	case BEM_ADJUSTSIZE:
		{
			RECT rc;
			int cx = LOWORD(lParam), cy = HIWORD(lParam);
			HDWP hdwp = BeginDeferWindowPos(5);
			SetRect(&rc, rcText.left, rcText.top,
				cx - rcPage.right + rcText.right,
				cy - rcPage.bottom + rcText.bottom);
			DeferWindowPos(hdwp, GetDlgItem(hWnd, IDC_RICHEDIT), 0, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, SWP_NOREPOSITION | SWP_NOMOVE | SWP_NOZORDER);
			SetRect(&rc, rcTime.left, cy - rcPage.bottom + rcTime.top,
				rcTime.right,
				cy - rcPage.bottom + rcTime.bottom);
			DeferWindowPos(hdwp, GetDlgItem(hWnd, IDC_RECVTIME), 0, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, SWP_NOREPOSITION | SWP_NOSIZE | SWP_NOZORDER);
			SetRect(&rc, cx - rcPage.right + rcReply.left, cy - rcPage.bottom + rcReply.top,
				cx - rcPage.right + rcReply.right,
				cy - rcPage.bottom + rcReply.bottom);
			DeferWindowPos(hdwp, GetDlgItem(hWnd, IDOK), 0, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, SWP_NOREPOSITION | SWP_NOSIZE | SWP_NOZORDER);
			SetRect(&rc, cx - rcPage.right + rcPrivate.left, cy - rcPage.bottom + rcPrivate.top,
				cx - rcPage.right + rcPrivate.right,
				cy - rcPage.bottom + rcPrivate.bottom);
			DeferWindowPos(hdwp, GetDlgItem(hWnd, IDC_PRIVATETALK), 0, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, SWP_NOREPOSITION | SWP_NOSIZE | SWP_NOZORDER);
			SetRect(&rc, cx - rcPage.right + rcCancel.left, cy - rcPage.bottom + rcCancel.top,
				cx - rcPage.right + rcCancel.right,
				cy - rcPage.bottom + rcCancel.bottom);
			DeferWindowPos(hdwp, GetDlgItem(hWnd, IDCANCEL), 0, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, SWP_NOREPOSITION | SWP_NOSIZE | SWP_NOZORDER);
			EndDeferWindowPos(hdwp);
			break;
		}

	case WM_GETMINMAXINFO:
		{
			MINMAXINFO* mmi = (MINMAXINFO*)lParam;
			mmi->ptMinTrackSize.x = rcPageNC.right - rcPageNC.left - rcReply.left + rcTime.right;
			mmi->ptMinTrackSize.y = rcPageNC.bottom - rcPageNC.top - rcText.bottom + rcText.top + 20;
			break;
		}

	case WM_ACTIVATE:
		JClientApp::jpApp->hdlgCurrent = wParam ? hWnd : 0;
		JClientApp::jpApp->haccelCurrent = 0;
		break;

	case WM_COMMAND:
		{
			switch (LOWORD(wParam))
			{
			case IDC_REPLY:
				{
					MapUser::const_iterator iu = pSource->m_mUser.find(m_idWho);
					if (iu == pSource->m_mUser.end()) break;
					if (JClient::s_mapAlert[iu->second.nStatus].fCanMessage) {
						ASSERT(pSource->m_clientsock);
						CreateDialogParam(JClientApp::jpApp->hinstApp, MAKEINTRESOURCE(IDD_MSGSEND), pSource->hwndPage, JClient::JSplashRtfEditor::DlgProcStub, (LPARAM)(JDialog*)new JClient::JMessageEditor(pSource, iu->second.name, false));
						DestroyWindow(hWnd);
					} else pSource->DisplayMessage(m_hwndPage, MAKEINTRESOURCE(IDS_MSG_PRIVATEMESSAGE), pSource->getSafeName(m_idWho).c_str(), 1);
					break;
				}

			case IDC_PRIVATETALK:
				{
					MapUser::const_iterator iu = pSource->m_mUser.find(m_idWho);
					if (iu == pSource->m_mUser.end()) break;
					if (JClient::s_mapAlert[iu->second.nStatus].fCanOpenPrivate) {
						ASSERT(pSource->m_clientsock);
						pSource->Send_Quest_JOIN(pSource->m_clientsock, iu->second.name);
						DestroyWindow(hWnd);
					} else pSource->DisplayMessage(m_hwndPage, MAKEINTRESOURCE(IDS_MSG_PRIVATETALK), pSource->getSafeName(m_idWho).c_str(), 1);
					break;
				}

			case IDCANCEL:
				DestroyWindow(hWnd);
				break;

			default:
				retval = __super::DlgProc(hWnd, message, wParam, lParam);
			}
			break;
		}

	default:
		retval = __super::DlgProc(hWnd, message, wParam, lParam);
	}
	return retval;
}

CALLBACK JClient::JMessage::JMessage(JClient* p)
: JAttachedDialog<JClient>(p)
{
	m_idWho = CRC_ANONYMOUS;

	m_fAlert = false;
	m_bCloseOnDisconnect = false;
	m_crSheet = GetSysColor(COLOR_WINDOW);
}

void JClient::JMessage::OnHook(JEventable* src)
{
	using namespace fastdelegate;

	__super::OnHook(src);

	pSource->EvLinkEstablished += MakeDelegate(this, &JClient::JMessage::OnLinkEstablished);
	pSource->EvLinkClose += MakeDelegate(this, &JClient::JMessage::OnLinkClose);
}

void JClient::JMessage::OnUnhook(JEventable* src)
{
	using namespace fastdelegate;

	pSource->EvLinkEstablished -= MakeDelegate(this, &JClient::JMessage::OnLinkEstablished);
	pSource->EvLinkClose -= MakeDelegate(this, &JClient::JMessage::OnLinkClose);

	__super::OnUnhook(src);
}

void JClient::JMessage::OnLinkEstablished(SOCKET sock)
{
	ASSERT(pSource);
	EnableWindow(GetDlgItem(m_hwndPage, IDC_REPLY), TRUE);
	EnableWindow(GetDlgItem(m_hwndPage, IDC_PRIVATETALK), TRUE);
}

void JClient::JMessage::OnLinkClose(SOCKET sock, UINT err)
{
	ASSERT(pSource);
	if (m_hwndPage && m_bCloseOnDisconnect) DestroyWindow(m_hwndPage);
	else {
		EnableWindow(GetDlgItem(m_hwndPage, IDC_REPLY), FALSE);
		EnableWindow(GetDlgItem(m_hwndPage, IDC_PRIVATETALK), FALSE);
	}
}

//-----------------------------------------------------------------------------

// The End.