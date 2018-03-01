
//-----------------------------------------------------------------------------

//
// Includes
//

#pragma region Includes

#include "stdafx.h"

// Common
#include "protocol.h"
#include "CRC.h"
#include "Profile.h"

// Project
#include "resource.h"
#include "client.h"

#pragma endregion

//-----------------------------------------------------------------------------

using namespace colibrichat;

//-----------------------------------------------------------------------------

#define REGISTER_STRING(val, name) lua_pushstring(L, TstrToANSI(val).c_str()), lua_setglobal(L, name)

//-----------------------------------------------------------------------------

static int getInt(lua_State *L)
{
	if (lua_isstring(L, -3) && lua_isstring(L, -2) && lua_isnumber(L, -1)) {
		std::tstring section = ANSIToTstr(lua_tostring(L, -3));
		std::tstring entry = ANSIToTstr(lua_tostring(L, -2));
		UINT nDefault = (UINT)lua_tointeger(L, -1);
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
	if (lua_isstring(L, -3) && lua_isstring(L, -2) && lua_isnumber(L, -1)) {
		std::tstring section = ANSIToTstr(lua_tostring(L, -3));
		std::tstring entry = ANSIToTstr(lua_tostring(L, -2));
		UINT nValue = (UINT)lua_tointeger(L, -1);
		profile::setInt(section, entry, nValue);
	} else {
		lua_pushstring(L, "incorrect argument in function \"setInt\"");
		lua_error(L);
	}
	return 0;
}

static int getStr(lua_State *L)
{
	if (lua_isstring(L, -3) && lua_isstring(L, -2) && lua_isstring(L, -1)) {
		std::tstring section = ANSIToTstr(lua_tostring(L, -3));
		std::tstring entry = ANSIToTstr(lua_tostring(L, -2));
		std::tstring szDefault = ANSIToTstr(lua_tostring(L, -1));
		std::tstring result = profile::getString(section, entry, szDefault);
		lua_pushstring(L, TstrToANSI(result).c_str());
	} else {
		lua_pushstring(L, "incorrect argument in function \"getStr\"");
		lua_error(L);
	}
	return 1;
}

static int setStr(lua_State *L)
{
	if (lua_isstring(L, -3) && lua_isstring(L, -2) && lua_isstring(L, -1)) {
		std::tstring section = ANSIToTstr(lua_tostring(L, -3));
		std::tstring entry = ANSIToTstr(lua_tostring(L, -2));
		std::tstring szValue = ANSIToTstr(lua_tostring(L, -1));
		profile::setString(section, entry, szValue);
	} else {
		lua_pushstring(L, "incorrect argument in function \"setStr\"");
		lua_error(L);
	}
	return 0;
}

static int hasstr(lua_State *L)
{
	if (lua_isstring(L, -2) && lua_isstring(L, -1)) {
		const char* host = lua_tostring(L, -2);
		const char* entry = lua_tostring(L, -1);
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

// --- Lua helper ---

void JClient::pushAlert(lua_State* L, const Alert& a)
{
	lua_newtable(L);
	lua_pushboolean(L, a.fFlashPageNew);
	lua_setfield(L, -2, "fFlashPageNew");
	lua_pushboolean(L, a.fFlashPageSayPrivate);
	lua_setfield(L, -2, "fFlashPageSayPrivate");
	lua_pushboolean(L, a.fFlahPageSayChannel);
	lua_setfield(L, -2, "fFlahPageSayChannel");
	lua_pushboolean(L, a.fFlashPageChangeTopic);
	lua_setfield(L, -2, "fFlashPageChangeTopic");
	lua_pushboolean(L, a.fCanOpenPrivate);
	lua_setfield(L, -2, "fCanOpenPrivate");
	lua_pushboolean(L, a.fCanAlert);
	lua_setfield(L, -2, "fCanAlert");
	lua_pushboolean(L, a.fCanMessage);
	lua_setfield(L, -2, "fCanMessage");
	lua_pushboolean(L, a.fCanSplash);
	lua_setfield(L, -2, "fCanSplash");
	lua_pushboolean(L, a.fCanSignal);
	lua_setfield(L, -2, "fCanSignal");
	lua_pushboolean(L, a.fCanRecvClipboard);
	lua_setfield(L, -2, "fCanRecvClipboard");
	lua_pushboolean(L, a.fPlayChatSounds);
	lua_setfield(L, -2, "fPlayChatSounds");
	lua_pushboolean(L, a.fPlayPrivateSounds);
	lua_setfield(L, -2, "fPlayPrivateSounds");
	lua_pushboolean(L, a.fPlayAlert);
	lua_setfield(L, -2, "fPlayAlert");
	lua_pushboolean(L, a.fPlayMessage);
	lua_setfield(L, -2, "fPlayMessage");
	lua_pushboolean(L, a.fPlayBeep);
	lua_setfield(L, -2, "fPlayBeep");
	lua_pushboolean(L, a.fPlayClipboard);
	lua_setfield(L, -2, "fPlayClipboard");
}

void JClient::popAlert(lua_State* L, Alert& a)
{
}

// --- Lua gluer ---

int colibrichat::lunareg_colibri(lua_State *L)
{
	static luaL_Reg methods1[] = {
		{"getInt", &getInt},
		{"setInt", &setInt},
		{"getStr", &getStr},
		{"setStr", &setStr},
		{ nullptr, nullptr }
	};
	static luaL_Reg methods2[] = {
		{"hasstr", &hasstr},
		{ nullptr, nullptr }
	};
	luaL_newlib(L, methods1);
	lua_setglobal(L, "profile");
	luaL_newlib(L, methods2);
	lua_setglobal(L, "str");
	return 2;
}

//-----------------------------------------------------------------------------

const char JClient::className[] = "JClient";

CLuaGluer<JClient>::RegType JClient::methods[] =
{
	RESPONSE_LUAMETHOD(getVars),
	RESPONSE_LUAMETHOD(setVars),
	RESPONSE_LUAMETHOD(PlaySound),
	RESPONSE_LUAMETHOD(ShowTopic),
	RESPONSE_LUAMETHOD(saveAutoopen),
	RESPONSE_LUAMETHOD(openAutoopen),
	RESPONSE_LUAMETHOD(Log),
	RESPONSE_LUAMETHOD(BaloonHide),
	RESPONSE_LUAMETHOD(Connect),
	RESPONSE_LUAMETHOD(Disconnect),
	RESPONSE_LUAMETHOD(getConnectCount),
	RESPONSE_LUAMETHOD(setConnectCount),
	RESPONSE_LUAMETHOD(getSocket),
	RESPONSE_LUAMETHOD(checkConnectionButton),
	RESPONSE_LUAMETHOD(WaitConnectStart),
	RESPONSE_LUAMETHOD(WaitConnectStop),
	RESPONSE_LUAMETHOD(MinimizeWindow),
	RESPONSE_LUAMETHOD(MaximizeWindow),
	RESPONSE_LUAMETHOD(RestoreWindow),
	RESPONSE_LUAMETHOD(FlashWindow),
	RESPONSE_LUAMETHOD(DestroyWindow),
	RESPONSE_LUAMETHOD(PageEnable),
	RESPONSE_LUAMETHOD(PageDisable),
	RESPONSE_LUAMETHOD(PageAppendScript),
	RESPONSE_LUAMETHOD(PageAppendRtf),
	RESPONSE_LUAMETHOD(PageSetIcon),
	RESPONSE_LUAMETHOD(Say),
	RESPONSE_LUAMETHOD(Message),
	RESPONSE_LUAMETHOD(Alert),
	RESPONSE_LUAMETHOD(Beep),
	{NULL, NULL}
};

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
	ASSERT(lua_isstring(L, -1));
	PlaySound(ANSIToTstr(lua_tostring(L, -1)).c_str());
	return 0;
}

IMPLEMENT_LUAMETHOD(JClient, ShowTopic)
{
	ShowTopic(lua_isstring(L, -1) ? ANSIToTstr(lua_tostring(L, -1)).c_str() : TEXT(""));
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
	if (lua_isstring(L, -2) && lua_isnumber(L, -1)) {
		EvLog(lua_tostring(L, -2), (ELog)lua_tointeger(L, -1));
	} else {
		lua_pushstring(L, "incorrect arguments in function \"Log\", it must be string and integer");
		lua_error(L);
	}
	return 0;
}

IMPLEMENT_LUAMETHOD(JClient, BaloonHide)
{
	BaloonHide();
	return 0;
}

IMPLEMENT_LUAMETHOD(JClient, Connect)
{
	bool getsetting = lua_isboolean(L, -1) ? lua_toboolean(L, -1) != 0 : false;
	Connect(getsetting);
	return 0;
}

IMPLEMENT_LUAMETHOD(JClient, Disconnect)
{
	EvLinkClose(m_clientsock, 0);
	return 0;
}

IMPLEMENT_LUAMETHOD(JClient, getConnectCount)
{
	lua_pushinteger(L, m_nConnectCount);
	return 1;
}

IMPLEMENT_LUAMETHOD(JClient, setConnectCount)
{
	m_nConnectCount = lua_isnumber(L, -1) ? (int)lua_tointeger(L, -1) : 0;
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
		int check = lua_isboolean(L, -1) ? lua_toboolean(L, -1) : 0;
		CheckDlgButton(jpPageServer->hwndPage, IDC_CONNECT, check);
	}
	return 0;
}

IMPLEMENT_LUAMETHOD(JClient, WaitConnectStart)
{
	int wait = lua_isnumber(L, -1) ? (int)lua_tointeger(L, -1) : 30*1000;
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
	BOOL flash = lua_isboolean(L, -1) ? (BOOL)lua_toboolean(L, -1) : TRUE;
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
	if (lua_isstring(L, -1)) id = tCRCJJ(ANSIToTstr(lua_tostring(L, -1)).c_str());
	else if (lua_isnumber(L, -1)) id = (DWORD)lua_tointeger(L, -1);
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
	if (lua_isstring(L, -1)) id = tCRCJJ(ANSIToTstr(lua_tostring(L, -1)).c_str());
	else if (lua_isnumber(L, -1)) id = (DWORD)lua_tointeger(L, -1);
	else id = CRC_SERVER;
	JPtr<JPage> jp = getPage(id);
	if (jp) {
		jp->Disable();
	}
	return 0;
}

IMPLEMENT_LUAMETHOD(JClient, PageAppendScript)
{
	DWORD id;
	if (lua_isstring(L, -2)) id = tCRCJJ(ANSIToTstr(lua_tostring(L, -2)).c_str());
	else if (lua_isnumber(L, -2)) id = (DWORD)lua_tointeger(L, -2);
	else id = CRC_SERVER;
	JPtr<JPageLog> jp = getPageLog(id);
	if (jp && lua_isstring(L, -1)) {
		jp->AppendScript(ANSIToTstr(lua_tostring(L, -1)));
	}
	return 0;
}

IMPLEMENT_LUAMETHOD(JClient, PageAppendRtf)
{
	DWORD id;
	std::string plain;
	if (lua_isstring(L, -2)) id = tCRCJJ(ANSIToTstr(lua_tostring(L, -2)).c_str());
	else if (lua_isnumber(L, -2)) id = (DWORD)lua_tointeger(L, -2);
	else id = CRC_SERVER;
	JPtr<JPageLog> jp = getPageLog(id);
	if (jp && lua_isstring(L, -1)) {
		plain = jp->AppendRtf(lua_tostring(L, -1));
	}
	lua_pushstring(L, plain.c_str());
	return 1;
}

IMPLEMENT_LUAMETHOD(JClient, PageSetIcon)
{
	DWORD id;
	if (lua_isstring(L, -2)) id = tCRCJJ(ANSIToTstr(lua_tostring(L, -2)).c_str());
	else if (lua_isnumber(L, -2)) id = (DWORD)lua_tointeger(L, -2);
	else id = CRC_SERVER;
	JPtr<JPageLog> jp = getPageLog(id);
	if (jp && lua_isnumber(L, -1)) {
		jp->setAlert((EAlert)lua_tointeger(L, -1));
	}
	return 0;
}

IMPLEMENT_LUAMETHOD(JClient, Say)
{
	DWORD id;
	if (lua_isstring(L, -2)) id = tCRCJJ(ANSIToTstr(lua_tostring(L, -1)).c_str());
	else if (lua_isnumber(L, -2)) id = (DWORD)lua_tointeger(L, -2);
	else id = CRC_SERVER;
	if (m_clientsock) {
		PushTrn(m_clientsock, Make_Cmd_SAY(id, SF_TEXT, lua_tostring(L, -1)));
	}
	return 0;
}

IMPLEMENT_LUAMETHOD(JClient, Message)
{
	DWORD id;
	if (lua_isstring(L, -2)) id = tCRCJJ(ANSIToTstr(lua_tostring(L, -2)).c_str());
	else if (lua_isnumber(L, -2)) id = (DWORD)lua_tointeger(L, -2);
	else id = CRC_SERVER;
	if (m_clientsock) {
		PushTrn(m_clientsock, Make_Quest_MESSAGE(id, lua_tostring(L, -1), false, GetSysColor(COLOR_WINDOW)));
	}
	return 0;
}

IMPLEMENT_LUAMETHOD(JClient, Alert)
{
	DWORD id;
	if (lua_isstring(L, -2)) id = tCRCJJ(ANSIToTstr(lua_tostring(L, -2)).c_str());
	else if (lua_isnumber(L, -2)) id = (DWORD)lua_tointeger(L, -2);
	else id = CRC_SERVER;
	if (m_clientsock) {
		PushTrn(m_clientsock, Make_Quest_MESSAGE(id, lua_tostring(L, -1), true, GetSysColor(COLOR_WINDOW)));
	}
	return 0;
}

IMPLEMENT_LUAMETHOD(JClient, Beep)
{
	DWORD id;
	if (lua_isstring(L, -1)) id = tCRCJJ(ANSIToTstr(lua_tostring(L, -1)).c_str());
	else if (lua_isnumber(L, -1)) id = (DWORD)lua_tointeger(L, -1);
	else id = CRC_SERVER;
	if (m_clientsock) {
		PushTrn(m_clientsock, Make_Cmd_BEEP(id));
	}
	return 0;
}

//-----------------------------------------------------------------------------

// The End.