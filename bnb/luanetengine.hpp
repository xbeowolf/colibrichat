// luanetengine.hpp
//

//-----------------------------------------------------------------------------

//
// Includes
//

#pragma region Includes
#pragma once

// Project
#include "luanetengine.h"

#pragma endregion

//-----------------------------------------------------------------------------

template<class JT>
IMPLEMENT_LUAMETHOD(netengine::JLuaEngine<JT>, Disconnect)
{
	auto sock = (SOCKET)luaL_checkinteger(L, 1);
	auto err = (int)luaL_checkinteger(L, 2);

	EvLinkClose(sock, err);
	return 0;
}

//-----------------------------------------------------------------------------

//
// class JLuaEngine
//

template<class JT>
void netengine::JLuaEngine<JT>::Init()
{
	lua_openVM();

	__super::Init();

	// Call Lua event
	{
		DOLUACS;
		lua_getmethod(L, "onInit");
		if (lua_isfunction(L, -1)) {
			lua_insert(L, -2);
			lua_call(L, 1, 0);
		} else lua_pop(L, 2);
		LUACALLCHECK;
	}
}

template<class JT>
void netengine::JLuaEngine<JT>::Done()
{
	// Call Lua event
	{
		DOLUACS;
		lua_getmethod(L, "onDone");
		if (lua_isfunction(L, -1)) {
			lua_insert(L, -2);
			lua_call(L, 1, 0);
		} else lua_pop(L, 2);
		LUACALLCHECK;
	}

	__super::Done();

	lua_closeVM();
}

template<class JT>
int  netengine::JLuaEngine<JT>::Run()
{
	__super::Run();

	// Call Lua event
	{
		DOLUACS;
		lua_getmethod(L, "onRun");
		if (lua_isfunction(L, -1)) {
			lua_insert(L, -2);
			lua_call(L, 1, 0);
		} else lua_pop(L, 2);
		LUACALLCHECK;
	}

	return m_State;
}

template<class JT>
void netengine::JLuaEngine<JT>::Stop()
{
	// Call Lua event
	{
		DOLUACS;
		lua_getmethod(L, "onStop");
		if (lua_isfunction(L, -1)) {
			lua_insert(L, -2);
			lua_call(L, 1, 0);
		} else lua_pop(L, 2);
		LUACALLCHECK;
	}

	__super::Stop();
}

template<class JT>
void netengine::JLuaEngine<JT>::lua_pushobject(lua_State* L)
{
	typedef CLuaGluer<JLuaEngine<JT>>::userdataType TD;
	TD* userData = (TD*)lua_newuserdata(L, sizeof(TD));
	userData->object = this;
	userData->bAutodelete = true;
	userData->object->JAddRef();
	luaL_getmetatable(L, ClassName());
	lua_setmetatable(L, -2);
}

template<class JT>
void netengine::JLuaEngine<JT>::lua_getmethod(lua_State* L, const char* method)
{
	lua_pushobject(L);
	lua_getfield(L, -1, method);
}

template<class JT>
void netengine::JLuaEngine<JT>::lua_openVM()
{
	__super::lua_openVM();
}

template<class JT>
void netengine::JLuaEngine<JT>::lua_closeVM()
{
	__super::lua_closeVM();
}

template<class JT>
void netengine::JLuaEngine<JT>::beforeDestruct()
{
	lua_closeVM();
	__super::beforeDestruct();
}

template<class JT>
DWORD netengine::JLuaEngine<JT>::ValidateTimeout(SOCKET sock)
{
	DWORD result = __super::ValidateTimeout(sock);
	// Call Lua event
	DOLUACS;
	lua_getmethod(L, "onValidateTime");
	if (lua_isfunction(L, -1)) {
		lua_insert(L, -2);
		lua_pushinteger(L, (lua_Integer)sock);
		lua_call(L, 2, 1);
		result = (DWORD)luaL_checkinteger(L, -1);
		lua_pop(L, 1);
	} else lua_pop(L, 2);
	LUACALLCHECK;
	return result;
}

//-----------------------------------------------------------------------------

// --- Events ---

template<class JT>
void netengine::JLuaEngine<JT>::OnLinkAccept(SOCKET sock)
{
	__super::OnLinkAccept(sock);

	// Call Lua event
	DOLUACS;
	lua_getmethod(L, "onLinkAccept");
	if (lua_isfunction(L, -1)) {
		lua_insert(L, -2);
		lua_pushinteger(L, (lua_Integer)sock);
		lua_call(L, 2, 0);
	} else lua_pop(L, 2);
	LUACALLCHECK;
}

template<class JT>
void netengine::JLuaEngine<JT>::OnLinkConnect(SOCKET sock)
{
	__super::OnLinkConnect(sock);

	// Call Lua event
	DOLUACS;
	lua_getmethod(L, "onLinkConnect");
	if (lua_isfunction(L, -1)) {
		lua_insert(L, -2);
		lua_pushinteger(L, (lua_Integer)sock);
		lua_call(L, 2, 0);
	} else lua_pop(L, 2);
	LUACALLCHECK;
}

template<class JT>
void netengine::JLuaEngine<JT>::OnLinkEstablished(SOCKET sock)
{
	__super::OnLinkEstablished(sock);

	// Call Lua event
	DOLUACS;
	lua_getmethod(L, "onLinkEstablished");
	if (lua_isfunction(L, -1)) {
		lua_insert(L, -2);
		lua_pushinteger(L, (lua_Integer)sock);
		lua_call(L, 2, 0);
	} else lua_pop(L, 2);
	LUACALLCHECK;
}

template<class JT>
void netengine::JLuaEngine<JT>::OnLinkStart(SOCKET sock)
{
	__super::OnLinkStart(sock);

	// Call Lua event
	DOLUACS;
	lua_getmethod(L, "onLinkStart");
	if (lua_isfunction(L, -1)) {
		lua_insert(L, -2);
		lua_pushinteger(L, (lua_Integer)sock);
		lua_call(L, 2, 0);
	} else lua_pop(L, 2);
	LUACALLCHECK;
}

template<class JT>
void netengine::JLuaEngine<JT>::OnLinkClose(SOCKET sock, UINT err)
{
	__super::OnLinkClose(sock, err);

	// Call Lua event
	DOLUACS;
	lua_getmethod(L, "onLinkClose");
	if (lua_isfunction(L, -1)) {
		lua_insert(L, -2);
		lua_pushinteger(L, (lua_Integer)sock);
		lua_pushinteger(L, err);
		lua_call(L, 3, 0);
	} else lua_pop(L, 2);
	LUACALLCHECK;
}

template<class JT>
void netengine::JLuaEngine<JT>::OnLinkFail(SOCKET sock, UINT err)
{
	__super::OnLinkFail(sock, err);

	// Call Lua event
	DOLUACS;
	lua_getmethod(L, "onLinkFail");
	if (lua_isfunction(L, -1)) {
		lua_insert(L, -2);
		lua_pushinteger(L, (lua_Integer)sock);
		lua_pushinteger(L, err);
		lua_call(L, 3, 0);
	} else lua_pop(L, 2);
	LUACALLCHECK;
}

template<class JT>
void netengine::JLuaEngine<JT>::OnTrnBadDOM(SOCKET sock, const std::string& data)
{
	// Call Lua event
	DOLUACS;
	lua_getmethod(L, "onTrnBadDOM");
	if (lua_isfunction(L, -1)) {
		lua_insert(L, -2);
		lua_pushinteger(L, (lua_Integer)sock);
		lua_pushlstring(L, data.c_str(), data.size());
		lua_call(L, 3, 0);
	} else lua_pop(L, 2);
	LUACALLCHECK;
}

template<class JT>
void netengine::JLuaEngine<JT>::OnTrnBadCRC(SOCKET sock)
{
	// Call Lua event
	DOLUACS;
	lua_getmethod(L, "onTrnBadCRC");
	if (lua_isfunction(L, -1)) {
		lua_insert(L, -2);
		lua_pushinteger(L, (lua_Integer)sock);
		lua_call(L, 2, 0);
	} else lua_pop(L, 2);
	LUACALLCHECK;
}

//-----------------------------------------------------------------------------

// The End.