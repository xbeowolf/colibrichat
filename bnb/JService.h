/******************************************************************************
*                                                                             *
* JService.h -- JService & JThread API class definition                       *
*                                                                             *
* Copyright (c) Podobashev Dmitry / BEOWOLF, 2009. All rights reserved.       *
*                                                                             *
******************************************************************************/

#ifndef _JSERVICE_
#define _JSERVICE_

//-----------------------------------------------------------------------------

//
// Includes
//

#pragma region Includes
#pragma once

// Common
#include "attachment.h"

#pragma endregion

//-----------------------------------------------------------------------------

//
// Interfaces
//

namespace attachment {
	class JService;
	typedef std::set<JPtr<JService>> SetService;

	class JService : virtual public JNode, public IService
	{
	public:

		JService(JNode* p = 0, bool lock = false);
#ifdef JPTR_ALLOW_PTR_CAST
	public:
#else
	protected:
#endif
		void beforeDestruct();

	public:

		void Init();
		void Done();
		int  Run();
		void Stop();
		void Suspend();
		void Resume();

		virtual void JobQuantum(); // for discrete services

		// Checkup whole tree on object conaining
		bool IsDescendant(const JService* obj) const;

		bool InsertChild(JService* obj);
		bool DeleteChild(JService* obj);

	public:

		// Child services
		JPROPERTY_RREF_CONST(SetService, aChild);

		// Current state
		JPROPERTY_R(EState, State);
	};

	class JThread : public JService // Safe thread class
	{
	public:

		JThread(JNode* p = 0, bool lock = false);

		int  Run();
		void Stop();
		void Suspend();
		void Resume();

		virtual void Wakeup(); // calls outside of thread

	protected:

		void Wait(DWORD dwMilliseconds); // calls inside of thread

		static DWORD WINAPI ThreadStub(LPVOID lpObject);

		virtual DWORD ThreadProc() = 0;

	public:

		JPROPERTY_R(HANDLE, hThread);
		JPROPERTY_R(DWORD, dwThreadId);
		JPROPERTY_R(HANDLE, hSleep);
		JPROPERTY_RW(bool, bSafeStop);
		JPROPERTY_RW(DWORD, dwSleepStep);
	};

}; // attachment

//-----------------------------------------------------------------------------

#endif // _JSERVICE_