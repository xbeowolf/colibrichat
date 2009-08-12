
//-----------------------------------------------------------------------------

//
// Includes
//

#pragma region Includes

#include "stdafx.h"

// Windows API
#include <strsafe.h>
#include <richedit.h>

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
	pSource->EvLinkDestroy += MakeDelegate(this, &JClient::JTopic::OnLinkDestroy);
	pSource->EvPageClose += MakeDelegate(this, &JClient::JTopic::OnPageClose);
}

void JClient::JTopic::OnUnhook(JEventable* src)
{
	using namespace fastdelegate;

	pSource->EvLinkEstablished -= MakeDelegate(this, &JClient::JTopic::OnLinkEstablished);
	pSource->EvLinkDestroy -= MakeDelegate(this, &JClient::JTopic::OnLinkDestroy);
	pSource->EvPageClose -= MakeDelegate(this, &JClient::JTopic::OnPageClose);

	__super::OnUnhook(src);
}

void JClient::JTopic::OnLinkEstablished(SOCKET sock)
{
	ASSERT(pSource);
}

void JClient::JTopic::OnLinkDestroy(SOCKET sock)
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
				{IML_FRCOL, rtf::idcFrColor,
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

			hwndTB = GetDlgItem(hWnd, IDC_TOOLBAR);
			hwndEdit = GetDlgItem(hWnd, IDC_SPLASHRTF);

			// Get initial windows sizes
			MapControl(hwndEdit, rcEdit);
			MapControl(IDC_AUTOCLOSE, rcAutoclose);
			MapControl(IDC_AUTOCLOSESPIN, rcAutocloseSpin);
			MapControl(IDOK, rcSend);
			MapControl(IDCANCEL, rcCancel);

			fTransparent = true;
			wCharFormatting = SCF_SELECTION;

			if (!pSource)
			{
				retval = FALSE;
				break;
			}

			// Set main window icons
			SNDMSG(hWnd, WM_SETICON, ICON_SMALL, (LPARAM)JClientApp::jpApp->hiMain16);
			SNDMSG(hWnd, WM_SETICON, ICON_BIG, (LPARAM)JClientApp::jpApp->hiMain32);

			// Inits tool bar
			SendMessage(hwndTB, TB_BUTTONSTRUCTSIZE, (WPARAM)sizeof(TBBUTTON), 0);
			SendMessage(hwndTB, TB_SETBITMAPSIZE, 0, MAKELONG(16, 16));
			SendMessage(hwndTB, TB_SETIMAGELIST, 0, (LPARAM)JClientApp::jpApp->himlEdit);
			SendMessage(hwndTB, TB_ADDBUTTONS, _countof(tbButtons), (LPARAM)&tbButtons);
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
			SendMessage(hwndEdit, EM_SETCHARFORMAT, SCF_ALL, (LPARAM)&cf);
			UpdateCharacterButtons();
			UpdateParagraphButtons();
			SendMessage(hwndTB, TB_CHECKBUTTON, rtf::idcBkMode, MAKELONG(fTransparent, 0));

			SendDlgItemMessage(hWnd, IDC_AUTOCLOSE, EM_LIMITTEXT, 3, 0);
			SendDlgItemMessage(hWnd, IDC_AUTOCLOSESPIN, UDM_SETRANGE, 0, MAKELONG(600, 3));
			SetDlgItemInt(hWnd, IDC_AUTOCLOSE, 30, FALSE);

			SendMessage(hwndEdit, EM_SETEVENTMASK, 0, EN_DRAGDROPDONE | ENM_SELCHANGE);
			SendMessage(hwndEdit, EM_SETBKGNDCOLOR, FALSE, (LPARAM)crSheet);

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
			DeferWindowPos(hdwp, hwndEdit, 0, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, SWP_NOREPOSITION | SWP_NOMOVE | SWP_NOZORDER);
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
			SIZE sMin;
			sMin.cx = rcPageNC.right - rcPageNC.left - rcSend.left + rcAutocloseSpin.right;
			sMin.cy = rcPageNC.bottom - rcPageNC.top - rcEdit.bottom + rcEdit.top + 20;

			MINMAXINFO* mmi = (MINMAXINFO*)lParam;
			mmi->ptMinTrackSize.x = sMin.cx;
			mmi->ptMinTrackSize.y = sMin.cy;
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
					GetClientRect(hwndEdit, &rect);
					MapWindowPoints(hwndEdit, 0, (LPPOINT)&rect, 2);
					BOOL bTranslated;
					DWORD dwCanclose = 2500;
					DWORD dwAutoclose = GetDlgItemInt(hWnd, IDC_AUTOCLOSE, &bTranslated, FALSE)*1000;
					if (!bTranslated || dwCanclose > dwAutoclose) dwAutoclose = dwCanclose;

					SetWindowText(hwndEdit, TEXT(""));

					pSource->Send_Cmd_SPLASHRTF(pSource->m_clientsock, dwWho,
						content.c_str(), rect,
						true, dwCanclose, dwAutoclose, fTransparent, crSheet);
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

CALLBACK JClient::JSplashRtfEditor::JSplashRtfEditor(JClient* p, DWORD who)
: JDialog(), rtf::Editor(), JAttachment<JClient>(p)
{
	dwWho = who;
}

void JClient::JSplashRtfEditor::OnHook(JEventable* src)
{
	using namespace fastdelegate;

	__super::OnHook(src);

	pSource->EvLinkEstablished += MakeDelegate(this, &JClient::JSplashRtfEditor::OnLinkEstablished);
	pSource->EvLinkDestroy += MakeDelegate(this, &JClient::JSplashRtfEditor::OnLinkDestroy);
}

void JClient::JSplashRtfEditor::OnUnhook(JEventable* src)
{
	using namespace fastdelegate;

	pSource->EvLinkEstablished -= MakeDelegate(this, &JClient::JSplashRtfEditor::OnLinkEstablished);
	pSource->EvLinkDestroy -= MakeDelegate(this, &JClient::JSplashRtfEditor::OnLinkDestroy);

	__super::OnUnhook(src);
}

void JClient::JSplashRtfEditor::OnLinkEstablished(SOCKET sock)
{
	ASSERT(pSource);
	EnableWindow(GetDlgItem(m_hwndPage, IDOK), TRUE);
}

void JClient::JSplashRtfEditor::OnLinkDestroy(SOCKET sock)
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

	pSource->EvLinkDestroy += MakeDelegate(this, &JClient::JSplash::OnLinkDestroy);
}

void JClient::JSplash::OnUnhook(JEventable* src)
{
	using namespace fastdelegate;

	pSource->EvLinkDestroy -= MakeDelegate(this, &JClient::JSplash::OnLinkDestroy);

	__super::OnUnhook(src);
}

void JClient::JSplash::OnLinkDestroy(SOCKET sock)
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
				SendMessage(hwndRtf, EM_SETBKGNDCOLOR, FALSE, (LPARAM)crSheet);
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

//-----------------------------------------------------------------------------

// The End.