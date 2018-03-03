/******************************************************************************
*                                                                             *
* luanetengine.h -- Beowolf Network Engine with Lua virtual machine managment.*
*                                                                             *
* Copyright (c) Podobashev Dmitry / BEOWOLF, 2010. All rights reserved.       *
*                                                                             *
******************************************************************************/

#ifndef _LUANETENGINE_
#define _LUANETENGINE_

//-----------------------------------------------------------------------------

//
// Includes
//

#pragma region Includes
#pragma once

// Lua
//#include "LuaGluer.h"
//#include "Luna.h"

// Common
#include "netengine.h"

#pragma endregion

//-----------------------------------------------------------------------------

#ifdef _DEBUG
#define LUACALLMARK(lvm) lua_State* L = lvm; int _top = lua_gettop(L); _ASSERT(_top >= 0);
#define LUACALLCHECK _ASSERT(lua_gettop(L) == _top);
#else
#define LUACALLMARK(lvm) lua_State* L = lvm;
#define LUACALLCHECK
#endif
#define DOLUACS DoCS _cs(&m_luaCS); LUACALLMARK(m_luaVM);

//-----------------------------------------------------------------------------

namespace netengine
{
	class LuaWrapper;
	template<class JT>
	class JLuaEngine;

	class LuaWrapper
	{
	public:

		friend class DoCS;

		explicit LuaWrapper();

		virtual void lua_openVM();
		virtual void lua_closeVM();
		bool isopened() const { return m_luaVM != nullptr; }
		void lua_setup(const LuaWrapper& lw);
		void lua_reset();

	protected:

		lua_State* m_luaVM; // Lua virtual machine
		mutable CRITICAL_SECTION m_luaCS; // Lua virtual machine critical section object
	}; // class LuaWrapper

	template<class JT>
	class JLuaEngine : public JT, public LuaWrapper
	{
	public:

		// Constructor
		JLuaEngine() {}
		virtual const char* ClassName() const = 0;

		void Init();
		void Done();
		int  Run();
		void Stop();

		void JobQuantum() {}

		virtual void lua_openVM();
		virtual void lua_closeVM();
		virtual void beforeDestruct();

		// Timeout to validate the link, ms
		DWORD ValidateTimeout(SOCKET sock);

		void lua_pushobject(lua_State* L);
		void lua_getmethod(lua_State* L, const char* method);

	protected:

		void OnLinkAccept(SOCKET sock);
		void OnLinkConnect(SOCKET sock);
		void OnLinkEstablished(SOCKET sock);
		void OnLinkStart(SOCKET sock);
		void OnLinkClose(SOCKET sock, UINT err);
		void OnLinkFail(SOCKET sock, UINT err);

		void OnTrnBadDOM(SOCKET sock, const std::string& data);
		void OnTrnBadCRC(SOCKET sock);

	protected:

		// Lua gluer functions
		DECLARE_LUAMETHOD(Disconnect);
	}; // class JLuaEngine

}; // netengine

//-----------------------------------------------------------------------------

#endif // _LUANETENGINE_