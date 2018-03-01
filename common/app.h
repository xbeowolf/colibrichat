/******************************************************************************
*                                                                             *
* app.h -- JApplication API frame class definition                            *
*                                                                             *
* Copyright (c) Podobashev Dmitry / BEOWOLF, 2007. All rights reserved.       *
*                                                                             *
******************************************************************************/

#ifndef _RHAPP_
#define _RHAPP_

//-----------------------------------------------------------------------------

//
// Includes
//

#pragma region Includes
#pragma once

#include "attachment.h"

#pragma endregion

//-----------------------------------------------------------------------------

namespace attachment {

	class JApplication : virtual public JNode, public IApplication
	{
	public:
		virtual DWORD IdleTimeout() const {return INFINITE;}
		virtual void getWaitObjects(std::vector<HANDLE>& handles) const {}
		virtual bool InitInstance() = 0;
		virtual void Clear();
		int Iteration();

		int Run();

		void AddWindow(HWND hwnd);
		void DelWindow(HWND hwnd);
		void AddPropertySheet(HWND hwnd);
		void DelPropertySheet(HWND hwnd);

		// Support functions
		std::tstring LoadString(UINT uID);

		virtual void onSignaled(HANDLE h) {}
		virtual void onInput() {}
		virtual void onIdleTimeout() {}

	protected:

		JApplication(HINSTANCE hInstance = 0, HINSTANCE hPrevInstance = 0, LPTSTR szcl = 0, int ncs = SW_SHOWDEFAULT);

	public: // data

		HINSTANCE hinstApp, hinstPrev; // given from WinMain() arguments
		LPTSTR lpszCmdLine; // given from WinMain() argument
		int nCmdShow; // given from WinMain() argument

		std::tstring sAppName; // name of application for misc. messages
		HICON hiMain16, hiMain32; // application icons for dialogs

		HWND hdlgCurrent; // managed by dialog functions
		HACCEL haccelCurrent; // managed by dialog functions

	protected:

		JPROPERTY_RREF_CONST(std::set<HWND>, setWindow); // top windows
		JPROPERTY_RREF_CONST(std::set<HWND>, setPropertySheet); // modeless property sheet dialogs
	}; // JApplication

}; // attachment

//-----------------------------------------------------------------------------

#endif // _RHAPP_