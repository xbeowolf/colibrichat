
//-----------------------------------------------------------------------------

//
// Includes
//

#pragma region Includes

#include "stdafx.h"

// Common
#include "Profile.h"

// Project
#include "..\ColibriProtocol.h"
#include "resource.h"
#include "client.h"

#pragma endregion

using namespace colibrichat;

//-----------------------------------------------------------------------------

#define DECLARE_METHODLUA(typename, methodname) {#methodname, &typename::lua_##methodname}
#define REGISTER_STRING(val, name) lua_pushstring(luaVM, tstrToANSI(val).c_str()), lua_setglobal(luaVM, name)

//-----------------------------------------------------------------------------

static int getInt(lua_State *luaVM)
{
	ASSERT(lua_gettop(luaVM) >= 3);
	if (lua_isstring(luaVM, -3) && lua_isstring(luaVM, -2) && lua_isnumber(luaVM, -1)) {
		std::tstring szSection = ANSIToTstr(lua_tostring(luaVM, -3));
		std::tstring szEntry = ANSIToTstr(lua_tostring(luaVM, -2));
		UINT nDefault = (UINT)lua_tointeger(luaVM, -1);
		UINT result = Profile::GetInt(szSection.c_str(), szEntry.c_str(), nDefault);
		lua_pushinteger(luaVM, result);
	} else {
		lua_pushstring(luaVM, "incorrect argument in function \"getInt\"");
		lua_error(luaVM);
	}
	return 1;
}

static int setInt(lua_State *luaVM)
{
	ASSERT(lua_gettop(luaVM) >= 3);
	if (lua_isstring(luaVM, -3) && lua_isstring(luaVM, -2) && lua_isnumber(luaVM, -1)) {
		std::tstring szSection = ANSIToTstr(lua_tostring(luaVM, -3));
		std::tstring szEntry = ANSIToTstr(lua_tostring(luaVM, -2));
		UINT nValue = (UINT)lua_tointeger(luaVM, -1);
		Profile::WriteInt(szSection.c_str(), szEntry.c_str(), nValue);
	} else {
		lua_pushstring(luaVM, "incorrect argument in function \"setInt\"");
		lua_error(luaVM);
	}
	return 0;
}

int JClient::lua_regFuncs(lua_State *luaVM)
{
	static luaL_Reg methods[] = {
		{"getInt", &getInt},
		{"setInt", &setInt},
		{0, 0}
	};
	luaL_register(luaVM, "profile", methods);
	return 0;
}

//-----------------------------------------------------------------------------

const char JClient::className[] = "Client";

CLuaGluer<JClient>::_tRegType JClient::methods[] =
{
	DECLARE_METHODLUA(JClient, regFuncs),
	DECLARE_METHODLUA(JClient, getGlobal),
	DECLARE_METHODLUA(JClient, getVars),
	DECLARE_METHODLUA(JClient, setVars),
	DECLARE_METHODLUA(JClient, PlaySound),
	DECLARE_METHODLUA(JClient, ShowTopic),
	DECLARE_METHODLUA(JClient, saveAutoopen),
	DECLARE_METHODLUA(JClient, openAutoopen),
	DECLARE_METHODLUA(JClient, Log),
	DECLARE_METHODLUA(JClient, HideBaloon),
	DECLARE_METHODLUA(JClient, Connect),
	DECLARE_METHODLUA(JClient, Disconnect),
	DECLARE_METHODLUA(JClient, getSocket),
	//DECLARE_METHOD(JClient, dump),
	{0, 0}
};

int JClient::lua_getGlobal(lua_State *luaVM) {
	// Register application waves
	REGISTER_STRING(JClientApp::jpApp->strWavMeline, "wavMeline");
	REGISTER_STRING(JClientApp::jpApp->strWavChatline, "wavChatline");
	REGISTER_STRING(JClientApp::jpApp->strWavConfirm, "wavConfirm");
	REGISTER_STRING(JClientApp::jpApp->strWavPrivateline, "wavPrivateline");
	REGISTER_STRING(JClientApp::jpApp->strWavTopic, "wavTopic");
	REGISTER_STRING(JClientApp::jpApp->strWavJoin, "wavJoin");
	REGISTER_STRING(JClientApp::jpApp->strWavPart, "wavPart");
	REGISTER_STRING(JClientApp::jpApp->strWavPrivate, "wavPrivate");
	REGISTER_STRING(JClientApp::jpApp->strWavAlert, "wavAlert");
	REGISTER_STRING(JClientApp::jpApp->strWavMessage, "wavMessage");
	REGISTER_STRING(JClientApp::jpApp->strWavBeep, "wavBeep");
	REGISTER_STRING(JClientApp::jpApp->strWavClipboard, "wavClipboard");
	ASSERT(lua_gettop(luaVM) == 0);
	return 0;
}

int JClient::lua_getVars(lua_State *luaVM)
{
	lua_getglobal(luaVM, "bReconnect");
	lua_getglobal(luaVM, "nConnectCount");
	lua_getglobal(luaVM, "bSendByEnter");
	lua_getglobal(luaVM, "bCheatAnonymous");
	m_bReconnect = lua_toboolean(luaVM, -4) != 0;
	m_nConnectCount = lua_isnumber(luaVM, -3) ? (int)lua_tointeger(luaVM, -1) : 0;
	m_bSendByEnter = lua_toboolean(luaVM, -2) != 0;
	m_bCheatAnonymous = lua_toboolean(luaVM, -1) != 0;
	lua_pop(luaVM, 4);
	ASSERT(lua_gettop(luaVM) == 0);
	return 0;
}

int JClient::lua_setVars(lua_State *luaVM)
{
	lua_pushboolean(luaVM, m_bReconnect), lua_setglobal(luaVM, "bReconnect");
	lua_pushinteger(luaVM, m_nConnectCount), lua_setglobal(luaVM, "nConnectCount");
	lua_pushboolean(luaVM, m_bSendByEnter), lua_setglobal(luaVM, "bSendByEnter");
	lua_pushboolean(luaVM, m_bCheatAnonymous), lua_setglobal(luaVM, "bCheatAnonymous");
	ASSERT(lua_gettop(luaVM) == 0);
	return 0;
}

int JClient::lua_PlaySound(lua_State *luaVM)
{
	ASSERT(lua_gettop(luaVM) >= 1);
	PlaySound(ANSIToTstr(luaL_checkstring(luaVM, -1)).c_str());
	return 0;
}

int JClient::lua_ShowTopic(lua_State *luaVM)
{
	ASSERT(lua_gettop(luaVM) >= 1);
	ShowTopic(ANSIToTstr(luaL_checkstring(luaVM, -1)).c_str());
	return 0;
}

int JClient::lua_saveAutoopen(lua_State *luaVM)
{
	saveAutoopen();
	return 0;
}

int JClient::lua_openAutoopen(lua_State *luaVM)
{
	openAutoopen();
	return 0;
}

int JClient::lua_Log(lua_State *luaVM)
{
	ASSERT(lua_gettop(luaVM) >= 1);
	if (lua_isstring(luaVM, -1)) {
		EvLog(ANSIToTstr(lua_tostring(luaVM, -1)), true);
	} else {
		lua_pushstring(luaVM, "incorrect string argument in function \"Log\"");
		lua_error(luaVM);
	}
	return 0;
}

int JClient::lua_HideBaloon(lua_State *luaVM)
{
	HideBaloon();
	return 0;
}

int JClient::lua_Connect(lua_State *luaVM)
{
	bool getsetting = lua_isboolean(luaVM, -1) ? lua_toboolean(luaVM, -1) != 0 : false;
	Connect(getsetting);
	return 0;
}

int JClient::lua_Disconnect(lua_State *luaVM)
{
	DeleteLink(m_clientsock);
	return 0;
}

int JClient::lua_getSocket(lua_State *luaVM)
{
	lua_pushinteger(luaVM, m_clientsock);
	return 1;
}

//-----------------------------------------------------------------------------

// The End.