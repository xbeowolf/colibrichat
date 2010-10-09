
//-----------------------------------------------------------------------------

//
// Includes
//

#pragma region Includes

#include "stdafx.h"

// Common
#include "stylepr.h"
#include "CRC.h"
#include "Profile.h"

// Project
#include "..\ColibriProtocol.h"
#include "resource.h"
#include "client.h"

#pragma endregion

//-----------------------------------------------------------------------------

using namespace colibrichat;

//-----------------------------------------------------------------------------

void JClient::DoHelper()
{
	TCHAR title[4][80], subtitle[4][256];
	LoadString(JClientApp::jpApp->hinstApp, IDS_QSW_QSW, title[0], _countof(title[0]));
	LoadString(JClientApp::jpApp->hinstApp, IDS_QSW_INTRO, subtitle[0], _countof(subtitle[0]));
	LoadString(JClientApp::jpApp->hinstApp, IDS_QSW_STEP1, title[1], _countof(title[1]));
	LoadString(JClientApp::jpApp->hinstApp, IDS_QSW_NICK, subtitle[1], _countof(subtitle[1]));
	LoadString(JClientApp::jpApp->hinstApp, IDS_QSW_STEP2, title[2], _countof(title[2]));
	LoadString(JClientApp::jpApp->hinstApp, IDS_QSW_PASS, subtitle[2], _countof(subtitle[2]));
	LoadString(JClientApp::jpApp->hinstApp, IDS_QSW_STEP3, title[3], _countof(title[3]));
	LoadString(JClientApp::jpApp->hinstApp, IDS_QSW_HOST, subtitle[3], _countof(subtitle[3]));

	PROPSHEETPAGE psp[] = {
		{
			sizeof(PROPSHEETPAGE), // dwSize
			PSP_USEHEADERTITLE | PSP_USEHEADERSUBTITLE | PSP_USEICONID, // dwFlags
			JClientApp::jpApp->hinstApp, // hInstance
			MAKEINTRESOURCE(IDD_HELPER0), // pszTemplate
			(HICON)MAKEINTRESOURCE(0), // pszIcon
			0, // pszTitle
			(DLGPROC)JClient::DlgProcHelper0, // pfnDlgProc
			(LPARAM)this, // lParam
			0, // pfnCallback
			0, // pcRefParent
			title[0], // pszHeaderTitle
			subtitle[0], // pszHeaderSubTitle
		},
		{
			sizeof(PROPSHEETPAGE), // dwSize
			PSP_USEHEADERTITLE | PSP_USEHEADERSUBTITLE | PSP_USEICONID, // dwFlags
			JClientApp::jpApp->hinstApp, // hInstance
			MAKEINTRESOURCE(IDD_HELPER1), // pszTemplate
			(HICON)MAKEINTRESOURCE(0), // pszIcon
			0, // pszTitle
			(DLGPROC)JClient::DlgProcHelper1, // pfnDlgProc
			(LPARAM)this, // lParam
			0, // pfnCallback
			0, // pcRefParent
			title[1], // pszHeaderTitle
			subtitle[1], // pszHeaderSubTitle
		},
		{
			sizeof(PROPSHEETPAGE), // dwSize
			PSP_USEHEADERTITLE | PSP_USEHEADERSUBTITLE | PSP_USEICONID, // dwFlags
			JClientApp::jpApp->hinstApp, // hInstance
			MAKEINTRESOURCE(IDD_HELPER2), // pszTemplate
			(HICON)MAKEINTRESOURCE(0), // pszIcon
			0, // pszTitle
			(DLGPROC)JClient::DlgProcHelper2, // pfnDlgProc
			(LPARAM)this, // lParam
			0, // pfnCallback
			0, // pcRefParent
			title[2], // pszHeaderTitle
			subtitle[2], // pszHeaderSubTitle
		},
		{
			sizeof(PROPSHEETPAGE), // dwSize
			PSP_USEHEADERTITLE | PSP_USEHEADERSUBTITLE | PSP_USEICONID, // dwFlags
			JClientApp::jpApp->hinstApp, // hInstance
			MAKEINTRESOURCE(IDD_HELPER3), // pszTemplate
			(HICON)MAKEINTRESOURCE(0), // pszIcon
			0, // pszTitle
			(DLGPROC)JClient::DlgProcHelper3, // pfnDlgProc
			(LPARAM)this, // lParam
			0, // pfnCallback
			0, // pcRefParent
			title[3], // pszHeaderTitle
			subtitle[3], // pszHeaderSubTitle
		},
	};

	PROPSHEETHEADER psh = {
		sizeof(PROPSHEETHEADER), // dwSize
		PSH_PROPSHEETPAGE | PSH_PROPTITLE | PSH_USEICONID | PSH_WIZARD97, // dwFlags
		0, // hwndParent
		JClientApp::jpApp->hinstApp, // hInstance
		(HICON)MAKEINTRESOURCE(IDI_SMALL), // pszIcon
		MAKEINTRESOURCE(IDS_QSW), // pszCaption
		_countof(psp), // nPages
		0, // nStartPage
		(LPCPROPSHEETPAGE)&psp, // ppsp
		0, // pfnCallback
	};

	PropertySheet(&psh);
}

INT_PTR WINAPI JClient::DlgProcHelper0(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	JPtr<JClient> obj = (JClient*)(LONG_PTR)(message != WM_INITDIALOG
		? GetWindowLongPtr(hWnd, GWLP_USERDATA)
		: ((PROPSHEETPAGE*)lParam)->lParam);
	if (obj) {
		switch (message)
		{
		case WM_INITDIALOG:
			SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG)((PROPSHEETPAGE*)lParam)->lParam);
			obj->JAddRef();
			break;
		case WM_DESTROY:
			obj->JRelease();
			SetWindowLongPtr(hWnd, GWLP_USERDATA, 0);
			break;
		}
	} else return FALSE;

	LRESULT retval = TRUE;
	switch (message)
	{
	case WM_INITDIALOG:
		{
			TOOLINFO ti;
			ti.cbSize = sizeof(ti);
			ti.uFlags = TTF_ABSOLUTE | TTF_IDISHWND | TTF_TRACK;
			ti.hwnd = hWnd;
			ti.uId = (UINT_PTR)hWnd;
			ti.hinst = JClientApp::jpApp->hinstApp;
			ti.lpszText = 0;
			VERIFY(SendMessage(m_hwndBaloon, TTM_ADDTOOL, 0, (LPARAM)&ti));

			break;
		}

	case WM_DESTROY:
		{
			BaloonHide(hWnd);
			TOOLINFO ti;
			ti.cbSize = sizeof(ti);
			ti.hwnd = hWnd;
			ti.uId = (UINT_PTR)hWnd;
			SendMessage(m_hwndBaloon, TTM_DELTOOL, 0, (LPARAM)&ti);

			break;
		}

	case WM_COMMAND:
		{
			break;
		}

	case WM_NOTIFY:
		{
			NMHDR* pnmh = (NMHDR*)lParam;
			switch (pnmh->code)
			{
			case PSN_SETACTIVE:
				PropSheet_SetWizButtons(GetParent(hWnd), PSWIZB_NEXT);
				retval = 0;
				break;
			case PSN_WIZBACK:
			case PSN_WIZNEXT:
				retval = 0;
				break;
			case PSN_WIZFINISH:
				retval = FALSE;
				break;
			default: retval = FALSE;
			}
			break;
		}

	case WM_TIMER:
		{
			switch (wParam)
			{
			case IDT_BALOONPOP:
				{
					BaloonHide();
					break;
				}
			}
			break;
		}

	default: retval = FALSE;
	}
	SetWindowLong(hWnd, DWL_MSGRESULT, (LONG)retval);
	return retval;
}

INT_PTR WINAPI JClient::DlgProcHelper1(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	JPtr<JClient> obj = (JClient*)(LONG_PTR)(message != WM_INITDIALOG
		? GetWindowLongPtr(hWnd, GWLP_USERDATA)
		: ((PROPSHEETPAGE*)lParam)->lParam);
	if (obj) {
		switch (message)
		{
		case WM_INITDIALOG:
			SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG)((PROPSHEETPAGE*)lParam)->lParam);
			obj->JAddRef();
			break;
		case WM_DESTROY:
			obj->JRelease();
			SetWindowLongPtr(hWnd, GWLP_USERDATA, 0);
			break;
		}
	} else return FALSE;

	LRESULT retval = TRUE;
	switch (message)
	{
	case WM_INITDIALOG:
		{
			TOOLINFO ti;
			ti.cbSize = sizeof(ti);
			ti.uFlags = TTF_ABSOLUTE | TTF_IDISHWND | TTF_TRACK;
			ti.hwnd = hWnd;
			ti.uId = (UINT_PTR)hWnd;
			ti.hinst = JClientApp::jpApp->hinstApp;
			ti.lpszText = 0;
			VERIFY(SendMessage(m_hwndBaloon, TTM_ADDTOOL, 0, (LPARAM)&ti));

			SetDlgItemText(hWnd, IDC_NICK, profile::getString(RF_CLIENT, RK_NICK, NAME_NONAME).c_str());
			break;
		}

	case WM_DESTROY:
		{
			BaloonHide(hWnd);
			TOOLINFO ti;
			ti.cbSize = sizeof(ti);
			ti.hwnd = hWnd;
			ti.uId = (UINT_PTR)hWnd;
			SendMessage(m_hwndBaloon, TTM_DELTOOL, 0, (LPARAM)&ti);

			break;
		}

	case WM_COMMAND:
		{
			break;
		}

	case WM_NOTIFY:
		{
			NMHDR* pnmh = (NMHDR*)lParam;
			switch (pnmh->code)
			{
			case PSN_SETACTIVE:
				PropSheet_SetWizButtons(GetParent(hWnd), PSWIZB_BACK | PSWIZB_NEXT);
				retval = 0;
				break;
			case PSN_WIZBACK:
			case PSN_WIZNEXT:
				{
					std::tstring nickbuf(obj->m_metrics.uNameMaxLength, 0), nick;
					const TCHAR* msg;
					GetDlgItemText(hWnd, IDC_NICK, &nickbuf[0], (int)nickbuf.size()+1);
					nick = nickbuf.c_str();
					if (JClient::CheckNick(nick, msg)) { // check content
						// on OK
						profile::setString(RF_CLIENT, RK_NICK, nick);
						retval = 0;
					} else {
						// on failure
						obj->BaloonShow(hWnd, GetDlgItem(hWnd, IDC_NICK), msg, MAKEINTRESOURCE(IDS_MSG_NICKERROR), 2);
						retval = -1;
					}
					break;
				}
			case PSN_WIZFINISH:
				retval = FALSE;
				break;
			default: retval = FALSE;
			}
			break;
		}

	case WM_TIMER:
		{
			switch (wParam)
			{
			case IDT_BALOONPOP:
				{
					BaloonHide();
					break;
				}
			}
			break;
		}

	default: retval = FALSE;
	}
	SetWindowLong(hWnd, DWL_MSGRESULT, (LONG)retval);
	return retval;
}

INT_PTR WINAPI JClient::DlgProcHelper2(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	JPtr<JClient> obj = (JClient*)(LONG_PTR)(message != WM_INITDIALOG
		? GetWindowLongPtr(hWnd, GWLP_USERDATA)
		: ((PROPSHEETPAGE*)lParam)->lParam);
	if (obj) {
		switch (message)
		{
		case WM_INITDIALOG:
			SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG)((PROPSHEETPAGE*)lParam)->lParam);
			obj->JAddRef();
			break;
		case WM_DESTROY:
			obj->JRelease();
			SetWindowLongPtr(hWnd, GWLP_USERDATA, 0);
			break;
		}
	} else return FALSE;

	LRESULT retval = TRUE;
	switch (message)
	{
	case WM_INITDIALOG:
		{
			TOOLINFO ti;
			ti.cbSize = sizeof(ti);
			ti.uFlags = TTF_ABSOLUTE | TTF_IDISHWND | TTF_TRACK;
			ti.hwnd = hWnd;
			ti.uId = (UINT_PTR)hWnd;
			ti.hinst = JClientApp::jpApp->hinstApp;
			ti.lpszText = 0;
			VERIFY(SendMessage(m_hwndBaloon, TTM_ADDTOOL, 0, (LPARAM)&ti));

			std::tstring pass = profile::getString(RF_CLIENT, RK_PASSWORDNET, TEXT("beowolf"));
			SetDlgItemText(hWnd, IDC_PASS1, pass.c_str());
			SetDlgItemText(hWnd, IDC_PASS2, pass.c_str());
			break;
		}

	case WM_DESTROY:
		{
			BaloonHide(hWnd);
			TOOLINFO ti;
			ti.cbSize = sizeof(ti);
			ti.hwnd = hWnd;
			ti.uId = (UINT_PTR)hWnd;
			SendMessage(m_hwndBaloon, TTM_DELTOOL, 0, (LPARAM)&ti);

			break;
		}

	case WM_COMMAND:
		{
			switch (LOWORD(wParam))
			{
			case IDC_PASS1:
			case IDC_PASS2:
				switch (HIWORD(wParam))
				{
				case EN_KILLFOCUS:
					{
						std::tstring pass1buf(obj->m_metrics.uPassMaxLength, 0), pass2buf(obj->m_metrics.uPassMaxLength, 0);
						GetDlgItemText(hWnd, IDC_PASS1, (TCHAR*)pass1buf.data(), (int)pass1buf.size());
						GetDlgItemText(hWnd, IDC_PASS2, (TCHAR*)pass2buf.data(), (int)pass2buf.size());
						std::tstring pass1 = pass1buf.data(), pass2 = pass2buf.data();
						if (pass1.empty()) {
							obj->BaloonShow(hWnd, GetDlgItem(hWnd, IDC_PASS1), MAKEINTRESOURCE(IDS_MSG_EMPTYPASS), MAKEINTRESOURCE(IDS_MSG_PASSAUTH), 1);
						} else if (pass2.empty()) {
							obj->BaloonShow(hWnd, GetDlgItem(hWnd, IDC_PASS2), MAKEINTRESOURCE(IDS_MSG_EMPTYPASS), MAKEINTRESOURCE(IDS_MSG_PASSAUTH), 1);
						} else if (pass1.length() <= 6) {
							obj->BaloonShow(hWnd, GetDlgItem(hWnd, IDC_PASS1), MAKEINTRESOURCE(IDS_MSG_EASYPASS), MAKEINTRESOURCE(IDS_MSG_PASSAUTH), 1);
						} else if (pass2.length() <= 6) {
							obj->BaloonShow(hWnd, GetDlgItem(hWnd, IDC_PASS2), MAKEINTRESOURCE(IDS_MSG_EASYPASS), MAKEINTRESOURCE(IDS_MSG_PASSAUTH), 1);
						} else if (pass1 != pass2) {
							obj->BaloonShow(hWnd, GetDlgItem(hWnd, IDC_PASS1), MAKEINTRESOURCE(IDS_MSG_NEWPASS), MAKEINTRESOURCE(IDS_MSG_PASSAUTH), 2);
						} else if (pass1.empty()) {
							obj->BaloonShow(hWnd, GetDlgItem(hWnd, IDC_PASS1), MAKEINTRESOURCE(IDS_MSG_EMPTYPASS), MAKEINTRESOURCE(IDS_MSG_PASSAUTH), 1);
						} else {
							obj->BaloonHide();
						}
						break;
					}

				case EN_CHANGE:
					{
						obj->BaloonHide();
						break;
					}
				}
				break;

			case IDC_SHOWPASS:
				{
					HWND hctrl;
					bool check = IsDlgButtonChecked(hWnd, IDC_SHOWPASS) != BST_UNCHECKED;
					hctrl = GetDlgItem(hWnd, IDC_PASS1);
					SendMessage(hctrl, EM_SETPASSWORDCHAR, check ? 0 : '*', 0);
					InvalidateRect(hctrl, 0, FALSE);
					hctrl = GetDlgItem(hWnd, IDC_PASS2);
					SendMessage(hctrl, EM_SETPASSWORDCHAR, check ? 0 : '*', 0);
					InvalidateRect(hctrl, 0, FALSE);
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
			case PSN_SETACTIVE:
				PropSheet_SetWizButtons(GetParent(hWnd), PSWIZB_BACK | PSWIZB_NEXT);
				retval = 0;
				break;
			case PSN_WIZBACK:
			case PSN_WIZNEXT:
				{
					std::tstring pass1buf(obj->m_metrics.uPassMaxLength, 0), pass2buf(obj->m_metrics.uPassMaxLength, 0);
					GetDlgItemText(hWnd, IDC_PASS1, (TCHAR*)pass1buf.data(), (int)pass1buf.size());
					GetDlgItemText(hWnd, IDC_PASS2, (TCHAR*)pass2buf.data(), (int)pass2buf.size());
					std::tstring pass1 = pass1buf.data(), pass2 = pass2buf.data();
					if (pass1 != pass2) {
						obj->BaloonShow(hWnd, GetDlgItem(hWnd, IDC_PASS1), MAKEINTRESOURCE(IDS_MSG_NEWPASS), MAKEINTRESOURCE(IDS_MSG_PASSAUTH), 2);
						retval = -1;
					} else if (pass1.empty()) {
						obj->BaloonShow(hWnd, GetDlgItem(hWnd, IDC_PASS1), MAKEINTRESOURCE(IDS_MSG_EMPTYPASS), MAKEINTRESOURCE(IDS_MSG_PASSAUTH), 1);
						retval = -1;
					} else {
						// on OK
						profile::setString(RF_CLIENT, RK_PASSWORDNET, pass1);
						retval = 0;
					}
					break;
				}
			case PSN_WIZFINISH:
				retval = FALSE;
				break;
			default: retval = FALSE;
			}
			break;
		}

	case WM_TIMER:
		{
			switch (wParam)
			{
			case IDT_BALOONPOP:
				{
					BaloonHide();
					break;
				}
			}
			break;
		}

	default: retval = FALSE;
	}
	SetWindowLong(hWnd, DWL_MSGRESULT, (LONG)retval);
	return retval;
}

INT_PTR WINAPI JClient::DlgProcHelper3(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	JPtr<JClient> obj = (JClient*)(LONG_PTR)(message != WM_INITDIALOG
		? GetWindowLongPtr(hWnd, GWLP_USERDATA)
		: ((PROPSHEETPAGE*)lParam)->lParam);
	if (obj) {
		switch (message)
		{
		case WM_INITDIALOG:
			SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG)((PROPSHEETPAGE*)lParam)->lParam);
			obj->JAddRef();
			break;
		case WM_DESTROY:
			obj->JRelease();
			SetWindowLongPtr(hWnd, GWLP_USERDATA, 0);
			break;
		}
	} else return FALSE;

	LRESULT retval = TRUE;
	switch (message)
	{
	case WM_INITDIALOG:
		{
			TOOLINFO ti;
			ti.cbSize = sizeof(ti);
			ti.uFlags = TTF_ABSOLUTE | TTF_IDISHWND | TTF_TRACK;
			ti.hwnd = hWnd;
			ti.uId = (UINT_PTR)hWnd;
			ti.hinst = JClientApp::jpApp->hinstApp;
			ti.lpszText = 0;
			VERIFY(SendMessage(m_hwndBaloon, TTM_ADDTOOL, 0, (LPARAM)&ti));

			SetDlgItemText(hWnd, IDC_HOST, profile::getString(RF_CLIENT, RK_HOST, TEXT("127.0.0.1")).c_str());
			SendDlgItemMessage(hWnd, IDC_HOST, CB_LIMITTEXT, 128, 0);
			SetDlgItemInt(hWnd, IDC_PORT, profile::getInt(RF_CLIENT, RK_PORT, CCP_PORT), FALSE);
			SendDlgItemMessage(hWnd, IDC_PORT, EM_LIMITTEXT, 5, 0);
			break;
		}

	case WM_DESTROY:
		{
			BaloonHide(hWnd);
			TOOLINFO ti;
			ti.cbSize = sizeof(ti);
			ti.hwnd = hWnd;
			ti.uId = (UINT_PTR)hWnd;
			SendMessage(m_hwndBaloon, TTM_DELTOOL, 0, (LPARAM)&ti);

			break;
		}

	case WM_COMMAND:
		{
			break;
		}

	case WM_NOTIFY:
		{
			NMHDR* pnmh = (NMHDR*)lParam;
			switch (pnmh->code)
			{
			case PSN_SETACTIVE:
				PropSheet_SetWizButtons(GetParent(hWnd), PSWIZB_BACK | PSWIZB_FINISH);
				retval = 0;
				break;
			case PSN_WIZBACK:
			case PSN_WIZNEXT:
				{
					char host[128];
					GetDlgItemTextA(hWnd, IDC_HOST, host, _countof(host));
					hostent* h = gethostbyname(host);
					BOOL bTranslated;
					UINT port = GetDlgItemInt(hWnd, IDC_PORT, &bTranslated, FALSE);
					if (!h) {
						obj->BaloonShow(hWnd, GetDlgItem(hWnd, IDC_HOST), MAKEINTRESOURCE(IDS_MSG_BADHOST), MAKEINTRESOURCE(IDS_MSG_HOSTVRF), 1);
						retval = -1;
					} else if (!bTranslated) {
						obj->BaloonShow(hWnd, GetDlgItem(hWnd, IDC_PORT), MAKEINTRESOURCE(IDS_MSG_BADPORT), MAKEINTRESOURCE(IDS_MSG_HOSTVRF), 1);
						retval = -1;
					} else if (port < 1000 || port > 0xFFFF) {
						obj->BaloonShow(hWnd, GetDlgItem(hWnd, IDC_PORT), MAKEINTRESOURCE(IDS_MSG_PORTRANGE), MAKEINTRESOURCE(IDS_MSG_HOSTVRF), 1);
						retval = -1;
					} else {
						// on OK
						profile::setString(RF_CLIENT, RK_HOST, ANSIToTstr(host));
						profile::setInt(RF_CLIENT, RK_PORT, port);
						retval = FALSE;
					}
					break;
				}
			case PSN_WIZFINISH:
				{
					char host[128];
					GetDlgItemTextA(hWnd, IDC_HOST, host, _countof(host));
					hostent* h = gethostbyname(host);
					BOOL bTranslated;
					UINT port = GetDlgItemInt(hWnd, IDC_PORT, &bTranslated, FALSE);
					if (!h) {
						obj->BaloonShow(hWnd, GetDlgItem(hWnd, IDC_HOST), MAKEINTRESOURCE(IDS_MSG_BADHOST), MAKEINTRESOURCE(IDS_MSG_HOSTVRF), 1);
						retval = -1;
					} else if (!bTranslated) {
						obj->BaloonShow(hWnd, GetDlgItem(hWnd, IDC_PORT), MAKEINTRESOURCE(IDS_MSG_BADPORT), MAKEINTRESOURCE(IDS_MSG_HOSTVRF), 1);
						retval = -1;
					} else if (port < 1000 || port > 0xFFFF) {
						obj->BaloonShow(hWnd, GetDlgItem(hWnd, IDC_PORT), MAKEINTRESOURCE(IDS_MSG_PORTRANGE), MAKEINTRESOURCE(IDS_MSG_HOSTVRF), 1);
						retval = -1;
					} else {
						// on OK
						profile::setString(RF_CLIENT, RK_HOST, ANSIToTstr(host));
						profile::setInt(RF_CLIENT, RK_PORT, port);
						profile::setInt(RF_CLIENT, RK_STATE, TRUE);
						retval = FALSE;
					}
					break;
				}
			default: retval = FALSE;
			}
			break;
		}

	case WM_TIMER:
		{
			switch (wParam)
			{
			case IDT_BALOONPOP:
				{
					BaloonHide();
					break;
				}
			}
			break;
		}

	default: retval = FALSE;
	}
	SetWindowLong(hWnd, DWL_MSGRESULT, (LONG)retval);
	return retval;
}

//-----------------------------------------------------------------------------

//
// JClient::JPassword dialog
//

JClient::JPassword::JPassword(JClient* p)
: JNode(p, false), JDialog()
{
	ASSERT(p);
	SetupHooks();
}

LRESULT WINAPI JClient::JPassword::DlgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	LRESULT retval = TRUE;

	static TCHAR* lvs[][4] = {
		{TEXT("COPY"), TEXT("0"), TEXT("0"), TEXT("no encryption")},
		{TEXT("HC-256"), TEXT("256"), TEXT("256"), TEXT("eSTREAM Portfolio - Profile 1 (SW)")},
		{TEXT("Rabbit"), TEXT("128"), TEXT("64"), TEXT("eSTREAM Portfolio - Profile 1 (SW)")},
		{TEXT("Salsa20"), TEXT("256"), TEXT("64"), TEXT("eSTREAM Portfolio - Profile 1 (SW)")},
		{TEXT("LEX-v2"), TEXT("128"), TEXT("128"), TEXT("eSTREAM Phase 3 - Profile 1 (SW)")},
		{TEXT("Grain-v1"), TEXT("80"), TEXT("64"), TEXT("eSTREAM Portfolio - Profile 2 (HW)")},
		{TEXT("Grain-128"), TEXT("128"), TEXT("96"), TEXT("eSTREAM Phase 3 - Profile 2 (HW)")},
		{TEXT("TRIVIUM"), TEXT("80"), TEXT("80"), TEXT("eSTREAM Portfolio - Profile 2 (HW)")},
		{TEXT("Edon80"), TEXT("80"), TEXT("64"), TEXT("eSTREAM Phase 3 - Profile 2 (HW)")},
	};

	switch (message)
	{
	case WM_INITDIALOG:
		{
			if (!pNode)
			{
				retval = FALSE;
				break;
			}

			m_hwndList = GetDlgItem(hWnd, IDC_ENCRYPTLIST);

			SetDlgItemText(m_hwndPage, IDC_OLDPASS, pNode->m_passwordNet.data());
			SetDlgItemText(m_hwndPage, IDC_NEWPASS1, pNode->m_passwordNet.data());
			SetDlgItemText(m_hwndPage, IDC_NEWPASS2, pNode->m_passwordNet.data());

			// Inits Channels list
			ListView_SetExtendedListViewStyle(m_hwndList,
				LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_HEADERDRAGDROP | LVS_EX_ONECLICKACTIVATE | LVS_EX_SUBITEMIMAGES);
			static LV_COLUMN lvc[] = {
				{LVCF_FMT | LVCF_SUBITEM | LVCF_TEXT | LVCF_WIDTH, LVCFMT_LEFT,
				80, TEXT("Cipher name"), -1, 0},
				{LVCF_FMT | LVCF_SUBITEM | LVCF_TEXT | LVCF_WIDTH, LVCFMT_LEFT,
				50, TEXT("Key, bits"), -1, 0},
				{LVCF_FMT | LVCF_SUBITEM | LVCF_TEXT | LVCF_WIDTH, LVCFMT_LEFT,
				50, TEXT("Nonce, bits"), -1, 0},
				{LVCF_FMT | LVCF_SUBITEM | LVCF_TEXT | LVCF_WIDTH, LVCFMT_LEFT,
				180, TEXT("Comments"), -1, 0},
			};
			for (int i = 0; i < _countof(lvc); ++i)
				ListView_InsertColumn(m_hwndList, i, &lvc[i]);

			for (int i = 0; i < _countof(lvs); ++i) {
				LVITEM lvi;
				lvi.iItem = i;
				lvi.iSubItem = 0;
				lvi.mask = 0;
				VERIFY(ListView_InsertItem(m_hwndList, &lvi) >= 0);
				ListView_SetItemText(m_hwndList, i, 0, lvs[i][0]);
				ListView_SetItemText(m_hwndList, i, 1, lvs[i][1]);
				ListView_SetItemText(m_hwndList, i, 2, lvs[i][2]);
				ListView_SetItemText(m_hwndList, i, 3, lvs[i][3]);
				if (ANSIToTstr(pNode->getEncryptorName()) == lvs[i][0])
					ListView_SetItemState(m_hwndList, i, LVIS_SELECTED, LVIS_SELECTED);
			}
			SetDlgItemTextA(m_hwndPage, IDC_ENCRYPTSEL, pNode->getEncryptorName());

			retval = TRUE;
			break;
		}

	case WM_DESTROY:
		{
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
					std::tstring oldbuf(pNode->m_metrics.uPassMaxLength, 0), pass1buf(pNode->m_metrics.uPassMaxLength, 0), pass2buf(pNode->m_metrics.uPassMaxLength, 0);
					GetDlgItemText(m_hwndPage, IDC_OLDPASS, (TCHAR*)oldbuf.data(), (int)oldbuf.size());
					GetDlgItemText(m_hwndPage, IDC_NEWPASS1, (TCHAR*)pass1buf.data(), (int)pass1buf.size());
					GetDlgItemText(m_hwndPage, IDC_NEWPASS2, (TCHAR*)pass2buf.data(), (int)pass2buf.size());
					std::tstring old = oldbuf.data(), pass1 = pass1buf.data(), pass2 = pass2buf.data();
					if (pNode->m_passwordNet != old) {
						pNode->BaloonShow(GetDlgItem(m_hwndPage, IDC_OLDPASS), MAKEINTRESOURCE(IDS_MSG_OLDPASS), MAKEINTRESOURCE(IDS_MSG_PASSAUTH), 2);
					} else if (pass1 != pass2) {
						pNode->BaloonShow(GetDlgItem(m_hwndPage, IDC_NEWPASS1), MAKEINTRESOURCE(IDS_MSG_NEWPASS), MAKEINTRESOURCE(IDS_MSG_PASSAUTH), 2);
					} else {
						char ciphbuf[32];
						pNode->m_passwordNet = pass1;
						GetDlgItemTextA(m_hwndPage, IDC_ENCRYPTSEL, ciphbuf, _countof(ciphbuf));
						pNode->m_encryptorname = ciphbuf;
						pNode->BaloonHide();
						DestroyWindow(hWnd);
					}
					break;
				}

			case IDCANCEL:
				pNode->BaloonHide();
				DestroyWindow(hWnd);
				break;

			case IDC_OLDPASS:
			case IDC_NEWPASS1:
			case IDC_NEWPASS2:
				switch (HIWORD(wParam))
				{
				case EN_KILLFOCUS:
					{
						std::tstring oldbuf(pNode->m_metrics.uPassMaxLength, 0), pass1buf(pNode->m_metrics.uPassMaxLength, 0), pass2buf(pNode->m_metrics.uPassMaxLength, 0);
						GetDlgItemText(m_hwndPage, IDC_OLDPASS, (TCHAR*)oldbuf.data(), (int)oldbuf.size());
						GetDlgItemText(m_hwndPage, IDC_NEWPASS1, (TCHAR*)pass1buf.data(), (int)pass1buf.size());
						GetDlgItemText(m_hwndPage, IDC_NEWPASS2, (TCHAR*)pass2buf.data(), (int)pass2buf.size());
						std::tstring old = oldbuf.data(), pass1 = pass1buf.data(), pass2 = pass2buf.data();
						if (pass1.empty()) {
							pNode->BaloonShow(GetDlgItem(m_hwndPage, IDC_NEWPASS1), MAKEINTRESOURCE(IDS_MSG_EMPTYPASS), MAKEINTRESOURCE(IDS_MSG_PASSAUTH), 1);
						} else if (pass2.empty()) {
							pNode->BaloonShow(GetDlgItem(m_hwndPage, IDC_NEWPASS2), MAKEINTRESOURCE(IDS_MSG_EMPTYPASS), MAKEINTRESOURCE(IDS_MSG_PASSAUTH), 1);
						} else if (pass1.length() <= 6) {
							pNode->BaloonShow(GetDlgItem(m_hwndPage, IDC_NEWPASS1), MAKEINTRESOURCE(IDS_MSG_EASYPASS), MAKEINTRESOURCE(IDS_MSG_PASSAUTH), 1);
						} else if (pass2.length() <= 6) {
							pNode->BaloonShow(GetDlgItem(m_hwndPage, IDC_NEWPASS2), MAKEINTRESOURCE(IDS_MSG_EASYPASS), MAKEINTRESOURCE(IDS_MSG_PASSAUTH), 1);
						} else if (pNode->m_passwordNet != old) {
							pNode->BaloonShow(GetDlgItem(m_hwndPage, IDC_OLDPASS), MAKEINTRESOURCE(IDS_MSG_OLDPASS), MAKEINTRESOURCE(IDS_MSG_PASSAUTH), 2);
						} else if (pass1 != pass2) {
							pNode->BaloonShow(GetDlgItem(m_hwndPage, IDC_NEWPASS1), MAKEINTRESOURCE(IDS_MSG_NEWPASS), MAKEINTRESOURCE(IDS_MSG_PASSAUTH), 2);
						} else {
							pNode->BaloonHide();
						}
						break;
					}

				case EN_CHANGE:
					{
						pNode->BaloonHide();
						break;
					}
				}
				break;

			case IDC_SHOWPASS:
				{
					HWND hctrl;
					bool check = IsDlgButtonChecked(m_hwndPage, IDC_SHOWPASS) != BST_UNCHECKED;
					hctrl = GetDlgItem(m_hwndPage, IDC_OLDPASS);
					SendMessage(hctrl, EM_SETPASSWORDCHAR, check ? 0 : '*', 0);
					InvalidateRect(hctrl, 0, FALSE);
					hctrl = GetDlgItem(m_hwndPage, IDC_NEWPASS1);
					SendMessage(hctrl, EM_SETPASSWORDCHAR, check ? 0 : '*', 0);
					InvalidateRect(hctrl, 0, FALSE);
					hctrl = GetDlgItem(m_hwndPage, IDC_NEWPASS2);
					SendMessage(hctrl, EM_SETPASSWORDCHAR, check ? 0 : '*', 0);
					InvalidateRect(hctrl, 0, FALSE);
					break;
				}

			default: retval = __super::DlgProc(hWnd, message, wParam, lParam);
			}
			break;
		}

	case WM_NOTIFY:
		{
			NMHDR* pnmh = (NMHDR*)lParam;
			switch (pnmh->code)
			{
			case LVN_ITEMCHANGED:
				{
					LPNMLISTVIEW pnmv = (LPNMLISTVIEW)lParam;
					if (pnmh->idFrom == IDC_ENCRYPTLIST && pnmv->uChanged == LVIF_STATE
						&& (pnmv->uNewState & LVIS_SELECTED) != 0)
					{
						SetDlgItemText(m_hwndPage, IDC_ENCRYPTSEL, lvs[pnmv->iItem][0]);
					}
					break;
				}

			default: retval = __super::DlgProc(hWnd, message, wParam, lParam);
			}
			break;
		}

		default: retval = __super::DlgProc(hWnd, message, wParam, lParam);
	}
	return retval;
}

//
// JClient::JTopic dialog
//

JClient::JTopic::JTopic(JClient* p, DWORD id, const std::tstring& n, const std::tstring& t)
: JNode(p, false), JDialog()
{
	ASSERT(p);
	SetupHooks();

	m_idChannel = id;
	m_name = n;
	m_topic = t;
}

LRESULT WINAPI JClient::JTopic::DlgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	LRESULT retval = TRUE;

	switch (message)
	{
	case WM_INITDIALOG:
		{
			if (!pNode)
			{
				retval = FALSE;
				break;
			}

			TCHAR buffer[40];
			GetWindowText(hWnd, buffer, _countof(buffer));
			SetWindowText(hWnd, tformat(TEXT("%s: #%s"), buffer, m_name.c_str()).c_str());

			SetDlgItemText(hWnd, IDC_TOPICTEXT, m_topic.c_str());

			OnMetrics(pNode->m_metrics);

			retval = TRUE;
			break;
		}

	case WM_DESTROY:
		{
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
					std::tstring buffer(pNode->m_metrics.uTopicMaxLength, 0);
					GetDlgItemText(hWnd, IDC_TOPICTEXT, &buffer[0], (int)buffer.size()+1);
					pNode->PushTrn(pNode->m_clientsock, pNode->Make_Cmd_TOPIC(m_idChannel, buffer));
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

void JClient::JTopic::OnHook(JNode* src)
{
	using namespace fastdelegate;

	__super::OnHook(src);

	JNODE(JClient, node, src);
	if (node) {
		node->EvLinkEstablished += MakeDelegate(this, &JClient::JTopic::OnLinkEstablished);
		node->EvLinkClose += MakeDelegate(this, &JClient::JTopic::OnLinkClose);
		node->EvPageClose += MakeDelegate(this, &JClient::JTopic::OnPageClose);
		node->EvMetrics += MakeDelegate(this, &JClient::JTopic::OnMetrics);
	}
}

void JClient::JTopic::OnUnhook(JNode* src)
{
	using namespace fastdelegate;

	JNODE(JClient, node, src);
	if (node) {
		node->EvLinkEstablished -= MakeDelegate(this, &JClient::JTopic::OnLinkEstablished);
		node->EvLinkClose -= MakeDelegate(this, &JClient::JTopic::OnLinkClose);
		node->EvPageClose -= MakeDelegate(this, &JClient::JTopic::OnPageClose);
		node->EvMetrics -= MakeDelegate(this, &JClient::JTopic::OnMetrics);
	}

	__super::OnUnhook(src);
}

void JClient::JTopic::OnLinkEstablished(SOCKET sock)
{
}

void JClient::JTopic::OnLinkClose(SOCKET sock, UINT err)
{
}

void JClient::JTopic::OnPageClose(DWORD id)
{
	if (m_hwndPage && id == m_idChannel)
		SendMessage(m_hwndPage, WM_COMMAND, IDCANCEL, 0);
}

void JClient::JTopic::OnMetrics(const Metrics& metrics)
{
	if (!m_hwndPage) return; // ignore if window closed

	SendDlgItemMessage(m_hwndPage, IDC_TOPICTEXT, EM_LIMITTEXT, metrics.uTopicMaxLength, 0);
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

			if (!pNode) {
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

			EnableWindow(GetDlgItem(m_hwndPage, IDOK), pNode->m_clientsock != 0);

			retval = TRUE;
			break;
		}

	case WM_DESTROY:
		{
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
					if (!pNode->m_clientsock) break;
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

					pNode->PushTrn(pNode->m_clientsock, pNode->Make_Cmd_SPLASHRTF(idWho,
						content.c_str(), rect,
						true, dwCanclose, dwAutoclose, m_fTransparent, crSheet));
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

JClient::JSplashRtfEditor::JSplashRtfEditor(JClient* p, DWORD who)
: JNode(p, false), JDialog(), rtf::Editor()
{
	ASSERT(p);
	SetupHooks();

	idWho = who;
}

void JClient::JSplashRtfEditor::OnHook(JNode* src)
{
	using namespace fastdelegate;

	__super::OnHook(src);

	JNODE(JClient, node, src);
	if (node) {
		node->EvLinkEstablished += MakeDelegate(this, &JClient::JSplashRtfEditor::OnLinkEstablished);
		node->EvLinkClose += MakeDelegate(this, &JClient::JSplashRtfEditor::OnLinkClose);
	}
}

void JClient::JSplashRtfEditor::OnUnhook(JNode* src)
{
	using namespace fastdelegate;

	JNODE(JClient, node, src);
	if (node) {
		node->EvLinkEstablished -= MakeDelegate(this, &JClient::JSplashRtfEditor::OnLinkEstablished);
		node->EvLinkClose -= MakeDelegate(this, &JClient::JSplashRtfEditor::OnLinkClose);
	}

	__super::OnUnhook(src);
}

void JClient::JSplashRtfEditor::OnLinkEstablished(SOCKET sock)
{
	EnableWindow(GetDlgItem(m_hwndPage, IDOK), TRUE);
}

void JClient::JSplashRtfEditor::OnLinkClose(SOCKET sock, UINT err)
{
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

			if (!pNode)
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

JClient::JSplash::JSplash(JClient* p)
: JDialog()
{
	m_trnid = 0;
	m_result = 0;

	m_bCloseOnDisconnect = true;
	m_dwCanclose = 2500;
	m_dwAutoclose = 30000;

	dwStarted = GetTickCount();
}

void JClient::JSplash::OnHook(JNode* src)
{
	using namespace fastdelegate;

	__super::OnHook(src);

	JNODE(JClient, node, src);
	if (node) {
		node->EvLinkClose += MakeDelegate(this, &JClient::JSplash::OnLinkClose);
	}
}

void JClient::JSplash::OnUnhook(JNode* src)
{
	using namespace fastdelegate;

	JNODE(JClient, node, src);
	if (node) {
		node->EvLinkClose -= MakeDelegate(this, &JClient::JSplash::OnLinkClose);
	}

	__super::OnUnhook(src);
}

void JClient::JSplash::OnLinkClose(SOCKET sock, UINT err)
{
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

JClient::JSplashRtf::JSplashRtf(JClient* p, const char* text, size_t size)
: JNode(p, false), JClient::JSplash(p), content(text, size)
{
	ASSERT(p);
	SetupHooks();
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

			m_fTransparent = true;
			wCharFormatting = SCF_SELECTION;

			if (!pNode) {
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

			EnableWindow(GetDlgItem(m_hwndPage, IDOK), pNode->m_clientsock != 0);

			SetDlgItemText(hWnd, IDC_NICK, strWho.c_str());

			CheckDlgButton(hWnd, IDC_ALERT, fAlert ? BST_CHECKED : BST_UNCHECKED);

			OnMetrics(pNode->m_metrics);

			retval = TRUE;
			break;
		}

	case WM_DESTROY:
		{
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
					if (!pNode->m_clientsock) break;
					// Get RTF content
					std::string content;
					getContent(content, SF_RTF);

					std::tstring nickbuf(pNode->m_metrics.uNameMaxLength, 0), nick;
					const TCHAR* msg;
					GetDlgItemText(hWnd, IDC_NICK, &nickbuf[0], (int)nickbuf.size()+1);
					nick = nickbuf.c_str();
					if (JClient::CheckNick(nick, msg)) { // check content
						pNode->PushTrn(pNode->m_clientsock, pNode->Make_Quest_MESSAGE(tCRCJJ(nick.c_str()),
							content.c_str(), fAlert, crSheet));
						DestroyWindow(hWnd);
					} else {
						pNode->BaloonShow(pNode->jpPageServer->hwndNick, msg, strWho.c_str(), 2);
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

JClient::JMessageEditor::JMessageEditor(JClient* p, const std::tstring& who, bool alert)
: JNode(p, false), JDialog(), rtf::Editor()
{
	ASSERT(p);
	SetupHooks();

	strWho = who;
	fAlert = alert;
}

void JClient::JMessageEditor::OnHook(JNode* src)
{
	using namespace fastdelegate;

	__super::OnHook(src);

	JNODE(JClient, node, src);
	if (node) {
		node->EvLinkEstablished += MakeDelegate(this, &JClient::JMessageEditor::OnLinkEstablished);
		node->EvLinkClose += MakeDelegate(this, &JClient::JMessageEditor::OnLinkClose);
		node->EvMetrics += MakeDelegate(this, &JClient::JMessageEditor::OnMetrics);
	}
}

void JClient::JMessageEditor::OnUnhook(JNode* src)
{
	using namespace fastdelegate;

	JNODE(JClient, node, src);
	if (node) {
		node->EvLinkEstablished -= MakeDelegate(this, &JClient::JMessageEditor::OnLinkEstablished);
		node->EvLinkClose -= MakeDelegate(this, &JClient::JMessageEditor::OnLinkClose);
		node->EvMetrics -= MakeDelegate(this, &JClient::JMessageEditor::OnMetrics);
	}

	__super::OnUnhook(src);
}

void JClient::JMessageEditor::OnLinkEstablished(SOCKET sock)
{
	EnableWindow(GetDlgItem(m_hwndPage, IDOK), TRUE);
}

void JClient::JMessageEditor::OnLinkClose(SOCKET sock, UINT err)
{
	EnableWindow(GetDlgItem(m_hwndPage, IDOK), FALSE);
}

void JClient::JMessageEditor::OnMetrics(const Metrics& metrics)
{
	if (!m_hwndPage) return; // ignore if window closed

	SendDlgItemMessage(m_hwndPage, IDC_NICK, EM_LIMITTEXT, metrics.uNameMaxLength, 0);
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

			if (!pNode) {
				retval = FALSE;
				break;
			}

			// Set main window icons
			SNDMSG(hWnd, WM_SETICON, ICON_SMALL, (LPARAM)JClientApp::jpApp->hiMain16);
			SNDMSG(hWnd, WM_SETICON, ICON_BIG, (LPARAM)JClientApp::jpApp->hiMain32);

			SetWindowText(hWnd, pNode->getSafeName(m_idWho).c_str());

			SendDlgItemMessage(hWnd, IDC_RICHEDIT, EM_SETBKGNDCOLOR, FALSE, (LPARAM)m_crSheet);
			SetDlgItemTextA(hWnd, IDC_RICHEDIT, m_content.c_str());

			SYSTEMTIME st;
			FileTimeToLocalTime(m_ftRecv, st);
			SendDlgItemMessage(hWnd, IDC_RECVTIME, DTM_SETSYSTEMTIME, GDT_VALID, (LPARAM)&st);

			EnableWindow(GetDlgItem(m_hwndPage, IDOK), pNode->m_clientsock != 0);
			EnableWindow(GetDlgItem(m_hwndPage, IDC_PRIVATETALK), pNode->m_clientsock != 0);

			if (m_fAlert) {
				SetWindowPos(hWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
				if (pNode->m_mUser[pNode->m_idOwn].accessibility.fPlayAlert)
					pNode->PlaySound(JClientApp::jpApp->strWavAlert.c_str());
				if (pNode->m_mUser[pNode->m_idOwn].accessibility.fFlashPageSayPrivate
					&& profile::getInt(RF_CLIENT, RK_FLASHPAGESAYPRIVATE, TRUE)
					&& !pNode->m_mUser[pNode->m_idOwn].isOnline)
					FlashWindow(m_hwndPage, TRUE);
			}

			retval = TRUE;
			break;
		}

	case WM_DESTROY:
		{
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
					MapUser::const_iterator iu = pNode->m_mUser.find(m_idWho);
					if (iu == pNode->m_mUser.end()) break;
					if (iu->second.accessibility.fCanMessage) {
						ASSERT(pNode->m_clientsock);
						CreateDialogParam(JClientApp::jpApp->hinstApp, MAKEINTRESOURCE(IDD_MSGSEND), pNode->hwndPage, JClient::JSplashRtfEditor::DlgProcStub, (LPARAM)(JDialog*)new JClient::JMessageEditor(pNode, iu->second.name, false));
						DestroyWindow(hWnd);
					} else pNode->BaloonShow(m_hwndPage, MAKEINTRESOURCE(IDS_MSG_PRIVATEMESSAGE), pNode->getSafeName(m_idWho).c_str(), 1);
					break;
				}

			case IDC_PRIVATETALK:
				{
					MapUser::const_iterator iu = pNode->m_mUser.find(m_idWho);
					if (iu == pNode->m_mUser.end()) break;
					if (iu->second.accessibility.fCanOpenPrivate) {
						ASSERT(pNode->m_clientsock);
						pNode->PushTrn(pNode->m_clientsock, pNode->Make_Quest_JOIN(iu->second.name));
						DestroyWindow(hWnd);
					} else pNode->BaloonShow(m_hwndPage, MAKEINTRESOURCE(IDS_MSG_PRIVATETALK), pNode->getSafeName(m_idWho).c_str(), 1);
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

JClient::JMessage::JMessage(JClient* p)
: JNode(p, false), JDialog()
{
	ASSERT(p);
	SetupHooks();

	m_idWho = CRC_ANONYMOUS;

	m_fAlert = false;
	m_bCloseOnDisconnect = false;
	m_crSheet = GetSysColor(COLOR_WINDOW);
}

void JClient::JMessage::OnHook(JNode* src)
{
	using namespace fastdelegate;

	__super::OnHook(src);

	JNODE(JClient, node, src);
	if (node) {
		node->EvLinkEstablished += MakeDelegate(this, &JClient::JMessage::OnLinkEstablished);
		node->EvLinkClose += MakeDelegate(this, &JClient::JMessage::OnLinkClose);
	}
}

void JClient::JMessage::OnUnhook(JNode* src)
{
	using namespace fastdelegate;

	JNODE(JClient, node, src);
	if (node) {
		node->EvLinkEstablished -= MakeDelegate(this, &JClient::JMessage::OnLinkEstablished);
		node->EvLinkClose -= MakeDelegate(this, &JClient::JMessage::OnLinkClose);
	}

	__super::OnUnhook(src);
}

void JClient::JMessage::OnLinkEstablished(SOCKET sock)
{
	EnableWindow(GetDlgItem(m_hwndPage, IDC_REPLY), TRUE);
	EnableWindow(GetDlgItem(m_hwndPage, IDC_PRIVATETALK), TRUE);
}

void JClient::JMessage::OnLinkClose(SOCKET sock, UINT err)
{
	if (m_hwndPage && m_bCloseOnDisconnect) DestroyWindow(m_hwndPage);
	else {
		EnableWindow(GetDlgItem(m_hwndPage, IDC_REPLY), FALSE);
		EnableWindow(GetDlgItem(m_hwndPage, IDC_PRIVATETALK), FALSE);
	}
}

//-----------------------------------------------------------------------------

// The End.