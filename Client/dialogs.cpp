
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

//-----------------------------------------------------------------------------

// The End.