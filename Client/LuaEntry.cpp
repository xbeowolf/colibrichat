
//-----------------------------------------------------------------------------

//
// Includes
//

#pragma region Includes

#include "stdafx.h"

// Common
#include "CRC.h"
#include "Profile.h"

// Project
#include "..\ColibriProtocol.h"
#include "resource.h"
#include "client.h"

#pragma endregion

using namespace colibrichat;

//-----------------------------------------------------------------------------

#define REGISTER_STRING(val, name) lua_pushstring(L, tstrToANSI(val).c_str()), lua_setglobal(L, name)

//-----------------------------------------------------------------------------

static int getInt(lua_State *L)
{
	ASSERT(lua_gettop(L) >= 3);
	if (lua_isstring(L, 1) && lua_isstring(L, 2) && lua_isnumber(L, 3)) {
		std::tstring section = ANSIToTstr(lua_tostring(L, 1));
		std::tstring entry = ANSIToTstr(lua_tostring(L, 2));
		UINT nDefault = (UINT)lua_tointeger(L, 3);
		UINT result = profile::getInt(section, entry, nDefault);
		lua_pushinteger(L, result);
	} else {
		lua_pushstring(L, "incorrect argument in function \"getInt\"");
		lua_error(L);
	}
	return 1;
}

static int setInt(lua_State *L)
{
	ASSERT(lua_gettop(L) >= 3);
	if (lua_isstring(L, 1) && lua_isstring(L, 2) && lua_isnumber(L, 3)) {
		std::tstring section = ANSIToTstr(lua_tostring(L, 1));
		std::tstring entry = ANSIToTstr(lua_tostring(L, 2));
		UINT nValue = (UINT)lua_tointeger(L, 3);
		profile::setInt(section, entry, nValue);
	} else {
		lua_pushstring(L, "incorrect argument in function \"setInt\"");
		lua_error(L);
	}
	return 0;
}

static int hasstr(lua_State *L)
{
	ASSERT(lua_gettop(L) >= 2);
	if (lua_isstring(L, 1) && lua_isstring(L, 2)) {
		const char* host = lua_tostring(L, 1);
		const char* entry = lua_tostring(L, 2);
		int i = 0;
		while ((host = strstr(host, entry)) != 0) {
			host += strlen(entry);
			i++;
		}
		lua_pushinteger(L, i);
	} else {
		lua_pushstring(L, "incorrect argument in function \"hasstr\"");
		lua_error(L);
	}
	return 1;
}

int JClient::lua_regFuncs(lua_State *L)
{
	static luaL_Reg methods1[] = {
		{"getInt", &getInt},
		{"setInt", &setInt},
		{0, 0}
	};
	static luaL_Reg methods2[] = {
		{"hasstr", &hasstr},
		{0, 0}
	};
	luaL_register(L, "profile", methods1);
	luaL_register(L, "str", methods2);
	return 2;
}

//-----------------------------------------------------------------------------

const char JClient::className[] = "JClient";

CLuaGluer<JClient>::_tRegType JClient::methods[] =
{
	RESPONSE_LUAMETHOD(JClient, regFuncs),
	RESPONSE_LUAMETHOD(JClient, getGlobal),
	RESPONSE_LUAMETHOD(JClient, getVars),
	RESPONSE_LUAMETHOD(JClient, setVars),
	RESPONSE_LUAMETHOD(JClient, PlaySound),
	RESPONSE_LUAMETHOD(JClient, ShowTopic),
	RESPONSE_LUAMETHOD(JClient, saveAutoopen),
	RESPONSE_LUAMETHOD(JClient, openAutoopen),
	RESPONSE_LUAMETHOD(JClient, Log),
	RESPONSE_LUAMETHOD(JClient, HideBaloon),
	RESPONSE_LUAMETHOD(JClient, Connect),
	RESPONSE_LUAMETHOD(JClient, Disconnect),
	RESPONSE_LUAMETHOD(JClient, getConnectCount),
	RESPONSE_LUAMETHOD(JClient, setConnectCount),
	RESPONSE_LUAMETHOD(JClient, getSocket),
	RESPONSE_LUAMETHOD(JClient, checkConnectionButton),
	RESPONSE_LUAMETHOD(JClient, WaitConnectStart),
	RESPONSE_LUAMETHOD(JClient, WaitConnectStop),
	RESPONSE_LUAMETHOD(JClient, MinimizeWindow),
	RESPONSE_LUAMETHOD(JClient, MaximizeWindow),
	RESPONSE_LUAMETHOD(JClient, RestoreWindow),
	RESPONSE_LUAMETHOD(JClient, FlashWindow),
	RESPONSE_LUAMETHOD(JClient, DestroyWindow),
	RESPONSE_LUAMETHOD(JClient, PageEnable),
	RESPONSE_LUAMETHOD(JClient, PageDisable),
	RESPONSE_LUAMETHOD(JClient, PageAppendScript),
	RESPONSE_LUAMETHOD(JClient, Say),
	RESPONSE_LUAMETHOD(JClient, Message),
	RESPONSE_LUAMETHOD(JClient, Alert),
	RESPONSE_LUAMETHOD(JClient, Beep),
	{NULL, NULL}
};

IMPLEMENT_LUAMETHOD(JClient, getGlobal) {
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
	ASSERT(lua_gettop(L) == 0);
	return 0;
}

IMPLEMENT_LUAMETHOD(JClient, getVars)
{
	lua_getglobal(L, "bReconnect");
	lua_getglobal(L, "bSendByEnter");
	lua_getglobal(L, "bCheatAnonymous");
	int i = 0;
	m_bReconnect = lua_toboolean(L, i++) != 0;
	m_bSendByEnter = lua_toboolean(L, i++) != 0;
	m_bCheatAnonymous = lua_toboolean(L, i++) != 0;
	lua_pop(L, i);
	ASSERT(lua_gettop(L) == 0);
	return 0;
}

IMPLEMENT_LUAMETHOD(JClient, setVars)
{
	lua_pushboolean(L, m_bReconnect), lua_setglobal(L, "bReconnect");
	lua_pushboolean(L, m_bSendByEnter), lua_setglobal(L, "bSendByEnter");
	lua_pushboolean(L, m_bCheatAnonymous), lua_setglobal(L, "bCheatAnonymous");
	ASSERT(lua_gettop(L) == 0);
	return 0;
}

IMPLEMENT_LUAMETHOD(JClient, PlaySound)
{
	ASSERT(lua_gettop(L) >= 1);
	PlaySound(ANSIToTstr(luaL_checkstring(L, 1)).c_str());
	return 0;
}

IMPLEMENT_LUAMETHOD(JClient, ShowTopic)
{
	ShowTopic(lua_isstring(L, 1) ? ANSIToTstr(lua_tostring(L, 1)).c_str() : TEXT(""));
	return 0;
}

IMPLEMENT_LUAMETHOD(JClient, saveAutoopen)
{
	saveAutoopen();
	return 0;
}

IMPLEMENT_LUAMETHOD(JClient, openAutoopen)
{
	openAutoopen();
	return 0;
}

IMPLEMENT_LUAMETHOD(JClient, Log)
{
	ASSERT(lua_gettop(L) >= 1);
	if (lua_isstring(L, 1)) {
		EvLog(ANSIToTstr(lua_tostring(L, 1)), true);
	} else {
		lua_pushstring(L, "incorrect string argument in function \"Log\"");
		lua_error(L);
	}
	return 0;
}

IMPLEMENT_LUAMETHOD(JClient, HideBaloon)
{
	HideBaloon();
	return 0;
}

IMPLEMENT_LUAMETHOD(JClient, Connect)
{
	bool getsetting = lua_isboolean(L, 1) ? lua_toboolean(L, 1) != 0 : false;
	Connect(getsetting);
	return 0;
}

IMPLEMENT_LUAMETHOD(JClient, Disconnect)
{
	DeleteLink(m_clientsock);
	return 0;
}

IMPLEMENT_LUAMETHOD(JClient, getConnectCount)
{
	lua_pushinteger(L, m_nConnectCount);
	return 1;
}

IMPLEMENT_LUAMETHOD(JClient, setConnectCount)
{
	m_nConnectCount = lua_isnumber(L, 1) ? (int)lua_tointeger(L, 1) : 0;
	return 0;
}

IMPLEMENT_LUAMETHOD(JClient, getSocket)
{
	lua_pushinteger(L, m_clientsock);
	return 1;
}

IMPLEMENT_LUAMETHOD(JClient, checkConnectionButton)
{
	if (jpPageServer->hwndPage) {
		int check = lua_isboolean(L, 1) ? lua_toboolean(L, 1) : 0;
		CheckDlgButton(jpPageServer->hwndPage, IDC_CONNECT, check);
	}
	return 0;
}

IMPLEMENT_LUAMETHOD(JClient, WaitConnectStart)
{
	ASSERT(lua_gettop(L) >= 1);
	int wait = lua_isnumber(L, 1) ? (int)lua_tointeger(L, 1) : 30*1000;
	SetTimer(m_hwndPage, IDT_CONNECT, wait, 0);
	return 0;
}

IMPLEMENT_LUAMETHOD(JClient, WaitConnectStop)
{
	KillTimer(hwndPage, IDT_CONNECT);
	return 0;
}

IMPLEMENT_LUAMETHOD(JClient, MinimizeWindow)
{
	ShowWindow(m_hwndPage, SW_MINIMIZE);
	return 0;
}

IMPLEMENT_LUAMETHOD(JClient, MaximizeWindow)
{
	ShowWindow(m_hwndPage, SW_MAXIMIZE);
	return 0;
}

IMPLEMENT_LUAMETHOD(JClient, RestoreWindow)
{
	ShowWindow(m_hwndPage, SW_RESTORE);
	return 0;
}

IMPLEMENT_LUAMETHOD(JClient, FlashWindow)
{
	BOOL flash = lua_isboolean(L, 1) ? (BOOL)lua_toboolean(L, 1) : TRUE;
	FlashWindow(m_hwndPage, flash);
	return 0;
}

IMPLEMENT_LUAMETHOD(JClient, DestroyWindow)
{
	DestroyWindow(m_hwndPage);
	return 0;
}

IMPLEMENT_LUAMETHOD(JClient, PageEnable)
{
	DWORD id;
	if (lua_isstring(L, 1)) id = tCRCJJ(ANSIToTstr(lua_tostring(L, 1)).c_str());
	else if (lua_isnumber(L, 1)) id = (DWORD)lua_tointeger(L, 1);
	else id = CRC_SERVER;
	JPtr<JPage> jp = getPage(id);
	if (jp) {
		jp->Enable();
	}
	return 0;
}

IMPLEMENT_LUAMETHOD(JClient, PageDisable)
{
	DWORD id;
	if (lua_isstring(L, 1)) id = tCRCJJ(ANSIToTstr(lua_tostring(L, 1)).c_str());
	else if (lua_isnumber(L, 1)) id = (DWORD)lua_tointeger(L, 1);
	else id = CRC_SERVER;
	JPtr<JPage> jp = getPage(id);
	if (jp) {
		jp->Disable();
	}
	return 0;
}

IMPLEMENT_LUAMETHOD(JClient, PageAppendScript)
{
	ASSERT(lua_gettop(L) >= 2);
	DWORD id;
	if (lua_isstring(L, 1)) id = tCRCJJ(ANSIToTstr(lua_tostring(L, 1)).c_str());
	else if (lua_isnumber(L, 1)) id = (DWORD)lua_tointeger(L, 1);
	else id = CRC_SERVER;
	JPtr<JPageLog> jp = getPageLog(id);
	if (jp && lua_isstring(L, 2)) {
		jp->AppendScript(ANSIToTstr(lua_tostring(L, 2)));
	}
	return 0;
}

IMPLEMENT_LUAMETHOD(JClient, Say)
{
	ASSERT(lua_gettop(L) >= 2);
	DWORD id;
	if (lua_isstring(L, 1)) id = tCRCJJ(ANSIToTstr(lua_tostring(L, 1)).c_str());
	else if (lua_isnumber(L, 1)) id = (DWORD)lua_tointeger(L, 1);
	else id = CRC_SERVER;
	if (m_clientsock) {
		PushTrn(m_clientsock, Make_Cmd_SAY(id, SF_TEXT, lua_tostring(L, 2)));
	}
	return 0;
}

IMPLEMENT_LUAMETHOD(JClient, Message)
{
	ASSERT(lua_gettop(L) >= 2);
	DWORD id;
	if (lua_isstring(L, 1)) id = tCRCJJ(ANSIToTstr(lua_tostring(L, 1)).c_str());
	else if (lua_isnumber(L, 1)) id = (DWORD)lua_tointeger(L, 1);
	else id = CRC_SERVER;
	if (m_clientsock) {
		PushTrn(m_clientsock, Make_Quest_MESSAGE(id, lua_tostring(L, 2), false, GetSysColor(COLOR_WINDOW)));
	}
	return 0;
}

IMPLEMENT_LUAMETHOD(JClient, Alert)
{
	ASSERT(lua_gettop(L) >= 2);
	DWORD id;
	if (lua_isstring(L, 1)) id = tCRCJJ(ANSIToTstr(lua_tostring(L, 1)).c_str());
	else if (lua_isnumber(L, 1)) id = (DWORD)lua_tointeger(L, 1);
	else id = CRC_SERVER;
	if (m_clientsock) {
		PushTrn(m_clientsock, Make_Quest_MESSAGE(id, lua_tostring(L, 2), true, GetSysColor(COLOR_WINDOW)));
	}
	return 0;
}

IMPLEMENT_LUAMETHOD(JClient, Beep)
{
	ASSERT(lua_gettop(L) >= 1);
	DWORD id;
	if (lua_isstring(L, 1)) id = tCRCJJ(ANSIToTstr(lua_tostring(L, 1)).c_str());
	else if (lua_isnumber(L, 1)) id = (DWORD)lua_tointeger(L, 1);
	else id = CRC_SERVER;
	if (m_clientsock) {
		PushTrn(m_clientsock, Make_Cmd_BEEP(id));
	}
	return 0;
}

//-----------------------------------------------------------------------------

// The End.