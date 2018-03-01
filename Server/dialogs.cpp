
//-----------------------------------------------------------------------------

//
// Includes
//

#pragma region Includes

#include "stdafx.h"

// Common
#include "CRC.h"
#include "Profile.h"

// Project
#include "protocol.h"
#include "resource.h"
#include "server.h"

#pragma endregion

//-----------------------------------------------------------------------------

using namespace colibrichat;

//-----------------------------------------------------------------------------

//
// JConnections dialog
//

JServer::JConnections::JConnections()
: JDialog()
{
}

LRESULT WINAPI JServer::JConnections::DlgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	LRESULT retval = TRUE;
	switch (message)
	{
	case WM_INITDIALOG:
		{
			m_hwndList = GetDlgItem(hWnd, IDC_LIST);

			// Get initial windows sizes
			MapControl(m_hwndList, rcList);

			if (!pNode)
			{
				retval = FALSE;
				break;
			}

			// Set main window icons
			SNDMSG(hWnd, WM_SETICON, ICON_SMALL, (LPARAM)JServerApp::jpApp->hiMain16);
			SNDMSG(hWnd, WM_SETICON, ICON_BIG, (LPARAM)JServerApp::jpApp->hiMain32);

			ListView_SetExtendedListViewStyle(m_hwndList,
				LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_HEADERDRAGDROP | LVS_EX_ONECLICKACTIVATE | LVS_EX_SUBITEMIMAGES);
			//ListView_SetImageList(m_hwndList, JServerApp::jpApp->himlTree, LVSIL_SMALL);
			ListView_SetItemCount(m_hwndList, 16);
			static LV_COLUMN lvc[] = {
				{LVCF_FMT | LVCF_SUBITEM | LVCF_TEXT | LVCF_WIDTH, LVCFMT_LEFT,
				80, TEXT("Nickname"), -1, 0},
				{LVCF_FMT | LVCF_SUBITEM | LVCF_TEXT | LVCF_WIDTH, LVCFMT_LEFT,
				80, TEXT("User ID"), -1, 0},
				{LVCF_FMT | LVCF_SUBITEM | LVCF_TEXT | LVCF_WIDTH, LVCFMT_LEFT,
				60, TEXT("Opens count"), -1, 0},
				{LVCF_FMT | LVCF_SUBITEM | LVCF_TEXT | LVCF_WIDTH, LVCFMT_LEFT,
				25, TEXT("God"), -1, 0},
				{LVCF_FMT | LVCF_SUBITEM | LVCF_TEXT | LVCF_WIDTH, LVCFMT_LEFT,
				25, TEXT("Devil"), -1, 0},
				{LVCF_FMT | LVCF_SUBITEM | LVCF_TEXT | LVCF_WIDTH, LVCFMT_LEFT,
				80, TEXT("IP-address"), -1, 0},
				{LVCF_FMT | LVCF_SUBITEM | LVCF_TEXT | LVCF_WIDTH, LVCFMT_LEFT,
				120, TEXT("Connected"), -1, 0},
			};
			for (int i = 0; i < _countof(lvc); ++i)
				ListView_InsertColumn(m_hwndList, i, &lvc[i]);

			BuildView();

			retval = TRUE;
			break;
		}

	case WM_DESTROY:
		{
			m_hwndList = 0;
			break;
		}

	case WM_SIZE:
		{
			if (wParam == SIZE_MINIMIZED) break;
			SendMessage(hWnd, BEM_ADJUSTSIZE, wParam, lParam);
			break;
		}

	case BEM_ADJUSTSIZE:
		{
			RECT rc;
			int cx = LOWORD(lParam), cy = HIWORD(lParam);
			HDWP hdwp = BeginDeferWindowPos(1);
			SetRect(&rc, rcList.left, rcList.top,
				cx - rcPage.right + rcList.right,
				cy - rcPage.bottom + rcList.bottom);
			DeferWindowPos(hdwp, m_hwndList, 0, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, SWP_NOMOVE | SWP_NOREPOSITION | SWP_NOZORDER);
			EndDeferWindowPos(hdwp);
			break;
		}

	case WM_ACTIVATE:
		if (JServerApp::jpApp)
		{
			JServerApp::jpApp->hdlgCurrent = wParam ? hWnd : 0;
			JServerApp::jpApp->haccelCurrent = 0;
		}
		break;

	case WM_COMMAND:
		{
			switch (LOWORD(wParam))
			{
			case IDCANCEL:
				DestroyWindow(hWnd);
				break;

			case IDC_RENAME:
				{
					int index = ListView_GetNextItem(m_hwndList, -1, LVNI_SELECTED);
					if (index >= 0) {
						ListView_EditLabel(m_hwndList, index);
					}
					break;
				}

			case IDC_GODMODE:
				{
					int index = -1;
					for (MapUser::iterator iu = getSelUser(index); index >= 0; iu = getSelUser(index)) {
						if (iu == pNode->m_mUser.end()) continue;
						iu->second.cheat.isGod = !iu->second.cheat.isGod;
						SetId set = iu->second.opened;
						set.insert(iu->first);
						pNode->BroadcastTrn(set, true, pNode->Make_Notify_STATUS_God(iu->first, iu->second.cheat.isGod));
					}
					break;
				}

			case IDC_DEVILMODE:
				{
					int index = -1;
					for (MapUser::iterator iu = getSelUser(index); index >= 0; iu = getSelUser(index)) {
						if (iu == pNode->m_mUser.end()) continue;
						iu->second.cheat.isDevil = !iu->second.cheat.isDevil;
						SetId set = iu->second.opened;
						set.insert(iu->first);
						pNode->BroadcastTrn(set, true, pNode->Make_Notify_STATUS_Devil(iu->first, iu->second.cheat.isDevil));
					}
					break;
				}

			case IDC_CLOSECONNECTION:
				{
					for (int index = ListView_GetNextItem(m_hwndList, -1, LVNI_SELECTED); index >= 0; index = ListView_GetNextItem(m_hwndList, index, LVNI_SELECTED)) {
						LVITEM lvi;
						lvi.mask = LVIF_PARAM;
						lvi.iItem = index;
						lvi.iSubItem = 0;
						if (ListView_GetItem(m_hwndList, &lvi)) {
							pNode->EvLinkClose((SOCKET)lvi.lParam, 0);
						}
					}
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
			case LVN_GETDISPINFO:
				{
					static TCHAR buffer[64];
					LV_DISPINFO* pnmv = (LV_DISPINFO*)lParam;

					if (pnmh->idFrom == IDC_LIST)
					{
						JPtr<JLink> link = JLink::get((JID)pnmv->item.lParam);
						if (!link) break;
						MapSocketId::const_iterator iid = pNode->mSocketId.find(link->ID);
						if (iid == pNode->mSocketId.end()) break;
						MapUser::const_iterator iu = pNode->mUser.find(iid->second);
						ASSERT(iu != pNode->m_mUser.end());
						if (pnmv->item.mask & LVIF_TEXT)
						{
							switch (pnmv->item.iSubItem)
							{
							case 0:
								if (iu != pNode->mUser.end()) {
									_stprintf_s(buffer, _countof(buffer),
										iu->second.name.c_str());
								} else {
									_stprintf_s(buffer, _countof(buffer), TEXT("N/A"));
								}
								pnmv->item.pszText = buffer;
								break;

							case 1:
								if (iu != pNode->mUser.end()) {
									_stprintf_s(buffer, _countof(buffer),
										TEXT("0x%08X"), iid->second);
								} else {
									_stprintf_s(buffer, _countof(buffer), TEXT("N/A"));
								}
								pnmv->item.pszText = buffer;
								break;

							case 2:
								if (iu != pNode->mUser.end()) {
									_stprintf_s(buffer, _countof(buffer), TEXT("%u"),
										iu->second.opened.size());
								} else {
									_stprintf_s(buffer, _countof(buffer), TEXT("N/A"));
								}
								pnmv->item.pszText = buffer;
								break;

							case 3:
								pnmv->item.pszText = (iu != pNode->mUser.end() && iu->second.cheat.isGod) ? TEXT("+") : TEXT("-");
								break;

							case 4:
								pnmv->item.pszText = (iu != pNode->mUser.end() && iu->second.cheat.isDevil) ? TEXT("+") : TEXT("-");
								break;

							case 5:
								_stprintf_s(buffer, _countof(buffer),
									TEXT("%i.%i.%i.%i"),
									link->m_saAddr.sin_addr.S_un.S_un_b.s_b1,
									link->m_saAddr.sin_addr.S_un.S_un_b.s_b2,
									link->m_saAddr.sin_addr.S_un.S_un_b.s_b3,
									link->m_saAddr.sin_addr.S_un.S_un_b.s_b4);
								pnmv->item.pszText = buffer;
								break;

							case 6:
								{
									SYSTEMTIME st;
									FileTimeToLocalTime(link->ftTime, st);

									_stprintf_s(buffer, _countof(buffer),
										TEXT("%02i:%02i:%02i, %02i.%02i.%04i"),
										st.wHour, st.wMinute, st.wSecond,
										st.wDay, st.wMonth, st.wYear);
									pnmv->item.pszText = buffer;
									break;
								}
							}
							pnmv->item.cchTextMax = _countof(buffer);
						}
						if (pnmv->item.mask & LVIF_IMAGE)
						{
							switch (pnmv->item.iSubItem)
							{
							case 0:
								//pnmv->item.iImage = IML_PASS;
								break;

							case 1:
								break;
							}
						}
					}
					break;
				}

			case LVN_BEGINLABELEDIT:
				{
					NMLVDISPINFO* pdi = (NMLVDISPINFO*)lParam;
					if (pnmh->idFrom == IDC_LIST) {
						retval = FALSE;
					}
					break;
				}

			case LVN_ENDLABELEDIT:
				{
					NMLVDISPINFO* pdi = (NMLVDISPINFO*)lParam;
					if (pnmh->idFrom == IDC_LIST) {
						if (pdi->item.pszText) {
							std::tstring nick = pdi->item.pszText;
							const TCHAR* msg;
							if (JServer::CheckNick(nick, msg)) {
								DWORD idOld = pNode->m_mSocketId[(SOCKET)pdi->item.lParam];
								pNode->RenameContact(
									idOld == CRC_NONAME ? (DWORD)pdi->item.lParam : CRC_SERVER,
									idOld,
									nick);
								retval = TRUE;
								break;
							}
						}
					}
					retval = FALSE;
					break;
				}

			case NM_DBLCLK:
				if (pnmh->idFrom == IDC_LIST
					&& profile::getInt(RF_SERVER, RK_CANEDITNICK, FALSE)) // disabled for not installed
				{
					SendMessage(hWnd, WM_COMMAND, IDC_RENAME, 0);
				}
				break;

			default: retval = __super::DlgProc(hWnd, message, wParam, lParam);
			}
			break;
		}

	case WM_CONTEXTMENU:
		{
			if ((HWND)wParam == m_hwndList
				&& ListView_GetSelectedCount(m_hwndList)) {
				RECT r;
				GetWindowRect((HWND)wParam, &r);
				TrackPopupMenu(GetSubMenu(JServerApp::jpApp->hmenuConnections, 0), TPM_LEFTALIGN | TPM_RIGHTBUTTON,
					min(max(GET_X_LPARAM(lParam), r.left), r.right),
					min(max(GET_Y_LPARAM(lParam), r.top), r.bottom), 0, hWnd, 0);
			} else {
				retval = __super::DlgProc(hWnd, message, wParam, lParam);
			}
			break;
		}

	case WM_INITMENUPOPUP:
		{
			if ((HMENU)wParam == GetSubMenu(JServerApp::jpApp->hmenuConnections, 0)) {
				int index = -1;
				MapUser::const_iterator iu = getSelUser(index);
				bool valid = iu != pNode->m_mUser.end();

				VERIFY(SetMenuDefaultItem((HMENU)wParam, IDC_RENAME, FALSE));
				EnableMenuItem((HMENU)wParam, IDC_RENAME,
					MF_BYCOMMAND | (profile::getInt(RF_SERVER, RK_CANEDITNICK, FALSE) ? MF_ENABLED : MF_GRAYED)); // disabled for not installed
				EnableMenuItem((HMENU)wParam, IDC_GODMODE,
					MF_BYCOMMAND | (profile::getInt(RF_SERVER, RK_CANMAKEGOD, FALSE) ? MF_ENABLED : MF_GRAYED)); // disabled for not installed
				CheckMenuItem((HMENU)wParam, IDC_GODMODE,
					MF_BYCOMMAND | (valid && iu->second.cheat.isGod ? MF_CHECKED : MF_UNCHECKED));
				EnableMenuItem((HMENU)wParam, IDC_DEVILMODE,
					MF_BYCOMMAND | (profile::getInt(RF_SERVER, RK_CANMAKEDEVIL, FALSE) ? MF_ENABLED : MF_GRAYED)); // disabled for not installed
				CheckMenuItem((HMENU)wParam, IDC_DEVILMODE,
					MF_BYCOMMAND | (valid && iu->second.cheat.isDevil ? MF_CHECKED : MF_UNCHECKED));
			} else {
				__super::DlgProc(hWnd, message, wParam, lParam);
			}
			break;
		}

	case WM_HELP:
		{
			//WinHelp(hWnd, szHelpFile, HELP_CONTEXTPOPUP, ((LPHELPINFO)lParam)->dwContextId);
			break;
		}

	default: retval = FALSE;
	}
	return retval;
}

int JServer::JConnections::AddLine(SOCKET sock)
{
	LVITEM lvi;
	int index = INT_MAX;
	lvi.mask = LVIF_PARAM | LVIF_TEXT | LVIF_IMAGE;
	lvi.iItem = index;
	lvi.iSubItem = 0;
	lvi.pszText = LPSTR_TEXTCALLBACK;
	lvi.cchTextMax = 0;
	lvi.iImage = I_IMAGECALLBACK;
	lvi.lParam = (LPARAM)sock;

	index = ListView_InsertItem(m_hwndList, &lvi);
	ListView_SetItemText(m_hwndList, index, 1, LPSTR_TEXTCALLBACK);
	ListView_SetItemText(m_hwndList, index, 2, LPSTR_TEXTCALLBACK);
	ListView_SetItemText(m_hwndList, index, 3, LPSTR_TEXTCALLBACK);
	ListView_SetItemText(m_hwndList, index, 4, LPSTR_TEXTCALLBACK);
	return index;
}

void JServer::JConnections::DelLine(SOCKET sock)
{
	LVFINDINFO lvfi;
	lvfi.flags = LVFI_PARAM;
	lvfi.lParam = (LPARAM)sock;
	ListView_DeleteItem(m_hwndList, ListView_FindItem(m_hwndList, -1, &lvfi));
}

void JServer::JConnections::BuildView()
{
	for each (SetJID::value_type const& v in pNode->aLinksEvent) {
		if (JLink::get(v)->isEstablished()) AddLine(v);
	}
	for each (SetJID::value_type const& v in pNode->aLinksAsync) {
		if (JLink::get(v)->isEstablished()) AddLine(v);
	}
}

MapUser::iterator JServer::JConnections::getSelUser(int& index)
{
	index = ListView_GetNextItem(m_hwndList, index, LVNI_SELECTED);
	if (index >= 0) {
		LVITEM lvi;
		lvi.mask = LVIF_PARAM;
		lvi.iItem = index;
		lvi.iSubItem = 0;
		if (ListView_GetItem(m_hwndList, &lvi)) {
			MapSocketId::const_iterator iid = pNode->mSocketId.find((SOCKET)lvi.lParam);
			if (iid != pNode->mSocketId.end()) {
				return pNode->m_mUser.find(iid->second);
			}
		}
	}
	return pNode->m_mUser.end();
}

void JServer::JConnections::OnHook(JNode* src)
{
	using namespace fastdelegate;

	__super::OnHook(src);

	JNODE(JServer, node, src);
	if (node) {
		node->EvLinkEstablished += MakeDelegate(this, &JServer::JConnections::OnLinkEstablished);
		node->EvLinkClose += MakeDelegate(this, &JServer::JConnections::OnLinkClose);
	}
}

void JServer::JConnections::OnUnhook(JNode* src)
{
	using namespace fastdelegate;

	JNODE(JServer, node, src);
	if (node) {
		node->EvLinkEstablished -= MakeDelegate(this, &JServer::JConnections::OnLinkEstablished);
		node->EvLinkClose -= MakeDelegate(this, &JServer::JConnections::OnLinkClose);
	}

	__super::OnUnhook(src);
}

void JServer::JConnections::OnLinkEstablished(SOCKET sock)
{
	if (m_hwndPage) AddLine(sock);
}

void JServer::JConnections::OnLinkClose(SOCKET sock, UINT err)
{
	if (m_hwndPage) DelLine(sock);
}

//
// JPasswords
//

JServer::JPasswords::JPasswords()
: JDialog()
{
}

LRESULT WINAPI JServer::JPasswords::DlgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
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

			// Set main window icons
			SNDMSG(hWnd, WM_SETICON, ICON_SMALL, (LPARAM)JServerApp::jpApp->hiMain16);
			SNDMSG(hWnd, WM_SETICON, ICON_BIG, (LPARAM)JServerApp::jpApp->hiMain32);

			SetDlgItemText(m_hwndPage, IDC_PASSWORDNET1, pNode->m_passwordNet.c_str());
			SetDlgItemText(m_hwndPage, IDC_PASSWORDNET2, pNode->m_passwordNet.c_str());
			SetDlgItemText(m_hwndPage, IDC_PASSWORDGOD1, pNode->m_passwordGod.c_str());
			SetDlgItemText(m_hwndPage, IDC_PASSWORDGOD2, pNode->m_passwordGod.c_str());
			SetDlgItemText(m_hwndPage, IDC_PASSWORDDEVIL1, pNode->m_passwordDevil.c_str());
			SetDlgItemText(m_hwndPage, IDC_PASSWORDDEVIL2, pNode->m_passwordDevil.c_str());

			OnMetrics(pNode->m_metrics);

			retval = TRUE;
			break;
		}

	case WM_DESTROY:
		{
			break;
		}

	case WM_ACTIVATE:
		if (JServerApp::jpApp)
		{
			JServerApp::jpApp->hdlgCurrent = wParam ? hWnd : 0;
			JServerApp::jpApp->haccelCurrent = 0;
		}
		break;

	case WM_COMMAND:
		{
			switch (LOWORD(wParam))
			{
			case IDOK:
				{
					std::tstring buffer1(pNode->m_metrics.uPassMaxLength, 0);
					std::tstring buffer2(pNode->m_metrics.uPassMaxLength, 0);
					GetDlgItemText(m_hwndPage, IDC_PASSWORDNET1, (TCHAR*)buffer1.data(), (int)buffer1.size() + 1);
					GetDlgItemText(m_hwndPage, IDC_PASSWORDNET2, (TCHAR*)buffer2.data(), (int)buffer2.size() + 1);
					if (buffer1 == buffer2) {
						std::tstring buffer1(pNode->m_metrics.uPassMaxLength, 0);
						std::tstring buffer2(pNode->m_metrics.uPassMaxLength, 0);
						GetDlgItemText(m_hwndPage, IDC_PASSWORDGOD1, (TCHAR*)buffer1.data(), (int)buffer1.size() + 1);
						GetDlgItemText(m_hwndPage, IDC_PASSWORDGOD2, (TCHAR*)buffer2.data(), (int)buffer2.size() + 1);
						if (buffer1 == buffer2) {
							std::tstring buffer1(pNode->m_metrics.uPassMaxLength, 0);
							std::tstring buffer2(pNode->m_metrics.uPassMaxLength, 0);
							GetDlgItemText(m_hwndPage, IDC_PASSWORDDEVIL1, (TCHAR*)buffer1.data(), (int)buffer1.size() + 1);
							GetDlgItemText(m_hwndPage, IDC_PASSWORDDEVIL2, (TCHAR*)buffer2.data(), (int)buffer2.size() + 1);
							if (buffer1 == buffer2) {
							} else {
								MessageBox(m_hwndPage, JServerApp::jpApp->LoadStringW(IDS_PASS_DEVIL).c_str(), JServerApp::jpApp->sAppName.c_str(), MB_OK | MB_ICONEXCLAMATION);
								SetFocus(GetDlgItem(m_hwndPage, IDC_PASSWORDDEVIL1));
								break;
							}
							pNode->m_passwordDevil = buffer1.c_str();
						} else {
							MessageBox(m_hwndPage, JServerApp::jpApp->LoadStringW(IDS_PASS_GOD).c_str(), JServerApp::jpApp->sAppName.c_str(), MB_OK | MB_ICONEXCLAMATION);
							SetFocus(GetDlgItem(m_hwndPage, IDC_PASSWORDGOD1));
							break;
						}
						pNode->m_passwordGod = buffer1.c_str();
					} else {
						MessageBox(m_hwndPage, JServerApp::jpApp->LoadStringW(IDS_PASS_NET).c_str(), JServerApp::jpApp->sAppName.c_str(), MB_OK | MB_ICONEXCLAMATION);
						SetFocus(GetDlgItem(m_hwndPage, IDC_PASSWORDNET1));
						break;
					}
					pNode->m_passwordNet = buffer1.c_str();

					DestroyWindow(hWnd);
					break;
				}

			case IDCANCEL:
				DestroyWindow(hWnd);
				break;

			default: retval = FALSE;
			}
			break;
		}

	case WM_HELP:
		{
			//WinHelp(hWnd, szHelpFile, HELP_CONTEXTPOPUP, ((LPHELPINFO)lParam)->dwContextId);
			break;
		}

	default: retval = FALSE;
	}
	return retval;
}

void JServer::JPasswords::OnMetrics(const Metrics& metrics)
{
	if (!m_hwndPage) return; // ignore if window closed

	SendDlgItemMessage(m_hwndPage, IDC_PASSWORDNET1, EM_LIMITTEXT, metrics.uPassMaxLength, 0);
	SendDlgItemMessage(m_hwndPage, IDC_PASSWORDNET2, EM_LIMITTEXT, metrics.uPassMaxLength, 0);
	SendDlgItemMessage(m_hwndPage, IDC_PASSWORDGOD1, EM_LIMITTEXT, metrics.uPassMaxLength, 0);
	SendDlgItemMessage(m_hwndPage, IDC_PASSWORDGOD2, EM_LIMITTEXT, metrics.uPassMaxLength, 0);
	SendDlgItemMessage(m_hwndPage, IDC_PASSWORDDEVIL1, EM_LIMITTEXT, metrics.uPassMaxLength, 0);
	SendDlgItemMessage(m_hwndPage, IDC_PASSWORDDEVIL2, EM_LIMITTEXT, metrics.uPassMaxLength, 0);
}

//-----------------------------------------------------------------------------

// The End.