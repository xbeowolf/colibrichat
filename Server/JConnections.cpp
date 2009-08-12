
//-----------------------------------------------------------------------------

//
// Includes
//

#pragma region Includes

#include "stdafx.h"

// Windows API
#include <strsafe.h>

// Common
#include "dCRC.h"
#include "Profile.h"

// Project
#include "..\ColibriProtocol.h"
#include "resource.h"
#include "server.h"

#pragma endregion

//-----------------------------------------------------------------------------

using namespace colibrichat;

//-----------------------------------------------------------------------------

// Time funstions
static void FileTimeToLocalTime(const FILETIME &ft, SYSTEMTIME &st)
{
	FILETIME temp;
	FileTimeToLocalFileTime(&ft, &temp);
	FileTimeToSystemTime(&temp, &st);
}

static void CALLBACK GetSystemFileTime(FILETIME& ft)
{
	SYSTEMTIME st;
	GetSystemTime(&st);
	SystemTimeToFileTime(&st, &ft);
}

//-----------------------------------------------------------------------------

//
// JConnections dialog
//

CALLBACK JServer::JConnections::JConnections()
: JAttachedDialog<JServer>()
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

			if (!pSource)
			{
				retval = FALSE;
				break;
			}

			ListView_SetExtendedListViewStyle(m_hwndList,
				LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_HEADERDRAGDROP | LVS_EX_ONECLICKACTIVATE | LVS_EX_SUBITEMIMAGES);
			//ListView_SetImageList(m_hwndList, JServerApp::jpApp->himlTree, LVSIL_SMALL);
			ListView_SetItemCount(m_hwndList, 16);
			static LV_COLUMN lvc[] = {
				{LVCF_FMT | LVCF_SUBITEM | LVCF_TEXT | LVCF_WIDTH, LVCFMT_LEFT,
				80, TEXT("Nickname"), -1, 0},
				{LVCF_FMT | LVCF_SUBITEM | LVCF_TEXT | LVCF_WIDTH, LVCFMT_LEFT,
				80, TEXT("User ID"), -1, 1},
				{LVCF_FMT | LVCF_SUBITEM | LVCF_TEXT | LVCF_WIDTH, LVCFMT_LEFT,
				60, TEXT("Opens count"), -1, 2},
				{LVCF_FMT | LVCF_SUBITEM | LVCF_TEXT | LVCF_WIDTH, LVCFMT_LEFT,
				80, TEXT("IP-address"), -1, 3},
				{LVCF_FMT | LVCF_SUBITEM | LVCF_TEXT | LVCF_WIDTH, LVCFMT_LEFT,
				120, TEXT("Time"), -1, 4},
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

					if (pnmv->hdr.idFrom == IDC_LIST)
					{
						netengine::MapLink::const_iterator iter = pSource->mLinks.find((SOCKET)pnmv->item.lParam);
						if (iter == pSource->mLinks.end()) break;
						MapSocketId::const_iterator iterId = pSource->mSocketId.find(iter->second.Sock);
						if (iterId == pSource->mSocketId.end()) break;
						MapUser::const_iterator iterU = pSource->mUser.find(iterId->second);
						if (pnmv->item.mask & LVIF_TEXT)
						{
							switch (pnmv->item.iSubItem)
							{
							case 0:
								if (iterU != pSource->mUser.end()) {
									StringCchPrintf(buffer, _countof(buffer),
										iterU->second.name.c_str());
								} else {
									StringCchPrintf(buffer, _countof(buffer), TEXT("N/A"));
								}
								pnmv->item.pszText = buffer;
								break;

							case 1:
								if (iterU != pSource->mUser.end()) {
									StringCchPrintf(buffer, _countof(buffer),
										TEXT("0x%08X"), iterId->second);
								} else {
									StringCchPrintf(buffer, _countof(buffer), TEXT("N/A"));
								}
								pnmv->item.pszText = buffer;
								break;

							case 2:
								if (iterU != pSource->mUser.end()) {
									StringCchPrintf(buffer, _countof(buffer), TEXT("%u"),
										iterU->second.opened.size());
								} else {
									StringCchPrintf(buffer, _countof(buffer), TEXT("N/A"));
								}
								pnmv->item.pszText = buffer;
								break;

							case 3:
								StringCchPrintf(buffer, _countof(buffer),
									TEXT("%i.%i.%i.%i"),
									iter->second.m_saAddr.sin_addr.S_un.S_un_b.s_b1,
									iter->second.m_saAddr.sin_addr.S_un.S_un_b.s_b2,
									iter->second.m_saAddr.sin_addr.S_un.S_un_b.s_b3,
									iter->second.m_saAddr.sin_addr.S_un.S_un_b.s_b4);
								pnmv->item.pszText = buffer;
								break;

							case 4:
								{
									SYSTEMTIME st;
									FileTimeToLocalTime(iter->second.ftTime, st);

									StringCchPrintf(buffer, _countof(buffer),
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

			default: retval = __super::DlgProc(hWnd, message, wParam, lParam);
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

int CALLBACK JServer::JConnections::AddLine(SOCKET sock)
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

void CALLBACK JServer::JConnections::DelLine(SOCKET sock)
{
	LVFINDINFO lvfi;
	lvfi.flags = LVFI_PARAM;
	lvfi.lParam = (LPARAM)sock;
	ListView_DeleteItem(m_hwndList, ListView_FindItem(m_hwndList, -1, &lvfi));
}

void CALLBACK JServer::JConnections::BuildView()
{
	for each (netengine::MapLink::value_type const& v in pSource->mLinks) {
		if (v.second.isEstablished()) AddLine(v.first);
	}
}

void JServer::JConnections::OnHook(JEventable* src)
{
	using namespace fastdelegate;

	__super::OnHook(src);

	pSource->EvLinkEstablished += MakeDelegate(this, &JServer::JConnections::OnLinkEstablished);
	pSource->EvLinkDestroy += MakeDelegate(this, &JServer::JConnections::OnLinkDestroy);
}

void JServer::JConnections::OnUnhook(JEventable* src)
{
	using namespace fastdelegate;

	pSource->EvLinkEstablished -= MakeDelegate(this, &JServer::JConnections::OnLinkEstablished);
	pSource->EvLinkDestroy -= MakeDelegate(this, &JServer::JConnections::OnLinkDestroy);

	__super::OnUnhook(src);
}

void JServer::JConnections::OnLinkEstablished(SOCKET sock)
{
	ASSERT(pSource);
	if (m_hwndPage) AddLine(sock);
}

void JServer::JConnections::OnLinkDestroy(SOCKET sock)
{
	ASSERT(pSource);
	if (m_hwndPage) DelLine(sock);
}

//-----------------------------------------------------------------------------

// The End.