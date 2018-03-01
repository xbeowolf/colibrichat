// luanetengine.cpp
//

//-----------------------------------------------------------------------------

//
// Includes
//

#pragma region Includes

#include "stdafx.h"

// Project
#include "luanetengine.h"

#pragma endregion

//-----------------------------------------------------------------------------

using namespace netengine;

//-----------------------------------------------------------------------------

//
// class JLuaWrapper
//

JLuaWrapper::JLuaWrapper()
{
	m_luaVM = 0; // no other Lua registration here!
	ZeroMemory(&m_luaCS, sizeof(m_luaCS));
}

void JLuaWrapper::beforeDestruct()
{
	lua_closeVM();
	__super::beforeDestruct();
}

void JLuaWrapper::lua_openVM()
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

void JLuaWrapper::lua_closeVM()
{
	if (m_luaVM) {
		// Close Lua virtual machine
		lua_close(m_luaVM);
		m_luaVM = nullptr;
		DeleteCriticalSection(&m_luaCS);
	}
}

// profile functions
bool JLuaWrapper::getBool(const std::string& name) const
{
	DoCS _cs(&m_luaCS);
	lua_getglobal(m_luaVM, name.c_str());
	ASSERT_MSG(lua_isboolean(m_luaVM, -1), name.c_str());
	bool retval = lua_toboolean(m_luaVM, -1) != 0;
	lua_pop(m_luaVM, 1);
	return retval;
}

bool JLuaWrapper::getBool(const std::string& table, const std::string& name) const
{
	DoCS _cs(&m_luaCS);
	lua_getglobal(m_luaVM, table.c_str());
	lua_getfield(m_luaVM, -1, name.c_str());
	ASSERT_MSG(lua_isboolean(m_luaVM, -1), name.c_str());
	bool retval = lua_toboolean(m_luaVM, -1) != 0;
	lua_pop(m_luaVM, 2);
	return retval;
}

int JLuaWrapper::getInt(const std::string& name) const
{
	DoCS _cs(&m_luaCS);
	lua_getglobal(m_luaVM, name.c_str());
	ASSERT_MSG(lua_isnumber(m_luaVM, -1), name.c_str());
	int retval = (int)lua_tointeger(m_luaVM, -1);
	lua_pop(m_luaVM, 1);
	return retval;
}

int JLuaWrapper::getInt(const std::string& table, const std::string& name) const
{
	DoCS _cs(&m_luaCS);
	lua_getglobal(m_luaVM, table.c_str());
	lua_getfield(m_luaVM, -1, name.c_str());
	ASSERT_MSG(lua_isnumber(m_luaVM, -1), name.c_str());
	int retval = (int)lua_tointeger(m_luaVM, -1);
	lua_pop(m_luaVM, 2);
	return retval;
}

lua_Number JLuaWrapper::getNum(const std::string& name) const
{
	DoCS _cs(&m_luaCS);
	lua_getglobal(m_luaVM, name.c_str());
	ASSERT_MSG(lua_isnumber(m_luaVM, -1), name.c_str());
	lua_Number retval = lua_tonumber(m_luaVM, -1);
	lua_pop(m_luaVM, 1);
	return retval;
}

lua_Number JLuaWrapper::getNum(const std::string& table, const std::string& name) const
{
	DoCS _cs(&m_luaCS);
	lua_getglobal(m_luaVM, table.c_str());
	lua_getfield(m_luaVM, -1, name.c_str());
	ASSERT_MSG(lua_isnumber(m_luaVM, -1), name.c_str());
	lua_Number retval = lua_tonumber(m_luaVM, -1);
	lua_pop(m_luaVM, 2);
	return retval;
}

const char* JLuaWrapper::getString(const std::string& name) const
{
	DoCS _cs(&m_luaCS);
	lua_getglobal(m_luaVM, name.c_str());
	ASSERT_MSG(lua_isstring(m_luaVM, -1), name.c_str());
	const char* retval = lua_tostring(m_luaVM, -1);
	lua_pop(m_luaVM, 1);
	return retval;
}

const char* JLuaWrapper::getString(const std::string& table, const std::string& name) const
{
	DoCS _cs(&m_luaCS);
	lua_getglobal(m_luaVM, table.c_str());
	lua_getfield(m_luaVM, -1, name.c_str());
	ASSERT_MSG(lua_isstring(m_luaVM, -1), name.c_str());
	const char* retval = lua_tostring(m_luaVM, -1);
	lua_pop(m_luaVM, 2);
	return retval;
}

//-----------------------------------------------------------------------------

// The End.