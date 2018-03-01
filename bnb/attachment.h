/******************************************************************************
*                                                                             *
* attachment.h -- JNode, JNodeDialog API class definition                     *
*                                                                             *
* Copyright (c) Podobashev Dmitry / BEOWOLF, 2008. All rights reserved.       *
*                                                                             *
******************************************************************************/

#ifndef _RHATTACHMENT_
#define _RHATTACHMENT_

//-----------------------------------------------------------------------------

//
// Includes
//

#pragma region Includes
#pragma once

// Common
#include "patterns.h"

#pragma endregion

//-----------------------------------------------------------------------------

#define HASMAP(map, val) (map.find(val) != map.end())

//-----------------------------------------------------------------------------

//
// Interfaces
//

namespace attachment {

	class DoCS
	{
	public:
		DoCS(CRITICAL_SECTION* pcs) {m_pCS = pcs; EnterCriticalSection(m_pCS);}
		~DoCS() {LeaveCriticalSection(m_pCS);}
	private:
		CRITICAL_SECTION* m_pCS;
	}; // class DoCS

#define GETJNODE(JT) \
	__declspec(property(get=getpNode)) JT* pNode; \
	JT* getpNode() const { ASSERT(m_mNode.size()); return dynamic_cast<JT*>(m_mNode.begin()->first); }
#define JNODE(JT, node, src) JT* node = dynamic_cast<JT*>(src)

	class __declspec(uuid("{EE1AE68C-8B75-4743-88BF-F76D2BCA3268}"))
	JNode : public JClass
	{
	public:
		fastdelegate::FastDelegateList1<JNode*> EvUnhook;
		fastdelegate::FastDelegateList1<JNode*> EvBeforeDestruct;
#ifdef JPTR_ALLOW_PTR_CAST
	public:
#else
	protected:
#endif
		void beforeDestruct();

	public:

		struct opt {
			opt() { lock = false, hook = false; }
			bool lock, hook;
		};

		typedef std::map<JNode*, opt> MapNode;

		JNode(JNode* p = 0, bool lock = false);

		void SetNode(JNode* p, bool lock, bool hook);
		void DelNode(JNode* p);
		void SetupHooks();
		void ResetHooks();

	public:

		virtual void OnHook(JNode* src);
		virtual void OnUnhook(JNode* src);
		virtual void OnBeforeDestruct(JNode* src);

	protected:

		JPROPERTY_RREF_CONST(MapNode, mNode);
	}; // class JNode

	class JWindow : virtual public JNode
	{
	public:

		static LRESULT WINAPI WndProcStub(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

		JWindow(JNode* p = 0, bool lock = false);

		void MapControl(HWND hwnd, RECT& rect) const;
		void MapControl(int id, RECT& rect) const;

	protected:

		virtual LRESULT WINAPI DlgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
			return DefWindowProc(hWnd, message, wParam, lParam);
		}

	protected:

		JPROPERTY_R(HWND, hwndPage);
		JPROPERTY_RREF_CONST(RECT, rcPage);
		JPROPERTY_RREF_CONST(RECT, rcPageNC);
	}; // class JWindow

	class JDialog : public JWindow
	{
	public:

		static INT_PTR WINAPI DlgProcStub(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

		JDialog(JNode* p = 0, bool lock = false);

	protected:

		virtual LRESULT WINAPI DlgProc(HWND, UINT, WPARAM, LPARAM) {return FALSE;}
	}; // class JDialog

	class JPropertyDialog : public JWindow
	{
	public:

		static INT_PTR WINAPI DlgProcStub(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

		JPropertyDialog(JNode* p = 0, bool lock = false);

	protected:

		virtual LRESULT WINAPI DlgProc(HWND, UINT, WPARAM, LPARAM) {return FALSE;}
	}; // class JPropertyDialog

}; // attachment

//-----------------------------------------------------------------------------

#endif // _RHATTACHMENT_