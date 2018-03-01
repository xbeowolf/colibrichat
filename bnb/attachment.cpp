// attachment.cpp : JNode, JNodeDialog API class implementation.
//

//------------------------------------------------------------------------------

//
// Includes
//

#pragma region Includes

#include "stdafx.h"

// Common
#include "attachment.h"

#pragma endregion

//------------------------------------------------------------------------------

using namespace attachment;

//------------------------------------------------------------------------------

//
// JNode
//

JNode::JNode(JNode* p, bool lock)
{
	if (p) SetNode(p, lock, false);
}

void JNode::beforeDestruct() {
	EvBeforeDestruct(this);
	for each (MapNode::value_type const& v in m_mNode) {
		if (v.second.hook) EvUnhook.Invoke(v.first);
		if (v.second.lock) v.first->JRelease();
	}
	__super::beforeDestruct();
};

void JNode::SetNode(JNode* p, bool lock, bool hook)
{
	ASSERT(p);
	opt& o = m_mNode[p];
	if (o.lock != lock) {
		if (lock) p->JAddRef();
		else p->JRelease();
		o.lock = lock;
	}
	if (o.hook != hook) {
		if (hook) OnHook(p);
		else EvUnhook.Invoke(p);
		o.hook = hook;
	}
}

void JNode::DelNode(JNode* p)
{
	ASSERT(p);
	MapNode::iterator iter = m_mNode.find(p);
	if (iter != m_mNode.end()) {
		if (iter->second.hook) EvUnhook.Invoke(p);
		if (iter->second.lock) p->JRelease();
		m_mNode.erase(iter);
	}
}

void JNode::SetupHooks()
{
	for (MapNode::iterator iter = m_mNode.begin(); iter != m_mNode.end(); iter++) {
		if (!iter->second.hook) {
			iter->second.hook = true;
			OnHook(iter->first);
		}
	}
}

void JNode::ResetHooks()
{
	for (MapNode::iterator iter = m_mNode.begin(); iter != m_mNode.end(); iter++) {
		if (iter->second.hook) {
			EvUnhook.Invoke(iter->first);
			iter->second.hook = false;
		}
	}
}

void JNode::OnHook(JNode* src)
{
	using namespace fastdelegate;

	src->EvUnhook += MakeDelegate(this, &JNode::OnUnhook);
	if (src != this)
		EvUnhook += MakeDelegate(this, &JNode::OnUnhook);
}

void JNode::OnUnhook(JNode* src)
{
	using namespace fastdelegate;

	src->EvUnhook -= MakeDelegate(this, &JNode::OnUnhook);
	if (src != this)
		EvUnhook -= MakeDelegate(this, &JNode::OnUnhook);
}

void JNode::OnBeforeDestruct(JNode* src)
{
}

//
// JWindow
//

JWindow::JWindow(JNode* p, bool lock)
: JNode(p, lock)
{
	m_hwndPage = 0;
	SetRectEmpty(&m_rcPage);
	SetRectEmpty(&m_rcPageNC);
}

LRESULT WINAPI JWindow::WndProcStub(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	LRESULT retval; // always processed
	switch (message)
	{
	case WM_CREATE:
		{
			JPtr<JWindow> jp = (JWindow*)((CREATESTRUCT*)lParam)->lpCreateParams;
			if (!jp) {
				retval = -1;
				break;
			}

			jp->JAddRef();
			SetWindowLongPtr(hWnd, GWLP_USERDATA,
				(LONG)(LONG_PTR)((CREATESTRUCT*)lParam)->lpCreateParams);

			jp->m_hwndPage = hWnd;
			GetClientRect(hWnd, &jp->m_rcPage);
			GetWindowRect(hWnd, &jp->m_rcPageNC);
			MapWindowPoints(0, hWnd, (POINT*)&jp->m_rcPageNC, 2);

			retval = jp->DlgProc(hWnd, message, wParam, lParam);

			if (retval == -1) { // Free object if creation failed in subclass
				SetWindowLongPtr(hWnd, GWLP_USERDATA, 0);
				jp->JRelease();
			}
			break;
		}

	case WM_DESTROY:
		{
			// Get pointer to data class
			JPtr<JWindow> jp = (JWindow*)(LONG_PTR)GetWindowLongPtr(hWnd, GWLP_USERDATA);
			if (!jp) break;

			retval = jp->DlgProc(hWnd, message, wParam, lParam);

			jp->m_hwndPage = 0;

			SetWindowLongPtr(hWnd, GWLP_USERDATA, 0);
			jp->JRelease();
			break;
		}

	default:
		{
			// Get pointer to data class
			JPtr<JWindow> jp = (JWindow*)(LONG_PTR)GetWindowLongPtr(hWnd, GWLP_USERDATA);
			if (jp) retval = jp->DlgProc(hWnd, message, wParam, lParam);
			else retval = DefWindowProc(hWnd, message, wParam, lParam);
			break;
		}
	}
	return retval;
}

void JWindow::MapControl(HWND hwnd, RECT& rect) const
{
	GetWindowRect(hwnd, &rect);
	MapWindowPoints(0, m_hwndPage, (POINT*)&rect, 2);
}

void JWindow::MapControl(int id, RECT& rect) const
{
	MapControl(GetDlgItem(m_hwndPage, id), rect);
}

//
// JDialog
//

JDialog::JDialog(JNode* p, bool lock)
: JNode(p, lock), JWindow()
{
}

INT_PTR WINAPI JDialog::DlgProcStub(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	LRESULT retval = FALSE; // not processed by default
	switch (message)
	{
	case WM_INITDIALOG:
		{
			JPtr<JDialog> jp = (JDialog*)lParam;
			if (!jp) break;

			jp->JAddRef();
			SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG)lParam);

			jp->m_hwndPage = hWnd;
			GetClientRect(hWnd, &jp->m_rcPage);
			GetWindowRect(hWnd, &jp->m_rcPageNC);
			MapWindowPoints(0, hWnd, (POINT*)&jp->m_rcPageNC, 2);

			retval = jp->DlgProc(hWnd, message, wParam, lParam);
			break;
		}

	case WM_DESTROY:
		{
			// Get pointer to data class
			JPtr<JDialog> jp = (JDialog*)(LONG_PTR)GetWindowLongPtr(hWnd, GWLP_USERDATA);
			if (!jp) break;

			retval = jp->DlgProc(hWnd, message, wParam, lParam);

			jp->m_hwndPage = 0;

			SetWindowLongPtr(hWnd, GWLP_USERDATA, 0);
			jp->JRelease();
			break;
		}

	default:
		{
			// Get pointer to data class
			JPtr<JDialog> jp = (JDialog*)(LONG_PTR)GetWindowLongPtr(hWnd, GWLP_USERDATA);
			if (jp) retval = jp->DlgProc(hWnd, message, wParam, lParam);
			break;
		}
	}
	return retval;
}

//
// JPropertyDialog
//

JPropertyDialog::JPropertyDialog(JNode* p, bool lock)
: JNode(p, lock), JWindow()
{
}

INT_PTR WINAPI JPropertyDialog::DlgProcStub(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	JPtr<JPropertyDialog> jp = (JPropertyDialog*)(LONG_PTR)(message != WM_INITDIALOG
		? GetWindowLongPtr(hWnd, GWLP_USERDATA)
		: ((PROPSHEETPAGE*)lParam)->lParam);
	LRESULT retval = TRUE;
	if (jp) {
		if (WM_INITDIALOG == message) {
			SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG)((PROPSHEETPAGE*)lParam)->lParam);
			jp->JAddRef();
		}
		jp->DlgProc(hWnd, message, wParam, lParam);
		if (WM_DESTROY == message) {
			jp->JRelease();
			SetWindowLongPtr(hWnd, GWLP_USERDATA, 0);
		}
	} else return FALSE;
	SetWindowLong(hWnd, DWL_MSGRESULT, (LONG)retval);
	return retval;
}

//-----------------------------------------------------------------------------

// The End.