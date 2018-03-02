// JService.cpp : Defines JService class implementation.
//

//------------------------------------------------------------------------------

//
// Includes
//

#pragma region Includes

#include "stdafx.h"

// STL
#include <algorithm>

// Common
#include "JService.h"

#pragma endregion

//-----------------------------------------------------------------------------

using namespace attachment;

//------------------------------------------------------------------------------

//
// JService
//

JService::JService(JNode* p, bool lock)
: JNode(p, lock)
{
	m_State = eNone;
}

void JService::beforeDestruct()
{
	if (m_State != eNone && m_State != eDone) {
		if (m_State != eInit && m_State != eStopped)
			Stop();
		Done();
	}
	__super::beforeDestruct();
}

void JService::Init()
{
	for each (SetService::value_type const& v in m_aChild)
		v->Init();

	m_State = eInit;

	SetupHooks();
}

void JService::Done()
{
	ResetHooks();

	for each (SetService::value_type const& v in m_aChild)
		v->Done();

	m_State = eDone;
}

int  JService::Run()
{
	for each (SetService::value_type const& v in m_aChild)
		v->Run();

	m_State = eRunning;
	return m_State;
}

void JService::Stop()
{
	for each (SetService::value_type const& v in m_aChild)
		v->Stop();

	m_State = eStopped;
}

void JService::Suspend()
{
	for each (SetService::value_type const& v in m_aChild)
		v->Suspend();

	m_State = eSuspended;
}

void JService::Resume()
{
	for each (SetService::value_type const& v in m_aChild)
		v->Resume();

	m_State = eRunning;
}

void JService::JobQuantum()
{
	for each (SetService::value_type const& v in m_aChild)
		if (v->State == JService::eRunning)
			v->JobQuantum();
}

bool JService::IsDescendant(const JService* obj) const
{
	for each (SetService::value_type const& v in m_aChild)
		if (v == obj || v->IsDescendant(obj)) return true;
	return false;
}

bool JService::InsertChild(JService* obj)
{
	if (obj == this || obj->IsDescendant(this)) return false;
	m_aChild.insert(obj);
	if (m_State != eNone && m_State != eDone) {
		obj->Init();
		if (m_State != eInit && m_State != eStopped)
			obj->Run();
	}
	return true;
}

bool JService::DeleteChild(JService* obj)
{
	SetService::iterator iter = m_aChild.find(obj);
	if (iter == m_aChild.end()) return false;
	if (obj->m_State != eNone && obj->m_State != eDone) {
		if (obj->m_State != eInit && obj->m_State != eStopped)
			obj->Stop();
		obj->Done();
	}
	m_aChild.erase(iter);
	return false;
}

//
// JThread
//

DWORD WINAPI JThread::ThreadStub(LPVOID lpObject)
{
	JThread* obj = (JThread*)lpObject;
	int refCount = obj->JGetRefCount() - 1; // dead check
	_ASSERT(refCount >= 0);
	obj->JRelease(); // unlock object
	if (!refCount) return 0; // no referances remains to object
	if (obj->m_State == eRunning) return 1; // thread already running

	obj->m_State = eRunning;
	DWORD retval = obj->ThreadProc();
	obj->m_State = eStopped;
	_VERIFY(CloseHandle(obj->m_hThread));
	obj->m_hThread = 0;
	obj->m_dwThreadId = 0;
	_ASSERT(!obj->m_hSleep);
	return retval;
}

JThread::JThread(JNode* p, bool lock)
: JNode(p, lock), JService()
{
	m_hThread = 0;
	m_dwThreadId = 0;
	m_hSleep = 0;
	m_bSafeStop = true;
	m_dwSleepStep = 0;
}

int  JThread::Run()
{
	if (m_State != eRunning) {
		__super::Run();

		m_State = eWantRun;
		_ASSERT(JGetRefCount() != 0); // object must not be local or static
		JAddRef(); // do not prevent to delete object until thread starting
		m_hThread = CreateThread(NULL, 0, ThreadStub, this, 0, &m_dwThreadId);
	}
	return m_State;
}

void JThread::Stop()
{
	__super::Stop();

#ifdef _DEBUG
	DWORD t0 = GetTickCount(), t1;
#endif
	if (m_bSafeStop) {
		m_State = eWantStop;
		Wakeup();
#ifdef _DEBUG
		t1 = GetTickCount();
#endif
		if (WaitForSingleObject(m_hThread, 2*m_dwSleepStep) == WAIT_TIMEOUT)
			TerminateThread(m_hThread, 0);
		m_State = eStopped;
	} else TerminateThread(m_hThread, 0);
#ifdef _DEBUG
	DWORD t2 = GetTickCount();
#endif
	if (m_hThread) {
		_VERIFY(CloseHandle(m_hThread));
		m_hThread = 0;
		m_dwThreadId = 0;
	}
	if (m_hSleep) {
		_VERIFY(CloseHandle(m_hSleep));
		m_hSleep = 0;
	}
}

void JThread::Suspend()
{
	__super::Suspend();

	SuspendThread(m_hThread);
}

void JThread::Resume()
{
	__super::Resume();

	ResumeThread(m_hThread);
}

void JThread::Wakeup()
{
	if (m_hSleep) {
		_VERIFY(SetEvent(m_hSleep));
		Sleep(0);
	}
}

void JThread::Wait(DWORD dwMilliseconds)
{
	_ASSERT(!m_hSleep);
	m_hSleep = CreateEvent(0, TRUE, FALSE, 0);
	WaitForSingleObject(m_hSleep, dwMilliseconds);
	_VERIFY(CloseHandle(m_hSleep));
	m_hSleep = 0;
}

//-----------------------------------------------------------------------------

// The End.