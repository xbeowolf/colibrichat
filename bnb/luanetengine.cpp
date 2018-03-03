// luanetengine.cpp
//

//-----------------------------------------------------------------------------

//
// Includes
//

#pragma region Includes

#include "stdafx.h"

// Lua
#include "LuaGluer.h"
//#include "Luna.h"

// Project
#include "luanetengine.h"

#pragma endregion

//-----------------------------------------------------------------------------

using namespace netengine;

//-----------------------------------------------------------------------------

//
// class LuaWrapper
//

LuaWrapper::LuaWrapper()
{
	m_luaVM = nullptr; // no other Lua registration here!
	ZeroMemory(&m_luaCS, sizeof(m_luaCS));
}

void LuaWrapper::lua_openVM()
{
	auto L = luaL_newstate();
	_ASSERT(L);
	luaL_openlibs(L);

	// insert "DevMode" indicator
	lua_pushboolean(L,
#ifdef _DEBUG
		true);
#else
		false);
#endif
	lua_setglobal(L, "devmode");
	// insert "Platform" string
	lua_pushstring(L,
#if defined(_M_IX86)
		"x86");
#elif defined(_M_AMD64)
		"amd64");
#elif defined(_M_IA64)
		"ia64");
#elif defined(_M_ARM)
		"arm");
#else
		"*");
#endif
	lua_setglobal(L, "platform");
	// insert "PtrSize" value
	lua_pushinteger(L, sizeof(size_t));
	lua_setglobal(L, "ptrsize");

	// register Lua data
	m_luaVM = L;
	InitializeCriticalSection(&m_luaCS);
}

void LuaWrapper::lua_closeVM()
{
	if (m_luaVM) {
		// Close Lua virtual machine
		lua_close(m_luaVM);
		m_luaVM = nullptr;
		DeleteCriticalSection(&m_luaCS);
	}
}

void LuaWrapper::lua_setup(const LuaWrapper& lw) {
	m_luaVM = lw.m_luaVM;
	m_luaCS = lw.m_luaCS;
}

void LuaWrapper::lua_reset() {
	m_luaVM = nullptr;
}

//-----------------------------------------------------------------------------

// The End.